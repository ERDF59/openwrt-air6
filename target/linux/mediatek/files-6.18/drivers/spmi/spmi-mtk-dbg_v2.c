#include <linux/irq.h>
#include <linux/device.h>
#include <linux/spmi.h>
#include <linux/module.h>
#include "spmi-mtk.h"
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MTK SPMI debug stubs");
void spmi_dump_wdt_reg(void) {}
void spmi_dump_pmif_acc_vio_reg(void) {}
void spmi_dump_pmic_acc_vio_reg(void) {}
void spmi_dump_pmif_busy_reg(void) {}
void spmi_dump_pmif_swinf_reg(void) {}
void spmi_dump_pmif_all_reg(void) {}
void spmi_dump_pmif_record_reg(void) {}
void spmi_dump_spmimst_all_reg(void) {}
int spmi_pmif_create_attr(struct device_driver *driver) { return 0; }
int spmi_pmif_dbg_init(struct spmi_controller *ctrl) { return 0; }
EXPORT_SYMBOL_GPL(spmi_dump_wdt_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmif_acc_vio_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmic_acc_vio_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmif_busy_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmif_swinf_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmif_all_reg);
EXPORT_SYMBOL_GPL(spmi_dump_pmif_record_reg);
EXPORT_SYMBOL_GPL(spmi_dump_spmimst_all_reg);
EXPORT_SYMBOL_GPL(spmi_pmif_create_attr);
EXPORT_SYMBOL_GPL(spmi_pmif_dbg_init);
