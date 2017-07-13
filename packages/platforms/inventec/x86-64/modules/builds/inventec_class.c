#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/inventec/common/inventec_sysfs.h>

static struct class *inventec_class_p = NULL;
static struct mutex rw_lock;

struct class *inventec_get_class(void)
{
	return inventec_class_p;
}
EXPORT_SYMBOL(inventec_get_class);

/*access userspace data to kernel space*/
enum {
        ACC_R,
        ACC_W,
        MAX_ACC_SIZE = 256,
        MAX_READ_SIZE = 256,
};

static ssize_t access_user_space(const char *name, int mode, char *buf, size_t len)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
	char *mark = NULL;
	ssize_t vfs_ret = 0;

	/*len max value is MAX_ACC_SIZE - 1 */
	if (len >= MAX_ACC_SIZE)
		len = MAX_ACC_SIZE - 1;

	if (mode == ACC_R) {
		fp = filp_open(name, O_RDONLY, S_IRUGO);
		if (IS_ERR(fp))
			return -ENODEV;

		fs = get_fs();
		set_fs(KERNEL_DS);

		pos = 0;
		vfs_ret = vfs_read(fp, buf, len, &pos);

		mark = strpbrk(buf, "\n");
		if (mark)
			*mark = '\0';

		filp_close(fp, NULL);
		set_fs(fs);
	} else if (mode == ACC_W) {
		fp = filp_open(name, O_WRONLY, S_IWUSR | S_IRUGO);
		if (IS_ERR(fp))
			return -ENODEV;

		fs = get_fs();
		set_fs(KERNEL_DS);

		pos = 0;
		vfs_ret = vfs_write(fp, buf, len, &pos);
		filp_close(fp, NULL);
		set_fs(fs);
	}

	return vfs_ret;
}

/* read userspace data to kernel space from a position */
static int read_user_space(const char *name, int mode, char *buf, size_t len, int loc)
{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos = loc;
	char *mark = NULL;

	/*len max value is MAX_ACC_SIZE - 1 */
	if (len >= MAX_READ_SIZE)
		len = MAX_READ_SIZE - 1;

	if (mode == ACC_R) {
		fp = filp_open(name, O_RDONLY, S_IRUGO);
		if (IS_ERR(fp))
			return -ENODEV;

		fs = get_fs();
		set_fs(KERNEL_DS);

		vfs_read(fp, buf, len, &pos);

		mark = strpbrk(buf, "\n");
		if (mark)
			*mark = '\0';

		filp_close(fp, NULL);
		set_fs(fs);
		return 0;
	}
	return -1;
}


int inventec_strtol(const char *sbufp, char **endp, unsigned int base)
{
    char *endptr;
    int value = simple_strtol(sbufp, &endptr, base);
    if (value == 0 && sbufp == endptr) {
	*endp = NULL;
	return value;
    }
    *endp = (char*)1;
    return value;
}
EXPORT_SYMBOL(inventec_strtol);

int inventec_store_input(char *inputp, int count)
{
        int i = 0;
        while(inputp[i] != '\n' && inputp[i] != '\0' && i < count) {
                i++;
        }
        inputp[i] = '\0';
        return strlen(inputp);
}
EXPORT_SYMBOL(inventec_store_input);

/*
 * Time stamps for kernel log on yocto
 */
#include <linux/rtc.h>
static ssize_t inventec_tmstmp(char *ts)
{
    struct timeval time;
    unsigned long local_time;
    struct rtc_time tm;

    do_gettimeofday(&time);
    local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
    rtc_time_to_tm(local_time, &tm);

    return sprintf(ts, "UTC %04d-%02d-%02d %02d:%02d:%02d: ", tm.tm_year + 1900, tm.tm_mon + 1,
                                                   tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void SYSFS_LOG(char *fmt, ...)
{
        char buf[80], ts[32];
	va_list args;
	int hlen;

	inventec_tmstmp(&ts[0]);
	hlen = sprintf(buf, "[SYSFS] %s ", ts);	// Do not edit this line

        va_start(args, fmt);
        vsprintf(&buf[hlen-1], fmt, args);
        va_end(args);
        printk(KERN_WARNING "%s\n", buf);
}
EXPORT_SYMBOL(SYSFS_LOG);

ssize_t
inventec_show_attr(char *buf_p, const char *invdevp)
{
    int  inv_len = MAX_ACC_SIZE;	/* INV driver return max length      */
    char tmp_buf[MAX_ACC_SIZE];
    char *str_negative = "-", *mark = NULL;

    /* [Step2] Get data by uaccess */
    memset(tmp_buf, 0, sizeof(tmp_buf));
    mutex_lock(&rw_lock);
    if (access_user_space(invdevp, ACC_R, tmp_buf, inv_len) < 0) {
        /* u_access fail */
        mutex_unlock(&rw_lock);
        return sprintf(tmp_buf, "%s\n", SHOW_ATTR_WARNING);
    }
    mutex_unlock(&rw_lock);

    /* [Step3] Check return value
     * - Ex: When transceiver not plugged
     *   => SWPS return error code "-202"
     *   => Pic8 need return "NA" (assume)
     */
    if (strcspn(tmp_buf, str_negative) == 0) {
        /* error case: <ex> "-202" */
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* OK case:*/
    mark = strpbrk(tmp_buf, "\n");
    if (mark) { *mark = '\0'; }

    return sprintf(buf_p, "%s\n", tmp_buf);
}
EXPORT_SYMBOL(inventec_show_attr);

ssize_t
inventec_show_attr_pos(char *buf_p, char *invdevp, int pos)
{
    int  inv_len = MAX_READ_SIZE;	/* INV driver return max length      */
    char tmp_buf[inv_len];
    char *str_negative = "-";

    /* [Step2] Get data by uaccess */
    memset(tmp_buf, 0, sizeof(tmp_buf));
    mutex_lock(&rw_lock);
    if (read_user_space(invdevp, ACC_R, tmp_buf, inv_len, pos) < 0) {
        /* u_access fail */
        mutex_unlock(&rw_lock);
        return sprintf(tmp_buf, "%s\n", SHOW_ATTR_WARNING);
    }
    mutex_unlock(&rw_lock);

    /* [Step3] Check return value
     * - Ex: When transceiver not plugged
     *   => SWPS return error code "-202"
     *   => Pic8 need return "NA" (assume)
     */
    if (strcspn(tmp_buf, str_negative) == 0) {
        /* error case: <ex> "-202" */
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* OK case:*/
    return sprintf(buf_p, "%s\n", tmp_buf);
}
EXPORT_SYMBOL(inventec_show_attr_pos);

ssize_t
inventec_show_bin_attr(char *buf_p, char *invdevp, int pos)
{
    int  inv_len = MAX_READ_SIZE;	/* INV driver return max length      */
    char tmp_buf[inv_len];
    char *str_negative = "-";

    /* [Step2] Get data by uaccess */
    memset(tmp_buf, 0, sizeof(tmp_buf));
    mutex_lock(&rw_lock);
    if (read_user_space(invdevp, ACC_R, tmp_buf, inv_len, pos) < 0) {
        /* u_access fail */
        mutex_unlock(&rw_lock);
        return sprintf(tmp_buf, "%s\n", SHOW_ATTR_WARNING);
    }
    mutex_unlock(&rw_lock);

    /* [Step3] Check return value
     * - Ex: When transceiver not plugged
     *   => SWPS return error code "-202"
     *   => Pic8 need return "NA" (assume)
     */
    if (strcspn(tmp_buf, str_negative) == 0) {
        /* error case: <ex> "-202" */
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* OK case:*/
    memcpy(buf_p, tmp_buf, 256);
    return 0;
}
EXPORT_SYMBOL(inventec_show_bin_attr);

ssize_t
inventec_store_attr(const char *buf_p, size_t count, const char *invdevp)
{
    ssize_t ret = 0;

    /* [Step2] Get data by uaccess */
    mutex_lock(&rw_lock);
    if ((ret = access_user_space(invdevp, ACC_W, (char*)buf_p, count)) < 0) {
        /* u_access fail */
        mutex_unlock(&rw_lock);
        return -EINVAL;
    }
    mutex_unlock(&rw_lock);

    /* OK case:*/
    return ret;
}
EXPORT_SYMBOL(inventec_store_attr);

static int
inventec_add_dev_tracking(struct device *dev, dev_t devt,
			dev_child_tracking_t *tracking)
{
    dev_child_tracking_t *node;
    dev_child_tracking_t *curr;
    
    node = kzalloc(sizeof(*node), GFP_KERNEL);
    if (!node) {
        return -1;
    }
    node->devt = devt;
    node->dev  = dev;
    node->next = NULL;
    curr = tracking;
    if (curr) {
        while (curr->next) {
            curr = curr->next;
        }
    }
    curr = node;
    return 0;
}

static void 
_dev_release(struct device *dev){
    pr_debug("device: '%s': %s\n", dev_name(dev), __func__);
    kfree(dev);
}

struct device *
inventec_child_device_create(struct device *parent,
                        dev_t devt, 
                        void *drvdata,
                        const char *fmt,
			void *tracking)
{
    struct device *dev = NULL;
    int retval = -ENODEV;
    char *err_msg = "ERR";
        
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev) {
        err_msg = "kzalloc device fail";
        goto err_inventec_child_device_create_1;
    }
    device_initialize(dev);
    dev->devt = devt;
    dev->parent = parent;
    dev->release = _dev_release;
    dev_set_name(dev, "%s", fmt);
    dev_set_drvdata(dev, drvdata);
    retval = kobject_add(&(dev->kobj), &(parent->kobj), fmt);
    if (retval < 0) {
        err_msg = "kzalloc device fail";
        goto err_inventec_child_device_create_2;
    }
    if (inventec_add_dev_tracking(dev, devt, (dev_child_tracking_t*)tracking) < 0){
        err_msg = "Add dev to tracking list fail";
        retval = -ENODEV;
        goto err_inventec_child_device_create_3;
    }
    return dev;

err_inventec_child_device_create_3:
    put_device(dev);
err_inventec_child_device_create_2:
    kfree(dev);
err_inventec_child_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return ERR_PTR(retval);
}

void
inventec_child_device_destroy(void *tracking) {
    
    dev_child_tracking_t *next = NULL;
    dev_child_tracking_t *curr = (dev_child_tracking_t*)tracking;
    
    while (curr) {
        next = curr->next;
	put_device(curr->dev);
        kfree(curr->dev);
        kfree(curr);
        curr = next;        
    }
}
EXPORT_SYMBOL(inventec_child_device_create);
EXPORT_SYMBOL(inventec_child_device_destroy);

static int __init
inventec_class_init(void)
{
    char *err_msg  = "ERR";

    /* Create class object */
    inventec_class_p = class_create(THIS_MODULE, INVENTEC_CLASS_NAME);
    if (IS_ERR(inventec_class_p)) {
        err_msg = "class_create fail!";
        goto err_inventec_class_init_1;
    }
    mutex_init(&rw_lock);
    printk(KERN_INFO "[%s/%d] Module initial success.\n",__func__,__LINE__);

    return 0;

err_inventec_class_init_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
inventec_class_exit(void)
{
    class_destroy(inventec_class_p);
    printk(KERN_INFO "[%s/%d] Remove module.\n",__func__,__LINE__);
}

module_init(inventec_class_init);
module_exit(inventec_class_exit);

MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);
