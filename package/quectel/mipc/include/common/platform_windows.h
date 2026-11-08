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

#ifndef _PLATFORM_WINDOWS_H_
#define _PLATFORM_WINDOWS_H_

#include "windows.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef NO_EXPORT
#define DLL_EXPORT
#else
#define DLL_EXPORT __declspec(dllexport)
#endif

#define HTOML(x) (x)
#define HTOMS(x) (x)
#define MTOHL(x) (x)
#define MTOHS(x) (x)
#define ALLOC malloc
#define FREE free
#define SLEEP(x) Sleep((x)*1000)
#define MEMCPY memcpy
#define MEMCMP memcmp
#define MEMSET memset

typedef uint32_t (*THREAD_FUNC)(void *priv_ptr);
typedef void (*MIPC_CALLBACK) (void *cb_priv_ptr);

typedef struct {
    HANDLE tid;
    THREAD_FUNC func;
    void *func_priv_ptr;
} THREAD;

typedef struct {
    HANDLE hEvent;
    void *result_ptr;
    uint32_t timeout_ms;
} EVENT;

typedef struct {
    HANDLE hMutex;
} MUTEX;

#ifdef __cplusplus
extern "C" {
#endif

THREAD *CREATE_THREAD(THREAD_FUNC func, void *func_priv_ptr);
void DELETE_THREAD(THREAD *thread_ptr);

//
EVENT *CREATE_EVENT();
void DELETE_EVENT(EVENT *event_ptr);
int32_t WAIT_EVENT(EVENT *event_ptr);
void WAKE_EVENT(EVENT *event_ptr, void *result_ptr);
//
MUTEX *CREATE_MUTEX();
void DELETE_MUTEX(MUTEX *mutex_ptr);
void LOCK_MUTEX(MUTEX *mutex_ptr);
void UNLOCK_MUTEX(MUTEX *mutex_ptr);
//
int32_t CHECK_MIPC_SUPPORT();
//
void INIT();
//
DLL_EXPORT void SETCOM(const char *port_name_ptr);
int32_t GETCOM();
const char *FINDCOM(const char *process_name_ptr);
int32_t OPENCOM(const char *port_name_ptr);
void CLOSECOM();
int32_t WRITECOM(uint8_t *buf_ptr, const uint32_t buf_len);
int32_t READCOM(uint8_t *buf_ptr, const uint32_t buf_len);

uint64_t GETTID();

#ifdef __cplusplus
}
#endif


void OPENCAP(const char *process_name_ptr);
void WRITECAP(uint32_t write_len, uint8_t *data, uint32_t msg_direction);
void WRITEERROR(char *error_msg);
void WRITEDEBUG(char *debug_msg);

#define ErrMsg(format, ...) while(1){char error_msg[256]={0}; snprintf(error_msg,sizeof(error_msg)-1,format,##__VA_ARGS__);WRITEERROR(error_msg);break;}
#define DbgMsg(format, ...) while(1){char debug_msg[256]={0}; snprintf(debug_msg,sizeof(debug_msg)-1,format,##__VA_ARGS__);WRITEDEBUG(debug_msg);break;}

DLL_EXPORT void WAIT_MD_READY_API(MIPC_CALLBACK cb, void* cb_priv);

#endif
