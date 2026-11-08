#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>

static int tries;
static bool logged;

static void vsim2_off_work(struct work_struct *work);

static DECLARE_DELAYED_WORK(vsim2_off_wq, vsim2_off_work);

static void vsim2_off_work(struct work_struct *work)
{
	struct regulator *r;
	int ret;

	r = regulator_get(NULL, "vsim2");
	if (IS_ERR(r)) {
		long err = PTR_ERR(r);

		if ((err == -EPROBE_DEFER || err == -ENODEV) && tries++ < 200) {
			schedule_delayed_work(&vsim2_off_wq,
					      msecs_to_jiffies(300));
			return;
		}
		if (!logged) {
			pr_err("vsim2-off: regulator_get failed: %ld\n", err);
			logged = true;
		}
	} else {
		ret = regulator_enable(r);
		if (!ret && !regulator_disable(r) && !logged) {
			pr_info("vsim2-off: vsim2 held off\n");
			logged = true;
		}
		regulator_put(r);
	}

	schedule_delayed_work(&vsim2_off_wq, msecs_to_jiffies(2000));
}

static int __init vsim2_off_init(void)
{
	schedule_delayed_work(&vsim2_off_wq, msecs_to_jiffies(2000));
	return 0;
}
late_initcall(vsim2_off_init);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Keep MT6361 vsim2 (eSIM) rail disabled");
