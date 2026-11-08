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

#include "atcid.h"
#include "atcid_serial.h"

#include <netinet/in.h>
#include <sys/socket.h>

#ifdef CONFIG_QUECTEL_MAIN_UART
#include <uci.h>
#include "main_uart.h"
#endif

#include "atcid_util.h"
#include "ql_utils.h"
#include "ql_send_URC.h"

/*misc global vars */
Serial serial;

#if defined(TELEMATICS)
#define BOOTMODE_PATH "/proc/boot_mode"
#else
#define BOOTMODE_PATH "/proc/device-tree/chosen/atag,boot"  //for common kernel
#endif

#define UNKNOWN_BOOT -1
#define NORMAL_BOOT 0
#define META_BOOT 1

extern int adb_socket_listen(int socketFd);
extern int get_control_socket(char* name);
extern bool atci_mipc_inited;

#ifdef CONFIG_QUECTEL_MAIN_UART
int getUCISystemCfgValue(char *name,char *value, int valueBufLen)
{
	if (NULL == name || NULL == value) {
		return -1;
	}

	struct uci_context * uciCtx = uci_alloc_context();
	const char * pValueData = NULL;
	struct uci_package * pkg = NULL;
	struct uci_element * e;

	if (UCI_OK != uci_load(uciCtx, "/etc/config/system", &pkg)) {
		goto cleanup;
	}

	uci_foreach_element(&pkg->sections, e)
	{
		struct uci_section *s = uci_to_section(e);
		if (NULL != (pValueData = uci_lookup_option_string(uciCtx, s, name))) {
			strncpy(value, pValueData, valueBufLen);
		}
	}

	uci_unload(uciCtx, pkg);

cleanup:
	uci_free_context(uciCtx);
	uciCtx = NULL;
	return 1;
}

bool quectel_main_uart_get()
{
    bool ret = 0;
    //get quectel main uart status
    char configStr[5] = {0};
    memset(configStr, 0, sizeof(configStr));
    if (getUCISystemCfgValue("quectel_main_uart", configStr, sizeof(configStr))) {
        if (atoi(configStr) == 0) {
            ret = 0;
        }
        else{
            ret = 1;
        }
    }
    return ret;
}
#endif

/*
* Purpose:  The main program loop
* Return:    0
*/
int main() {
    int i = 0;
    int ret = 0;
    pthread_t urc_pid = 0;

    LOGATCI(LOG_INFO, "atcid-daemon start!!!");

    //Initial the parameter for serial dervice
    initSerialDevice(&serial);

    //Initial "atci" mipc_port
    if(check_modem_status())
    {
        mipc_init("atci");
        atci_mipc_inited = true;
    }

#ifdef MTK_DATACARD_ATCI
    //AP AT port is GS3 in factory mode
    snprintf(serial.devicename[0], strlen(TTY_GS3) + 1, "%s", TTY_GS3);
    int listenVCOM = 0; //don't listen AT port in normal mode
    int bootMode = readSys_int(BOOTMODE_PATH);

    if (bootMode == META_BOOT) {
        LOGATCI(LOG_INFO, "meta mode for data card, listen");
        listenVCOM = 1;
    }
#else
    //for CPE / IVT listen TTYGS0
    snprintf(serial.devicename[0], strlen(TTY_GS0) + 1, "%s", TTY_GS0);
    int listenVCOM = 1; //listen ETS port as default
    int bootMode = readSys_int(BOOTMODE_PATH);

    LOGATCI(LOG_INFO, "bootMode = %d", bootMode);

    if (bootMode == META_BOOT) {
        LOGATCI(LOG_INFO, "meta mode for CPE/ IVT, don't listen");
        listenVCOM = 0;
    }
#endif

#ifdef CONFIG_QUECTEL_MAIN_UART
    //for listen main uart
    if(quectel_main_uart_get() == true)
        snprintf(serial.devicename[MAIN_UART_POST], strlen(QL_MAIN_UART) + 1, "%s", QL_MAIN_UART);
#endif

    if (listenVCOM == 1) {
        for (i = 0; i < MAX_DEVICE_VCOM_NUM; i++) {
#ifdef CONFIG_QUECTEL_MAIN_UART
            //for listen main uart
            if(i==MAIN_UART_POST && quectel_main_uart_get() != true) {
                continue;
            }
#endif
            if ((serial.fd[i] = open_serial_device(&serial, serial.devicename[i])) ==
                    INVALIDE_SOCKET_FD) {
                LOGATCI(LOG_ERR, "Could not open serial device [%d] and start atci service", i);
                // return ATCID_OPEN_SERIAL_DEV_ERR;
            }
#ifdef CONFIG_QUECTEL_MAIN_UART
            else if(i == MAIN_UART_POST && quectel_main_uart_get() == true) {
                LOGATCI(LOG_ERR, "Set serial device [%d] speed", i);
                serial_set_speed(serial.fd[1], 115200);
            }
#endif
        }
    } else {
        LOGATCI(LOG_INFO, "current mode, don't open VCOM");
    }

    tcflush(serial.fd[1], TCIOFLUSH);
    serial.fd[ADB_SKT_SERVER_NUM] = get_control_socket(ADB_ATCI_SOCKET);

    if (serial.fd[ADB_SKT_SERVER_NUM] < 0) {
        LOGATCI(LOG_ERR, "Failed to get socket '%s'", ADB_ATCI_SOCKET);
    } else {
        if (ATCID_SUCCESS != adb_socket_listen(serial.fd[ADB_SKT_SERVER_NUM])) {
            LOGATCI(LOG_ERR, "Could not open adb socket [%d] and start atci service", i);
            // return ATCID_OPEN_SERIAL_DEV_ERR;
        }
    }

    serial.fd[NET_SERVER_NUM] = bind_network_sock(INADDR_LOOPBACK, NET_SERVER_PORT);
    // serial.fd[NET_SERVER_NUM] = bind_network_sock(INADDR_ANY, NET_SERVER_PORT);   //accept all ip address
    if (serial.fd[NET_SERVER_NUM] < 0) {
        LOGATCI(LOG_ERR, "Could not open Internet socket");
        // return ATCID_OPEN_SERIAL_DEV_ERR;
    }

    ql_update_svn();
    
    ret = pthread_create(&urc_pid, NULL, ql_GetReadyForURC, NULL);
	if(!ret)
	{
		LOGATCI(LOG_INFO,"Create ql_GetReadyForURC thread success!\n");
	}
    readerLoop((void*) &serial);

    return 0;
}
