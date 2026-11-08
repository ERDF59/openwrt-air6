#include <stdint.h>
#include <stddef.h>
#include "quectel_atcid_sender.h"

COMM_DEV *ql_atcid_sender_init(ATCID_INIT_TYPE type)
{
	(void)type;
	return NULL;
}

void ql_atcid_sender_deinit(COMM_DEV *pcomm_dev)
{
	(void)pcomm_dev;
}

int ql_atcid_sender_sending(COMM_DEV *pcomm_dev, atcmd_sender_msg_t *atcmd_sender)
{
	(void)pcomm_dev;
	(void)atcmd_sender;
	return QUECTEL_ATCID_SENDER_ERROR;
}

int ql_atcid_sender_init_cb(AT_IND_FUNC ind_cb)
{
	(void)ind_cb;
	return QUECTEL_ATCID_SENDER_ERROR;
}
