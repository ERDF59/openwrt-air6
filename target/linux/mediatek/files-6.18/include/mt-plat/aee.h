#ifndef __MT_PLAT_AEE_H
#define __MT_PLAT_AEE_H
#include <linux/kernel.h>
static inline void aee_kernel_warning(const char *mod, const char *fmt, ...)
{
	pr_warn("%s: %s\n", mod, fmt);
}
#endif
