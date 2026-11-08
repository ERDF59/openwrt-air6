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
 * MediaTek Inc. (C) 2020. All rights reserved.
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

#include "atci_gpio_cmd.h"
#include "atci_service.h"
#include "atci_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#define _STR(s) #s
#define STR(s) _STR(s)
#define _JOIN(a,b) a ## b
#define JOIN(a,b) _JOIN(a,b)

#define NUM_mt2737 2737
#define NUM_mt2735 2735
#define CHIP_ID JOIN(NUM_,PLATFORM)

#if defined (TELEMATICS)
#if (CHIP_ID == NUM_mt2735)
#define GPIO_SYS "/sys/devices/platform/10005000.pinctrl/mt_gpio"
#elif (CHIP_ID == NUM_mt2737)
#define GPIO_SYS "/sys/devices/platform/10004000.pinctrl/mt_gpio"
#else
#error UNSUPPORTED
#endif  /* TARGET_PLATFORM */
#else
#define GPIO_SYS "/sys/devices/platform/10004000.pinctrl/mt_gpio"
#endif /* TELEMATICS */

// #pragma message STR(TELEMATICS)
// #pragma message STR(PLATFORM)
// #pragma message STR(GPIO_SYS)
// #error stop here

int gpio_count;

static bool __DEBUG__ = true;
#define MGP_LOG(fmt, args...)                           \
	do                                                  \
	{                                                   \
		if (__DEBUG__)                                  \
			printf("GPIO: %s: " fmt, __func__, ##args); \
	} while (0)

static int gpio_execute(char *file_path, char **args, int read_fd, int write_fd)
{
	/*
	 * Execute command with args. At the same time, put data which is  in bread_fd to STDIN
	 * and put STDOUT after the command to write_fd. Use fd from pipe(.) to easily handle data
	 *
	 * Some application:
	 * 	1. run simple commad
	 * 		$chmod 664 /full/path/to/file ==> gpio_execute(...., -1, -1)
	 * 	2. get command result to fd
	 * 		$echo "A"
	 * 		==> gpio_execute(...., -1, fd);
	 *  3. run command with STDIN data (E.g. redirect operation ">")
	 *  	echo 7 1 > /full/path/to/my_file
	 * 		==> gpio_execute(...., -1, fd_of_my_file);
	 *  4. pass result in STDIN of cmd1 to cmd2 ( E.g. pipe operation "|")
	 * 		echo -e "A1\nB2" | grep "A"
	 * 		==> //wd_fd1, rd_fd1 in in from the same pipe(.)
	 * 			gpio_execute(...., -1, wr_fd1);
	 * 			gpio_execute(...., rd_fd1, fd);
	 */
	pid_t pid = 0;
	int status = 0;
	int ret = 0;

	pid = fork();

	if (pid < 0)
	{
		return -1;
	}
	else if (pid > 0)
	{
		ret = waitpid(pid, &status, 0);
		if ((ret == -1) || (!(WIFEXITED(status))))
		{
			return -1;
		}
		return 0;
	}
	else
	{ /* pid == 0 */
		if (write_fd >= 0)
			dup2(write_fd, STDOUT_FILENO);
		if (read_fd >= 0)
			dup2(read_fd, STDIN_FILENO);
		if (execv(file_path, args) == -1)
		{
			/* Handle error */
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
}

int gpio_set_direction(int gpio_pin, enum GPIO_DIR dir)
{
	int ret = 0;
	int ret_pin = 0;
	int ret_val = 0;
	int mt_gpio_fd;

	char gpio_pin_str[] = "gpio_pin";
	char dir_str[] = "input";
	char *args[5] = {"/bin/echo", "dir", NULL, NULL, NULL};

	if (dir == GPIO_DIR_OUT)
	{
		// Set out direction
		ret_pin = snprintf(gpio_pin_str, sizeof(gpio_pin_str),
						   "%d", gpio_pin);
		if (ret_pin < 0)
		{
			ret = -1;
			goto _err;
		}

		ret_val = snprintf(dir_str, sizeof(dir_str),
						   "%d", 1);
		if (ret_val < 0)
		{
			ret = -1;
			goto _err;
		}
	}
	else if (dir == GPIO_DIR_IN)
	{
		// Set in direction
		ret_pin = snprintf(gpio_pin_str, sizeof(gpio_pin_str),
						   "%d", gpio_pin);
		if (ret_pin < 0)
		{
			ret = -1;
			goto _err;
		}

		ret_val = snprintf(dir_str, sizeof(dir_str),
						   "%d", 0);
		if (ret_val < 0)
		{
			ret = -1;
			goto _err;
		}
	}
	else
	{
		MGP_LOG("error dir = %d\r\n", dir);
		ret = -1;
		goto _err;
	}

	args[2] = &(gpio_pin_str[0]);
	args[3] = &(dir_str[0]);

	mt_gpio_fd = open(GPIO_SYS,O_WRONLY);
	if(mt_gpio_fd < 0 )
		return -1;
	ret = gpio_execute(args[0], args, -1, mt_gpio_fd);
	close(mt_gpio_fd);

	MGP_LOG("exec[%s] %s,%s,%s,%s,%s (%d)(%d,%d)\r\n",
			args[0], args[0], args[1], args[2], args[3], args[4],
			ret, ret_pin, ret_val);
_err:
	return ret;
}

int gpio_set_value(int gpio_pin, enum GPIO_OUT level)
{
	int ret = 0;
	int ret_pin = 0;
	int ret_val = 0;
	char gpio_pin_str[] = "gpio_pin";
	char out_str[] = "high";
	char *args[7] = {"/bin/echo", "out", NULL, NULL, NULL};
	int mt_gpio_fd;

	ret = gpio_set_direction(gpio_pin, GPIO_DIR_OUT);
	if (ret)
		return -1;

	if (level == GPIO_OUT_ONE)
	{
		// Set GPIO high status
		ret_pin = snprintf(gpio_pin_str, sizeof(gpio_pin_str),
						   "%d", gpio_pin);
		if (ret_pin < 0)
		{
			ret = -1;
			goto _err;
		}

		ret_val = snprintf(out_str, sizeof(out_str),
						   "%d", 1);
		if (ret_val < 0)
		{
			ret = -1;
			goto _err;
		}
	}
	else
	{
		// Set GPIO low status
		ret_pin = snprintf(gpio_pin_str, sizeof(gpio_pin_str),
						   "%d", gpio_pin);
		if (ret_pin < 0)
		{
			ret = -1;
			goto _err;
		}

		ret_val = snprintf(out_str, sizeof(out_str),
						   "%d", 0);
		if (ret_val < 0)
		{
			ret = -1;
			goto _err;
		}
	}
	args[2] = &(gpio_pin_str[0]);
	args[3] = &(out_str[0]);


	mt_gpio_fd = open(GPIO_SYS,O_WRONLY);
	if (mt_gpio_fd < 0)
		return -1;
	ret = gpio_execute(args[0], args, -1, mt_gpio_fd);
	close(mt_gpio_fd);

	MGP_LOG("exec [%s]%s,%s,%s,%s,%s,%s,%s (%d)(%d,%d)\r\n",
			args[0], args[0], args[1], args[2], args[3], args[4], args[5], args[6],
			ret, ret_pin, ret_val);

	if (ret)
		return -1;
	else
		return 0;
_err:
	return ret;
}

int gpio_get_value(int gpio_pin, enum GPIO_OUT *level)
{
	int ret = 0;
	int mt_gpio_fd;
	int p_fds_1[2];
	int p_fds_2[2];
	char *echo_arg[] = {"/bin/echo", "start", NULL, NULL};
	char *cat_arg[] = {"/bin/cat", GPIO_SYS, NULL};
	char *grep_arg[] = {"/bin/grep", NULL, NULL};
	char pin_string[5] = {0};
	char pin_prefix[5] = {0};
	char res[512] = {0};

	/* change dir as intput */
	ret = gpio_set_direction(gpio_pin, GPIO_DIR_IN);
	if (ret)
		return -1;

	/* wait 150 ms to stable*/
	usleep(150 * 1000);

	/* set to-be cat range*/
	ret = snprintf(pin_string, sizeof(pin_string),
					"%03d", gpio_pin);
	if (ret < 0 || ret > sizeof(pin_string))
	{
		MGP_LOG("copy cmd fail\r\n");
		goto _err;
	}
	echo_arg[2] = &(pin_string[0]);

	mt_gpio_fd = open(GPIO_SYS, O_WRONLY);
	if (mt_gpio_fd < 0)
		return -1;
	if (gpio_execute(echo_arg[0], echo_arg, -1, mt_gpio_fd))
	{
		MGP_LOG("set range fail\r\n");
		close(mt_gpio_fd);
		goto _err;
	}
	close(mt_gpio_fd);

	/* read gpio_pin state*/
	ret = snprintf(pin_prefix, sizeof(pin_prefix),
					"%03d:", gpio_pin);
	if (ret < 0 || ret > sizeof(pin_prefix))
	{
		MGP_LOG("copy cmd fail\r\n");
		goto _err;
	}
	grep_arg[1] = &(pin_prefix[0]);

	if(pipe(p_fds_1))
		goto _err;
	if(pipe(p_fds_2))
	{
		close(p_fds_1[0]);
		close(p_fds_1[1]);
		goto _err;
	}

	if (!gpio_execute(cat_arg[0], cat_arg, -1, p_fds_1[1])) {
			close(p_fds_1[1]);
	} else {
		MGP_LOG("cat cmd fail\r\n");
		close(p_fds_1[0]);
		close(p_fds_1[1]);
		close(p_fds_2[0]);
		close(p_fds_2[1]);
		goto _err;
	}

	if (!gpio_execute(grep_arg[0], grep_arg, p_fds_1[0], p_fds_2[1])) {
			close(p_fds_1[0]);
			close(p_fds_2[1]);
	} else {
		MGP_LOG("grep cmd fail\r\n");
		close(p_fds_1[0]);
		close(p_fds_2[0]);
		close(p_fds_2[1]);
		goto _err;
	}

	ret = read(p_fds_2[0], res, sizeof(res) - 1);
	res[sizeof(res) - 1] = '\0';
	if (ret < 0) {
		MGP_LOG("read pipe fail\r\n");
		close(p_fds_2[0]);
		goto _err;
	}
	close(p_fds_2[0]);


	MGP_LOG("res %s, [3] = %c, [7] = %c\r\n", res, res[3], res[7]);

	/* String format
	 * PIN:(MODE)(DIR)(DOUT)(DIN)(DRIVE)(SMT)(IES)(PULL_EN)(PULL_SEL)(R1 R0)
	 * 000:0000001110
	 */
	if (res[7] == '1')
	{
		// Current GPIO status high
		*level = GPIO_OUT_ONE;
	}
	else if (res[7] == '0')
	{
		// Current GPIO status low
		*level = GPIO_OUT_ZERO;
	}
	else
	{
		goto _err;
	}

	return 0;

_err:
	return -1;
}

bool GPIO_Init(void)
{
	int ret = 0;
	int p_fds_1[2];
	int p_fds_2[2];
	int p_fds_3[2];
	char res[512];
	char *p_end = &(res[0]);
	char *chmod_args[4] = {"/bin/chmod", "644", GPIO_SYS, NULL};
	char *cat_arg[] = {"/bin/cat", GPIO_SYS, NULL};
	char *grep_arg[] = {"/bin/grep", "count", NULL};
	char *awk_arg[] = {"/usr/bin/awk", "{print $6}", NULL};

	ret = gpio_execute(chmod_args[0], chmod_args, -1, -1);

	MGP_LOG("exec %s %s %s %s (%d)\r\n",
			chmod_args[0], chmod_args[1], chmod_args[2], chmod_args[3],
			ret);

	if (ret)
		goto _err;

	if(pipe(p_fds_1))
		goto _err;
	if(pipe(p_fds_2))
	{
		close(p_fds_1[1]);
		close(p_fds_1[0]);
		goto _err;
	}
	if(pipe(p_fds_3))
	{
		close(p_fds_1[1]);
		close(p_fds_1[0]);
		close(p_fds_2[1]);
		close(p_fds_2[0]);
		goto _err;
	}

	ret = gpio_execute(cat_arg[0], cat_arg, -1, p_fds_1[1]);
	if (ret)
	{
		MGP_LOG("cat cmd fail\r\n");
		goto _err_1;
	}
	close(p_fds_1[1]);

	ret = gpio_execute(grep_arg[0], grep_arg, p_fds_1[0], p_fds_2[1]);
	if (ret) {
		MGP_LOG("grep cmd fail\r\n");
		goto _err_2;
	}
	close(p_fds_1[0]);
	close(p_fds_2[1]);

	ret = gpio_execute(awk_arg[0], awk_arg, p_fds_2[0], p_fds_3[1]);
	if (ret)
	{
		MGP_LOG("cat cmd fail\r\n");
		goto _err_3;
	}
	close(p_fds_2[0]);
	close(p_fds_3[1]);

	memset(res, 0, 512);
	ret = read(p_fds_3[0], res, sizeof(res) - 1);
	res[sizeof(res) - 1] = '\0';
	if (ret < 0) {
		MGP_LOG("read pipe fail\r\n");
		goto _err_4;
	}
	close(p_fds_3[0]);

	errno = 0;
	gpio_count = strtol(res, &p_end, 10);
	if ((gpio_count == LONG_MIN) || (gpio_count == LONG_MAX) ||
		(errno != 0) ||
		(res == p_end))
	{
		ret = -1;
		goto _err;
	}

	MGP_LOG("res %s, gpio_count = %d\r\n", res, gpio_count);

	return true;
_err_1:
	/* close fds*/
	if (!close(p_fds_1[1])) {
		MGP_LOG("Warning: close p_fds_1[1] fail\r\n");
	}
_err_2:
	if (!close(p_fds_1[0])) {
		MGP_LOG("Warning: close p_fds_1[0] fail\r\n");
	}
	if (!close(p_fds_2[1])) {
		MGP_LOG("Warning: close p_fds_2[0] fail\r\n");
	}
_err_3:
	if (!close(p_fds_2[0])) {
		MGP_LOG("Warning: close p_fds_2[1] fail\r\n");
	}
	if (!close(p_fds_3[1])) {
		MGP_LOG("Warning: close p_fds_3[0] fail\r\n");
	}
_err_4:
	if (!close(p_fds_3[0])) {
		MGP_LOG("Warning close p_fds_3[1] fail\r\n");
	}
_err:
	return false;
}

bool GPIO_Deinit(void)
{
	int ret = 0;
	char *args[4] = {"/bin/chmod", "644", GPIO_SYS, NULL};

	ret = gpio_execute(args[0], args, -1, -1);

	MGP_LOG("exec %s %s %s %s (%d)\r\n",
			args[0], args[1], args[2], args[3],
			ret);

	if (ret)
		return false;
	else
		return true;
}

/*****************************************************************************/
#define TAG "[GPIO_TEST] "

int gpio_cmd_handler(char *cmdline, ATOP_t at_op, char *response)
{
	int ret = 0;
	int gpio_no = -1, op_code = -1;
	int val = 0;
	int response_cnt = 0;
	char *p_end;
	char *args;

	ALOGD(TAG "gpio cmdline=%s, at_op=%d \r\n", cmdline, at_op);

	if (!GPIO_Init())
	{
		ret = -1;
		goto out;
	}

	switch (at_op)
	{
	case AT_SET_OP:
		args = strtok(cmdline, " ,");
		if (args == NULL)
		{
			ret = -1;
			goto out;
		}
		else
		{
			errno = 0;
			gpio_no = strtol(args, &p_end, 10);
			if ((gpio_no == LONG_MIN) || (gpio_no == LONG_MAX) ||
				(errno != 0) ||
				(args == p_end))
			{
				ALOGD(TAG "input count error"
						  "on gpio_no\r\n");
				ret = -1;
				goto out;
			}
		}

		args = strtok(NULL, " ,");
		if (args == NULL)
		{
			ret = -1;
			goto out;
		}
		else
		{
			errno = 0;
			op_code = strtol(args, &p_end, 10);
			if ((op_code == LONG_MIN) || (op_code == LONG_MAX) ||
				(errno != 0) ||
				(args == p_end))
			{
				ALOGD(TAG "input count error"
						  "on op_code\r\n");
				ret = -1;
				goto out;
			}
		}

		ALOGD(TAG "gpio_no=%d, op_code=%d, ret=%d\r\n",
			  gpio_no, op_code, ret);

		/* get both gpio_no and op_code*/

		if (gpio_no >= gpio_count)
		{
			ALOGD(TAG "set gpio_no=%d larger than total count %d\r\n",
				  gpio_no, gpio_count);
			ret = -1;
			goto out;
		}

		switch (op_code)
		{
		case SET_GPIO_OUTPUT_LOW:
			ALOGD(TAG "set gpio_no=%d low\r\n", gpio_no);
			ret = gpio_set_value(gpio_no, GPIO_OUT_ZERO);
			if (!ret)
			{
				response_cnt =
					snprintf(
						response + response_cnt,
						1024,
						"OK\r\n");
				if (response_cnt < 0)
					goto out;
			}
			break;
		case SET_GPIO_OUTPUT_HIGH:
			ALOGD(TAG "set gpio_no=%d high\r\n", gpio_no);
			ret = gpio_set_value(gpio_no, GPIO_OUT_ONE);
			if (!ret)
			{
				response_cnt =
					snprintf(
						response + response_cnt,
						1024,
						"OK\r\n");
				if (response_cnt < 0)
					goto out;
			}
			break;
		case GET_GPIO_LEVEL:
			ALOGD(TAG "get gpio_no=%d level\r\n", gpio_no);
			ret = gpio_get_value(gpio_no, &val);
			if (!ret)
			{
				response_cnt =
					snprintf(
						response + response_cnt,
						1024,
						"+TGPIO="
						"%d,%d\r\nOK\r\n",
						gpio_no, val);
				ALOGD(TAG
					  "get gpio_no=%d"
					  "val=%d\r\n",
					  gpio_no, val);
				if (response_cnt < 0)
					goto out;
			}
			break;
		default:
			ret = -1;
			break;
		}
		break;
	default:
		ALOGD(TAG "not supported at_op=%d\r\n", at_op);
		ret = -1;
		break;
	}

out:
	GPIO_Deinit();

	if (ret)
	{
		ALOGD(TAG "%s, gpio_no=%d, op=%d, ret=%d failed\r\n",
			  cmdline, gpio_no, op_code, ret);

		response_cnt =
			snprintf(
				response + response_cnt,
				1024,
				"Error\r\n");
		if (response_cnt < 0)
			return ret;
	}

	return ret;
}
