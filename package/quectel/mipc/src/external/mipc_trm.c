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
#include "mipc_trm.h"

void *libtrm_handle;

void (*LIBTRM_INIT)() = NULL;
void (*LIBTRM_DEINIT)() = NULL;
int (*LIBTRM_RESET)(int ccci_fd) = NULL;
int (*LIBTRM_ASSERT)(int ccci_fd) = NULL;
int (*LIBTRM_POWER_OFF_MD)(int ccci_fd) = NULL;
int (*LIBTRM_POWER_ON_MD)(int ccci_fd) = NULL;
void (*LIBTRM_MD_EVENT_REGISTER)(MIPC_MD_EVENT_CB cb, void *cb_priv_ptr) = NULL;

void libtrm_init()
{
    static int32_t singleton = 0;
    if(singleton == 0){
        //init libtrm
        libtrm_handle = dlopen("/usr/lib64/libtrm.so", RTLD_LAZY);

        if (libtrm_handle) {
            LIBTRM_INIT = (void(*)())dlsym(libtrm_handle, "libtrm_init");
            LIBTRM_DEINIT = (void(*)())dlsym(libtrm_handle, "libtrm_deinit");
            LIBTRM_RESET = (int(*)(int))dlsym(libtrm_handle, "libtrm_reset");
            LIBTRM_ASSERT = (int(*)(int))dlsym(libtrm_handle, "libtrm_md_assert");
            LIBTRM_POWER_OFF_MD = (int(*)(int))dlsym(libtrm_handle, "libtrm_power_off_md");
            LIBTRM_POWER_ON_MD = (int(*)(int))dlsym(libtrm_handle, "libtrm_power_on_md");
            LIBTRM_MD_EVENT_REGISTER = (void(*)(MIPC_MD_EVENT_CB, void *))dlsym(libtrm_handle, "libtrm_md_event_register");
        }
        singleton = 1;
    }
    if (LIBTRM_INIT) {
        LIBTRM_INIT();
    }
}

void libtrm_deinit()
{
    if (LIBTRM_DEINIT) {
        LIBTRM_DEINIT();
    }
}

int mipc_reset()
{
    if (GETCOM() >= 0 && LIBTRM_RESET) {
        LIBTRM_RESET(GETCOM());
        return 0;
    }
    return -1;
}

int mipc_assert()
{
    if (GETCOM() >= 0 && LIBTRM_ASSERT) {
        LIBTRM_ASSERT(GETCOM());
        return 0;
    }
    return -1; 
}

int mipc_power_off()
{
    if (GETCOM() >= 0 && LIBTRM_POWER_OFF_MD) {
        LIBTRM_POWER_OFF_MD(GETCOM());
        return 0;
    }
    return -1;
}

int mipc_power_on()
{
    if (GETCOM() >= 0 && LIBTRM_POWER_ON_MD) {
        LIBTRM_POWER_ON_MD(GETCOM());
        return 0;
    }
    return -1;
}

void mipc_md_event_register(MIPC_MD_EVENT_CB cb, void *cb_priv_ptr)
{
    if (LIBTRM_MD_EVENT_REGISTER) {
        LIBTRM_MD_EVENT_REGISTER(cb, cb_priv_ptr);
    }
}
#endif