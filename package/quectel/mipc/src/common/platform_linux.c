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

#include "platform.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#ifdef GIGAUT
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include "mipc_hashmap.h"

static int fd = -1;
static char port_name[2048] = {0};
int32_t debug_fp = -1;
static MUTEX *dump_mutex_ptr = 0;
static uint32_t msg_count = 0;
static char client_name[24];
#ifdef PORT_MAP_FILE
typedef struct {
    char* process_name;
    char* port_path;
} MIPC_PORT_MAP;
#endif
#define XSTR(x) #x
#define STR(x) XSTR(x)

#ifdef UNIT_TEST
static uint8_t unit_test_read_buf[65536];
static uint8_t unit_test_write_buf[65536];
static uint32_t unit_test_read_buf_len;
static uint32_t unit_test_write_buf_len;

static EVENT *unit_test_event;
static uint8_t unit_test_reset = 0;

void UNIT_TEST_RESET()
{
    memset(unit_test_read_buf, 0, sizeof(unit_test_read_buf));
    unit_test_read_buf_len = 0;
    memset(unit_test_write_buf, 0, sizeof(unit_test_write_buf));
    unit_test_write_buf_len = 0;
    unit_test_reset = 1;
    WAKE_EVENT(unit_test_event, NULL);
}

void UNIT_TEST_READ_BUF_ADD(uint8_t *ptr, uint32_t ptr_len)
{
    MEMCPY(unit_test_read_buf, ptr, ptr_len);
    unit_test_read_buf_len = ptr_len;
    WAKE_EVENT(unit_test_event, NULL);
}

uint8_t *UNIT_TEST_WRITE_BUF_GET(uint32_t *len_ptr)
{
    *len_ptr = unit_test_write_buf_len;
    return unit_test_write_buf;
}
#endif

#ifdef ENABLE_MIPC_SUPPORT_CHECKER
int32_t CHECK_MIPC_SUPPORT()
{
    struct stat st;
    //wait MD ready
    do {
        char buf[64];
        int ccci_fd = open("/sys/kernel/ccci/boot", O_RDONLY);
        if (ccci_fd >= 0) {
            memset(buf, 0, sizeof(buf));
            read(ccci_fd, buf, sizeof(buf));
            close(ccci_fd);

            if (strncmp(buf, "md1:4", 5) == 0) {
                //MD1 is ready
                break;
            }
        }
        SLEEP(1);
    } while (1);

    //check if the file is existed
    if (stat("/dev/ttyCMIPC0", &st) < 0) {
        return 0;
    } else {
        return 1;
    }
}
#endif
void INIT()
{
#ifndef UNIT_TEST
//    fd = -1;
#else
    unit_test_event = CREATE_EVENT(0);
#endif
}

void SETCOM(const char *port_name_ptr)
{
    int16_t n = 0;
    memset(port_name, 0, sizeof(port_name));
    n = snprintf(port_name, sizeof(port_name)-1, "%s", port_name_ptr);
    if (n < 0 || n >= sizeof(port_name)){
        memset(port_name, 0, sizeof(port_name));
    }
}

int32_t GETCOM()
{
    return fd;
}


const char *FINDCOM(const char *process_name_ptr)
{
#ifdef PORT_MAP_FILE
    MIPC_PORT_MAP *port_map_ptr;
    static MIPC_PORT_MAP port_map[] = {
        #include STR(PORT_MAP_FILE)
        {0,0}
    };
    port_map_ptr = port_map;
    while(port_map_ptr->process_name){
        if (!strcasecmp(port_map_ptr->process_name, process_name_ptr)) {
            return port_map_ptr->port_path;
        }
        port_map_ptr++;
    }
#endif
    if (strlen(port_name)) {
        return port_name;
    } else {
#ifdef SLEEP_FOR_UNREGISTERED_PROCESS
        DbgMsg("Wait 30 seconds, because %s is not registered", process_name_ptr);
        SLEEP(30);
#endif
        return "/dev/ttyCMIPC0";
    }
}

void WAIT_MD_READY_API(CALLBACK cb, void* cb_priv)
{
    do {
        char buf[64];
        int ccci_fd = open("/sys/kernel/ccci/boot", O_RDONLY);
        if (ccci_fd >= 0) {
            memset(buf, 0, sizeof(buf));
            read(ccci_fd, buf, sizeof(buf));
            close(ccci_fd);

            if (strncmp(buf, "md1:4", 5) == 0) {
                //MD1 is ready
                break;
            }
        }
        cb(cb_priv);
    } while (1);
}

static void sleep_for_modem_ready(void* cb_priv)
{
    sleep(1);
}

int32_t OPENCOM(const char *port_name_ptr)
{
    int32_t retry_count = 30;

#ifndef UNIT_TEST
    struct termios options;
#ifdef WAIT_MD_READY
    //wait MD ready
    WAIT_MD_READY_API(sleep_for_modem_ready, NULL);
#endif

    DbgMsg("OPENCOM start");
#ifdef GIGAUT
    {
        struct sockaddr_un un;
        int un_len;
        
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) {
            ErrMsg("GIGAUT SOCKET fail, count = %d, errno = %d (%s)", retry_count, errno, strerror(errno));
            return -1;
        }

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        sprintf(un.sun_path, "%s", port_name_ptr);
        if (connect(fd, (struct sockaddr *) &un, sizeof(struct sockaddr_un)) == -1) {
            ErrMsg("GIGAUT CONNECT fail 1, count = %d, errno = %d (%s)", retry_count, errno, strerror(errno));
            return -1;
        }
    }
#else
    while ((--retry_count > 0) && ((fd = open(port_name_ptr, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)) {
        ErrMsg("OPENCOM fail, count = %d, errno = %d (%s)", retry_count, errno, strerror(errno));
        SLEEP(1);
    }

    if (fd < 0) {
        ErrMsg("OPENCOM fail - end");
        return -1;
    } else {
        // clear non-blocking
        int flags = fcntl(fd, F_GETFL);
        DbgMsg("OPENCOM %s", port_name_ptr);
        flags &= ~O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0) {
            ErrMsg("fcntl F_SETFL error");
        }
    }

    //set terminal options

    if (tcgetattr(fd, &options) != 0) {
        ErrMsg("tcgetattr fail");
    }
    cfmakeraw(&options);
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        ErrMsg("tcsetattr fail");
    }

#endif
#endif
    return 0;
}

void CLOSECOM()
{
#ifndef UNIT_TEST
    if (fd >= 0) {
        close(fd);
    }
#endif
    fd = -1;
}

int32_t WRITECOM(uint8_t *buf_ptr, const uint32_t buf_len)
{
    uint16_t total_write_bytes;
#ifndef UNIT_TEST
    int32_t write_bytes;

    total_write_bytes = 0;
    while (write_bytes = write(fd, buf_ptr + total_write_bytes, buf_len - total_write_bytes)) {
        DbgMsg("write_bytes=%d", write_bytes);
        if (write_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        total_write_bytes += write_bytes;
        if (total_write_bytes == buf_len) {
            break;
        }
    }
#else
    total_write_bytes = (buf_len > sizeof(unit_test_write_buf)) ? sizeof(unit_test_write_buf) : buf_len;
    MEMCPY(unit_test_write_buf, buf_ptr, total_write_bytes);
    unit_test_write_buf_len = total_write_bytes;
#endif
    return total_write_bytes;
}

int32_t READCOM(uint8_t *buf_ptr, const uint32_t buf_len)
{
    uint16_t total_read_bytes;
#ifndef UNIT_TEST
    int32_t read_bytes;

    total_read_bytes = 0;
    while (read_bytes = read(fd, buf_ptr + total_read_bytes, buf_len - total_read_bytes)) {
        DbgMsg("read_bytes=%d", read_bytes);
        if (read_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
#ifdef GIGAUT
            fd = -1;
#endif
            break;
        }
#ifdef GIGAUT
        else if (read_bytes == 0) {
            fd = -1;
            break;
        }
#endif
        total_read_bytes += read_bytes;
        if (total_read_bytes == buf_len) {
            break;
        }
    }
#else
    if (unit_test_read_buf_len == 0 ) {
        unit_test_event->ready = 0;
        WAIT_EVENT(unit_test_event);
    }

    if (unit_test_reset == 0) {
        total_read_bytes = (buf_len > sizeof(unit_test_read_buf)) ? sizeof(unit_test_read_buf) : buf_len;
        MEMCPY(buf_ptr, unit_test_read_buf, total_read_bytes);
        unit_test_read_buf_len -= total_read_bytes;
        if (unit_test_read_buf_len) {
            MEMCPY(unit_test_read_buf, unit_test_read_buf + total_read_bytes, unit_test_read_buf_len);
        }
    } else {
        unit_test_reset = 0;
        total_read_bytes = 0;
    }
#endif
    return total_read_bytes;
}

void *_THREAD_FUNC(void *priv_ptr)
{
    THREAD *thread_ptr = (THREAD *)priv_ptr;

    thread_ptr->func(thread_ptr->func_priv_ptr);

    return NULL;
}

THREAD *CREATE_THREAD(THREAD_FUNC func, void *func_priv_ptr)
{
    THREAD *thread_ptr = ALLOC(sizeof(THREAD));

    if (thread_ptr) {
        thread_ptr->func = func;
        thread_ptr->func_priv_ptr = func_priv_ptr;
        pthread_create(&thread_ptr->tid, NULL, _THREAD_FUNC, thread_ptr);
        pthread_detach(thread_ptr->tid); // pthread_detach() should be called for each thread
    }

    return thread_ptr;
}

void DELETE_THREAD(THREAD *thread_ptr)
{
    if (thread_ptr){
        FREE(thread_ptr);
    }
}

EVENT *CREATE_EVENT(uint32_t timeout_ms)
{
    EVENT *event_ptr = (EVENT *)ALLOC(sizeof(EVENT));

    if (event_ptr) {
        pthread_cond_init(&event_ptr->cond, NULL);
        pthread_mutex_init(&event_ptr->lock, NULL);
        event_ptr->ready = 0;
        event_ptr->timeout_ms = timeout_ms;
    }

    return event_ptr;
}

void DELETE_EVENT(EVENT *event_ptr)
{
    if (event_ptr) {
        FREE(event_ptr);
    }
}

int32_t WAIT_EVENT(EVENT *event_ptr)
{
    if (event_ptr) {
        struct timespec to;
        if (event_ptr->timeout_ms) {
            clock_gettime(CLOCK_REALTIME, &to);
            if (event_ptr->timeout_ms < 1000) {
                to.tv_sec += 1;
            } else {
                to.tv_sec += (event_ptr->timeout_ms / 1000);
            }
            pthread_mutex_lock(&event_ptr->lock);
            //infinite
#ifdef GIGAUT
            while (event_ptr->ready == 0 && fd >= 0) {
#else
            while (event_ptr->ready == 0) {
#endif
                if (pthread_cond_timedwait(&event_ptr->cond, &event_ptr->lock, &to) == ETIMEDOUT) {
                    break;
                }
            }
            pthread_mutex_unlock(&event_ptr->lock);
        } else {
            pthread_mutex_lock(&event_ptr->lock);
            //infinite
            while (event_ptr->ready == 0) {
                pthread_cond_wait(&event_ptr->cond, &event_ptr->lock);
            }
            pthread_mutex_unlock(&event_ptr->lock);
        }

        if (event_ptr->ready == 1) {
            return 0;
        } else {
            return 1; //timeout
        }
    }

    return 0;
}

void WAKE_EVENT(EVENT *event_ptr, void *result_ptr)
{
    if (event_ptr) {
        pthread_mutex_lock(&event_ptr->lock);
        event_ptr->ready = 1;
        event_ptr->result_ptr = result_ptr;
        pthread_cond_signal(&event_ptr->cond);
        pthread_mutex_unlock(&event_ptr->lock);
    }
}

MUTEX *CREATE_MUTEX()
{
    MUTEX *mutex_ptr = (MUTEX *)ALLOC(sizeof(MUTEX));

    if (mutex_ptr) {
        pthread_mutex_init(&mutex_ptr->hMutex, NULL);
    }

    return mutex_ptr;
}

void DELETE_MUTEX(MUTEX *mutex_ptr)
{
    if (mutex_ptr) {
        FREE(mutex_ptr);
    }
}

void LOCK_MUTEX(MUTEX *mutex_ptr)
{
    if (mutex_ptr) {
        pthread_mutex_lock(&mutex_ptr->hMutex);
    }
}

void UNLOCK_MUTEX(MUTEX *mutex_ptr)
{
    if (mutex_ptr) {
        pthread_mutex_unlock(&mutex_ptr->hMutex);
    }
}

uint64_t GETTID()
{
    return (uint64_t)pthread_self();
}

static void WRITEFILE(int log_file, void *buf_ptr, const uint32_t buf_len)
{
    uint16_t total_write_bytes;
    int32_t write_bytes;

    total_write_bytes = 0;
    while (write_bytes = write(log_file, buf_ptr + total_write_bytes, buf_len - total_write_bytes)) {
        if (write_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        total_write_bytes += write_bytes;
        if (total_write_bytes == buf_len) {
            break;
        }
    }
}

static void WRITETIME()
{
    if(debug_fp >= 0){
        struct timeval tv;
        struct timezone tz;
        uint32_t t;
        uint32_t t_ms;        
        gettimeofday(&tv, &tz);
        t = tv.tv_sec & 0xFFFFFFFF;
        t_ms = tv.tv_usec & 0xFFFFFFFF;
        WRITEFILE(debug_fp, &t, sizeof(t)); // time(s)
        WRITEFILE(debug_fp, &t_ms, sizeof(t_ms)); // time(ms)
    }    
}

void OPENCAP(const char *process_name_ptr)
{
    if(debug_fp < 0){
        char tmp_file_name[128] = {0};

#ifdef GIGAUT
        char *p = 0;

        p = strstr(process_name_ptr, "runtime/");
        if (p) {
            char tmp_path2[128] = {0};
            strncpy(tmp_path2, process_name_ptr, p-process_name_ptr);
            p = p+strlen("runtime/");
            snprintf(tmp_file_name, sizeof(tmp_file_name) - 1, "%slog/%s.cap", tmp_path2, p);
        } else {
            snprintf(tmp_file_name, sizeof(tmp_file_name) - 1, "%s.cap", process_name_ptr);
        }
#else
        char dt[64] = {0};
        time_t t;
        struct tm tt;
        uint32_t t_ms;
        memset(client_name, 0, sizeof(client_name));
        snprintf(client_name, sizeof(client_name) - 1, "%s", process_name_ptr);
        t=time(NULL);
        if (t == (time_t)-1){
            // time(t) fail
            snprintf(tmp_file_name, sizeof(tmp_file_name) - 1, "%s.cap", process_name_ptr);
        } else {
            localtime_r(&t, &tt);
            if (strftime(dt,sizeof(dt),"%Y%m%d%H%M%S",&tt) > 0){
                snprintf(tmp_file_name, sizeof(tmp_file_name) - 1, "%s_%s.cap", process_name_ptr, dt);
            } else {
                snprintf(tmp_file_name, sizeof(tmp_file_name) - 1, "%s.cap", process_name_ptr);
            }
            
        }
#endif
        debug_fp = open(tmp_file_name, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXO);
        dump_mutex_ptr = CREATE_MUTEX();
        if(debug_fp >= 0){
            uint8_t cap_hdr[24] = {0xD4,0xC3,0xB2,0XA1,0x02,0x00,0x04,0x00,
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                0xFF,0xFF,0x00,0X00,0x00,0x00,0x00,0x00};
            WRITEFILE(debug_fp, cap_hdr, sizeof(cap_hdr));
            DbgMsg("Host Version: 0");
        }
    }
}

void WRITECAP(uint32_t write_len, uint8_t *data, uint32_t msg_direction)
{
    LOCK_MUTEX(dump_mutex_ptr);
    if(debug_fp >= 0){
        uint32_t port = 0;
        WRITETIME();
        write_len = write_len + 36; // index length(4) + msg_direction(4) + port(4) + client_name(24)
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &msg_count, sizeof(msg_count));
        WRITEFILE(debug_fp, &port, sizeof(port));
        WRITEFILE(debug_fp, &client_name[0], sizeof(client_name));
        WRITEFILE(debug_fp, &msg_direction, sizeof(msg_direction));
        write_len = write_len - 36;
        WRITEFILE(debug_fp, data, write_len);
        msg_count = msg_count + 1;
    }
    UNLOCK_MUTEX(dump_mutex_ptr);
}

void WRITEERROR(char *error_msg)
{
    LOCK_MUTEX(dump_mutex_ptr);
    if(debug_fp >= 0){
        uint8_t error_magic[4] = {0x00, 0x02, 0x13, 0x07};
        uint32_t write_len = strlen(error_msg)+1;
        uint32_t msg_direction = 2;
        uint32_t port = 0;
        WRITETIME();
        write_len = write_len + 40; // index length(4) + msg_direction(4) + debug magic(4) + port(4) + client_name(24)
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &msg_count, sizeof(msg_count)); // write msg count
        WRITEFILE(debug_fp, &port, sizeof(port));
        WRITEFILE(debug_fp, &client_name[0], sizeof(client_name));
        WRITEFILE(debug_fp, &msg_direction, sizeof(msg_direction)); // write msg direction (host->host)
        WRITEFILE(debug_fp, error_magic, sizeof(error_magic));
        WRITEFILE(debug_fp, error_msg, strlen(error_msg)+1);
        msg_count = msg_count + 1;
    }
    UNLOCK_MUTEX(dump_mutex_ptr);
}

void WRITEDEBUG(char *debug_msg)
{
    LOCK_MUTEX(dump_mutex_ptr);
    if(debug_fp >= 0){
        uint8_t debug_magic[4] = {0x01, 0x02, 0x13, 0x07};
        uint32_t write_len = strlen(debug_msg)+1;
        uint32_t msg_direction = 2;
        uint32_t port = 0;
        WRITETIME();
        write_len = write_len + 40; // index length(4) + msg_direction(4) + debug magic(4) + port(4) + client_name(24)
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &write_len, sizeof(write_len)); // write length
        WRITEFILE(debug_fp, &msg_count, sizeof(msg_count)); // write msg count
        WRITEFILE(debug_fp, &port, sizeof(port));
        WRITEFILE(debug_fp, &client_name[0], sizeof(client_name));
        WRITEFILE(debug_fp, &msg_direction, sizeof(msg_direction)); // write msg direction (host->host)
        WRITEFILE(debug_fp, debug_magic, sizeof(debug_magic));
        WRITEFILE(debug_fp, debug_msg, strlen(debug_msg)+1);
        msg_count = msg_count + 1;
    }
    UNLOCK_MUTEX(dump_mutex_ptr);
}