#include "pings.h"

void send_packets(pings_params *pings)
{
    int index;
    int sent_count;
    int current_ip_num;
    int i;

    current_ip_num = pings->dest_ip_num - pings->already_sent_ip_num;

    if(current_ip_num > PINGS_CURRENT_DEST_IP_NUM){
        current_ip_num = PINGS_CURRENT_DEST_IP_NUM;
    }

    for(i = 0; i < current_ip_num; i++){
        index = pings->already_sent_ip_num;
        sent_count = _send_packets(pings->sock_fd, g_record[index]->ping_info->ip, pings->pid);

        if(sent_count < PINGS_SEND_PACKET_NUMBER){
            if(errno != EAGAIN){
                pings->send_error = 1;
                pings_error("send_packets");
            }

            break;
        }

        g_record[index]->ping_info->sent = sent_count;
        pings_insert(&g_tree, g_record[index]);
        pings->already_sent_ip_num++;
    }

    if(pings->already_sent_ip_num == pings->dest_ip_num){
        pings->sent_all_flag = PINGS_TRUE;
    }

}

int _send_packets(int sock_fd, struct in_addr dest_ip, pid_t pid)
{
    int sent_count = 0;
    int index;
    int send_len;
    int packet_size;
    char send_packet_buf[PINGS_PACKET_SIZE] = {0};
    struct sockaddr_in dest_addr = {0};

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = dest_ip;

    for(index = 0; index < PINGS_SEND_PACKET_NUMBER; index ++){
        packet_size = pack(index, send_packet_buf, pid);
        send_len = sendto(sock_fd, send_packet_buf, packet_size, 0, \
                            (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if(send_len < 0){
            break;
        }

        sent_count ++;
    }

    return sent_count;
}

void resend_packets(pings_params *pings)
{
    int sent_count = 0;
    struct info *data = NULL;
    int index = pings->resent_probe_ip_num;
    int already_resent_ip_num = 0;

    for( ; index < pings->dest_ip_num; index ++){
        data = g_record[index]->ping_info;

        if(data && data->sent != 0 && data->recv == 0){
            sent_count = _send_packets(pings->sock_fd, data->ip, pings->pid);

            if(sent_count < PINGS_SEND_PACKET_NUMBER){
                if(errno != EAGAIN){
                    pings->send_error = 1;
                    pings_error("resend_packets");
                }

                break;
            }

            data->sent += sent_count;
            pings->is_resent = PINGS_TRUE;
            already_resent_ip_num ++;

            if(already_resent_ip_num == PINGS_CURRENT_DEST_IP_NUM){
                break;
            }
        }
    }

    pings->resent_probe_ip_num = index;
}
