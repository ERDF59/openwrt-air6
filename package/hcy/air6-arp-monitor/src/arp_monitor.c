/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <linux/input.h>
#include <signal.h>
#include "log.h"

#include "arp_monitor.h"

#define ARP_LIST_FILE  "/mnt/vendor/nvcfg/arp_list"
#define ALLOW_ARP_SYNC_FILE  "/mnt/vendor/nvcfg/allow_arp_sync"
#define MAC_STR_LEN      (2*6 + 5) /* AA:BB:CC:DD:EE:FF */

static int is_mac_str_valid(char* mac_str)
{
    int i = 0;
    char ch = 0;
    if(mac_str == NULL)
    {
        return -1;
    }
    if (strlen(mac_str) != MAC_STR_LEN){
        APR_QLOGE("%s.%d Invalid MAC Address: [%s]!\n",TAG_STR, __LINE__, mac_str);
        return 0;
    }

    for (i = 0; i < MAC_STR_LEN; i++){
        ch = mac_str[i];
        if ((i + 1) % 3){
            if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'f') && !(ch >= 'A' && ch <= 'F')){
                APR_QLOGE("%s.%d Invalid MAC Address: [%s]!\n",TAG_STR, __LINE__, mac_str);
                return 0;
            }
        }else{
            if (ch != ':' && i + 1 != MAC_STR_LEN){
                APR_QLOGE("%s.%d Invalid MAC Address: [%s]!\n",TAG_STR, __LINE__, mac_str);
                return 0;
            }
        }
    }

    return 1;
}

static FILE * install_file(char *file_name)
{
	FILE *fp = fopen(file_name, "r");
	if(fp == NULL)
	{
		APR_QLOGE("%s:open %s failed\n",TAG_STR, file_name);
		return NULL;
	}
	APR_QLOGD("%s:open %s success\n",TAG_STR, file_name);
    return fp;
}

static void uninstall_file(FILE *fp, char *file_name)
{
	if(fp != NULL)
	{
        fflush(fp);
		fclose(fp);
		fp = NULL;
		APR_QLOGD("%s:close %s success\n",TAG_STR, file_name);
	}
	return;
}

/*
pop a item from arp_list file when this is not NULL or eof
the item is valid when ip_addr && mac_addr not NULL;
the item is invalid when ip_addr && mac_addr is NULL;
*/
static void pop_arp_list_item(FILE *fp, char **ip_addr, char **mac_addr)
{
	if(fp == NULL)
	{
		*ip_addr = NULL;
		*mac_addr = NULL;
        APR_QLOGD("%s.%d:arp_list_fp == NULL\n",TAG_STR, __LINE__);
		return;
	}
    static char one_line[128];	//arp list item less 128 byte
    char *delim = " ", *delim1 = "\n";
    //pop a arp item from arp_list file.
	if(!feof(fp) && fgets(one_line, sizeof(one_line), fp) != NULL)
	{
		// APR_QLOGD("%s:%s\n",TAG_STR, one_line);
        *ip_addr = strtok(one_line, delim);
        *mac_addr = strtok(strtok(NULL, delim), delim1);
	}
	else
	{
        APR_QLOGD("%s.%d:is end file\n",TAG_STR, __LINE__);
		*ip_addr = NULL;
		*mac_addr = NULL;
	}
}

void arp_list_sync(FILE *arp_list_fp)
{
    char *ip_addr = NULL;
    char *mac_addr = NULL;
    char run_cmd[128] = {0};
    do
    {
        pop_arp_list_item(arp_list_fp, &ip_addr, &mac_addr);
        if(ip_addr == NULL || mac_addr == NULL || is_mac_str_valid(mac_addr) != 1)
        {
            break;
        }
        //set to PERM arp list(system)
        APR_QLOGD("set %s %sto PERM arp\r\n\n", ip_addr, mac_addr);
        snprintf(run_cmd,sizeof(run_cmd),"arp -s %s %s",ip_addr, mac_addr);
        // snprintf(run_cmd,sizeof(run_cmd),"ip neigh add %s lladdr %s",ip_addr, mac_addr);
        system(run_cmd);
        ip_addr = NULL;
        mac_addr = NULL;
    }while(1);
}

bool if_allow_arp_sync(FILE **arp_list_fp, FILE **allow_arp_sync_fp)
{
    *arp_list_fp = install_file(ARP_LIST_FILE);
    *allow_arp_sync_fp = install_file(ALLOW_ARP_SYNC_FILE);
    if(*arp_list_fp == NULL || *allow_arp_sync_fp == NULL)
    {
        if(*arp_list_fp != NULL)
            uninstall_file(*arp_list_fp, ARP_LIST_FILE);
        if(*allow_arp_sync_fp != NULL)
            uninstall_file(*allow_arp_sync_fp, ALLOW_ARP_SYNC_FILE);
        APR_QLOGE("%s.%d:arp_list_fp == NULL or allow_arp_sync_fp ==  NULL\n",TAG_STR, __LINE__);
        return false;
    }
    return true;
}

void arp_list_init(void)
{
	FILE *arp_list_fp = NULL, *allow_arp_sync_fp = NULL;
    if(if_allow_arp_sync(&arp_list_fp, &allow_arp_sync_fp) == false)
    {
        APR_QLOGE("%s.%d:system do not allow arp sync |break\n",TAG_STR, __LINE__);
        return;
    }
    //pop all arp_list file item and set per arp item to PERM(note the arp_list_fp will move to eof).
    arp_list_sync(arp_list_fp);
    uninstall_file(allow_arp_sync_fp, ALLOW_ARP_SYNC_FILE);
    uninstall_file(arp_list_fp, ARP_LIST_FILE);
}

int main()
{
    arp_list_init();
    char run_cmd[128] = {0};
    snprintf(run_cmd,sizeof(run_cmd),"arp -a | awk '{print $2,$4}' | sed 's/(//g' | sed 's/)//g' | sed  '/<incomplete>/d' > %s", ARP_LIST_FILE);
    // snprintf(run_cmd,sizeof(run_cmd),"ip neigh | awk '{print $1,$5}' | sed 's/(//g' | sed 's/)//g' > %s", ARP_LIST_FILE);
    while (1)
    {
        FILE *arp_list_fp = NULL, *allow_arp_sync_fp = NULL;
        sleep(1);
        system(run_cmd);    //update arp cache to arp_list file.
        if(if_allow_arp_sync(&arp_list_fp, &allow_arp_sync_fp) == false)
        {
            APR_QLOGE("%s.%d:system do not allow arp sync |continue\n",TAG_STR, __LINE__);
            continue;
        }
        else
        {
            //pop all arp_list file item and set per arp item to PERM(note the arp_list_fp will move to eof).
            arp_list_sync(arp_list_fp);

            uninstall_file(allow_arp_sync_fp, ALLOW_ARP_SYNC_FILE);
            uninstall_file(arp_list_fp, ARP_LIST_FILE);
        }
    }

    return 0;
}

