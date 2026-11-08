/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include "mipc_msg_host.h"
#include "mipc_msg_tlv_api.h"
#include "uci.h"
#include <syslog.h>
#include <json.h>
#include <list.h>
#include "event_trigger_logging.h"

//uci get mdlog.property.dump_or_pls_flush
#define PLS_FLUSH_START_PRE_STATE  1 
#define PLS_FLUSH_START_STATE  2    //pls flush start state
#define PLS_FLUSH_END_STATE    0    //pls flush end state

#define UCI_CONFIG_FILE_SYSTEM "/etc/config/system"
#define UCI_CONFIG_FILE_MDLOG  "/etc/config/mdlog"
#define TRIGGER_EVENT_PATCH    "/etc/event_trigger.json" //Add events you follow

static LIST_HEAD(event_list);
static int trigger_log_enable = 0;
static int trigger_log_state = 0;
static pthread_cond_t cond;
static pthread_mutex_t mutex;
static void ql_mipc_at_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr);

void insertNode(struct list_head* list, const char* data)
{
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = strdup(data);
    list_add_tail(&newNode->list, list);
}

void printList(struct list_head* head)
{
    struct Node* node;
    if(list_empty(head))
    {
        return;
    }

    list_for_each_entry(node, head, list)
    {
        QLOGE("%s\n", node->data);
    }
}

void clearList(struct list_head* head)
{
    struct Node* node, *tmp;
    if(list_empty(head))
    {
        return;
    }

    list_for_each_entry_safe(node, tmp, head, list)
    {
        free(node->data);
        list_del(&node->list);
        free(node);
    }
}

void parseJsonAndFillList(struct json_object *jsonObj, struct list_head* list)
{
    struct json_object *trigger_event = json_object_object_get(jsonObj, "trigger_event");

    if (json_object_is_type(trigger_event, json_type_array))
    {
        size_t len = json_object_array_length(trigger_event);
        for (size_t i = 0; i < len; i++)
        {
            struct json_object *item = json_object_array_get_idx(trigger_event, i);
            const char *eventName = json_object_get_string(item);
            insertNode(list, eventName);
        }
    }

    json_object_put(jsonObj);
}

int parseJsonFile(char* file_path)
{
    struct json_object *jsonObj = json_object_from_file(file_path);
    if(jsonObj == NULL)
    {
        QLOGD("Error opening file:%s",file_path);
        return -1;
    }

    INIT_LIST_HEAD(&event_list);
    parseJsonAndFillList(jsonObj, &event_list);

    printList(&event_list);
    return 0;
}

static int getCfgValue(char *cfg_file_path, char *name, char *value, int valueBufLen)
{
	if (NULL == name || NULL == value)
    {
		return -1;
	}

	struct uci_context * uciCtx = uci_alloc_context();
	const char * pValueData = NULL;
	struct uci_package * pkg = NULL;
	struct uci_element * e;

	if (UCI_OK != uci_load(uciCtx, cfg_file_path, &pkg))
    {
		goto cleanup;
	}

	uci_foreach_element(&pkg->sections, e)
	{
		struct uci_section *s = uci_to_section(e);
		if (NULL != (pValueData = uci_lookup_option_string(uciCtx, s, name)))
        {
			strncpy(value, pValueData, valueBufLen);
		}
	}

	uci_unload(uciCtx, pkg);

cleanup:
	uci_free_context(uciCtx);
	uciCtx = NULL;
	return 1;
}

void check_event_trigger_logging_cfg(void)
{
    int md_start = 0, md_save_mode =0;
    char configStr[128];

    memset(configStr, 0, sizeof(configStr));
	if (getCfgValue(UCI_CONFIG_FILE_SYSTEM, "md_start", configStr, sizeof(configStr)))
    {
		md_start = atoi(configStr);
	}
    memset(configStr, 0, sizeof(configStr));
	if (getCfgValue(UCI_CONFIG_FILE_SYSTEM, "md_save_mode", configStr, sizeof(configStr)))
    {
		md_save_mode = atoi(configStr);
	}

    if(md_start == 1 && md_save_mode == 3 )
    {
        if(!trigger_log_enable)
        {
            mipc_init("voice_server");  
            mipc_msg_register_ind(MIPC_MSG_PS0, MIPC_SYS_AT_IND, (void*)ql_mipc_at_ind_cb, NULL);
            QLOGD("event_trigger_logging_enable");
        }
        trigger_log_enable = 1;

    }
    else
    {
        if(trigger_log_enable)
        {
            mipc_msg_register_ind(MIPC_MSG_PS0, MIPC_SYS_AT_IND, NULL, NULL);
            mipc_deinit();
            QLOGD("event_trigger_logging_disable");
        }
        trigger_log_enable = 0;
    }
}

void catch_signal(int param)
{
    //QLOGE("signal number=%d\n", param);
	switch (param) {
	case SIGUSR1:
        check_event_trigger_logging_cfg();
		break;
    default:
        break;
    }
}

static void ql_mipc_at_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr)
{
    char *urc_ptr;
    uint16_t urc_len;
    char *pos = NULL, *final_urc_ptr = NULL;
    uint16_t wio_fmt_urc_len = 0;

    urc_ptr = mipc_sys_at_ind_get_atcmd(msg_ptr, &urc_len);

    QLOGD("URC to atcid =%s\n", urc_ptr);

    pos = strchr(urc_ptr,'+');
    if(pos != NULL)
    {
        wio_fmt_urc_len = strlen(pos);
        while(1)
        {
            if((pos[wio_fmt_urc_len-1] == 0x0d) || (pos[wio_fmt_urc_len-1] == 0x0a))
                wio_fmt_urc_len--;
            else
                break;
        }
        final_urc_ptr = (char *)malloc(wio_fmt_urc_len+5);
        memset(final_urc_ptr, 0, wio_fmt_urc_len+5);
        strncat(final_urc_ptr, pos, wio_fmt_urc_len);

        if(trigger_log_enable && trigger_log_state==0)
        {
            struct Node* node;
            if(!list_empty(&event_list))
            {
                list_for_each_entry(node, &event_list, list) {
                    if(strstr(pos, node->data) != NULL)
                    {
                        QLOGE("Event Trigger Logging is enable for %s", node->data);
                        pthread_mutex_lock(&mutex);
                        trigger_log_state = 1;
                        pthread_cond_signal(&cond);
                        pthread_mutex_unlock(&mutex);
                        break;
                    }
                }
            }
        }
        free(final_urc_ptr);
    }
}

int get_dump_or_pls_flush_state(void)
{
    int flush_state = -1;
    char configStr[128] = {0};

	if (getCfgValue(UCI_CONFIG_FILE_MDLOG, "dump_or_pls_flush", configStr, sizeof(configStr)))
    {
		flush_state = atoi(configStr);
	}

    return flush_state;
}

void *event_trigger_logging_handle_thead(void *arg)
{
    int timeout = 100; //10s
    QLOGE("event_trigger_logging_handle_thead\n");

    while(1)
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex);
        QLOGE("event_trigger_logging enter\n");
        // 1. set md_flush_pls_log=1 to notify the modem to start saving the mdlog
        system("uci set system.@system[0].md_flush_pls_log='1';uci commit;reload_config");
        // 2. wait for the modem to start saving the mdlog
        timeout = 100;
        while(timeout > 0)
        {
            if ( get_dump_or_pls_flush_state() == PLS_FLUSH_START_PRE_STATE \
             || get_dump_or_pls_flush_state() == PLS_FLUSH_START_STATE) {
                break;
            } else {
                usleep(100*1000);
                timeout--;
            }
        }
        if (timeout <= 0) {
            QLOGE("flush state timeout\n")
        }
        // 3. wait for the modem to finish saving the mdlog
        while(get_dump_or_pls_flush_state() != PLS_FLUSH_END_STATE)
        {
           usleep(100*1000);
        }
        // 4. set md_flush_pls_log=0 to notify the modem to wait for the next trigger to save the mdlog
        // must set md_flush_pls_log=0 after the modem finishes saving the mdlog, otherwise the next start will not be triggered
        system("uci set system.@system[0].md_flush_pls_log='0';uci commit;reload_config");
        QLOGE("event_trigger_logging exit\n");
        trigger_log_state = 0;
        pthread_mutex_unlock(&mutex);
    }
}

int main() 
{
	if(signal(SIGUSR1, catch_signal)==SIG_ERR)
    {
        QLOGE("can't catch SIGUSR1\n");
    }

    pthread_t etlog_thread = 0;
    QLOGE("entry Event Trigger Logging demo\n");
    system("uci set system.@system[0].md_flush_pls_log='0';uci commit;reload_config");

    INIT_LIST_HEAD(&event_list);
    parseJsonFile(TRIGGER_EVENT_PATCH);
    check_event_trigger_logging_cfg();

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond,NULL);
    pthread_create(&etlog_thread,NULL,event_trigger_logging_handle_thead,NULL);

    pthread_join(etlog_thread,NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    clearList(&event_list);
    QLOGE("Event Trigger Logging demo exit\n");

    return 0;
}