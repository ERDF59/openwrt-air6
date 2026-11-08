#include "atcid.h"
#include "atcid_serial.h"
#include "atcid_async_notify.h"
#include "platform.h"
#include <termios.h>
extern Serial serial;
pthread_mutex_t async_list_mutex;

static bool g_at_in_process = false;

void set_at_in_process(bool state)
{
    pthread_mutex_lock(&async_list_mutex);
    //LOGATCI(LOG_INFO, "set_at_in_process %d",state);
    g_at_in_process = state;
    pthread_mutex_unlock(&async_list_mutex);
}

bool get_at_cmd_in_progress()
{
    return g_at_in_process;
}

int get_async_list_count()
{
    return g_async_list_node_count;
}

static void async_list_append_node(atcid_async_notify_t *node)
{
    //LOGATCI(LOG_INFO, "async_list_append_node");
    if (g_async_list == NULL) {
        g_async_list = node;
    } else {
        /* Insert to tail */
        atcid_async_notify_t *next_node = g_async_list;
        while (next_node->next != NULL) {
            next_node = next_node->next;
        }
        next_node->next = node;
    }
    
    g_async_list_node_count++;
}

static void async_list_remove_head(void)
{
    if (g_async_list == NULL) {
        return;
    }

    free(g_async_list->rsp_string);
    g_async_list->rsp_string = NULL;
    atcid_async_notify_t *previous = g_async_list;
    g_async_list = g_async_list->next;
    free(previous);
    previous = NULL;
    g_async_list_node_count--;
    
}


void delayed_async_print(const char *message, size_t rsp_length)
{
    /*lint -esym(429,message) message will freed where it defined*/
    if (message == NULL) {
        return;
    }

    pthread_mutex_lock(&async_list_mutex);
    while ((g_async_list_node_count + 1 > ASYNC_LIST_NODE_MAX_COUNT) &&
        g_async_list_node_count != 0) {
        async_list_remove_head();
    }
    pthread_mutex_unlock(&async_list_mutex);

    atcid_async_notify_t *node = (atcid_async_notify_t*)malloc(sizeof(atcid_async_notify_t));
    if (node == NULL) {
        return;
    }
    node->rsp_string = malloc(rsp_length);
    if (node->rsp_string == NULL) {
        LOGATCI(LOG_INFO, "return NULL");
        free(node);
        return;
    }
    
    memcpy(node->rsp_string, message, rsp_length);
       
    node->rsp_length = rsp_length;
    node->next = NULL;

    pthread_mutex_lock(&async_list_mutex);
    async_list_append_node(node);
    pthread_mutex_unlock(&async_list_mutex);
 
    return;
}


void delayed_async_output_list(void)
{
    int i = 0;
    int max_fd = -1;
    fd_set write_fds;
    struct timeval m_time_out;

    int ql_urc_port[] = {
        0,                      //USB_ETS
        1,                      //MAIN_UART
        NET_CLIENT_NUM          //NET_PORT
    };
    pthread_mutex_lock(&async_list_mutex);
    while (g_async_list != NULL) {
        //LOGATCI(LOG_INFO, "g_async_list");
        //write to usb AT port & main uart & internet
        for (i = 0; i < sizeof(ql_urc_port)/sizeof(int); i++) 
        {
            if(serial.fd[ql_urc_port[i]] > 0)
            {
                tcflush(serial.fd[ql_urc_port[i]], TCIOFLUSH);
                FD_ZERO(&write_fds);
                FD_SET(serial.fd[ql_urc_port[i]], &write_fds);
                max_fd = serial.fd[ql_urc_port[i]] + 1;
                LOGATCI(LOG_INFO, "URC before write to device [%d] port",i);
                if(select(max_fd, NULL, &write_fds, NULL, &m_time_out) > 0)
                {
                    if (FD_ISSET(serial.fd[ql_urc_port[i]], &write_fds))
                    {
                        //LOGATCI(LOG_INFO, "URC before write to device [%d] port",i);
                        write(serial.fd[ql_urc_port[i]], g_async_list->rsp_string, g_async_list->rsp_length);
                        //LOGATCI(LOG_INFO, "URC after write to device [%d] port",ql_urc_port[i]);
                    }
                }
            }
        }    

        async_list_remove_head();
    }
    
    pthread_mutex_unlock(&async_list_mutex);
}


void atcid_async_notify_init(void)
{
    pthread_mutex_init(&async_list_mutex,NULL);
}
