#include "atci_efuse_cmd.h"
#include "atci_service.h"
#include "atci_util.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HUID_FILE_PATH "/data/huid"

#define MAX_HRID_SIZE  256

void print_msg(char *buffer, char *msg) {
    int ret = 0;

    ret = snprintf(buffer, MAX_HRID_SIZE, "%s\r\n", msg);
    if (ret < 0)
        ALOGE("sprint function issue:%d\n", errno);
}

int read_huid_from_file(char *huid) {
    FILE *fd = NULL;
    int len = 0;

    memset(huid, 0, MAX_HRID_SIZE);
    if ((fd = fopen(HUID_FILE_PATH, "r")) == NULL) {
        ALOGE("open file: %s.error:%d\n", HUID_FILE_PATH, errno);
        print_msg(huid, "File open issue\r\n");
        return -1;
    }

    len = fread(huid, sizeof(char), MAX_HRID_SIZE, fd);
    if (len < 0) {
        ALOGE("fread error:%d\n", errno);
        print_msg(huid, "File read issue\r\n");
        if (fclose(fd))
            ALOGE("fclose error:%d\n", errno);
        return -1;
    }
    ALOGD("HUID INFO:%s\n", huid);
    if (fclose(fd))
        ALOGE("fclose error:%d\n", errno);

    return 0;
}

int efuse_huid_cmd_handler(char *cmdline, ATOP_t at_op, char *response) {
    int ret = 0;
    char huid[MAX_HRID_SIZE];

    ALOGD("EFuse cmdline=%s, at_op=%d, \n", cmdline, at_op);

    switch (at_op) {
        case AT_ACTION_OP:
        case AT_READ_OP:
            ret = read_huid_from_file(huid);
            if (!ret)
                ret = snprintf(response, 2048, "HUID: %s\r\n", huid);
            else
                ret = snprintf(response, 2048, "ERROR: %s\r\n", huid);
            if (ret < 0) {
                ALOGE("Error in sprintf:%d\n", errno);
                return -1;
            }
            ALOGD("Command is executed done\n");
            return 0;
            break;

        case AT_TEST_OP:
        case AT_SET_OP:
            break;

        default:
            break;
    }

    return 0;
}
