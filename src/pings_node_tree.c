#include "pings.h"

int compare(unsigned long  a, unsigned long  b)
{
    if(a > b){
        return 1;
    }
    else if(a < b){
        return -1;
    }
    else{
        return 0;
    }
}

struct ping_node* pings_search(struct rb_root *root, unsigned long from_ip)
{
    struct rb_node *node = root->rb_node;
    struct ping_node *data = NULL;
    int result;

    while(node){
        data = container_of(node, struct ping_node, node);
        result = compare(data->ping_info->ip.s_addr, from_ip);

        if(result > 0){
            node = node->rb_left;
        }
        else if(result < 0){
            node = node->rb_right;
        }
        else{
            return data;
        }
    }

    return NULL;
}

int pings_insert(struct rb_root *root, struct ping_node *node)
{
    struct rb_node **new = &(root->rb_node);
    struct rb_node *parent = NULL;
    struct ping_node *this = NULL;
    int result;

    while(*new){
        this = container_of(*new, struct ping_node, node);
        result = compare(node->ping_info->ip.s_addr, this->ping_info->ip.s_addr);
        parent = *new;

        if (result < 0){
            new = &((*new)->rb_left);
        }
        else if(result > 0){
            new = &((*new)->rb_right);
        }
        else{
            return 0;
        }
    }

    rb_link_node(&node->node, parent, new);
    rb_insert_color(&node->node, root);

    return 1;
}

