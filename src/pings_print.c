#include "pings.h"

#ifdef PINGS_DEBUG
    inline void pings_error(char *error) {printf("pings error: %s falied\n", error);}
#else
    inline void pings_error(char *error) {}
#endif

void print_record(pings_params *pings)
{
    switch(pings->output_format){
        case 'Y':
            print_yaml_format(pings);
            break;
        case 'J':
            print_json_format(pings);
            break;
        case 'S':
            print_stream_format(pings);
            break;
    }
}
void print_stream_format(pings_params *pings)
{
    int i;
    for(i = 0; i < pings->dest_ip_num; i++){
        if((g_record[i] != NULL) && (g_record[i]->ping_info->sent != 0)){
            printf("ip:%s ",inet_ntoa(g_record[i]->ping_info->ip));

            if(g_record[i]->ping_info->recv == 0){
                printf("avg: ");
            }
            else{
                printf("avg:%.0f ",g_record[i]->ping_info->avg_rtt / g_record[i]->ping_info->recv );
            }

            printf("dest:%s ", inet_ntoa(g_record[i]->ping_info->ip));
            printf("loss:%d recv:%d sent:%d\n",g_record[i]->ping_info->sent - g_record[i]->ping_info->recv ,
                                                g_record[i]->ping_info->recv,
                                                g_record[i]->ping_info->sent);

        }
    }
}

void print_yaml_format(pings_params *pings)
{
    int i;
    for(i = 0; i < pings->dest_ip_num; i++){
        if((g_record[i] != NULL) && (g_record[i]->ping_info->sent != 0)){
            printf("%s:\n", inet_ntoa(g_record[i]->ping_info->ip));
            printf(" ip: %s\n", inet_ntoa(g_record[i]->ping_info->ip));

            if(g_record[i]->ping_info->recv == 0){
                printf(" avg: null\n");
            }
            else{
                printf(" avg: %.0f\n",g_record[i]->ping_info->avg_rtt / g_record[i]->ping_info->recv );
            }

            printf(" dest: %s\n", inet_ntoa(g_record[i]->ping_info->ip));
            printf(" loss: %d\n", g_record[i]->ping_info->sent - g_record[i]->ping_info->recv );
            printf(" recv: %d\n", g_record[i]->ping_info->recv );
            printf(" sent: %d\n", g_record[i]->ping_info->sent );
        }
    }
}

void print_json_format(pings_params *pings)
{
    int i;
    for(i = 0; i < pings->dest_ip_num; i++){
        if((g_record[i] != NULL) && (g_record[i]->ping_info->sent != 0)){
            if(i == 0){
                printf("{\"%s\": {",inet_ntoa(g_record[i]->ping_info->ip));
            }
            else{
                printf(" \"%s\": {",inet_ntoa(g_record[i]->ping_info->ip));
            }

            printf(" \"ip\": \"%s\",", inet_ntoa(g_record[i]->ping_info->ip));

            if(g_record[i]->ping_info->recv == 0){
                printf(" \"avg\": null,");
            }
            else{
                printf(" \"avg\": %.0f,",g_record[i]->ping_info->avg_rtt / g_record[i]->ping_info->recv );
            }

            printf(" \"dest\": \"%s\",", inet_ntoa(g_record[i]->ping_info->ip));
            printf(" \"loss\": %d,", g_record[i]->ping_info->sent - g_record[i]->ping_info->recv );
            printf(" \"recv\": %d,", g_record[i]->ping_info->recv );
            if(i == pings->dest_ip_num - 1){
                printf(" \"sent\": %d}}", g_record[i]->ping_info->sent );
            }
            else{
                printf(" \"sent\": %d},", g_record[i]->ping_info->sent );
            }
        }
    }
}
