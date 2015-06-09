#include "pings.h"

int pack(int pack_no, char *sendpacket, pid_t pid)
{
    int packet_size;
    struct icmp *icmp;
    struct timeval *tval;

    icmp = (struct icmp*)sendpacket;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = htons(0);
    icmp->icmp_seq = htons(pack_no);
    icmp->icmp_id = htons(pid);

    packet_size = PINGS_ICMP_HEADER_SIZE + PINGS_ICMP_DEFAULT_DATA_SIZE;
    tval = (struct timeval *)icmp->icmp_data;
    gettimeofday(tval, NULL);
    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packet_size);

    return packet_size;
}

int unpack(char *buf, int len, float *rtt, pid_t pid)
{
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *time_sent;
    struct timeval time_recv;

    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2;
    icmp = (struct icmp *)(buf + iphdrlen);
    len -= iphdrlen;

    if(len < PINGS_ICMP_HEADER_SIZE){
        return PINGS_FAILED;
    }

    if((icmp->icmp_type == ICMP_ECHOREPLY) && (ntohs(icmp->icmp_id) == pid)){

        gettimeofday(&time_recv, NULL);
        time_sent = (struct timeval *)icmp->icmp_data;
        *rtt = time_sub(&time_recv,time_sent);

        return PINGS_SUCCESS;
    }

    return PINGS_FAILED;
}
