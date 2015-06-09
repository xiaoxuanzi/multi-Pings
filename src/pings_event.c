#include "pings.h"

void handle_send_event(pings_params *pings)
{
    if(pings->sent_all_flag == PINGS_FALSE){
        send_packets(pings);

        if(pings->send_error){
            return;
        }
    }

    if(pings->sent_all_flag && pings->resend_flag \
                            && pings->resend_num < PINGS_RESEND_NUMBER){

        resend_packets(pings);

        if(pings->send_error){
            return;
        }

        if(pings->resent_probe_ip_num == pings->dest_ip_num){
            if(pings->is_resent == PINGS_TRUE){
                pings->resend_num++;
                pings->resent_probe_ip_num = 0;
                pings->is_resent = PINGS_FALSE;
            }
            else{
                pings->resend_num = PINGS_RESEND_NUMBER;
            }
        }

        pings->resend_flag = PINGS_FALSE;
    }

    pings->event.events = EPOLLIN | EPOLLET;
    epoll_ctl(pings->epoll_fd, EPOLL_CTL_MOD, pings->sock_fd, &(pings->event));

}

void handle_recv_event(pings_params *pings)
{
    struct timeval current_time =  {0};
    int is_recv_packet;

    is_recv_packet = recv_packets(pings);

    if(pings->recv_error){
        return;
    }

    if(PINGS_TRUE == is_recv_packet){
        gettimeofday(&(pings->recv_last_packet_time), NULL);
        pings->recv_interval = 0.0;
    }
    else{
        gettimeofday(&current_time, NULL);
        pings->recv_interval = time_sub(&current_time, &(pings->recv_last_packet_time));
    }

    if(pings->resend_num < PINGS_RESEND_NUMBER){
        pings->event.events = EPOLLOUT | EPOLLET;
        epoll_ctl(pings->epoll_fd, EPOLL_CTL_MOD, pings->sock_fd, &(pings->event));
    }
    else{
        pings->event.events = EPOLLIN | EPOLLET;
        epoll_ctl(pings->epoll_fd, EPOLL_CTL_MOD, pings->sock_fd, &(pings->event));
    }
}

int handle_timeout_event(pings_params *pings)
{
    if(pings->resend_num >= PINGS_RESEND_NUMBER){
        return PINGS_FAILED;
    }

    if(PINGS_TRUE == pings->sent_all_flag){
        pings->resend_flag = PINGS_TRUE;
    }

    if(pings->event.events != (EPOLLOUT | EPOLLET)){
        pings->event.events = EPOLLOUT | EPOLLET;
        epoll_ctl(pings->epoll_fd, EPOLL_CTL_MOD, pings->sock_fd, &(pings->event));
    }

    gettimeofday(&(pings->recv_last_packet_time), NULL);

    return PINGS_SUCCESS;
}
