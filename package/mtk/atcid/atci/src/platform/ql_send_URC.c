/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <uci.h>   //modify by quectel for DSDS
#include "mipc_msg_tlv_api.h"
#include "mipc_msg_host.h"
#include "mipc_msg_tlv_api.h"
#include "atcid.h"
#include "atcid_async_notify.h"
#include "ql_send_URC.h"

#define ATCI_SIM "persist.vendor.service.atci.sim"
#define BOOTMODE_PATH "/proc/device-tree/chosen/atag,boot"  //for common kernel
#define META_BOOT 1
#define MAX_LINE_LENGTH 512
#define RETRY_TIME 20

extern bool listen_URC;

// Colgin/Bollinger/RDKB
static int readSys_int(char const * path)
{
    int fd;

    if (path == NULL) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        int buffer[8] = {0};
        int len = read(fd, &buffer, sizeof(int)*8);
        if(len > 0)
        {
            LOGATCI(LOG_DEBUG,"boot mode size = %d, tag = %d, mode = %d\n", buffer[0], buffer[1], buffer[2]);
			close(fd);
            return buffer[2];
        }
        close(fd);
    }
    LOGATCI(LOG_ERR,"read boot mode failed to open %s\n", path);
    return -1;
}

void ql_inner_run_at_cmd(char* cmd, char *atci_res) {
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    char *atcicmd_req_ptr, *atcmd_res_ptr;
    uint16_t atcmd_req_len, atcmd_res_len;

    char simIDProperty[MTK_PROPERTY_VALUE_MAX]="0";
    int simID = 0;

    int char_sub = 0x1a;
    int char_esc = 0x1b;
    bool contain_sub_esc = false;
    char *pLine = cmd;

	if(!atci_res)
		return;

    LOGATCI(LOG_INFO, "AT command execution:%s", cmd);

    #ifdef QUECTEL_DSDS_ESUO    //modify by quectel for DSDS,start	
		//atci_property_get(ATCI_SIM, simIDProperty, "0");
		atci_property_get("atci_sim", simIDProperty, "0");
    	simID = atoi(simIDProperty);
    	if (simID == 0) {
        	//LOGATCI(LOG_INFO, "AT command execution on SIM1 (init msg...)");
        	msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_SIM0);
        	//LOGATCI(LOG_INFO, "AT command execution on SIM1    (init msg done)");
    	} else {
        	//LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg...)");
        	msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_SIM1);
        	//LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg done) ");
    	}	
	#else  //modify by quectel for DSDS,end
		atci_property_get(ATCI_SIM, simIDProperty, "0");
    	simID = atoi(simIDProperty);
    	if (simID == 0) {
        	//LOGATCI(LOG_INFO, "AT command execution on SIM1 (init msg...)");
        	msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_PS0);
        	//LOGATCI(LOG_INFO, "AT command execution on SIM1    (init msg done)");
    	} else {
        	//LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg...)");
        	msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_PS1);
        	//LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg done) ");
    	}
	#endif

    //LOGATCI(LOG_INFO, "AT command string format processing...");

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if(char_sub == currChar || char_esc == currChar) {
            contain_sub_esc = true;
        }
        pLine++;
    }

    //LOGATCI(LOG_INFO, "AT command string format processing... (2)");

    // add '\r' for all cmd
    // In AT spec, the string need including '\r' but not is the case that is the last sms message finished by Ctrl+Z or ESC
    // In MIPC spec said that the size passing need including '\0'
    if (!contain_sub_esc) {
        atcmd_req_len = strlen(cmd);
        atcicmd_req_ptr = (char *)malloc(atcmd_req_len + 2);  // 1 for '\r', 1 for '\0'

        if (atcicmd_req_ptr == NULL) {
            LOGATCI(LOG_INFO, "AT command execution : atcicmd_req_ptr alloc fail");
        } else {
            strncpy(atcicmd_req_ptr, cmd, atcmd_req_len);
            atcicmd_req_ptr[atcmd_req_len] = '\r';
            atcicmd_req_ptr[atcmd_req_len + 1] = '\0';
            atcmd_req_len +=2;

            LOGATCI(LOG_INFO, "AT command execution (with \\r\\0):%s", atcicmd_req_ptr);
        }
    } else {
        atcmd_req_len = strlen(cmd);
        atcicmd_req_ptr = (char *)malloc(atcmd_req_len + 1);  // 1 for '\0'

        if (atcicmd_req_ptr == NULL) {
            LOGATCI(LOG_INFO, "AT command execution : atcicmd_req_ptr alloc fail");
        } else {
            strncpy(atcicmd_req_ptr, cmd, atcmd_req_len);
            atcicmd_req_ptr[atcmd_req_len] = '\0';
            atcmd_req_len +=1;

            LOGATCI(LOG_INFO, "AT command execution (with \\r):%s", atcicmd_req_ptr);
        }
    }

    //LOGATCI(LOG_INFO, "AT command string format processing... (done)");

    //LOGATCI(LOG_INFO, "start mipc_msg_add_tlv");
    mipc_msg_add_tlv(msg_req_ptr, MIPC_SYS_AT_REQ_T_ATCMD, atcmd_req_len, atcicmd_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_msg_sync_timeout");
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_msg_deinit");
    mipc_msg_deinit(msg_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_get_result");
    result = mipc_get_result(msg_cnf_ptr);

    free(atcicmd_req_ptr);

    if (result == MIPC_RESULT_SUCCESS) {
        atcmd_res_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_SYS_AT_CNF_T_ATCMD, &atcmd_res_len);
        int res_len = strlen(atcmd_res_ptr);
		strncpy(atci_res, "\r\n", 3); //add \r\n
		strncpy(atci_res + 2, atcmd_res_ptr, res_len); //copy reponse string
		atci_res[res_len + 2] = '\0';  // add termination
    } else {
        LOGATCI(LOG_INFO, "Failed to execute:%d\n", result);
    }
    mipc_msg_deinit(msg_cnf_ptr);

    return ;
}

int extract_first_line_with_plus(const char *input) 
{
	char line[MAX_LINE_LENGTH] = {0};
	const char *ptr = input;
	size_t line_length = 0;

	line[line_length++]= '\r';
	line[line_length++]= '\n';

	while (*ptr) {
		if (*ptr == '\n' || line_length >= MAX_LINE_LENGTH - 1) {
			line[line_length] = *ptr;
			if (strchr(line, '+')) {
				delayed_async_print(line, strlen(line));
				return 1; // Found the first line with '+'
			}
			line_length = 0;
			memset(line, 0, sizeof(line));
			line[line_length++]= '\r';
			line[line_length++]= '\n';
		} else {
			line[line_length++] = *ptr;
		}
		ptr++;
	}

	// Check the last line if it doesn't end with '\n'
	if (line_length > 0) {
		line[line_length] = '\0';
		if (strchr(line, '+')) {
			delayed_async_print(line, strlen(line));
			return 1; // Found the first line with '+'
		}
	}

	return 0; // No line with '+' found
}

void *ql_GetReadyForURC(void *arg)
{
	//struct termios options;
	mipc_msg_t *msg_req_ptr = NULL;
	mipc_msg_t *msg_cnf_ptr = NULL;
	uint8_t sms_state_value = 0;
	unsigned int count = 0;
	char recv_buf[MAX_LINE_LENGTH] = {0};

	sleep(3);	//wait for atcid mipc ready
	if(!listen_URC)
	{
		startUrcThread();
	}

	/* Fix meta tool connect to AP failed. If it is meta mode, return directly, and do not open the AT port to report the URC.*/
    int bootMode = readSys_int(BOOTMODE_PATH);
    if (bootMode == META_BOOT) {
        LOGATCI(LOG_ERR,"meta mode");
        return NULL;
    }

	//start to send URC
	delayed_async_print("RDY\r\n",strlen("RDY\r\n"));
	//cfun
	memset(recv_buf, 0, sizeof(recv_buf));
	ql_inner_run_at_cmd("AT+CFUN?\r\n", recv_buf);
	if (!extract_first_line_with_plus(recv_buf))
		LOGATCI(LOG_ERR, "Extracted line failed!\n");

	//cpin
	memset(recv_buf, 0, sizeof(recv_buf));
	ql_inner_run_at_cmd("AT+CPIN?\r\n", recv_buf);
	if (!extract_first_line_with_plus(recv_buf))
		LOGATCI(LOG_ERR, "Extracted line failed!\n");

	//esimmap
	memset(recv_buf, 0, sizeof(recv_buf));
	ql_inner_run_at_cmd("AT+ESIMMAP?\r\n", recv_buf);
	if (!extract_first_line_with_plus(recv_buf))
		LOGATCI(LOG_ERR, "Extracted line failed!\n");

	LOGATCI(LOG_DEBUG, "Start Query sms status\n");
	count = 0;
RESEND_SMS:
	//sms status
	msg_req_ptr = mipc_msg_init(MIPC_SMS_CFG_REQ, MIPC_MSG_PS0);
	if(mipc_sms_cfg_req_add_get_sms_state(msg_req_ptr,sms_state_value) == MIPC_MSG_API_RESULT_SUCCESS)
	{
		msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);
		mipc_msg_deinit(msg_req_ptr);
		if(mipc_msg_get_val_uint32(msg_cnf_ptr,MIPC_T_RESULT,MIPC_RESULT_FAILURE) == MIPC_RESULT_SUCCESS)
		{
			sms_state_value = mipc_sms_cfg_cnf_get_sms_state(msg_cnf_ptr,0);
			LOGATCI(LOG_DEBUG,"sms_state_value = %u\n",sms_state_value);
			if(sms_state_value == MIPC_SMS_STATE_INITIALIZED)
			{
				delayed_async_print("\r\n+QIND: SMS DONE\r\n",strlen("\r\n+QIND: SMS DONE\r\n"));
			}
			else
			{
				LOGATCI(LOG_DEBUG,"SMS NOT READY.count = %d\n",count);
				if(count < RETRY_TIME)
				{
					mipc_msg_deinit(msg_cnf_ptr);
					sleep(5);
					count++;
					goto RESEND_SMS;
				}
			}
		}
		else
		{
			LOGATCI(LOG_DEBUG,"SMS NOT READY.count = %d\n",count);
			if(count < RETRY_TIME)
			{
				mipc_msg_deinit(msg_cnf_ptr);
				sleep(5);
				count++;
				goto RESEND_SMS;
			}
		}
		mipc_msg_deinit(msg_cnf_ptr);
	}

	LOGATCI(LOG_DEBUG, "Start Query phone-book status\n");
	count = 0;
RESEND_PB:
	//phonebook status
	ql_inner_run_at_cmd("AT+CPBS?\r\n", recv_buf);
	if(strstr(recv_buf,"ERROR") == NULL)
	{
		//eind=2
		ql_inner_run_at_cmd("AT+EIND=2\r\n", recv_buf);
		if(strstr(recv_buf,"OK") != NULL)
		{
			delayed_async_print("\r\n+EIND: 2\r\n\r\n+QIND: PB DONE\r\n",strlen("\r\n+EIND: 2\r\n\r\n+QIND: PB DONE\r\n"));
		}
		else
		{
			LOGATCI(LOG_ERR,"set eind 2 error\n");
		}
	}
	else
	{
		LOGATCI(LOG_DEBUG,"PB NOT READY.\n");
		if(count < RETRY_TIME)
		{
			sleep(5);
			count++;
			goto RESEND_PB;
		}
	}

	return NULL;
}