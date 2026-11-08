/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef ATCI_SERVICE_H
#define ATCI_SERVICE_H

#include <stdio.h>
#if defined(TELEMATICS) || defined(RDKB)
#include <syslog.h>
#endif

#ifdef _cplusplus
extern "C"{
#endif
#include "qlog.h"
#ifdef _cplusplus
}
#endif
#define QUECTEL_LOG

#ifndef QUECTEL_LOG
#define LOG_ERR     1
#define LOG_WARNING 2
#define LOG_INFO    3
#define LOG_DEBUG   4
#endif

#if defined(TELEMATICS) || defined(RDKB)
#define ALOGD(...)   {\
        syslog(LOG_DEBUG, ## __VA_ARGS__);}
#define ALOGE(...)   {\
        syslog(LOG_ERR, ## __VA_ARGS__);}
#define ALOGW(...)   {\
        syslog(LOG_WARNING, ## __VA_ARGS__);}
#define ALOGI(...)   {\
        syslog(LOG_INFO, ## __VA_ARGS__);}
#else
#define ALOGD printf
#define ALOGE printf
#define ALOGW printf
#define ALOGI printf
#endif

#define MTK_PROPERTY_VALUE_MAX 256

#ifdef QUECTEL_LOG
#ifdef _cplusplus
extern "C"{
#endif
#define LOGATCI(lvl, f, ...) do { \
        if (lvl <= LOG_ERR) {QLOGE("ATCI Service", f "\n", ##__VA_ARGS__);} \
        else if (lvl <= LOG_WARNING) {QLOGW("ATCI Service", f "\n", ##__VA_ARGS__);} \
        else if (lvl <= LOG_INFO) {QLOGI("ATCI Service", f "\n", ##__VA_ARGS__);} \
        else if (lvl == LOG_DEBUG) {QLOGD("ATCI Service", f "\n", ##__VA_ARGS__);} \
    } while (0)
#ifdef _cplusplus
}
#endif
#else
#define LOGATCI(lvl, f, ...) do { \
        if (lvl <= LOG_ERR) {ALOGE("[ATCI Service] %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);} \
        else if (lvl == LOG_WARNING) {ALOGW("[ATCI Service] %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);} \
        else if (lvl <= LOG_INFO) {ALOGI("[ATCI Service] %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);} \
        else if (lvl == LOG_DEBUG) {ALOGD("[ATCI Service] %d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);} \
    } while (0)
#endif

#define UNUSED(x) do { (void)(x); } while (0)

extern void processAtcidResponse(char* data, int dataLen);

#endif
