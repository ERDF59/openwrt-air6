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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>

#include <uci.h>

#include "atcid_serial.h"
#include "atcid_util.h"
#include "atcid_cust_cmd.h"
#include "atcid_common.h"
#include "at_tok.h"

#define ATCI_SIM "persist.vendor.service.atci.sim"
#define PROPERTY_SYS_USB_STATE "sys.usb.state"
#define PROPERTY_VENDOR_USB_CONFIG "vendor.usb.config"
#define ENABLE_URC_PROP "persist.vendor.service.atci_urc.enable"
#define MULTI_SIM "persist.radio.multisim.config"

#define UCI_CONFIG_FILE "/etc/config/system"

int gCsdfMode = 1;
int gCsdfAuxmode = 1;
int gCclkTz = 0xFFFF;

/*misc global vars */
extern Serial serial;

ATRESPONSE_t pas_csdf_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    int mode = 0;
    int auxmode = 0;

    LOGATCI(LOG_DEBUG, "[pas_csdf_handler] cmdline %s", cmdline);

    switch(opType) {
        case AT_READ_OP:
            snprintf(response, MAX_AT_RESPONSE, "+CSDF: %d,%d\n",
                    gCsdfMode, gCsdfAuxmode);
            LOGATCI(LOG_DEBUG, "[pas_csdf_handler] response %s", response);

            return AT_OK;
            break;
        case AT_SET_OP:
            if (!at_tok_hasmore(&cmdline)) {
                return AT_ERROR;
            }

            err = at_tok_nextint(&cmdline, &mode);
            if (err < 0) {
                mode = 1;
            } else if (!(mode >= 1 && mode <= 7)) {
                mode = -1;
            }

            err = at_tok_nextint(&cmdline, &auxmode);
            if (err < 0) {
                auxmode = 1;
            } else if (!(auxmode >= 1 && auxmode <= 2)) {
                auxmode = -1;
            }

            if (mode != -1 && auxmode != -1) {
                gCsdfMode = mode;
                gCsdfAuxmode = auxmode;
                LOGATCI(LOG_DEBUG, "[pas_csdf_handler] After set: mode %d, auxmode %d",
                        gCsdfMode, gCsdfAuxmode);
            } else {
                return AT_ERROR;
            }

            return AT_OK;
            break;
        case AT_TEST_OP:
            snprintf(response, MAX_AT_RESPONSE, "+CSDF: (1,2,3,4,5,6,7),(1,2)\n");
            LOGATCI(LOG_DEBUG, "[pas_csdf_handler] response %s", response);

            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

static int get_cfg_value(char *name, char *value, int value_buf_len) {
    int ret = 0;
    struct uci_context *uci_ctx = NULL;
    const char *value_data = NULL;
    struct uci_package *pkg = NULL;
    struct uci_element *e = NULL;

    if (NULL == name || NULL == value)
        return 0;

    uci_ctx = uci_alloc_context();
    if (!uci_ctx)
        return 0;

    if (UCI_OK != uci_load(uci_ctx, UCI_CONFIG_FILE, &pkg)) {
        LOGATCI(LOG_ERR, "uci_load(%s) fail!\n", UCI_CONFIG_FILE );
        goto cleanup;
    }
    uci_foreach_element(&pkg->sections, e) {
        struct uci_section *s = uci_to_section( e );
        if (NULL != (value_data = uci_lookup_option_string(uci_ctx, s, name))) {
            strncpy(value, value_data, value_buf_len);
            ret = 1;
        }
    }
    uci_unload(uci_ctx, pkg);

cleanup:
    uci_free_context(uci_ctx);
    uci_ctx = NULL;

    return ret;
}

// ql add array length macro
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]) ) 
// ql end

ATRESPONSE_t pas_cclk_handler(char* cmdline, ATOP_t opType, char* response) {
    LOGATCI(LOG_DEBUG, "cmdline %s", cmdline);
    char config_str[16];
    switch (opType) {
        case AT_READ_OP: {
            time_t timep;
            struct tm *p;
            //struct timeval tv;
            struct timezone tz;
            int qTz = 0;

            time(&timep);
            //p=localtime(&timep);
            p = gmtime(&timep);
            //gettimeofday(&tv, &tz);
            memset(&tz,0,sizeof(struct timezone));
            gettimeofday(NULL, &tz);
            LOGATCI(LOG_DEBUG, "timezone: %d", tz.tz_minuteswest);
            if (gCclkTz == 0xFFFF) {
                qTz = (0 - tz.tz_minuteswest) / 60 * 4;
                if (qTz == 0) {
                    //tz in kernel was not updated, get it from uci
                    memset(config_str, 0, sizeof(config_str));
                    if (get_cfg_value("timezone", config_str, sizeof(config_str))) {
                        LOGATCI(LOG_DEBUG, "config_str: %s", config_str);
                        //the timezone string is like "<+08>-08"
                        qTz = atoi(config_str+1) * 4;
                        LOGATCI(LOG_DEBUG, "qTz: %d", qTz);
                    }
                }
            } else {
                qTz = gCclkTz;
            }

            if (gCsdfAuxmode == 1) {
                if (qTz >= 0) {
                    snprintf(response, MAX_AT_RESPONSE, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\n",
                            (1900+p->tm_year)%100, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                } else {
                    snprintf(response, MAX_AT_RESPONSE, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d%03d\"\n",
                            (1900+p->tm_year)%100, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                }
            } else if (gCsdfAuxmode == 2) {
                if (qTz >= 0) {
                    snprintf(response, MAX_AT_RESPONSE, "+CCLK: \"%04d/%02d/%02d,%02d:%02d:%02d+%02d\"\n",
                            1900+p->tm_year, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                } else {
                    snprintf(response, MAX_AT_RESPONSE, "+CCLK: \"%04d/%02d/%02d,%02d:%02d:%02d%03d\"\n",
                            1900+p->tm_year, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                }
            } else {
                return AT_ERROR;
            }
            return AT_OK;
            break;
        }
        case AT_SET_OP: {
            char * line;
            struct timeval tv = {0,0};
            time_t mktimeFromInput;
            unsigned int i = 0;
            struct tm setclk;
            char cclkSetCmd[40] = {0};
            bool hasInputTimeZone = false;
            int inputTimeZoneValue = 0;

            line = cclkSetCmd;
            //ql modify string copy to be bounded by buffer capacity
            for (i = 0; i < ARRAY_LENGTH(cclkSetCmd) - 1 && i < strlen(cmdline); i++) {
            // ql end
                if (cmdline[i] != ' ' && cmdline[i] != '\"' &&
                        cmdline[i] != '\''&& cmdline[i] != '\n') {
                    *(line++) = cmdline[i];
                }
            }
            *line = '\0';

            if (strlen(cclkSetCmd) == strlen("yy/MM/dd,hh:mm:ss+tz")) {
                hasInputTimeZone = true;
            } else if (strlen(cclkSetCmd) == strlen("yy/MM/dd,hh:mm:ss")) {
                hasInputTimeZone = false;
            } else {
                snprintf(response, MAX_AT_RESPONSE, "CME ERROR: Invalid parameters: [%d] cmdline:%s; [%d] cclkSetCmd:%s",
                        (int) strlen(cmdline), cmdline, (int) strlen(cclkSetCmd), cclkSetCmd);
                return AT_ERROR;
            }

            LOGATCI(LOG_DEBUG, "hasInputTimeZone %d", hasInputTimeZone);

            memset(&setclk, 0, sizeof(setclk));
            if (hasInputTimeZone) {
                sscanf(cclkSetCmd, "%2d/%2d/%2d,%2d:%2d:%2d%03d", &setclk.tm_year, &setclk.tm_mon,
                        &setclk.tm_mday, &setclk.tm_hour, &setclk.tm_min, &setclk.tm_sec, &inputTimeZoneValue);
                if (inputTimeZoneValue >= -96 && inputTimeZoneValue <= 96) {
                    gCclkTz = inputTimeZoneValue;
                } else {
                    return AT_ERROR;
                }
                LOGATCI(LOG_DEBUG, "Input time zone:%02d", inputTimeZoneValue);
            } else {
                sscanf(cclkSetCmd, "%2d/%2d/%2d,%2d:%2d:%2d", &setclk.tm_year, &setclk.tm_mon,
                        &setclk.tm_mday, &setclk.tm_hour, &setclk.tm_min, &setclk.tm_sec);
            }
            if (setclk.tm_year < 0 ||
                    setclk.tm_mon < 1 ||
                    setclk.tm_mon > 12 ||
                    setclk.tm_mday < 1 ||
                    setclk.tm_mday > 31 ||
                    setclk.tm_hour < 0 ||
                    setclk.tm_hour > 24 ||
                    setclk.tm_min < 0 ||
                    setclk.tm_min > 60 ||
                    setclk.tm_sec < 0 ||
                    setclk.tm_sec > 60) {
                LOGATCI(LOG_ERR, "Input format error");
                return AT_ERROR;
            }

            // convert tm format with input data
            setclk.tm_year += 2000; // make 2000 year
            setclk.tm_year -= 1900; // UTC -1900 year
            setclk.tm_mon  -= 1;    // from 0 to 11
            // Get UTC version tm
            char *envTz = getenv("TZ");
            char *mEnvTz = NULL;
            setenv("TZ", "UTC", 1);
            tzset();
            mktimeFromInput = mktime(&setclk);
            if (envTz) {
                mEnvTz = strdup(envTz);
                setenv("TZ", mEnvTz, 1);
                free(mEnvTz);
            } else {
                unsetenv("TZ");
            }
            tzset();
            if (mktimeFromInput < (time_t) 0) {
                LOGATCI(LOG_DEBUG, "mktime error: %d %s\n", errno, strerror(errno));
                return AT_ERROR;
            }
            /* time_t change to timeval */
            tv.tv_sec = mktimeFromInput;

            if (settimeofday(&tv, NULL) != 0) {
                LOGATCI(LOG_DEBUG, "Unable to set clock to %d.%d: %s\n",
                        (int)tv.tv_sec, (int)tv.tv_usec, strerror(errno));
                return AT_ERROR;
            }

            return AT_OK;
            break;
        }
        case AT_TEST_OP:
            snprintf(response, strlen("+CCLK: yy/MM/dd,hh:mm:ss[+/-tz]") + 1, "%s",
                    "+CCLK: yy/MM/dd,hh:mm:ss[+/-tz]");
            return AT_OK;
            break;
        default:

            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_echo_handler(char* cmdline, ATOP_t opType, char* response) {
    int len = 0, i = 0, flag = -1;

    LOGATCI(LOG_DEBUG, "cmdline %s", cmdline);

    len = strlen(cmdline);
    flag = serial.echo[serial.currDevice];;

    switch (opType) {
        case AT_BASIC_OP:
        case AT_SET_OP:
            for (i = 0; i < len; i ++) {
                if ( cmdline[i] == 'E' || cmdline[i] == 'e') {
                    if (i < len-1) {
                        flag = cmdline[i+1] - '0';
                        break;
                    }
                } else {
                    i++;
                }
            }

            if (flag == 0 || flag == 1) {
                setEchoOption(flag);
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            snprintf(response, MAX_AT_RESPONSE, "%d\r\n", flag);
            return AT_OK;
            break;
        case AT_TEST_OP:
            snprintf(response, MAX_AT_RESPONSE, "0:disable echo, 1:enable echo\r\n");
            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_eurc_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    LOGATCI(LOG_DEBUG, "pas_eurc_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);

            if (err < 0) return -1;

            if (1 == cmdID) {
                LOGATCI(LOG_DEBUG, "start to listen URC", opType);
                startUrcThread();
            } else if (0 == cmdID) {
                stopUrcThread();
            }
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

#ifdef QUECTEL_DSDS_ESUO   //modify by quectel for DSDS,start
ATRESPONSE_t pas_esuo_handler(char* cmdline, ATOP_t opType, char* response)
{
    int ret = 0;
    int err = 0, cmdID = 0;
    int atci_sim = 0;
    char simIDProperty[MTK_PROPERTY_VALUE_MAX] = {0};   
    
    LOGATCI(LOG_DEBUG, "pas_esuo_handler enter with opType: %d", opType);
    
    switch (opType) 
    {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;
            
            if(cmdID == 4) { //Configure the default SIM to SIM1
                atci_property_set("atci_sim", "0");
                LOGATCI(LOG_DEBUG, "Set default to SIM1");
                return AT_OK;
            } else if(cmdID == 5) {
                //Configure the default SIM to SIM2
                atci_property_set("atci_sim", "1");
                LOGATCI(LOG_DEBUG, "Set default to SIM2");
                return AT_OK;
            } else if(cmdID == 6) {
                //Configure the default SIM to SIM3
                atci_property_set("atci_sim", "2");
                LOGATCI(LOG_DEBUG, "Set default to SIM3");
                return AT_OK;                
            } else {
            LOGATCI(LOG_DEBUG, "Error: not supported SIM Id.");
            }
            break;
        case AT_TEST_OP:
            writeDataToSerial("+ESUO: (4-5)", strlen("+ESUO: (4-5)"));
            return AT_OK;
            break;
        case AT_READ_OP:
            atci_property_get("atci_sim", simIDProperty, "0");
            atci_sim = atoi(simIDProperty);
            if (atci_sim == 1) {
                snprintf(response, strlen("+ESUO:5\r\n") + 1, "%s", "+ESUO:5\r\n");
            } else if (atci_sim == 2) {
                snprintf(response, strlen("+ESUO:6\r\n") + 1, "%s", "+ESUO:6\r\n");
            } else if (atci_sim == 3) {
                snprintf(response, strlen("+ESUO:7\r\n") + 1, "%s", "+ESUO:7\r\n");
            } else if (atci_sim == 9) {
                snprintf(response, strlen("+ESUO:9\r\n") + 1, "%s", "+ESUO:9\r\n");
            } else if (atci_sim == 0) {
                snprintf(response, strlen("+ESUO:4\r\n") + 1, "%s", "+ESUO:4\r\n");
            } else {
                snprintf(response, strlen("+ESUO:Value unsupport\r\n") + 1, "%s",
                            "+ESUO:Value unsupport\r\n");
            }
            return AT_OK;
            break;
        default:
            break;
    }
    return ret;
}
#else   //modify by quectel for DSDS,end
ATRESPONSE_t pas_esuo_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    char simIDProperty[MTK_PROPERTY_VALUE_MAX] = {0};
    char multiSIMProperty[MTK_PROPERTY_VALUE_MAX] = {0};
    int simID = 0;

    atci_property_get(MULTI_SIM, multiSIMProperty, "dsds");
    LOGATCI(LOG_DEBUG, "pas_esuo_handler enter with opType: %d, multiSIM: %s", opType, multiSIMProperty);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;

            if(cmdID == 4) { //Configure the default SIM to SIM1
                atci_property_set(ATCI_SIM, "0");
                LOGATCI(LOG_DEBUG, "Set default to SIM1");
                return AT_OK;
            } else if(cmdID == 5) {
                //Configure the default SIM to SIM2
                if (strcmp(multiSIMProperty,"ss") != 0) {
                    atci_property_set(ATCI_SIM, "1");
                    LOGATCI(LOG_DEBUG, "Set default to SIM2");
                    return AT_OK;
                } else {
                    LOGATCI(LOG_DEBUG, "Error single SIM.");
                }
            } else if(cmdID == 6) {
                //Configure the default SIM to SIM3
                if (strcmp(multiSIMProperty,"tsts") == 0) {
                    atci_property_set(ATCI_SIM, "2");
                    LOGATCI(LOG_DEBUG, "Set default to SIM3");
                    return AT_OK;
                } else {
                    LOGATCI(LOG_DEBUG, "Error: Not triple SIM.");
                }
            } else {
                 LOGATCI(LOG_DEBUG, "Error: not supported SIM Id.");
            }
            break;
        case AT_TEST_OP:
            if (strcmp(multiSIMProperty,"ss") != 0) {
                if (strcmp(multiSIMProperty,"tsts") == 0) {
                    writeDataToSerial("+ESUO: (4-6)", strlen("+ESUO: (4-6)"));
                } else {
                    writeDataToSerial("+ESUO: (4-5)", strlen("+ESUO: (4-5)"));
                }
            } else {
                writeDataToSerial("+ESUO: (4)", strlen("+ESUO: (4)"));
            }
            return AT_OK;
            break;
        case AT_READ_OP:
            atci_property_get(ATCI_SIM, simIDProperty, "0");
            simID = atoi(simIDProperty);
            if (simID == 1) {
                snprintf(response, strlen("+ESUO:5\r\n") + 1, "%s", "+ESUO:5\r\n");
            } else if (simID == 2) {
                snprintf(response, strlen("+ESUO:6\r\n") + 1, "%s", "+ESUO:6\r\n");
            } else if (simID == 3) {
                snprintf(response, strlen("+ESUO:7\r\n") + 1, "%s", "+ESUO:7\r\n");
            } else if (simID == 9) {
                snprintf(response, strlen("+ESUO:9\r\n") + 1, "%s", "+ESUO:9\r\n");
            } else if (simID == 0) {
                snprintf(response, strlen("+ESUO:4\r\n") + 1, "%s", "+ESUO:4\r\n");
            } else {
                snprintf(response, strlen("+ESUO:Value unsupport\r\n") + 1, "%s",
                        "+ESUO:Value unsupport\r\n");
            }
            return AT_OK;
            break;
        default:
            break;

    }
    return AT_ERROR;
}
#endif   //modify by quectel for DSDS,end

ATRESPONSE_t pas_reboot_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    LOGATCI(LOG_DEBUG, "handle cmdline:%s", cmdline);

    if (opType != AT_ACTION_OP) {
        return AT_ERROR;
    }

    switch (opType) {
        case AT_ACTION_OP:
            system("reboot -d 3");
            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_check_clock_handler(char* cmdline, ATOP_t opType, char* response) {
	LOGATCI(LOG_DEBUG, "cmdline %s", cmdline);
	int clock_type=0;
	int fd;
	const char *clock_info_path = "/proc/mt6685";
	char buf[5];
	int result;

	switch (opType) {
		case AT_READ_OP: {
			fd = open(clock_info_path, O_RDONLY);

			if(fd < 0) {
				result = snprintf(response, strlen("\r\nERROR\r\n") + 1, "%s", "\r\nERROR\r\n");
				if (result < 0)
					LOGATCI(LOG_DEBUG, "Open FD error");
				return AT_ERROR;
			}
			read(fd, buf, 4);
			clock_type = atoi(buf);
			close(fd);

			if(clock_type == 0) {
				result = snprintf(response, strlen("+GDCXO:0\r\n") + 1, "%s", "+GDCXO:0\r\n");
				if (result < 0)
					return AT_ERROR;
			} else if (clock_type == 1) {
				result = snprintf(response, strlen("+GDCXO:1\r\n") + 1, "%s", "+GDCXO:1\r\n");
				if (result < 0)
					return AT_ERROR;
			} else {
				result = snprintf(response, strlen("+GDCXO:Value unsupport\r\n") + 1, "%s", "+GDCXO:Value unsupport\r\n");
				if (result < 0)
					return AT_ERROR;
			}

			return AT_OK;
		break;
		}
		case AT_SET_OP:
		case AT_TEST_OP:
			result = snprintf(response, strlen("+GDCXO:Not support\r\n") + 1, "%s", "+GDCXO:Not support\r\n");
			if (result < 0)
				return AT_ERROR;
			return AT_OK;
			break;
		default:
			result = snprintf(response, strlen("+GDCXO:Not support\r\n") + 1, "%s", "+GDCXO:Not support\r\n");
			if (result < 0)
				return AT_ERROR;
		break;
	}

	return AT_ERROR;

}
