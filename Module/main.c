#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/kthread.h>

#include "main.h"
#include "kbuf.h"
#include "stat.h"





MODULE_AUTHOR("Walter Da Col");
MODULE_DESCRIPTION("Sistemi Operativi 2 - Module");
MODULE_LICENSE("GPL");

static struct miscdevice my_device;

static struct kb kb_fifo; /*	Buffer fifo	*/

static struct mutex dev_mutex; /*	Dev's mutex	*/

static wait_queue_head_t wait_r; /*	Wait Queue	*/
static wait_queue_head_t wait_w;

static struct task_struct *kt_stat_desc; /* KThread Descriptors */
static struct task_struct *kt_realloc_desc;

static struct ds dev_stat; /* Struct for statistics */

static int dev_run;

static int kt_stat(void *arg){
	int dim;
	int med,min,max;

	while (!kthread_should_stop()){
		mutex_lock(&dev_mutex);
		set_current_state(TASK_INTERRUPTIBLE);
		if (likely(dev_run)){
			dim = ds_populate(&dev_stat,&kb_fifo);
			if (dim){
				med = ds_med(&dev_stat);
				max = ds_max(&dev_stat);
				min = ds_min(&dev_stat,max);
				printk(KERN_DEBUG "** Device Stats **\n");
				printk(KERN_DEBUG "** Number of node on device: %d **\n",dim);
				printk(KERN_DEBUG "** Medium lenght of node data: %d **\n",med);
				printk(KERN_DEBUG "** Max lenght of node data: %d **\n",max);
				printk(KERN_DEBUG "** Min lenght of node data: %d **\n",min);
				ds_reset_stat(&dev_stat);
				
			} else {
				printk(KERN_DEBUG "** Device Stats **\n");
				printk(KERN_DEBUG "** No node on device **\n");

			}
			ds_add_kb_value(&dev_stat,dim);
			mutex_unlock(&dev_mutex);
			schedule_timeout(1 * HZ);
		} else {
			printk(KERN_DEBUG "** Device Stats **\n");
			printk(KERN_DEBUG "** Nothing to do **\n");
			mutex_unlock(&dev_mutex);
			schedule();
		}
	}

	return 0;
}

static int kt_realloc(void *arg){
	int per,med_occ;
	int tmp = KB_OCC;
	while (!kthread_should_stop()){
		mutex_lock(&dev_mutex);
		set_current_state(TASK_INTERRUPTIBLE);
		if (likely(dev_run)){
			per = ds_get_kb_occupation(&dev_stat);
			med_occ = my_div(dev_stat.ds_kb_sum,dev_stat.ds_kb_dim);
			if (per!= tmp){
				printk(KERN_DEBUG "** Device Reallocator **\n");
				printk(KERN_DEBUG "** Actual buffer occupation: %d (percento) **\n",per);
				ds_set_kb_occupation(&dev_stat,med_occ);
				printk(KERN_DEBUG "** New Buffer Max size: %d **\n",dev_stat.ds_kb_max);
				ds_reset_alloc(&dev_stat);
			} else {
				printk(KERN_DEBUG "** Device Reallocator **\n");
				printk(KERN_DEBUG "** No reallocation needed **\n");
			}
			mutex_unlock(&dev_mutex);
			schedule_timeout(3 * HZ);
		} else {
			printk(KERN_DEBUG "** Device Reallocator **\n");
			printk(KERN_DEBUG "** Nothing to do **\n");
			mutex_unlock(&dev_mutex);
			schedule();
		}
	}
	return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
	mutex_lock(&dev_mutex);
	printk(KERN_DEBUG "**Device Open\n");
	dev_run++;
	if (dev_run == 1){
		wake_up_process(kt_stat_desc);
		wake_up_process(kt_realloc_desc);
	}
	mutex_unlock(&dev_mutex);
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
	mutex_lock(&dev_mutex);
	printk(KERN_DEBUG "**Device Close\n");
	dev_run--;
	mutex_unlock(&dev_mutex);
	return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t dim, loff_t *ppos)
{
	int res;
	char* value;

	mutex_lock(&dev_mutex);
	value = kmalloc(dim,GFP_USER);
	if (unlikely(value == NULL)){
		res = 1;
		printk(KERN_ERR "**Error in allocating value");
		goto r_end;
	}
	/* If empty buffer appends reader on device */
	while (kb_isempty(&kb_fifo)){
		printk(KERN_DEBUG "Reader Waiting on Empty Device\n");
		mutex_unlock(&dev_mutex);
		wait_event_interruptible(wait_r,(!kb_isempty(&kb_fifo)));
		mutex_lock(&dev_mutex);
	}
	res = kb_pop(value,&kb_fifo);
	if (unlikely(res)){
		res = 1;
		printk(KERN_ERR "**Error in kb_pop");
		goto r_end;
	}
	res = copy_to_user(buf,value,dim);
	if (unlikely(res)){
		res = -EFAULT;
		printk(KERN_ERR "**Error in copy_to_user");
		goto r_end;
	}
	wake_up_interruptible(&wait_w); /* Awake Writer */
	printk(KERN_DEBUG "Read %d byte from fifo: %s\n",dim,value);
	r_end:
	mutex_unlock(&dev_mutex);
	return res;
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t dim, loff_t *ppos)
{
	int res, err;
	char *value;

	mutex_lock(&dev_mutex);
	value = kmalloc(sizeof(char)*dim,GFP_KERNEL);
	if (unlikely(value == NULL)){
		res = 1;
		printk(KERN_ERR "**Error in allocating value");
		goto w_end;
	}
		while (kb_isfull(&kb_fifo,dev_stat.ds_kb_max)){
		printk(KERN_DEBUG "Writer Waiting on Full Device\n");
		mutex_unlock(&dev_mutex);
		wait_event_interruptible(wait_w,(!kb_isfull(&kb_fifo,dev_stat.ds_kb_max)));
		mutex_lock(&dev_mutex);
	}
	err = copy_from_user(value,buf,dim);
	if (unlikely(err)){
		res = -EFAULT;
		printk(KERN_ERR "**Error in copy_from_user");
		goto w_end;
	}
	res = kb_push(value,&kb_fifo);
	wake_up_interruptible(&wait_r); /* Awake Reader */
	printk(KERN_DEBUG "Write %d byte into fifo: %s\n",dim,value);

	w_end:
	kfree(value);
	mutex_unlock(&dev_mutex);
	return res;	
}

static int my_module_init(void)
{
	int res;
	int tmp = START_KB_DIM;
	
	res = misc_register(&my_device);
	if (unlikely(res)){
		printk(KERN_ERR "**Device error: %d\n",res);
		return res;
	}
	printk(KERN_DEBUG "**Device Init\n");
	mutex_init(&dev_mutex);
	kb_init(&kb_fifo);
	init_waitqueue_head(&wait_r);
	init_waitqueue_head(&wait_w);

	kt_stat_desc = kthread_run(kt_stat, NULL, "Statistics_Thread");
	if (IS_ERR(kt_stat_desc)) {
		printk("**Error creating kernel thread!\n");
		return PTR_ERR(kt_stat_desc);
	}
	kt_realloc_desc = kthread_run(kt_realloc, NULL, "Reallocation_Thread");
	if (IS_ERR(kt_realloc_desc)) {
		printk("**Error creating kernel thread!\n");
		return PTR_ERR(kt_realloc_desc);
	}
	ds_init_alloc(&dev_stat, tmp);
	dev_run = 0;
	return 0;
}

static void my_module_exit(void)
{
	mutex_destroy(&dev_mutex);
	mutex_destroy(&write_mutex);
	kthread_stop(kt_stat_desc);
	misc_deregister(&my_device);
	printk(KERN_DEBUG "*Device Exit\n");
}

static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
  .write =        my_write,
};

static struct miscdevice my_device = {
	MISC_DYNAMIC_MINOR, "mydev", &my_fops
};

module_init(my_module_init);
module_exit(my_module_exit);
