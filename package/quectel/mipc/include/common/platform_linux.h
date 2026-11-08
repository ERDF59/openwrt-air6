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

#ifndef _PLATFORM_LINUX_H_
#define _PLATFORM_LINUX_H_

#include "pthread.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#define DLL_EXPORT

#define HTOML(x) (x)
#define HTOMS(x) (x)
#define MTOHL(x) (x)
#define MTOHS(x) (x)
#define ALLOC malloc
#define FREE free
#define SLEEP(x) sleep(x)
#define MEMCPY memcpy
#define MEMCMP memcmp
#define MEMSET memset

typedef uint32_t (*THREAD_FUNC)(void *priv_ptr);
typedef void (*CALLBACK) (void *cb_priv_ptr);

typedef struct {
    pthread_t tid;
    THREAD_FUNC func;
    void *func_priv_ptr;
} THREAD;

typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int32_t ready;
    void *result_ptr;
    uint32_t timeout_ms;
} EVENT;

typedef struct {
    pthread_mutex_t hMutex;
} MUTEX;

#ifdef __cplusplus
extern "C" {
#endif

THREAD *CREATE_THREAD(THREAD_FUNC func, void *func_priv_ptr);
void DELETE_THREAD(THREAD *thread_ptr);

//
MUTEX *CREATE_MUTEX();
void DELETE_MUTEX(MUTEX *mutex_ptr);
void LOCK_MUTEX(MUTEX *mutex_ptr);
void UNLOCK_MUTEX(MUTEX *mutex_ptr);
//
#ifdef ENABLE_MIPC_SUPPORT_CHECKER
int32_t CHECK_MIPC_SUPPORT();
#endif
//
void INIT();
//
void SETCOM(const char *port_name_ptr);
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

#define ErrMsg(format, ...) while(1){char error_msg[256]={0}; (void)snprintf(error_msg,sizeof(error_msg)-1,format,##__VA_ARGS__);WRITEERROR(error_msg);break;}
#define DbgMsg(format, ...) while(1){char debug_msg[256]={0}; (void)snprintf(debug_msg,sizeof(debug_msg)-1,format,##__VA_ARGS__);WRITEDEBUG(debug_msg);break;}

#ifdef __cplusplus
extern "C" {
#endif

//
DLL_EXPORT void UNIT_TEST_READ_BUF_ADD(uint8_t *ptr, uint32_t ptr_len);
DLL_EXPORT uint8_t *UNIT_TEST_WRITE_BUF_GET(uint32_t *len_ptr);

//
DLL_EXPORT EVENT *CREATE_EVENT(uint32_t timeout_ms);
DLL_EXPORT void DELETE_EVENT(EVENT *event_ptr);
DLL_EXPORT int32_t WAIT_EVENT(EVENT *event_ptr);
DLL_EXPORT void WAKE_EVENT(EVENT *event_ptr, void *result_ptr);
//
DLL_EXPORT void UNIT_TEST_RESET();
//
DLL_EXPORT void WAIT_MD_READY_API(CALLBACK cb, void* cb_priv);

#ifdef __cplusplus
}
#endif

#endif
