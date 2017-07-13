#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/inventec/common/inventec_sysfs.h>

extern int psus_control(void);
extern int fans_control(void);
extern int thread_control(void);
extern void thread_control_set(int val);

#define THREAD_SLEEP_MINS	(3)
#define THREAD_DELAY_MINS	(THREAD_SLEEP_MINS + THREAD_SLEEP_MINS + 1)

extern void psu_get_voltin(void);

static struct task_struct *thread_st;
static int thread_data;

// Function executed by kernel thread
static int thread_fn(void *unused)
{
    /* Delay for guarantee HW ready */
    ssleep(THREAD_DELAY_MINS);
    /* Default status init */
    status_led_change(STATUS_LED_RED_PATH, "0", STATUS_LED_GRN_PATH, "7");

    psu_get_voltin();

#ifdef YOCTO_REDWOOD
    led_set_gpio_to_change_status_led();
#endif

    while (1)
    {
	ssleep(THREAD_SLEEP_MINS);

	if (thread_control() == 0) {
	    printk(KERN_INFO "%s/%d: Thread Stop by inventec_thread control\n",__func__,__LINE__);
	    break;
	}

	if (status_led_check_diag_mode() == STATUS_LED_MODE_MANU) {
            /* status led in change color/freq mode, higher priority. Ignore fans sttaus */
            continue;
	}

	if (fans_control() == 1) {
	    continue;
	}
	else
	if (psus_control() == 1) {
	    continue;
	}

	if (status_led_check_color() != STATUS_LED_GRN7) {      /* status led red, change it to green */
	    status_led_change(STATUS_LED_RED_PATH, "0", STATUS_LED_GRN_PATH, "7");
	}
    }

    do_exit(0);
    printk(KERN_INFO "%s/%d: Thread Stopped\n",__func__,__LINE__);
    return 0;
}

static int __init inventec_thread_init(void)
{
    thread_control_set(1);

    printk(KERN_INFO "%s/%d: Creating Thread\n",__func__,__LINE__);
    //Create the kernel thread with name 'inventec_thread'
    thread_st = kthread_run(thread_fn, (void*)&thread_data, "inventec_thread");
    if (thread_st)
        printk(KERN_INFO "Thread Created successfully\n");
    else
        printk(KERN_ERR "Thread creation failed\n");

    return 0;
}

static void __exit inventec_thread_exit(void)
{
    thread_control_set(0);
    /* Delay for guarantee thread exit */
    ssleep(THREAD_DELAY_MINS);
    printk(KERN_INFO "Thread cleaning Up\n");
}

module_init(inventec_thread_init);
module_exit(inventec_thread_exit);

MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);
