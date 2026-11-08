/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  Copyright (c) [2020], MediaTek Inc. All rights reserved.
*  This software/firmware and related documentation ("MediaTek Software") are
*  protected under relevant copyright laws.
*
*  The information contained herein is confidential and proprietary to
*  MediaTek Inc. and/or its licensors. Except as otherwise provided in the
*  applicable licensing terms with MediaTek Inc. and/or its licensors, any
*  reproduction, modification, use or disclosure of MediaTek Software, and
*  information contained herein, in whole or in part, shall be strictly
*  prohibited.
*****************************************************************************/
#ifdef ENABLE_TRM
#include <dlfcn.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

#define DLL_EXPORT

struct md_status_event {
    struct timeval time_stamp;
    int md_id;
    int event_type;
    char reason[32];
};

typedef void (*MIPC_MD_EVENT_CB)(void *priv_ptr, struct md_status_event *event_ptr);
#ifdef __cplusplus
extern "C" {
#endif
extern void (*LIBTRM_INIT)();
extern void (*LIBTRM_DEINIT)();
extern int (*LIBTRM_RESET)(int ccci_fd);
extern int (*LIBTRM_ASSERT)(int ccci_fd);
extern int (*LIBTRM_POWER_OFF_MD)(int ccci_fd);
extern int (*LIBTRM_POWER_ON_MD)(int ccci_fd);

extern void (*LIBTRM_MD_EVENT_REGISTER)(MIPC_MD_EVENT_CB cb, void *cb_priv_ptr);

DLL_EXPORT int mipc_reset();
DLL_EXPORT int mipc_power_off();
DLL_EXPORT int mipc_power_on();
DLL_EXPORT void mipc_md_event_register(MIPC_MD_EVENT_CB cb, void *cb_priv_ptr);

void libtrm_init();
void libtrm_deinit();
#ifdef __cplusplus
}
#endif
#endif