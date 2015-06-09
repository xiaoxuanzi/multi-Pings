#include "pings.h"

void *alloc_memory()
{
    int block_size;
    int ping_node_size;
    int info_size;
    void *mem_ptr = NULL;
    int index;

    ping_node_size = sizeof(struct ping_node);
    info_size = sizeof(struct info);
    block_size = ping_node_size + info_size;

    mem_ptr = (void *)malloc(block_size * PINGS_DEST_IP_NUMBERS);

    if(NULL == mem_ptr){
        return NULL;
    }

    memset(mem_ptr, 0, block_size * PINGS_DEST_IP_NUMBERS);

    for(index = 0 ; index < PINGS_DEST_IP_NUMBERS; index ++){
        g_record[index] = (struct ping_node *)((char *)mem_ptr + block_size * index);
        g_record[index]->ping_info = (struct info *)((char *)g_record[index]  + ping_node_size);
    }

    return mem_ptr;
}

void free_memory(void *mem_ptr)
{
    int index;

    for(index = 0 ; index < PINGS_DEST_IP_NUMBERS; index ++){
        g_record[index]->ping_info = NULL;
        g_record[index] = NULL;
    }

   if(mem_ptr != NULL){
        free(mem_ptr);
        mem_ptr = NULL;
    }
}

