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
#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd.h"
#include "atcid_util.h"
#include "at_tok.h"
#include "atcid_adaptation.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <termios.h>
#include <signal.h>
#include <regex.h>
#include <unistd.h>

#define QUECTEL_WITH_HEAD
#include <sys/un.h>

#define MAX_FAILURE_RETRY 3
#define IS_EQUAL(a,b) ((strcmp(a,b)==0)?true:false)
#define MAX(a,b) ((a)>(b)?(a):(b))

/*misc global vars */
extern Serial serial;

#define CONN_VCOM 1
#define CONN_ADB 2
#define CONN_HIDL 3
#define CONN_NET 4

int s_fdRild_command = INVALIDE_SOCKET_FD;
int s_fdService_command_vendor = INVALIDE_SOCKET_FD;
int s_fdService_command = INVALIDE_SOCKET_FD;
int atci_conn_mode = CONN_VCOM;

int getMsgCmdParameterCount(char* line);
int startMergeMsgCmdInputIfNeed(Serial *serial, char* line);

bool isRilAtciSocketExist = true;
bool isATCmdInject = false;
bool isMsgCmdInput = false;
bool isNeedMergeMsgCmdInput = false;
int msgCmdMode = 0;
bool isNumericFormat = false;
bool isLastResponseOk = false;

#define ATCI_SIM "persist.vendor.service.atci.sim"
#define IS_RILDFDVALID() ((s_fdRild_command > 0)&&(true == isRilAtciSocketExist)? true:false)
#define NOMODEM_RSP "ERROR:No Modem"
#define NOUE_RSP "ERROR:UE is not ready"
#define INTERMIDIATE_RSP "\r\n> "

#define CTRLCHAR_SUB 0x1a
#define CTRLCHAR_ESC 0x1b
#define CTRLCHAR_LF 0x0a
#define CTRLCHAR_BS 0x08
#define CTRLCHAR_DEL 0x7F

void sigpipe_handler(int s) {
    UNUSED(s);
    LOGATCI(LOG_ERR, "Caught SIGPIPE\n");
}

bool check_SUB_ESC_char(char* line) {
    char *pLine = line;

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if(CTRLCHAR_SUB == currChar || CTRLCHAR_ESC == currChar) {
            return true;
        }
        pLine++;
    }
    return false;
}

int sendDataToRild(char* line) {
    int sendLen = 0, len = 0;
    LOGATCI(LOG_DEBUG,"Enter");
    len = strlen(line);

    sendCommandByMIPC(line, len);

    isATCmdInject = true;
    isLastResponseOk = false;
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to ril. errno = %d, sendLen = %d", errno, sendLen);
    }
    LOGATCI(LOG_DEBUG," send data over");
    if (check_SUB_ESC_char(line)) {
        LOGATCI(LOG_DEBUG," SUB or ESC in line, clear isMsgCmdInput");
        isMsgCmdInput = false;
    }
    return 0;
}

void writeDataToserialByResponseType(ATRESPONSE_t response_type) {
    char response[MAX_AT_RESPONSE];
    int i = 0;

    memset(response, 0, sizeof(response));

    response[i++] = AT_CR;
    response[i++] = AT_LF;

    switch(response_type) {
        case AT_OK:
            memcpy(response+i, AT_OK_STRING, strlen(AT_OK_STRING));
            i+=strlen(AT_OK_STRING);
            break;
        case AT_CONNECT:
            break;
        case AT_ERROR:
            memcpy(response+i, AT_ERROR_STRING, strlen(AT_ERROR_STRING));
            i+=strlen(AT_ERROR_STRING);
            break;
        case AT_NOT_IMPL:
            memcpy(response+i, AT_NOT_IMPL_STRING, strlen(AT_NOT_IMPL_STRING));
            i+=strlen(AT_NOT_IMPL_STRING);
            break;
        default:
            memcpy(response+i, AT_UNKNOWN_STRING, strlen(AT_UNKNOWN_STRING));
            i+=strlen(AT_UNKNOWN_STRING);
            break;
    }
    response[i++] = AT_CR;
    response[i++] = AT_LF;

    writeDataToSerial(response, strlen(response));
}

void writeDataToSerialByDataType(ATCI_DataType dataType, char* input, int length) {
    int c = 0;
    int fd = INVALIDE_SOCKET_FD;
    char buffer[length + 1];
    char response[length + 1];
    char output[length + 5];
    bool convertedToNumeric = false;

    //LOGATCI(LOG_DEBUG,"Enter with dataType:%d", dataType);
    //LOGATCI(LOG_DEBUG,"Enter with data:%s", input);
    //LOGATCI(LOG_DEBUG,"Enter with length:%d", length);

    if(ADB_SKT_SERVER_NUM  == serial.currDevice)
        fd = serial.fd[ADB_SKT_CLIENT_NUM];
    else if(NET_SERVER_NUM  == serial.currDevice)
        fd = serial.fd[NET_CLIENT_NUM];
    else
        fd = serial.fd[serial.currDevice];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    memset(output, 0, sizeof(output));

    if (fd >=0) {
        /*audio type response must be 1 of 4 prefix above,
          audio response is not AT command, should not be add <CR><LF> here*/
        if (!(strncmp(input, "AUD", 3) == 0 ||
              strncmp(input, "aud", 3) == 0 ||
              strncmp(input, "MM", 2) == 0 ||
              strncmp(input, "mm", 2) == 0 )) {
            // remove <CR><LF> head and tail
            LOGATCI(LOG_DEBUG,"Not audio");
            if (strncmp(input, "\r\n", 2) == 0) {
                if (strncmp((input + length - 2), "\r\n", 2) == 0) {
                    memcpy(buffer, input + 2, length - 4);
                    LOGATCI(LOG_DEBUG,"remove head and tail");
                } else {
                    memcpy(buffer, input + 2, length - 2);
                    LOGATCI(LOG_DEBUG,"remove head");
                }
            } else if (strncmp((input+length - 2), "\r\n", 2) == 0) {
                memcpy(buffer, input, length - 2);
                LOGATCI(LOG_DEBUG,"remove tail");
            } else {
                memcpy(buffer, input, length);
                LOGATCI(LOG_DEBUG,"No remove");
            }

            if (strncmp(buffer, "OK", strlen("OK")) == 0) {
                isLastResponseOk = true;
            } else {
                isLastResponseOk = false;
            }

            // Convert to response
            if (isNumericFormat) {
                convertedToNumeric = Response2Numeric(buffer, response);
            } else {
                memcpy(response, buffer, sizeof(buffer));
            }

            if (dataType == RIL_TYPE && !convertedToNumeric) {
                // Output strings from rild as-is
                memcpy(output, input, length);
            } else {
                // Add <CR><LF> head and tail
#ifdef QUECTEL_WITH_HEAD
                LOGATCI(LOG_DEBUG, "add head and tail");
            	snprintf(output, sizeof(output), "\r\n%s\r\n", response);
                length = strlen(response) + 4;
#else
                LOGATCI(LOG_DEBUG, "add tail");
                snprintf(output, sizeof(output), "\r\n%s\r\n", response);
                length = strlen(response) + 2;
#endif
            }
        } else {
            LOGATCI(LOG_INFO,"audio type response");
            memcpy(output, input, length);
        }

        char *token;
        char at_res[length+1024];
        memset(at_res,0,sizeof(at_res));
        strcat(at_res,"\r\n");
        token = strtok(output,"\r\n");
        while( token != NULL ) 
        {
            LOGATCI(LOG_INFO, "at response%s\n",token);
            if (strstr(token,"OK") != NULL || strstr(token,"ERROR") != NULL || strstr(token,"Error") != NULL)
            {
                strcat(at_res,"\r\n");
                strcat(at_res,token);
                strcat(at_res,"\r\n");
            }
            else
            {
                strcat(at_res,token);
                strcat(at_res,"\r\n");
            }
            token = strtok(NULL, "\r\n");
            length = strlen(at_res);
        }
        if (strncmp(at_res, "\r\n\r\n", 4) == 0) //Delete \r\n when only OK / ERROR
        {            
            memset(output,0,sizeof(output));
            memcpy(output, at_res + 2, length - 2);
            LOGATCI(LOG_DEBUG,"remove head");
        }
        else
        {
            memset(output,0,sizeof(output));
            memcpy(output, at_res, length );
        }
        length = strlen(output);
        LOGATCI(LOG_DEBUG,"length=%d", length);
        if (HIDL_CONN_NUM != serial.currDevice) 
        {
            c = write(fd, output, length);
        }
        if (length != c) 
        {
            LOGATCI(LOG_WARNING, "Couldn't write all data to the serial port. Wrote only %d bytes with %d", c, errno);
        }
    } 
    else 
    {
        LOGATCI(LOG_ERR, "Wrong fd with %d, currDevice = %d", fd, serial.currDevice);
    }
    set_at_in_process(false);
    return;
}

void writeDataToSerial(char* input, int length) {
    writeDataToSerialByDataType(UNKNOWN_TYPE, input, length);
    set_at_in_process(false);
}

int sendDataToGenericService(char* line) {

    int len = 0;
    len = strlen(line);

    LOGATCI(LOG_DEBUG,"Enter");

    if (s_fdService_command_vendor < 0) {
        LOGATCI(LOG_ERR, "fd invalid when send to atci service. errno = %d", errno);
        return -1;
    }

    int sendLen = send(s_fdService_command_vendor, line, len, 0);
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to atci service. errno = %d", errno);
        return -1;
    }

    LOGATCI(LOG_INFO, "send to atci service: %s", line);

    return 0;
}

int sendDataToGenericSysService(char* line) {

    int len = 0;
    len = strlen(line);

    LOGATCI(LOG_DEBUG,"Enter");

    if (s_fdService_command < 0) {
        LOGATCI(LOG_ERR, "fd invalid when send to atci sys service. errno = %d", errno);
        return -1;
    }

    int sendLen = send(s_fdService_command, line, len, 0);
    if (sendLen != len) {
        LOGATCI(LOG_ERR, "lose data when send to atci sys service. errno = %d", errno);
        return -1;
    }

    LOGATCI(LOG_INFO, "send to atci sys service: %s", line);

    return 0;
}


void readDataFromTargetWithResult(ATCI_DataType dataType, int* s_fd_listen, char* buffer, int buffer_size) {
    int recvLen = 0;

    LOGATCI(LOG_DEBUG,"Enter");
    LOGATCI(LOG_INFO,"Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, buffer_size);

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if(errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if(recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown");
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        }
    } while(recvLen <= 0);

    LOGATCI(LOG_INFO, "data receive from %d is %s, data length is %d", dataType, buffer, recvLen);
    writeDataToSerial(buffer, recvLen);
}

void readDataFromTarget(ATCI_DataType dataType, int* s_fd_listen) {
    int recvLen = 0;
    char buffer[MAX_DATA_SIZE + 1];

    LOGATCI(LOG_INFO,"Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, sizeof(buffer));

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if(errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if(recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown, isATCmdInject %d",isATCmdInject);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            if(true == isATCmdInject) {
                writeDataToSerial(NOUE_RSP, strlen(NOUE_RSP)+1);
                isATCmdInject = false;
            }
            return;
        }
    } while(recvLen <= 0);

    isATCmdInject = false;
    LOGATCI(LOG_INFO, "data receive from %d is %s %d", dataType, buffer,recvLen);
    writeDataToSerialByDataType(dataType, buffer, recvLen);

    if (isMsgResponse(buffer)) {
        isMsgCmdInput = false;
    }
}

int socket_local_client (char* name) {
    struct sockaddr_un server;
    int fd = 0;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "Can't open stream socket (%s)", name);
        return -1;
    }

    server.sun_family = AF_UNIX;
    memset(server.sun_path, '\0', sizeof(server.sun_path));
    strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);

    if (connect(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(fd);
        LOGATCI(LOG_ERR, "Can't connect to server side, path: %s, errno:%d", name, errno);
        return -1;
    }
    return fd;
}

void connectTarget(ATCI_DataType dataType) {
    LOGATCI(LOG_DEBUG,"Enter");

    if (RIL_TYPE == dataType) {

        s_fdRild_command = connectToMPCI();
        if (s_fdRild_command < 0) {
            if (errno == ENOENT) { // No such file or directory
                LOGATCI(LOG_ERR, "ENOENT[%d] : No such file or directory",ENOENT);
                isRilAtciSocketExist = false;
            }
            LOGATCI(LOG_ERR, "fail to open atci-ril socket. errno:%d", errno);
            close(s_fdRild_command);
            return;
        }

    } else if (GENERIC_TYPE == dataType) {

        if (s_fdService_command_vendor == INVALIDE_SOCKET_FD) {

            LOGATCI(LOG_INFO, "connectTarget path = %s", SOCKET_NAME_GENERIC);

            s_fdService_command_vendor = socket_local_client(SOCKET_NAME_GENERIC);
            LOGATCI(LOG_INFO, "s_fdService_command_vendor = %d", s_fdService_command_vendor);

            if (s_fdService_command_vendor < 0) {
                LOGATCI(LOG_ERR, "fail to open atci generic socket. errno:%d", errno);
                close(s_fdService_command_vendor);
                return;
            }
        }

    } else if (GENERIC_SYS_TYPE == dataType) {
        #if defined(TELEMATICS) || defined(RDKB)
        // connect to demo App
        if (s_fdService_command == INVALIDE_SOCKET_FD) {

            LOGATCI(LOG_INFO, "connectTarget path = %s", SOCKET_NAME_GENERIC_SYS);

            s_fdService_command = socket_local_client(SOCKET_NAME_GENERIC_SYS);
            LOGATCI(LOG_INFO, "s_fdService_command_vendor = %d", s_fdService_command);

            if (s_fdService_command < 0) {
                LOGATCI(LOG_ERR, "fail to open atci generic socket. errno:%d", errno);
                close(s_fdService_command);
                return;
            }
        }
    #endif
    }
}

void processCtrlChar_BackSpace(char* line) {
    char buf[MAX_DATA_SIZE + 1] = {0};
    char *pLine = line;
    int currLineIdx = 0;

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if (CTRLCHAR_BS == currChar || CTRLCHAR_DEL == currChar) {
            if(0 < currLineIdx)
                currLineIdx--;//Move back one char
        } else {
            if (0 <= currLineIdx) {
                buf[currLineIdx] = *pLine;
                currLineIdx++;
            }
        }
        pLine++;
    }
    strncpy(line, buf, strlen(buf));
    line[currLineIdx] = '\0';
}
/**
 * Reads a line from the AT channel, returns NULL on timeout.
 * Assumes it has exclusive read access to the FD
 *
 * This line is valid only until the next call to readline
 *
 */

static const char *readline(Serial *serial) {
    fd_set rfds;
    char *p_read = NULL;
    int ret = 0;
    char buffer[MAX_DATA_SIZE + 1] = {0};
    char tmpBuffer[MAX_DATA_SIZE + 1] = {0};
    char *p_buffer = buffer;
    int readCurr = 0, readCount = 0, tmp_readCurr = 0;
    int err = 0, i = 0;
    int fd = INVALIDE_SOCKET_FD, currDevice = 0;
    int headerSize = 0;

    LOGATCI(LOG_DEBUG,"Enter");

    memset(buffer, 0, sizeof(buffer));
    memset(tmpBuffer, 0, sizeof(tmpBuffer));
    atci_conn_mode = CONN_VCOM;
    //Start to read the command data from USB devices
    for(;;) {

        //Initial the default value for each local paramters
        //Set the buffer data to zero
        if (!isNeedMergeMsgCmdInput) {
            memset(serial->ATBuffer, 0, sizeof(serial->ATBuffer));
            serial->totalSize = 0;
        }
        if (CONN_VCOM == atci_conn_mode) {
            memset(buffer, 0, sizeof(buffer));
        }
        serial->at_state = FIND_A;      //Inital state to find 'A' character in command line
        readCurr = 0;
        FD_ZERO(&rfds);

        for(i = 0; i < MAX_DEVICE_NUM; i ++) {
            if(INVALIDE_SOCKET_FD != serial->fd[i])
                FD_SET(serial->fd[i], &rfds);
            else
                LOGATCI(LOG_DEBUG, "invalide fd set [%d].", i);
        }
///* for read rild data */

        //if the mode is dual talk, then we need to make sure which socket need to be connect by the property "ATCI_SIM"
        if(true == isRilAtciSocketExist) {

            if (s_fdRild_command < 0) {
                connectTarget(RIL_TYPE);
            }
            if (s_fdRild_command > 0) {
                FD_SET(s_fdRild_command, &rfds);
            } else {
                // if not usleep, connectTarget() will be run many times and too many log will be output.
                usleep(100*1000);
                return NULL;
            }
        }

        if (s_fdService_command_vendor > 0) {
            FD_SET(s_fdService_command_vendor, &rfds);
        }

        if (s_fdService_command > 0) {
            FD_SET(s_fdService_command, &rfds);
        }

        LOGATCI(LOG_DEBUG, "Wait for select data from USB");
        int maxFd = 0;
        for (i = 0; i < MAX_DEVICE_NUM; i++) {
            maxFd = MAX(maxFd, serial->fd[i]);
        }
        if(IS_RILDFDVALID()) {
            maxFd = MAX(s_fdRild_command, maxFd);
        }
        if (s_fdService_command_vendor > 0) {
            maxFd = MAX(s_fdService_command_vendor, maxFd);
        }
        if (s_fdService_command > 0) {
            maxFd = MAX(s_fdService_command, maxFd);
        }

        ret = select(maxFd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            if(errno == EINTR || errno == EAGAIN) {
                continue;
            }
            LOGATCI(LOG_ERR, "Fail to select in readline. error: %d", errno);
            for(i = 0; i < MAX_DEVICE_NUM; i ++) {
                close(serial->fd[i]);
                serial->fd[i] = INVALIDE_SOCKET_FD;
            }
///* for read rild data */
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
////
            return NULL;
        } else if(ret == 0) {
            LOGATCI(LOG_DEBUG, "ERROR:No data from USB devices");
            continue;
        }
///* for read rild data */
        if(IS_RILDFDVALID() && FD_ISSET(s_fdRild_command, &rfds)) {
            readDataFromTarget(RIL_TYPE, &s_fdRild_command);
            continue;
        }
////
        if (s_fdService_command_vendor > 0 && FD_ISSET(s_fdService_command_vendor, &rfds)) {
            LOGATCI(LOG_DEBUG, "readline: Prepare read the data from generic service");
            readDataFromTarget(GENERIC_TYPE, &s_fdService_command_vendor);
            continue;
        }

        if (s_fdService_command > 0 && FD_ISSET(s_fdService_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "readline: Prepare read the data from generic service");
            readDataFromTarget(GENERIC_SYS_TYPE, &s_fdService_command);
            continue;
        }

        for(i = 0; i < MAX_DEVICE_NUM; i++) {
            if (serial->fd[i] != INVALIDE_SOCKET_FD && FD_ISSET(serial->fd[i], &rfds)) {
                LOGATCI(LOG_DEBUG, "Read data from USB:%d:%d", i, serial->fd[i]);
                fd = serial->fd[i];
                currDevice = i;
                if(ADB_SKT_SERVER_NUM == currDevice) {
                    close(serial->fd[ADB_SKT_CLIENT_NUM]);
                    serial->fd[ADB_SKT_CLIENT_NUM] = adb_socket_accept(fd);
                    atci_conn_mode = CONN_ADB;
                    p_buffer = tmpBuffer;
                } else if(ADB_SKT_CLIENT_NUM == currDevice) {
                    atci_conn_mode = CONN_ADB;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    p_buffer = tmpBuffer;
                } else if (HIDL_CONN_NUM == currDevice) {
                    atci_conn_mode = CONN_HIDL;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    p_buffer = tmpBuffer;
                } else if (NET_SERVER_NUM == currDevice) {
                    fd = adb_socket_accept(fd);
                    if (serial->fd[NET_CLIENT_NUM] >= 0) {
                        shutdown(serial->fd[NET_CLIENT_NUM], SHUT_RDWR);
                        close(serial->fd[NET_CLIENT_NUM]);
                    }
                    serial->fd[NET_CLIENT_NUM] = fd;
                    atci_conn_mode = CONN_NET;
                    p_buffer = tmpBuffer;
                } else if (NET_CLIENT_NUM == currDevice) {
                    atci_conn_mode = CONN_NET;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    p_buffer = tmpBuffer;
                } else {
                    atci_conn_mode = CONN_VCOM;
                    memset(buffer, 0, sizeof(buffer));
                    p_buffer = buffer;
                }
                break;
            }
        }

        do {
            if(currDevice == ADB_SKT_SERVER_NUM) {
                currDevice = ADB_SKT_CLIENT_NUM;
                fd = serial->fd[ADB_SKT_CLIENT_NUM];
            } else if (currDevice == NET_SERVER_NUM) {
                break;
            }

            LOGATCI(LOG_DEBUG, "Wait for read data from USB :%d", fd);
            readCount = read(fd, p_buffer, MAX_DATA_SIZE);  //Read data from VCOM driver

            if(readCount > 0) {
                LOGATCI(LOG_DEBUG, "the readCount is %d with %s:%d", readCount, p_buffer, p_buffer[0]);
                if(atci_conn_mode == CONN_VCOM) {
                    LOGATCI(LOG_DEBUG, "VCOM mode, sleep 0.01s");
                    usleep(10*1000);
                }
                break;
            } else if(readCount <= 0) {
                err = errno;
                if(err == EINTR)
                {
                    usleep(100*1000);
                }
                if(/*err != EINTR && */err != EAGAIN) { //fix wake up cannot send at commd
                    LOGATCI(LOG_ERR, "FATAL Error in serail read:%d", err);
                    close(fd);

                    //Reopen this serial port
                    if(currDevice < MAX_DEVICE_VCOM_NUM) {
                        for(i = 0; i < MAX_FAILURE_RETRY; i++) {
#ifdef CONFIG_QUECTEL_MAIN_UART
                            //for listen main uart
                            if(i==MAIN_UART_POST && quectel_main_uart_get() != true) {
                                continue;
                            }
#endif
                            serial->fd[currDevice] = open_serial_device(serial, serial->devicename[currDevice]);
                            if(serial->fd[currDevice] != INVALIDE_SOCKET_FD) {
                                LOGATCI(LOG_DEBUG, "The FD is reopen successfully");
                                return NULL;
                            }
                        }
                    } else {
                        if(currDevice == ADB_SKT_CLIENT_NUM)
                            serial->fd[ADB_SKT_CLIENT_NUM] = INVALIDE_SOCKET_FD;
                        else if(currDevice == NET_CLIENT_NUM)
                            serial->fd[NET_CLIENT_NUM] = INVALIDE_SOCKET_FD;
                        LOGATCI(LOG_DEBUG, "currDevice[%d]",currDevice);
                        return NULL;
                    }
                    LOGATCI(LOG_ERR, "The process is terminated due to fatal error");
                    exit(1);
                }
                continue;
            }
        } while(1);

        // Update global status after successfully read, i.e. don't update when just handling error.
        // e.g. when machine is using socket but some error occcurs on COM,
        // we should handle the COM error but keep serial->currDevice be socket.
        serial->currDevice = currDevice;

        LOGATCI(LOG_DEBUG, "atci_conn_mode = [%d], [%x]",atci_conn_mode,p_buffer[0]);

        if (CONN_VCOM != atci_conn_mode && 0 < readCount) {
            int i = 0;
            for(i = 0; i < readCount; i++) {
                buffer[tmp_readCurr] = p_buffer[i];
                tmp_readCurr++;
                if ((AT_CR == p_buffer[i] || AT_LF == p_buffer[i])
                        || tmp_readCurr >= MAX_DATA_SIZE) {
                    readCount = tmp_readCurr;
                    tmp_readCurr = 0;
                    LOGATCI(LOG_DEBUG, ".AT command read done.");
                    break;
                } else {
                    if(i + 1 >= readCount) {
                        LOGATCI(LOG_DEBUG, ".AT command read more.");
                    }
                }
            }

            if(i >= readCount) {
                LOGATCI(LOG_DEBUG, ".go on read.");
                continue;
            }
        }
        LOGATCI(LOG_DEBUG, "state %d with %d:%d", serial->at_state, readCurr, buffer[readCurr]);

        if (isMsgCmdInput && !((int)buffer[0] == CTRLCHAR_LF && readCount == 1)) {
            if (isNeedMergeMsgCmdInput) {
                // merge this input after current parcial command
                processCtrlChar_BackSpace(buffer);
                LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput, text=%s", buffer);
                if (strlen(serial->ATBuffer) + strlen(buffer) + strlen("\"\r\n") + 1
                        <= (int) sizeof(serial->ATBuffer)) {
                    strncpy(serial->ATBuffer + strlen(serial->ATBuffer), buffer, strlen(buffer));
                    serial->totalSize += readCount;
                }
                LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput update buffer: %s", serial->ATBuffer);

                if (check_SUB_ESC_char(buffer)) {
                    // finish the command string
                    LOGATCI(LOG_DEBUG, " SUB or ESC in line, clear isNeedMergeMsgCmdInput");
                    isNeedMergeMsgCmdInput = false;
                    // -1 to backspace the ESC char
                    strncpy(serial->ATBuffer + strlen(serial->ATBuffer) - 1, "\"\r\n", strlen("\"\r\n"));
                    serial->totalSize += strlen("\"\r\n");
                    serial->ATBuffer[strlen("AT+")] = 'E';
                    LOGATCI(LOG_DEBUG, "isNeedMergeMsgCmdInput update buffer: %s", serial->ATBuffer);
                }
                break;
            } else {
                serial->totalSize = readCount;
                LOGATCI(LOG_DEBUG, "isMsgCmdInput, text=%s", buffer);
                if (sizeof(buffer) <= sizeof(serial->ATBuffer))
                    memcpy(serial->ATBuffer, buffer, sizeof(buffer));
                break;
            }
        } else {

            readCurr = 0;
            headerSize = 2;
            processCtrlChar_BackSpace(buffer);
            if(serial->at_state == FIND_A && readCount > 2) {
                if((UCASE(buffer[0]) == AT_A) && (UCASE(buffer[1]) == AT_T)) {
                    p_read = &buffer[0];
                    serial->at_state = FIND_DONE;
                    readCurr = 2;
                }
            }

            //There is no AT string in this input; discard this input and get the input again.
            if(serial->at_state == FIND_DONE) {
                serial->totalSize = readCount - readCurr + headerSize; //Add the length of "AT"
                LOGATCI(LOG_DEBUG, "the readCurr is %d & the total size is %d", readCurr, serial->totalSize);
                memcpy(serial->ATBuffer, p_read, serial->totalSize);
                serial->ATBuffer[serial->totalSize-1] = '\0';  //Skip the "\r" character
                break;
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }
    set_at_in_process(true);
    return &serial->ATBuffer[0];
}

void *readerLoop(void *arg) {
    Serial *p_serial = (Serial *)arg;
    ATCI_DataType dataType = UNKNOWN_TYPE;
    char operator[MTK_PROPERTY_VALUE_MAX] = {0};

    signal(SIGPIPE, sigpipe_handler);

    LOGATCI(LOG_DEBUG,"Enter");

    connectTarget(GENERIC_TYPE);
#if defined(TELEMATICS) || defined(RDKB)
    connectTarget(GENERIC_SYS_TYPE);
#endif

    for (;;) {
        char* line = NULL;

        line = (char*) readline(p_serial);

        if (line == NULL) {
            continue;
        }

        if (isMsgCmdInput) {
            if (isNeedMergeMsgCmdInput) {
                writeDataToSerial(INTERMIDIATE_RSP, strlen(INTERMIDIATE_RSP) + 1);
            } else {
                if (true == isRilAtciSocketExist) {
                    if (s_fdRild_command < 0) {
                        connectTarget(RIL_TYPE);
                    }
                    sendDataToRild(line);
                }
            }
            set_at_in_process(false);
            continue;
        }

        convertToUpperCase(line,'=');
        trim_string(&line);
        LOGATCI(LOG_INFO, "Command:%s",line);

        dataType = process_cmd_line(line);

        LOGATCI(LOG_INFO, "The command type is belonged to :%d",dataType);

        if(dataType == RIL_TYPE) {
            if (isRilAtciSocketExist) {
                if (s_fdRild_command < 0) {
                    connectTarget(RIL_TYPE);
                }

                if (isMsgCmd(line)) {
                    LOGATCI(LOG_INFO, "set isMsgCmd true");
                    isMsgCmdInput = true;
                    if (strcmp(operator, "OP12") == 0
                            && startMergeMsgCmdInputIfNeed(p_serial, line)) {
                        isNeedMergeMsgCmdInput = true;
                        set_at_in_process(false);
                        continue;
                    }
                }

                if (strncmp(line, "ATV1", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag false");
                    isNumericFormat = false;
                }
                if (strncmp(line, "ATV0", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag true");
                    isNumericFormat = true;
                }

                sendDataToRild(line);
            } else {
                if (s_fdRild_command < 0) {
                    LOGATCI(LOG_INFO, "Re-connect to RILD.");
                    isRilAtciSocketExist = true;
                    connectTarget(RIL_TYPE);
                }

                if (isMsgCmd(line)) {
                    LOGATCI(LOG_INFO, "set isMsgCmd true");
                    isMsgCmdInput = true;
                    if (strcmp(operator, "OP12") == 0
                            && startMergeMsgCmdInputIfNeed(p_serial, line)) {
                        isNeedMergeMsgCmdInput = true;
                        set_at_in_process(false);
                        continue;
                    }
                }

                if (strncmp(line, "ATV1", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag false");
                    isNumericFormat = false;
                }
                if (strncmp(line, "ATV0", 4) == 0) {
                    LOGATCI(LOG_INFO, "set Numeric flag true");
                    isNumericFormat = true;
                }

                if (IS_RILDFDVALID()) {
                    sendDataToRild(line);
                } else {
                    LOGATCI(LOG_INFO, "Response No Modem.2");
                    writeDataToSerial(NOMODEM_RSP, strlen(NOMODEM_RSP)+1);
                }
            }

        } else if(dataType == ATCI_TYPE) {
            LOGATCI(LOG_INFO, "ATCI COMMAND");
            // no need send cmd here because already send in custom_command_hdlr()

        } else if(dataType == GENERIC_TYPE) {
            LOGATCI(LOG_INFO, "GENERIC COMMAND");
            if (s_fdService_command_vendor < 0 ||s_fdService_command < 0 ) {
                connectTarget(GENERIC_TYPE);
            }
            sendDataToGenericService(line);
            LOGATCI(LOG_INFO, "Generic AT COMMAND");

        } else if(dataType == GENERIC_SYS_TYPE) {
            LOGATCI(LOG_INFO, "GENERIC SYS COMMAND");
            if (s_fdService_command_vendor < 0 ||s_fdService_command < 0 ) {
                connectTarget(GENERIC_SYS_TYPE);
            }
            sendDataToGenericSysService(line);
            LOGATCI(LOG_INFO, "Generic SYS    AT COMMAND");
        }
#ifdef  QUECTEL_AT_SUPPORT
        else if(dataType == QUECTEL_COMMON_TYPE)
        {
            LOGATCI(LOG_INFO, "QUECTEL_COMMON_TYPE COMMAND");
        }
#endif

    }

    LOGATCI(LOG_INFO, "ReaderLoop thread Closed");

    return NULL;
}

/*
* Purpose:  Initial the default value of serial device.
* Input:      serial - the serial struct
* Return:    void
*/
void initSerialDevice(Serial *serial) {

    int i = 0;
    LOGATCI(LOG_DEBUG,"Enter");
    memset(serial,0,sizeof(Serial));

    for (i = 0; i < MAX_DEVICE_NUM; i++) {
        serial->fd[i] = INVALIDE_SOCKET_FD;
    }

    serial->echo[0] = 1;
    serial->echo[1] = 1;
    serial->totalSize = 0;
}

void setEchoOption(int flag) {
    struct termios deviceOptions;
    int fd = serial.fd[serial.currDevice];

    LOGATCI(LOG_INFO, "set echo option:%d for device:%d", flag, serial.currDevice);

    // get the parameters
    tcgetattr(fd,&deviceOptions);
    if(flag == 1) {
        deviceOptions.c_lflag = ICANON | ECHO;
    } else {
        deviceOptions.c_lflag = ICANON ;
    }

    serial.echo[serial.currDevice] = flag;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&deviceOptions);

}

/*
* Purpose:  Open and initialize the serial device used.
* Input:      serial - the serial struct
* Return:    0 if port successfully opened, else -1.
*/
int open_serial_device(Serial* serial, char* devicename) {
    LOGATCI(LOG_DEBUG, "Enter");
    int fdflags;
    struct termios deviceOptions;
    int fd = INVALIDE_SOCKET_FD;
    int is_echo = 1;
    int ret = 0;

    LOGATCI(LOG_INFO, "Opened serial port:%s", devicename);

    do {
        fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
    } while (fd < 0 && errno == EINTR);

    if(fd < 0) {
        LOGATCI(LOG_ERR, "Fail to open serial device(%s) with error code:%d", devicename, errno);
        return INVALIDE_SOCKET_FD;
    }

    if(strcmp(devicename, TTY_GS0) == 0) {
        is_echo = serial->echo[0];
    }
    else if(strcmp(devicename, TTY_GS1) == 0) {
        is_echo = serial->echo[1];
    }

    LOGATCI(LOG_DEBUG, "serial->fd is %d ", fd);

    // Set FD to Sync IO
    fdflags = fcntl(fd, F_GETFL);
    ret = fcntl(fd, F_SETFL, fdflags & ~O_NONBLOCK);
    if (ret < 0) {
        LOGATCI(LOG_ERR, "Error clearing O_NONBLOCK errno: %d", errno);
    }

    // get the parameters
    tcgetattr(fd,&deviceOptions);

    // set raw input
    if(is_echo == 1) {
        deviceOptions.c_lflag = ICANON | ECHO;
    } else {
        deviceOptions.c_lflag = ICANON ;
    }

    // set raw output
    deviceOptions.c_oflag &= ~OPOST;
    deviceOptions.c_oflag &= ~OLCUC;
    deviceOptions.c_oflag &= ~ONLRET;
    deviceOptions.c_oflag &= ~ONOCR;
    deviceOptions.c_oflag &= ~OCRNL;

    deviceOptions.c_cc[VEOL] = 0x1a; /*Ctrl+Z*/
    deviceOptions.c_lflag = deviceOptions.c_lflag | IEXTEN;
    deviceOptions.c_cc[VEOL2] = 0x1b; /*ESC*/

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&deviceOptions);

    return fd;
}

#if defined(TELEMATICS)
// IVT
int readSys_int(char const * path) {
    int fd;

    if (path == NULL) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        char buffer[128] = {0};
        int amt = read(fd, buffer, 128);
        close(fd);
        char *mode = strrchr(buffer,':');
        char *isMeda = strstr(mode, "META BOOT");

        LOGATCI(LOG_DEBUG, "boot mode     = <%s>, isMeda = [%s]", mode, isMeda);

        if (isMeda != NULL) {
            LOGATCI(LOG_DEBUG, "boot mode = 1");
            return 1;
        }
        LOGATCI(LOG_DEBUG, "boot mode = 0");
        return 0;
      //  return amt == -1 ? -errno : atoi(buffer);
    }
    LOGATCI(LOG_DEBUG, "read boot mode failed to open %s\n", path);
    return -1;
}

#else

// Colgin/Bollinger/RDKB
int readSys_int(char const * path)
{
    int fd;

    if (path == NULL) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        int buffer[8] = {0};
        int len = read(fd, &buffer, sizeof(int)*8);
        LOGATCI(LOG_DEBUG, "read boot mode struct len = %d\n", len);
        if(len > 0)
        {
            LOGATCI(LOG_DEBUG, "boot mode size = %d, tag = %d, mode = %d\n", buffer[0], buffer[1], buffer[2]);
            return buffer[2];
        }
        close(fd);
    }
    LOGATCI(LOG_DEBUG, "read boot mode failed to open %s\n", path);
    return -1;
}
#endif

int adb_socket_accept(int socketFd) {
    struct sockaddr_in clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    int fd_clientSk = INVALIDE_SOCKET_FD;

    LOGATCI(LOG_DEBUG, "accept socket on fd :%d", socketFd);

    if(-1 == (fd_clientSk = accept(socketFd, (struct sockaddr *)&clientAddr, &sinSize))) {
        LOGATCI(LOG_DEBUG, "Fail to accept socket port err:%d",errno);
    }
    return fd_clientSk;
}
int adb_socket_listen(int socketFd) {
    LOGATCI(LOG_DEBUG, "Listen socket on fd :%d", socketFd);
    if(-1 == listen(socketFd,BACK_LOG)) {
        LOGATCI(LOG_DEBUG, "Fail to listen socket port err:%d", errno);
        return ATCID_CREATE_ADB_SOCKET_ERR;
    }
    return ATCID_SUCCESS;
}

int get_control_socket(char* name) {

    struct sockaddr_un server;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "Can't open stream socket (%s)", name);
        return 0;
    }

    server.sun_family = AF_UNIX;
    memset(server.sun_path, '\0', sizeof(server.sun_path));
    strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);
    unlink(name);

    if (bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        LOGATCI(LOG_ERR, "bind failed name=[%s] reason=[%s]\n", name, strerror(errno));
        close(fd);
        return 0;
    }
    return fd;
}

int bind_network_sock(uint32_t addr, int port) {
    int sock;
    int reuse = 1;
    struct sockaddr_in serv_addr; /* Local address */

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOGATCI(LOG_ERR, "Can't open stream socket (%s)", strerror(errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((short)port);
	serv_addr.sin_addr.s_addr = htonl(addr);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        LOGATCI(LOG_ERR, "bind failed port=[%d] reason=[%s]", port, strerror(errno));
        close(sock);
        return -1;
    }

    listen(sock, BACK_LOG);
    LOGATCI(LOG_DEBUG, "Listen socket on fd:%d nw port:%d", sock, port);

    return sock;
}

/** returns 1 if line starts with prefix, 0 if it does not */
int strStartsWith(const char *line, const char *prefix) {
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}

char* cut_msg_line(char* line) {
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for(i = 0; i < size; i++) {
        if(line[i] == '=' && line[i+1] != '?' && line[i+1] != '\0' ) {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        LOGATCI(LOG_DEBUG, "malloc result fail, return NULL");
        return NULL;
    }
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}

int isMsgCmd(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_msg_line(line);
    int result = -1;
    int err = -1;
    char* tmp = NULL;
    char* tmpBuf = NULL;

    if (line_cut == NULL) {
        return 0;
    }

    table_size = (sizeof(msg_cmd_table)/sizeof(generic_cmd_type));

    for(i = 0; i < table_size; i++) {
        if(strcmp(line_cut, msg_cmd_table[i].cmdName) == 0) {
            free(line_cut);
            return 1;
        }
    }

    LOGATCI(LOG_DEBUG, "line_cut: %s", line_cut);
    if (strcmp(line_cut, "AT+CMGF") == 0) {
        tmpBuf = (char *)malloc(strlen(line) + 1);
        if (tmpBuf == NULL) {
            LOGATCI(LOG_DEBUG, "fail to malloc tmpBuf, return 0");
            free(line_cut);
            return 0;
        }
        memcpy(tmpBuf, line, strlen(line) + 1);

        tmp = tmpBuf;
        err = at_tok_start_flag(&tmp, '=');
        if (err == 0 && at_tok_hasmore(&tmp) && tmp[0] != '?') {
            err = at_tok_nextint(&tmp, &result);
        }
        if (err == 0) {
            msgCmdMode = result;
            LOGATCI(LOG_DEBUG, "msgCmdMode = %d", result);
        }

        free(tmpBuf);
    }

    free(line_cut);
    return 0;
}

int isMsgResponse(const char *line) {
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_MsgResponses); i++) {
        if (strStartsWith(line, s_MsgResponses[i])) {
            return 1;
        }
    }
    return 0;
}

int getMsgCmdParameterCount(char* line) {
    int err = -1;
    char* out = NULL;
    char* tmp = NULL;
    char* tmpBuf = NULL;
    int count = 0;

    err = at_tok_start_flag(&line, '=');
    if (err < 0 || line[0] == '?') {
        return 0;
    }

    tmpBuf = (char *)malloc(strlen(line) + 1);
    if (tmpBuf == NULL) {
        LOGATCI(LOG_DEBUG, "fail to malloc tmpBuf, return 0");
        return 0;
    }

    memcpy(tmpBuf, line, strlen(line) + 1);

    tmp = tmpBuf;
    while (at_tok_hasmore(&tmp)) {
        err = at_tok_nextstr(&tmp, &out);
        if (err == 0) {
            count++;
        }
    }
    free(tmpBuf);

    return count;
}

int startMergeMsgCmdInputIfNeed(Serial *serial, char* line) {
    // for msg cmd, merge multi line input to single command if need
    char simIDProperty[MTK_PROPERTY_VALUE_MAX];
    int simID = 0;

    int count = getMsgCmdParameterCount(line);
    LOGATCI(LOG_DEBUG, "startMergeMsgCmdInputIfNeed count %d msgCmdMode %d", count, msgCmdMode);
    if (count == 1 || (count == 2 && msgCmdMode == 1)) {
        const char* str = ",\"";
        if (count == 1 && msgCmdMode == 1) {
            str = ",,\"";
        }
        strncpy(serial->ATBuffer + strlen(serial->ATBuffer), str, strlen(str));
        serial->totalSize += strlen(str);
        LOGATCI(LOG_DEBUG, "startMergeMsgCmdInputIfNeed update buffer: %s", serial->ATBuffer);
        writeDataToSerial(INTERMIDIATE_RSP, strlen(INTERMIDIATE_RSP) + 1);
        return 1;
    }
    return 0;
}

bool Response2Numeric(char *input, char *output) {
    if (isNumericFormat) {
        size_t i;
        for (i = 0; i < NUM_ELEMS(numericResponse_table); i++) {
            if (strcmp(input, numericResponse_table[i].Response) == 0) {
                LOGATCI(LOG_DEBUG, "Response2Numeric, i=%d", (int) i);
                memcpy(output, numericResponse_table[i].Number, 1);
                return true;
            }
        }
        memcpy(output, input, strlen(input));
    }
    return false;
}

void readDataFromTargetSync(ATCI_DataType dataType, int* s_fd_listen, char* response) {
    int recvLen = 0;
    char buffer[MAX_DATA_SIZE + 1];

    LOGATCI(LOG_DEBUG, "Enter");
    LOGATCI(LOG_INFO, "Read data from target:%d with fd:%d", dataType, *s_fd_listen);

    memset(buffer, 0, sizeof(buffer));

    do {
        recvLen = recv(*s_fd_listen, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOGATCI(LOG_ERR, "fail to receive data from target socket. errno = %d", errno);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        } else if (recvLen == 0) {
            LOGATCI(LOG_ERR, "The peer has performed an orderly shutdown, isATCmdInject %d", isATCmdInject);
            close(*s_fd_listen);
            *s_fd_listen = INVALIDE_SOCKET_FD;
            return;
        }
    } while (recvLen <= 0);

    LOGATCI(LOG_INFO, "data receive from %d is %s", dataType, buffer);
    LOGATCI(LOG_INFO, "data receive data length is %d", recvLen);
    strncpy(response, buffer, MAX_DATA_SIZE);
}

void rildReaderLoopSync(char* response) {
    int ret;
    fd_set rfds;

    LOGATCI(LOG_DEBUG, "Enter");

    if (s_fdRild_command != INVALIDE_SOCKET_FD) {
        FD_ZERO(&rfds);
        FD_SET(s_fdRild_command, &rfds);

        ret = select(s_fdRild_command+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;
            }
            LOGATCI(LOG_ERR, "Fail to select in rildReaderLoop. error: %d", errno);
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        } else if (ret == 0) {
            LOGATCI(LOG_DEBUG, "timeout for select in rildREaderLoop");
            close(s_fdRild_command);
            s_fdRild_command = INVALIDE_SOCKET_FD;
            return;
        }

        if (FD_ISSET(s_fdRild_command, &rfds)) {
            LOGATCI(LOG_DEBUG, "Prepare read the data from rild driver");
            readDataFromTargetSync(RIL_TYPE, &s_fdRild_command, response);
        }
    }
}

int sendDataToRildSync(char* line, char* response) {
    int len = 0;
    LOGATCI(LOG_DEBUG, "Enter");

    len = strlen(line);
    if (sendDataToRild(line) < 0) {
        snprintf(response, MAX_DATA_SIZE, NOMODEM_RSP);
    }
    LOGATCI(LOG_DEBUG, "send data over");

    rildReaderLoopSync(response);
    return 0;
}

