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

#include <stdlib.h>
#include "atcid_common.h"
#include "atcid.h"
#include "uci.h"   //modify by quectel for DSDS

#if defined(TELEMATICS)
extern int property_get(const char *key, char *value, const char *default_value);
extern int property_set(const char *key, const char *in_value);
#endif

#ifdef QUECTEL_DSDS_ESUO   //modify by quectel for DSDS,start
int setCfgValue(const char *key, const char *new_value, const char *package, const char *section) 
{
    if (NULL == key || NULL == new_value) {
        LOGATCI(LOG_DEBUG,"key or value is null.\n");
        return -1;
    }
    if (NULL == package || NULL == section) {
        LOGATCI(LOG_DEBUG,"package or section is null.\n");
        return -1;
    }
    int retValue = 0;
    struct uci_context *_ctx = uci_alloc_context();
    struct uci_ptr ptr;
    memset(&ptr, 0, sizeof(ptr));
    char uci_cmd_string[128] = {0};
    snprintf(uci_cmd_string, 128, "%s.%s.%s=%s",package, section, key, new_value);
    char *a = strdup (uci_cmd_string);
    if (uci_lookup_ptr(_ctx, &ptr, a, true) != UCI_OK) {
        LOGATCI(LOG_DEBUG,"failed to find the specified section\n");
         goto cleanup;
    }
    if (uci_set(_ctx, &ptr) != UCI_OK) {
        LOGATCI(LOG_DEBUG,"uci_set fail %s=%s\n", key, new_value);
        retValue = 1;
    } else {
        if (uci_commit(_ctx, &ptr.p, false) != UCI_OK) {
            LOGATCI(LOG_DEBUG,"uci_commit fail %s=%s\n", key, new_value);
            retValue = 1;
        } else {
            LOGATCI(LOG_DEBUG,"uci_commit ok. %s=%s\n", key, new_value);
            retValue = 0;
        }
    }
cleanup:
    free(a);
    uci_free_context(_ctx);
    _ctx = NULL;
    return retValue;
}

int atci_property_get(const char *key, char *value, const char *default_value) {

    #if defined(TELEMATICS)
        int ali_pro_res = property_get(key, value, default_value);
        LOGATCI(LOG_DEBUG, "get key is %s, value is %s", key, value);
        return ali_pro_res;
    #else
        int ali_pro_res = getUCISystemCfgValue(key, value, sizeof(value));
        LOGATCI(LOG_DEBUG, "get key is %s, value is %s", key, value);
        return ali_pro_res;
    #endif
    }
    
int atci_property_set(const char *key, const char *value) {
    
    #if defined(TELEMATICS)
        int ret_val = property_set(key, value);
        LOGATCI(LOG_DEBUG, "set key is %s, value is %s", key, value);
        return ret_val;
    #else
        int ret_val = setCfgValue(key, value, "system", "@system[0]");
        LOGATCI(LOG_DEBUG, "set key is %s, value is %s", key, value);
        return ret_val;
    #endif
}
#else   //modify by quectel for DSDS,end
int atci_property_get(const char *key, char *value, const char *default_value) {

#if defined(TELEMATICS)
    int ali_pro_res = property_get(key, value, default_value);
    LOGATCI(LOG_DEBUG, "get key is %s, value is %s", key, value);
    return ali_pro_res;
#else
    int ali_pro_res = atoi(default_value);
    LOGATCI(LOG_DEBUG, "get key is %s, value is %s", key, value);
    return ali_pro_res;
#endif
}

int atci_property_set(const char *key, const char *value) {

#if defined(TELEMATICS)
    int ret_val = property_set(key, value);
    LOGATCI(LOG_DEBUG, "set key is %s, value is %s", key, value);
    return ret_val;
#else
    int ret_val = 0;
    LOGATCI(LOG_DEBUG, "set key is %s, value is %s", key, value);
    return ret_val;
#endif
}
#endif   //modify by quectel for DSDS,end