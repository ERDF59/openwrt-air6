/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#ifndef QUECTEL_LOG_H
#define QUECTEL_LOG_H

#include <uci.h>
#include <syslog.h>  //openlog(), syslog(), closelog()

extern unsigned int quectel_log_initFlag;
extern unsigned int quectel_log_print;
extern unsigned int quectel_log_level;
void quectel_log_init();

#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_LEV_ERR     3
#define LOG_LEV_WARNING 4
#define LOG_LEV_NOTICE  5
#define LOG_LEV_INFO    6
#define LOG_LEV_DEBUG   7

// Log on logd
#define QLOGD(tag,f,...)        do{\
        if(!quectel_log_initFlag) \
                quectel_log_init(); \
        if((quectel_log_print) && (quectel_log_level >= LOG_LEV_DEBUG)) \
                syslog(LOG_LEV_DEBUG,"[%s]%s():%d " f " ",tag,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

#define QLOGI(tag,f,...)        do{\
        if(!quectel_log_initFlag) \
                quectel_log_init(); \
        if((quectel_log_print) && (quectel_log_level >= LOG_LEV_INFO)) \
                syslog(LOG_LEV_INFO,"[%s]%s():%d " f " ",tag,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

#define QLOGN(tag,f,...)        do{\
        if(!quectel_log_initFlag) \
                quectel_log_init(); \
        if((quectel_log_print) && (quectel_log_level >= LOG_LEV_NOTICE)) \
                syslog(LOG_LEV_NOTICE,"[%s]%s():%d " f " ",tag,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

#define QLOGW(tag,f,...)        do{\
        if(!quectel_log_initFlag) \
                quectel_log_init(); \
        if((quectel_log_print) && (quectel_log_level >= LOG_LEV_WARNING)) \
                syslog(LOG_LEV_WARNING,"[%s]%s():%d " f " ",tag,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

#define QLOGE(tag,f,...)        do{\
        if(!quectel_log_initFlag) \
                quectel_log_init(); \
        if((quectel_log_print) && (quectel_log_level >= LOG_LEV_ERR)) \
                syslog(LOG_LEV_ERR,"[%s]%s():%d " f " ",tag,__FUNCTION__,__LINE__,## __VA_ARGS__);}while(0);

#endif