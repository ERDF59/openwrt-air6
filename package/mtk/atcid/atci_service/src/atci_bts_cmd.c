#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>

#include "atci_service.h"
#include "atci_util.h"
#include "atci_bts_cmd.h"
#include <errno.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <linux/input.h>
#include <math.h>
/******************************************************************************
 * Structure
 *****************************************************************************/

typedef struct
{
	char bts_sensor_name[32];
	float bts_temperature;
} ftm_ata_bts;

typedef struct
{
	int num; //number of return data
	ftm_ata_bts bts[5];
} ftm_ata_bts_num;

typedef struct{
	ftm_ata_bts_num nbts;
} sp_ata_data;

sp_ata_data return_data;

struct bts_priv
{
    char            *dev;
    int             fd;
    char            tz0[32];
    char            tz1[32];
    char            tz2[32];
    char            tz3[32];
    char            tz4[32];
    char            tz5[32];
    char            tz6[32];
    char            tz7[32];
    char            tz8[32];
    char            tz9[32];
    char            tz10[32];
    char            tz11[32];
    char            tz12[32];
    char            tz13[32];
    char            tz14[32];
    char            tz15[32];
    char            tz16[32];
    char            tz17[32];
    char            tz18[32];
    char            tz19[32];
    char            tz20[32];
    char            tz21[32];
    char            tz22[32];
    char            tz23[32];

    int             tz0_temp;
    int             tz1_temp;
    int             tz2_temp;
    int             tz3_temp;
    int             tz4_temp;
    int             tz5_temp;
    int             tz6_temp;
    int             tz7_temp;
    int             tz8_temp;
    int             tz9_temp;
    int             tz10_temp;
    int             tz11_temp;
    int             tz12_temp;
    int             tz13_temp;
    int             tz14_temp;
    int             tz15_temp;
    int             tz16_temp;
    int             tz17_temp;
    int             tz18_temp;
    int             tz19_temp;
    int             tz20_temp;
    int             tz21_temp;
    int             tz22_temp;
    int             tz23_temp;
};

static struct bts_priv bts_data;

/******************************************************************************
 * Functions
 *****************************************************************************/

static int
read_int(char const* path)
{
    int fd;
    int ret = -1;

    if (path == NULL)
    	return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
    	char buffer[20];
    	int len = read(fd, buffer, 19);
		if (len < 0) {
            close(fd);
            return -1;
		}

    	if (sscanf(buffer, "%d", &ret))
    	{
    	    close(fd);
    	    // success
    	    return ret;
    	}
    	close(fd);
    	return -1;
    }
    return -errno;
}

static int
read_str(char const* path, char *buf, unsigned int size)
{
    int fd, ret;

    if (path == NULL)
    	return -1;

    if (buf == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
		char buffer[32] = {0};

		if (size > 32)
			size = 32;

    	int len = read(fd, buffer, size-1);
		if (len < 0) {
            close(fd);
            return -1;
		}
        buffer[size-1] = '\0';

        ret = snprintf(buf, size, "%s", buffer);
		if (ret < 0) {
			ALOGD("%s: sprintf fail,ret=%d\n", __FUNCTION__, ret);
		}

    	close(fd);
    	return -1;
    }
    else
    {
        ret = snprintf(buf, size, "not exist\n");
		if (ret < 0) {
			ALOGD("%s: sprintf fail,ret=%d\n", __FUNCTION__, ret);
		}
    }
    return -errno;
}

static int bts_init_priv(struct bts_priv *bts)
{
    memset(bts, 0x00, sizeof(*bts));
    bts->fd = -1;
    bts->dev = "/proc/mtktsAP/mtkts_AP_param";
    return 0;
}

static int bts_close(struct bts_priv *bts)
{
    //unsigned int flags = 0;
    //int err = 0;
    if (bts->fd != -1) {
        bts->fd = -1;
    }
    bts_init_priv(bts);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int bts_update_info(struct bts_priv *bts)
{

    bts->tz0_temp = read_int("/sys/class/thermal/thermal_zone0/temp");
    bts->tz1_temp = read_int("/sys/class/thermal/thermal_zone1/temp");
    bts->tz2_temp = read_int("/sys/class/thermal/thermal_zone2/temp");
    bts->tz3_temp = read_int("/sys/class/thermal/thermal_zone3/temp");
    bts->tz4_temp = read_int("/sys/class/thermal/thermal_zone4/temp");
    bts->tz5_temp = read_int("/sys/class/thermal/thermal_zone5/temp");
    bts->tz6_temp = read_int("/sys/class/thermal/thermal_zone6/temp");
    bts->tz7_temp = read_int("/sys/class/thermal/thermal_zone7/temp");
    bts->tz8_temp = read_int("/sys/class/thermal/thermal_zone8/temp");
    bts->tz9_temp = read_int("/sys/class/thermal/thermal_zone9/temp");
    bts->tz10_temp = read_int("/sys/class/thermal/thermal_zone10/temp");
    bts->tz11_temp = read_int("/sys/class/thermal/thermal_zone11/temp");
    bts->tz12_temp = read_int("/sys/class/thermal/thermal_zone12/temp");
    bts->tz13_temp = read_int("/sys/class/thermal/thermal_zone13/temp");
    bts->tz14_temp = read_int("/sys/class/thermal/thermal_zone14/temp");
    bts->tz15_temp = read_int("/sys/class/thermal/thermal_zone15/temp");
    bts->tz16_temp = read_int("/sys/class/thermal/thermal_zone16/temp");
    bts->tz17_temp = read_int("/sys/class/thermal/thermal_zone17/temp");
    bts->tz18_temp = read_int("/sys/class/thermal/thermal_zone18/temp");
    bts->tz19_temp = read_int("/sys/class/thermal/thermal_zone19/temp");
    bts->tz20_temp = read_int("/sys/class/thermal/thermal_zone20/temp");
    bts->tz21_temp = read_int("/sys/class/thermal/thermal_zone21/temp");
    bts->tz22_temp = read_int("/sys/class/thermal/thermal_zone22/temp");
    bts->tz23_temp = read_int("/sys/class/thermal/thermal_zone23/temp");

    return 0;
}

int findKeyIndex(struct bts_priv *bts, char *key)
{
	int len = strlen(key);
	ALOGD("s: %s, l: %d\n", key, len);

	if(strncmp(bts->tz0, key, len) == 0)
		return 0;
	if(strncmp(bts->tz1, key, len) == 0)
		return 1;
	if(strncmp(bts->tz2, key, len) == 0)
		return 2;
	if(strncmp(bts->tz3, key, len) == 0)
		return 3;
	if(strncmp(bts->tz4, key, len) == 0)
		return 4;
	if(strncmp(bts->tz5, key, len) == 0)
		return 5;
	if(strncmp(bts->tz6, key, len) == 0)
		return 6;
	if(strncmp(bts->tz7, key, len) == 0)
		return 7;
	if(strncmp(bts->tz8, key, len) == 0)
		return 8;
	if(strncmp(bts->tz9, key, len) == 0)
		return 9;
	if(strncmp(bts->tz10, key, len) == 0)
		return 10;
	if(strncmp(bts->tz11, key, len) == 0)
		return 11;
	if(strncmp(bts->tz12, key, len) == 0)
		return 12;
	if(strncmp(bts->tz13, key, len) == 0)
		return 13;
	if(strncmp(bts->tz14, key, len) == 0)
		return 14;
	if(strncmp(bts->tz15, key, len) == 0)
		return 15;
	if(strncmp(bts->tz16, key, len) == 0)
		return 15;
	if(strncmp(bts->tz16, key, len) == 0)
		return 16;
	if(strncmp(bts->tz17, key, len) == 0)
		return 17;
	if(strncmp(bts->tz18, key, len) == 0)
		return 18;
	if(strncmp(bts->tz19, key, len) == 0)
		return 19;
	if(strncmp(bts->tz20, key, len) == 0)
		return 20;
	if(strncmp(bts->tz21, key, len) == 0)
		return 21;
	if(strncmp(bts->tz22, key, len) == 0)
		return 22;
	if(strncmp(bts->tz23, key, len) == 0)
		return 23;

	return -1;
}

float getIndexTemp(struct bts_priv *bts, int index)
{
	switch(index){
		case 0:
			return bts->tz0_temp / 1000;
			break;
		case 1:
			return bts->tz1_temp / 1000;
			break;
		case 2:
			return bts->tz2_temp / 1000;
			break;
		case 3:
			return bts->tz3_temp / 1000;
			break;
		case 4:
			return bts->tz4_temp / 1000;
			break;
		case 5:
			return bts->tz5_temp / 1000;
			break;
		case 6:
			return bts->tz6_temp / 1000;
			break;
		case 7:
			return bts->tz7_temp / 1000;
			break;
		case 8:
			return bts->tz8_temp / 1000;
			break;
		case 9:
			return bts->tz9_temp / 1000;
			break;
		case 10:
			return bts->tz10_temp / 1000;
			break;
		case 11:
			return bts->tz11_temp / 1000;
			break;
		case 12:
			return bts->tz12_temp / 1000;
			break;
		case 13:
			return bts->tz13_temp / 1000;
			break;
		case 14:
			return bts->tz14_temp / 1000;
			break;
		case 15:
			return bts->tz15_temp / 1000;
			break;
		case 16:
			return bts->tz16_temp / 1000;
			break;
		case 17:
			return bts->tz17_temp / 1000;
			break;
		case 18:
			return bts->tz18_temp / 1000;
			break;
		case 19:
			return bts->tz19_temp / 1000;
			break;
		case 20:
			return bts->tz20_temp / 1000;
			break;
		case 21:
			return bts->tz21_temp / 1000;
			break;
		case 22:
			return bts->tz22_temp / 1000;
			break;
		case 23:
			return bts->tz23_temp / 1000;
			break;
		default:
			return -1;
	}
}
/*---------------------------------------------------------------------------*/
static int bts_update(char* cmdline)
{
    struct bts_priv *bts = &bts_data;
    int err = 0, len = 0, ret = 0;
    char i = 0;
    int ataKey[5] = {0};

    ALOGD("%s: Start\n", __FUNCTION__);
    memset(&bts_data, 0, sizeof(bts_data));
    read_str("/sys/class/thermal/thermal_zone0/type", &(bts->tz0[0]), 20);
    read_str("/sys/class/thermal/thermal_zone1/type", &(bts->tz1[0]), 20);
    read_str("/sys/class/thermal/thermal_zone2/type", &(bts->tz2[0]), 20);
    read_str("/sys/class/thermal/thermal_zone3/type", &(bts->tz3[0]), 20);
    read_str("/sys/class/thermal/thermal_zone4/type", &(bts->tz4[0]), 20);
    read_str("/sys/class/thermal/thermal_zone5/type", &(bts->tz5[0]), 20);
    read_str("/sys/class/thermal/thermal_zone6/type", &(bts->tz6[0]), 20);
    read_str("/sys/class/thermal/thermal_zone7/type", &(bts->tz7[0]), 20);
    read_str("/sys/class/thermal/thermal_zone8/type", &(bts->tz8[0]), 20);
    read_str("/sys/class/thermal/thermal_zone9/type", &(bts->tz9[0]), 20);
    read_str("/sys/class/thermal/thermal_zone10/type", &(bts->tz10[0]), 20);
    read_str("/sys/class/thermal/thermal_zone11/type", &(bts->tz11[0]), 20);
    read_str("/sys/class/thermal/thermal_zone12/type", &(bts->tz12[0]), 20);
    read_str("/sys/class/thermal/thermal_zone13/type", &(bts->tz13[0]), 20);
    read_str("/sys/class/thermal/thermal_zone14/type", &(bts->tz14[0]), 20);
    read_str("/sys/class/thermal/thermal_zone15/type", &(bts->tz15[0]), 20);
    read_str("/sys/class/thermal/thermal_zone16/type", &(bts->tz16[0]), 20);
    read_str("/sys/class/thermal/thermal_zone17/type", &(bts->tz17[0]), 20);
    read_str("/sys/class/thermal/thermal_zone18/type", &(bts->tz18[0]), 20);
    read_str("/sys/class/thermal/thermal_zone19/type", &(bts->tz19[0]), 20);
    read_str("/sys/class/thermal/thermal_zone20/type", &(bts->tz20[0]), 20);
    read_str("/sys/class/thermal/thermal_zone21/type", &(bts->tz21[0]), 20);
    read_str("/sys/class/thermal/thermal_zone22/type", &(bts->tz22[0]), 20);
    read_str("/sys/class/thermal/thermal_zone23/type", &(bts->tz23[0]), 20);

    ataKey[0] = findKeyIndex(bts, "soc_dram_ntc");
    ataKey[1] = findKeyIndex(bts, "ltepa_ntc");
    ataKey[2] = findKeyIndex(bts, "nrpa_ntc");
    ataKey[3] = findKeyIndex(bts, "rf_ntc");
    if (ataKey[3] == -1)
        ataKey[3] = findKeyIndex(bts, "ambient_ntc");

/*did not find ntc thermal zone*/
    if ((ataKey[0] == -1) || (ataKey[1] == -1) || (ataKey[2] == -1) || (ataKey[3] == -1))
        return -1;

    return_data.nbts.num = 4;
    ret = snprintf(return_data.nbts.bts[0].bts_sensor_name,
			sizeof(return_data.nbts.bts[0].bts_sensor_name), "%13s", "soc_dram_ntc");
	if (ret < 0) {
		ALOGD("%s: snprintf fail,ret=%d\n", __FUNCTION__, ret);
	}
    ret = snprintf(return_data.nbts.bts[1].bts_sensor_name,
			sizeof(return_data.nbts.bts[1].bts_sensor_name), "%10s", "ltepa_ntc");
	if (ret < 0) {
		ALOGD("%s: snprintf fail,ret=%d\n", __FUNCTION__, ret);
	}
    ret = snprintf(return_data.nbts.bts[2].bts_sensor_name,
			sizeof(return_data.nbts.bts[2].bts_sensor_name), "%9s", "nrpa_ntc");
	if (ret < 0) {
		ALOGD("%s: snprintf fail,ret=%d\n", __FUNCTION__, ret);
	}
    if (findKeyIndex(bts, "rf_ntc") == -1) {
        ret = snprintf(return_data.nbts.bts[3].bts_sensor_name,
				sizeof(return_data.nbts.bts[3].bts_sensor_name), "%12s", "ambient_ntc");
		if (ret < 0) {
			ALOGD("%s: snprintf fail,ret=%d\n", __FUNCTION__, ret);
		}
	}
    else {
        ret = snprintf(return_data.nbts.bts[3].bts_sensor_name,
				sizeof(return_data.nbts.bts[3].bts_sensor_name), "%7s", "rf_ntc");
		if (ret < 0) {
			ALOGD("%s: snprintf fail,ret=%d\n", __FUNCTION__, ret);
		}
	}

    err = bts_update_info(bts);
    return_data.nbts.bts[0].bts_temperature = getIndexTemp(bts, ataKey[0]);
    return_data.nbts.bts[1].bts_temperature = getIndexTemp(bts, ataKey[1]);
    return_data.nbts.bts[2].bts_temperature = getIndexTemp(bts, ataKey[2]);
    return_data.nbts.bts[3].bts_temperature = getIndexTemp(bts, ataKey[3]);
    ALOGD("%s: %d %f, %s: %d %f, %s: %d %f, %s: %d %f\n", return_data.nbts.bts[0].bts_sensor_name, ataKey[0], return_data.nbts.bts[0].bts_temperature,
    return_data.nbts.bts[1].bts_sensor_name, ataKey[1], return_data.nbts.bts[1].bts_temperature,
    return_data.nbts.bts[2].bts_sensor_name, ataKey[2], return_data.nbts.bts[2].bts_temperature,
    return_data.nbts.bts[3].bts_sensor_name, ataKey[3], return_data.nbts.bts[3].bts_temperature);

    bts_close(bts);
    ALOGD("%s: Exit\n", __FUNCTION__);
    return 0;
}
/*---------------------------------------------------------------------------*/

#define RESPONSE_SIZE 1024
int bts_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int ret = 0, result;

    ALOGD("bts cmdline=%s, at_op=%d, \n", cmdline, at_op);

    switch(at_op){
        case AT_READ_OP:
            ret = bts_update(cmdline);
            if (ret < 0)
            {
                result = snprintf(response, RESPONSE_SIZE, "\r\n+BTS: ERROR\r\n");
				if (result < 0) {
					ALOGD("%s: sprintf fail,ret=%d\n", __FUNCTION__, result);
				}
                break;
            }
            result = snprintf(response, RESPONSE_SIZE, "\r\n+BTS: <%d>,<%s>,<%f>,<%s>,<%f>,<%s>,<%f>,<%s>,<%f>\r\nOK\r\n",
                return_data.nbts.num,
                return_data.nbts.bts[0].bts_sensor_name,
                return_data.nbts.bts[0].bts_temperature,
                return_data.nbts.bts[1].bts_sensor_name,
                return_data.nbts.bts[1].bts_temperature,
                return_data.nbts.bts[2].bts_sensor_name,
                return_data.nbts.bts[2].bts_temperature,
                return_data.nbts.bts[3].bts_sensor_name,
                return_data.nbts.bts[3].bts_temperature);
			if (result < 0) {
				ALOGD("%s: sprintf fail,ret=%d\n", __FUNCTION__, result);
			}
            break;
        case AT_ACTION_OP:
        case AT_TEST_OP:
        case AT_SET_OP:
            break;
    default:
        break;
    }

    return 0;
}

