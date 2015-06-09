#include "pings.h"

int recv_packets(pings_params *pings)
{
    struct ping_node *data = NULL;
    struct in_addr dest_addr = {0};
    int is_recv_packet = PINGS_FALSE;
    int recv_one_flag;
    float rtt;

    while(1){
        recv_one_flag = recv_one_packet(pings, &dest_addr, &rtt);

        if(PINGS_RECV_FAILED == recv_one_flag){
            if(errno != EAGAIN){
                pings->recv_error = 1;
                pings_error("recv_packets");
            }

            break;
        }

        if(PINGS_UNPACK_FAILED == recv_one_flag){
            continue;
        }

        data = record_recv_info(dest_addr, rtt);

        if(NULL == data){
            continue;
        }

        is_recv_packet = PINGS_TRUE;

    }

    return is_recv_packet;
}

int recv_one_packet(pings_params *pings, struct in_addr *dest_addr, float *rtt)
{
    char recv_packet_buf[PINGS_PACKET_SIZE] = {0};
    struct sockaddr_in from ;
    socklen_t fromlen = sizeof(struct sockaddr_in);
    int unpack_flag;
    int recv_len;

    recv_len = recvfrom(pings->sock_fd, recv_packet_buf, sizeof(recv_packet_buf), \
            0, (struct sockaddr *)&from, &fromlen);
    if(recv_len < 0){
        return PINGS_RECV_FAILED;
    }

    unpack_flag = unpack(recv_packet_buf, recv_len, rtt, pings->pid);

    if(PINGS_FAILED == unpack_flag){
        return PINGS_UNPACK_FAILED;
    }

    dest_addr->s_addr = from.sin_addr.s_addr;

    return PINGS_SUCCESS;
}

struct ping_node *record_recv_info(struct in_addr dest_addr, float rtt)
{
    struct ping_node *data = NULL;

    data = pings_search(&g_tree, dest_addr.s_addr);
    if(NULL != data){
        data->ping_info->avg_rtt += rtt;
        data->ping_info->recv += 1;
    }

    return data;
}

float time_sub(struct timeval *out, struct timeval *in)
{
    int micro_second = 1000000;
    float interval= 0.0;

    if((out->tv_usec -= in->tv_usec) < 0){
        --out->tv_sec;
        out->tv_usec += micro_second;
    }
    out->tv_sec -= in->tv_sec;
    interval = out->tv_sec * 1000 + out->tv_usec / 1000;

    return  interval;
}
