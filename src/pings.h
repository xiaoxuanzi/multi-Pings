#ifndef MULTI_PINGS_H
#define MULTI_PINGS_H

#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <fcntl.h>
#include "pings_rbtree.h"

#define PINGS_PACKET_SIZE     4096
#define PINGS_SEND_PACKET_NUMBER  3
#define PINGS_DEST_IP_NUMBERS  4000
#define PINGS_ICMP_HEADER_SIZE 8
#define PINGS_IP_HEADER_SIZE 20
#define PINGS_ICMP_DEFAULT_DATA_SIZE 16 /*56*/
#define PINGS_IP_INFO_SIZE 17
#define PINGS_EPOLL_MAX_EVENTS 1
#define PINGS_EPOLL_TIMEOUT 2000 /*millisecond*/
#define PINGS_RECV_INTERVAL_THRESHOLD 1000
#define PINGS_CURRENT_DEST_IP_NUM 100
#define PINGS_RESEND_NUMBER 2
#define PINGS_FAILED -1
#define PINGS_SUCCESS 1
#define PINGS_TRUE 1
#define PINGS_FALSE 0
#define PINGS_RECV_FAILED -2
#define PINGS_UNPACK_FAILED -3
#define PINGS_ARGC 3
#define PINGS_DEBUG
#define PINGS_OK 0
#define PINGS_ERROR 1

extern struct rb_root g_tree ;
extern struct ping_node *g_record[PINGS_DEST_IP_NUMBERS];

struct info{
    int sent;
    int recv;
    float avg_rtt;
    struct in_addr ip;
};

struct ping_node{
    struct rb_node node;
    struct info *ping_info;
};

typedef struct{
    int sock_fd;
    int dest_ip_num;
    int already_sent_ip_num;
    int sent_all_flag;
    int resent_probe_ip_num;
    int is_resent;
    int resend_flag;
    int resend_num;
    int send_error;
    int recv_error;
    float recv_interval;
    struct timeval recv_last_packet_time;
    struct epoll_event event;
    pid_t pid;
    int epoll_fd;
    char output_format;
}pings_params;

int init_socket();
int init_dest_ip();
int init_pings(pings_params *pings, int dest_ip_number, char *output_format);
int check_argv(int argc, char *argv[]);

int handle_timeout_event(pings_params *pings);
void handle_send_event(pings_params *pings);
void send_packets(pings_params *send_arg);
int _send_packets(int sock_fd, struct in_addr dest_ip, pid_t pid);
void resend_packets(pings_params *send_arg);

void handle_recv_event(pings_params *pings);
int recv_packets(pings_params *send_arg);
int recv_one_packet(pings_params *send_arg, struct in_addr *dest_addr, float *rtt);
struct ping_node *record_recv_info(struct in_addr dest_addr, float rtt);

int pack(int pack_no, char *send_packet_buf, pid_t pid);
int unpack(char *buf, int len, float *rtt, pid_t pid);

float time_sub(struct timeval *out, struct timeval *in);
int set_socket_non_blocking(int fd);
unsigned short cal_chksum(unsigned short *addr, int len);

int pings_insert(struct rb_root *root, struct ping_node *node);
struct ping_node* pings_search(struct rb_root *root, unsigned long from_ip);
int compare(unsigned long a, unsigned long b);

void *alloc_memory();
void free_memory();

void print_record(pings_params *pings);
void print_yaml_format(pings_params *pings);
void print_json_format(pings_params *pings);
void print_stream_format(pings_params *pings);
inline void pings_error(char *error);

#endif
