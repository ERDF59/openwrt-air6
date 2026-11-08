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

#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <linux/input.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "atci_generic_cmd_table.h"
#include "atci_kpd_cmd.h"
#include "atci_service.h"
#include "atci_util.h"

// input event
#define MAX_INPUT_DEVICES 	(8)
#define MAX_EVENT_QUEUE		(256)

/*
 Usage : AT+KEY=timeout
 Author : sr.huang@mediatek.com
*/

// key event input queue
static pthread_mutex_t event_queue_mutex;
static pthread_cond_t  event_queue_cond;
static int key_pendding;
static int key_timeout;

struct key {
	int pressed;
	int code;
	char name[128];
};

static struct key test_keys[] = {
//	{0, 116, "PWRKEY"},
	{0, 112, "Factory Mode"},
	{0, 114, "Wif"},
	{0, 115, "WPS"},
};
static struct pollfd ev_fds[MAX_INPUT_DEVICES];
static int event_queue[MAX_EVENT_QUEUE];
static unsigned fd_count;

/*
 Get a input event by using poll() and read(),
 The event will be save at the *ev.
*/
int ev_get(struct input_event *ev)
{
	int r;
	unsigned n;

	r = poll(ev_fds, fd_count, 1000);

	if ( r > 0) {
		ALOGD("[KEYTEST] poll: number of fds: (%d)\n", r);
		//read each fds
		for (n = 0; n <fd_count; n++) {
			if (ev_fds[n].revents & POLLIN) {
				r = read(ev_fds[n].fd, ev, sizeof(*ev));
				ALOGD("[KEYTEST] ev_get: read (%d) bytes\n", r);
				return 0;
			}
		}
	}

	return 0;
}

/*
 Receive the input events and update the
*/
static void *input_thread(void *data)
{
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));

	while(1) {

		do { //until received a valid key (EV_KEY)
			ev_get(&ev);

			if (ev.type == EV_SYN)
				continue;
		} while(ev.type != EV_KEY || ev.code > KEY_MAX);
		pthread_mutex_lock(&event_queue_mutex);
		event_queue[key_pendding++] = ev.code;
		pthread_cond_signal(&event_queue_cond);
		pthread_mutex_unlock(&event_queue_mutex);
	}

	return NULL;
}

/*
 Main thread for waiting input events
*/
int wait_key()
{
	struct timespec outtime;
	struct timeval now;
	int key;
	int err;

	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + key_timeout;
	outtime.tv_nsec = 0;

	ALOGD("[KEYTEST] Starting waiting keys...\n");

	err = pthread_mutex_lock(&event_queue_mutex);
	if (err != 0) {
		ALOGE("[KEYTEST] pthread_mutex_lock() failed, reason=[%s]%d", strerror(err), err);
	}
	//No need waiting when having pendding event
	while (0 == key_pendding){
		err = pthread_cond_timedwait(&event_queue_cond, &event_queue_mutex, &outtime);
		if (err == ETIMEDOUT) {
			ALOGD("[KEYTEST] Timeout for (%d) seconds\n", key_timeout);
			return -1;
		}
	}//while

	//handle the pedding event
	key = event_queue[0];
	memmove(&event_queue[0], &event_queue[1], sizeof(int) * --key_pendding); //shift left one space
	pthread_mutex_unlock(&event_queue_mutex);

	return key;
}

/*
 Open /dev/input/eventX and add into ev_fds[], update fd_count.
*/
int ev_init(void)
{
	char path[128]= {0};
	char name[128] = {0};
	int fd;
	int i, n;

	for (i = 0; i < MAX_INPUT_DEVICES; i++) {
		n = snprintf(path, sizeof(path), "/dev/input/event%d", i);
		if (n < 0)
			ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		if ((fd = open(path, O_RDONLY, 0)) > 0) {
			n = ioctl(fd, EVIOCGNAME(sizeof(name)), name);
			if (n < 0) {
				ALOGD("[KEYTEST] %s open failed\n", name);
				close(fd);
				continue;
			}
			if (memcmp(name, "gpio-keys", 9) == 0 ||
				memcmp(name, "mtk-kpd", 7) == 0  ||
				memcmp(name, "mtk-pmic-keys", 13) == 0) {
				ALOGD("[KEYTEST] ev_init: open fd = %d, name=%s\n", fd, name);
				ev_fds[fd_count].fd = fd;
				ev_fds[fd_count].events = POLLIN;
				fd_count++;
			}else{
				close(fd);
			}
		} else {
			ALOGD("[KEYTEST] open device %s failed\n", path);
		}
	}
	return 0;
}

void kpd_init(void)
{
	int ret;
	int test_keys_size = sizeof(test_keys)/sizeof(test_keys[0]);

	ALOGD("[KEYTEST] Initialization\n");

	//reset static variables
	key_pendding = 0;
	fd_count = 0;

	//reset pressed data structure
	for(int i=0; i<test_keys_size; i++)
	{
		test_keys[i].pressed = 0;
	}

	//reset event queue
	for(int i=0; i<MAX_EVENT_QUEUE; i++)
	{
		event_queue[i]= -1;
	}

	//mutex and cond
	ret = pthread_mutex_init(&event_queue_mutex, 0);
	if (ret != 0)
		ALOGD("[KEYTEST] pthread_mutex_init() failed, reason=[%s]%d", strerror(ret), errno);
	ret = pthread_cond_init(&event_queue_cond, 0);
	if (ret != 0) {
		ALOGD("[KEYTEST] event_queue_cond create fail!!!\n");
	}

	ev_init();
}

void kpd_deinit(void)
{
	int ret;

	ALOGE("[KEYTEST] kpd_deinit()\n");
	//mutex and cond
	ret = pthread_mutex_destroy(&event_queue_mutex);
	if (ret)
		ALOGE("[KEYTEST] Failed to destory a mutex lock: %s\n", strerror(ret));
	ret = pthread_cond_destroy(&event_queue_cond);
	if (ret != 0)
		ALOGE("[KEYTEST] pthread_cond_destroy failed in kpd_deinit\n");

	//close fd
	for (int i = 0; i <fd_count; i++) {
		close(ev_fds[i].fd);
	}
}

int keypad_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	time_t begin, current;
	int key;
	int test_keys_size = sizeof(test_keys)/sizeof(test_keys[0]);
	int untest = test_keys_size;
	int i, n;

	ALOGD("\r\n\n[KEYTEST] at_op=%d\n", at_op);
	pthread_t t;
	n = pthread_create(&t, NULL, input_thread, NULL);
	if (n != 0) {
		ALOGE("[KEYTEST] Failed to create input_thread. errno:%d\n", errno);
	}

  switch(at_op){
  	case AT_ACTION_OP:
  	case AT_READ_OP:
    case AT_TEST_OP:
    	ALOGD("[KEYTEST] AT OP(%d) not support\n", at_op);
    	n = snprintf(response, MAX_AT_RESPONSE, "\r\nERROR\r\n");
    	if (n < 0)
    		ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
      break;

   case AT_SET_OP:
	if (at_tok_nextint(&cmdline, &key_timeout) == -1) {
		ALOGD("[KEYTEST] Invalid commnad, format AT+KEY=timeout\n");
		n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
		if (n < 0)
			ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		break;
        }else{
		ALOGD("[KEYTEST] Timeout = %d\n", key_timeout);
        }

   	   ALOGD("[KEYTEST] Start Keypad Test\n");
   	   ALOGD("[KEYTEST] number of keys : %d\n", test_keys_size);
       kpd_init();

       begin = time(NULL);
       do {
       		current = time(NULL);
       		if(difftime(current, begin) > key_timeout)
       			break;
       		key = wait_key();
       		if(-1==key){
       			break;
			}
       		ALOGD("[KEYTEST] Test KEY = %d\n", key);

       		for (i = 0; i < test_keys_size; i++) {
				if (test_keys[i].code == key &&
					!test_keys[i].pressed){
					test_keys[i].pressed=1;
					untest--;
				}
			}

	       	//check test done
	       	if(0==untest)
	       		break;

       	}while(1);

       	//check result
       	if (0==untest){
       		ALOGD("[KEYTEST] Test all (%d) keys pressed pass!\n", test_keys_size);
       		n = snprintf(response, MAX_AT_RESPONSE, "\r\nOK\r\n");
       		if (n < 0)
       			ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
       	}else{
       		ALOGD("[KEYTEST] There are (%d) keys fail\n", untest);
       		for (i = 0; i < test_keys_size; i++) {
				if (!test_keys[i].pressed){
					ALOGD("[KEYTEST] Key code (%d) not pressed!\n", test_keys[i].code);
				}
			}
       		n = snprintf(response, MAX_AT_RESPONSE, "\r\nERROR\r\n");
       		if (n < 0)
       			ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
       	}

       break;
    default:
    	ALOGD("[KEYTEST] Can't recognize AT OP(%d)\n", at_op);
    	n = snprintf(response, MAX_AT_RESPONSE, "\r\nERROR\r\n");
    	if (n < 0)
    		ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
        break;
  }

  n = pthread_cancel(t);
  if (n)
    ALOGD("[KEYTEST] fail to cancel input_thread: %s\n", strerror(n));
  n = pthread_join(t, NULL);
  if (n)
    ALOGD("[KEYTEST] pthread_join fail, n = %d\n", n);
  kpd_deinit();

  ALOGD("[KEYTEST] End Keypad Test\n");
  return 0;
}

