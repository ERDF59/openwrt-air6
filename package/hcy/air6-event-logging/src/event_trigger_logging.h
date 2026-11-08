/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#ifndef __EVENT_TRIGGER_LOGGING_H__
#define __EVENT_TRIGGER_LOGGING_H__

#define LOG_LEV_ERR     3
#define LOG_LEV_DEBUG   7
#define TAG "event_trigger"
#define QLOGD(f,...)  do{syslog(LOG_LEV_DEBUG,"[%s]%s():%d " f " ",TAG,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);
#define QLOGE(f,...)  do{syslog(LOG_LEV_ERR,"[%s]%s():%d " f " ",TAG,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

struct Node {
    char* data;
    struct list_head list; 
};

#endif