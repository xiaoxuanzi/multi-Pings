#include "pings.h"

int set_socket_non_blocking(int fd)
{
    int flags;
    int s;

    flags = fcntl(fd, F_GETFD, 0);
    if(flags == -1){
        return PINGS_FAILED;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if(s == -1){
        return PINGS_FAILED;
    }

    return PINGS_SUCCESS;
}

int init_dest_ip()
{
    int dest_ip_num = 0;
    char buf[PINGS_IP_INFO_SIZE]={0};
    struct in_addr dest_addr;

    while(fgets(buf, PINGS_IP_INFO_SIZE, stdin)){

        if(( dest_ip_num + 1 ) > PINGS_DEST_IP_NUMBERS){
            return PINGS_FAILED;
        }

        buf[strlen(buf)-1] = '\0';
        inet_pton(AF_INET, buf, &dest_addr);

        if(g_record[dest_ip_num] != NULL && g_record[dest_ip_num]->ping_info != NULL){
            g_record[dest_ip_num]->ping_info->ip = dest_addr;
        }

        dest_ip_num ++;

    }


    return dest_ip_num;
}


int init_socket()
{
    struct protoent *protocol = NULL;
    int sock_fd;
    int ret;
    socklen_t optlen;
    int send_buf_size;
    int icmp_packet_size = PINGS_ICMP_DEFAULT_DATA_SIZE + PINGS_ICMP_HEADER_SIZE + PINGS_IP_HEADER_SIZE;
    int send_buf_min_size = PINGS_SEND_PACKET_NUMBER * PINGS_CURRENT_DEST_IP_NUM * icmp_packet_size;
    int recv_buf_size;
    int recv_buf_min_size = PINGS_SEND_PACKET_NUMBER * PINGS_DEST_IP_NUMBERS * icmp_packet_size;

    protocol = getprotobyname("icmp");

    if(protocol == NULL){
        return PINGS_FAILED;
    }

    sock_fd  = socket(AF_INET, SOCK_RAW, protocol->p_proto);

    if(sock_fd < 0){
        return PINGS_FAILED;
    }

    ret = set_socket_non_blocking(sock_fd);

    if(PINGS_FAILED == ret){
        goto socket_failed;
    }

    optlen = sizeof(send_buf_size);
    ret = getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, &optlen);

    if(ret < 0){
        goto socket_failed;
    }

    if(send_buf_size < send_buf_min_size){
        ret = setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUFFORCE, &send_buf_min_size,\
                                    sizeof(send_buf_min_size));
        if(ret < 0){
            goto socket_failed;
        }
    }


    optlen = sizeof(recv_buf_size);
    ret = getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, &optlen);

    if(ret < 0){
        goto socket_failed;
    }

    if(recv_buf_size < recv_buf_min_size){
        ret = setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUFFORCE, &recv_buf_min_size,\
                                        sizeof(recv_buf_min_size));

        if(ret < 0){
            goto socket_failed;
        }
    }

    return sock_fd;

socket_failed:

    close(sock_fd);
    return PINGS_FAILED;

}

int init_pings(pings_params *pings, int dest_ip_num, char *output_format)
{
    int ret;
    pid_t pid = getpid();

    pings->sock_fd = init_socket();

    if(PINGS_FAILED == pings->sock_fd){
        return PINGS_FAILED;
    }

    pings->pid = pid;
    pings->sent_all_flag = PINGS_FALSE;
    pings->resend_num = 0;
    pings->send_error = 0;
    pings->recv_error = 0;
    pings->dest_ip_num = dest_ip_num;
    pings->already_sent_ip_num = 0;
    pings->resent_probe_ip_num = 0;
    pings->resend_flag = PINGS_FALSE;
    pings->is_resent = PINGS_FALSE;
    pings->recv_interval = 0.0;
    gettimeofday(&(pings->recv_last_packet_time), NULL);

    if(!strcmp(output_format, "stream")){
        pings->output_format = 'S';
    }

    if(!strcmp(output_format, "json")){
        pings->output_format = 'J';
    }
    if(!strcmp(output_format, "yaml")){
        pings->output_format = 'Y';
    }


    pings->epoll_fd = epoll_create(1);

    if(-1 == pings->epoll_fd){
        return  PINGS_FAILED;
    }

    pings->event.data.fd = pings->sock_fd;
    pings->event.events = EPOLLOUT | EPOLLET;

    ret = epoll_ctl(pings->epoll_fd, EPOLL_CTL_ADD, pings->sock_fd, &(pings->event));

    if(ret == -1){
        close(pings->epoll_fd);
        return PINGS_FAILED;
    }

    return PINGS_SUCCESS;
}

int check_argv(int argc, char *argv[])
{
    if(argc < PINGS_ARGC){
        printf("Usage : %s -o [OUTPUT_STYLE]\n",argv[0]);
        printf("OUTPUT_STYLE := { stream | json | yaml }\n");

        return PINGS_FAILED;
    }

    if(strcmp(argv[argc - 2], "-o")){
        printf("Option \"%s\" is unknow ,try \"%s\" for help\n",argv[1], argv[0]);

        return PINGS_FAILED;
    }

    if(!strcmp(argv[argc - 1], "stream") || !strcmp(argv[argc - 1], "json")\
                                        || !strcmp(argv[argc - 1], "yaml")){
        return PINGS_SUCCESS;
    }

    printf("Output format \"%s\" is unknow ,try \"%s\" for help\n", argv[argc - 1], argv[0]);

    return PINGS_FAILED;
}
