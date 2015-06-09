#include "pings.h"

struct rb_root g_tree = RB_ROOT;
struct ping_node *g_record[PINGS_DEST_IP_NUMBERS];

int main(int argc,char *argv[])
{
    struct epoll_event events[PINGS_EPOLL_MAX_EVENTS];
    pings_params pings = {0};
    int ret;
    int dest_ip_numbers;
    void *mem_ptr = NULL;
    int nfds;

    ret = check_argv(argc, argv);

    if(PINGS_FAILED == ret){
        return PINGS_ERROR;
    }

    mem_ptr = alloc_memory();

    if(NULL == mem_ptr){
        pings_error("alloc_memory");
        return PINGS_ERROR;
    }

    dest_ip_numbers = init_dest_ip();

    if(PINGS_FAILED == dest_ip_numbers){
        pings_error("too many ip addrs");
        goto failed;
    }

    ret = init_pings(&pings, dest_ip_numbers, argv[argc - 1]);

    if(PINGS_FAILED == ret){
        pings_error("init_pings");
        goto failed;
    }

    while(1){
        nfds = epoll_wait(pings.epoll_fd, events, PINGS_EPOLL_MAX_EVENTS, PINGS_EPOLL_TIMEOUT);

        if(nfds == -1){
            if(EINTR == errno){
                continue;
            }

            break;
        }

        if(nfds == 0){
          goto timeout;
        }

        if(events[0].events & EPOLLOUT){
            handle_send_event(&pings);

            if(pings.send_error){
                break;
            }

            continue;
        }

        if(events[0].events & EPOLLIN){
            handle_recv_event(&pings);

            if(pings.recv_error){
                break;
            }

            if(pings.recv_interval < PINGS_RECV_INTERVAL_THRESHOLD){
                continue;
            }
        }

timeout:
        ret = handle_timeout_event(&pings);

        if(PINGS_FAILED == ret){
            break;
        }
    }

    close(pings.sock_fd);
    close(pings.epoll_fd);

    if(pings.send_error || pings.recv_error){
        goto failed;
    }

    print_record(&pings);
    free_memory(mem_ptr);

    return PINGS_OK;

failed:
    free_memory(mem_ptr);

    return PINGS_ERROR;
}


