#ifndef __ATCID_ASYNC_NOTIFY_H__
#define __ATCID_ASYNC_NOTIFY_H__


typedef struct _atcid_async_notify_t {
    char *rsp_string;
    size_t rsp_length;
    struct _atcid_async_notify_t *next;
}atcid_async_notify_t;

static atcid_async_notify_t *g_async_list = NULL;
static uint16_t g_async_list_node_count;
#define ASYNC_LIST_NODE_MAX_COUNT   300

void delayed_async_print(const char *message, size_t rsp_length);
void delayed_async_output_list(void);
void atcid_async_notify_init(void);
bool get_at_cmd_in_progress();
void set_at_in_process(bool state);
#endif
