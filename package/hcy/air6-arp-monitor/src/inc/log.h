/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#ifndef LOG_FOR_APR_H
#define LOG_FOR_APR_H

#include "qlog.h"

#define TAG_STR "arp_monitor"

#define APR_QLOGD(...)  QLOGD(TAG_STR,## __VA_ARGS__)
#define APR_QLOGI(...)  QLOGI(TAG_STR,## __VA_ARGS__)
#define APR_QLOGN(...)  QLOGN(TAG_STR,## __VA_ARGS__)
#define APR_QLOGW(...)  QLOGW(TAG_STR,## __VA_ARGS__)
#define APR_QLOGE(...)  QLOGE(TAG_STR,## __VA_ARGS__)

#endif