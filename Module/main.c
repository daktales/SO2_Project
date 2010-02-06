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

/*	Buffer fifo	*/
static struct kb kb_fifo;

/*	Dev's mutex	*/
static struct mutex dev_mutex;

/*	Wait Queue	*/
static wait_queue_head_t wait_r;
static wait_queue_head_t wait_w;

/*	KThread Descriptors	*/
static struct task_struct *kt_stat_desc;
static struct task_struct *kt_realloc_desc;

/*	Struct for statistics	*/
static struct ds dev_stat;

/*	Used to verify if device is currently in use	*/
static int dev_run;


/* * FUNCTIONS * */

/*	Kthread for gathering statistics, create a 'snapshot' and
 * calculate some value.
 * Cooperate with Reallocator for calculating average occupation
 */
static int kt_stat(void *arg){
	int dim;
	int med,min,max;

	while (!kthread_should_stop()){
		mutex_lock(&dev_mutex);
		set_current_state(TASK_INTERRUPTIBLE);
		if (likely(dev_run)){
			/* Popolate stats from current kbuf state */
			dim = ds_populate(&dev_stat,&kb_fifo);
			/* dim = current number of nodes in kbuf */
			if (dim){
				med = ds_med(&dev_stat);
				max = ds_max(&dev_stat);
				min = ds_min(&dev_stat,max);
				printk(KERN_DEBUG "[MYDEV][Statistics]: %d node, strings lenght AVG/MAX/MIN; %d/%d/%d\n",dim,med,max,min);
				ds_reset_stat(&dev_stat);
				
			} else {
				printk(KERN_DEBUG "[MYDEV][Statistics]: Empty device\n");
			}
			/* Add a dim value to dev_stat, used by reallocator thread */
			ds_add_kb_value(&dev_stat,dim);
			mutex_unlock(&dev_mutex);
			schedule_timeout(1 * HZ);
		} else {
			printk(KERN_DEBUG "[MYDEV][Statistics]: Nothing to do\n");
			mutex_unlock(&dev_mutex);
			schedule();
		}
	}
	return 0;
}

/* Reallocation Thread
 * Try to keep kbuf occupation around 75%
 * doesn't really reallocate memory, simply set max kbuf size.
 */
static int kt_realloc(void *arg){
	int per,med_occ;
	while (!kthread_should_stop()){
		mutex_lock(&dev_mutex);
		set_current_state(TASK_INTERRUPTIBLE);
		if (likely(dev_run)){
			/*	Calculate average occupation (stored in 'per')	*/
			per = ds_get_kb_occupation(&dev_stat);
			/*	Calculate average kbuf size */
			med_occ = my_div(dev_stat.ds_kb_sum,dev_stat.ds_kb_dim);
			if (per!= KB_OCC){
				ds_set_kb_occupation(&dev_stat,med_occ);
				printk(KERN_DEBUG "[MYDEV][Reallocator]: Actual buffer occupation: %d%%, new size: %d\n",per,dev_stat.ds_kb_max);
				ds_reset_alloc(&dev_stat);
			} else {
				printk(KERN_DEBUG "[MYDEV][Reallocator]: No reallocation needed **\n");
				ds_reset_alloc(&dev_stat);
			}
			mutex_unlock(&dev_mutex);
			schedule_timeout(3 * HZ);
		} else {
			printk(KERN_DEBUG "[MYDEV][Reallocator]: Nothing to do\n");
			mutex_unlock(&dev_mutex);
			schedule();
		}
	}
	return 0;
}

/*	Open device	*/
static int my_open(struct inode *inode, struct file *file)
{
	mutex_lock(&dev_mutex);
	printk(KERN_DEBUG "[MYDEV]: Device Open\n");
	dev_run++;
	if (dev_run == 1){
		wake_up_process(kt_stat_desc);
		wake_up_process(kt_realloc_desc);
	}
	mutex_unlock(&dev_mutex);
	return 0;
}

/*	Close device	*/
static int my_close(struct inode *inode, struct file *file)
{
	mutex_lock(&dev_mutex);
	printk(KERN_DEBUG "[MYDEV]: Device Close\n");
	dev_run--;
	mutex_unlock(&dev_mutex);
	return 0;
}

/*	Read from device	*/
ssize_t my_read(struct file *file, char __user *buf, size_t dim, loff_t *ppos)
{
	int res;
	char *value;

	mutex_lock(&dev_mutex);
	value = kmalloc(dim,GFP_USER);
	if (unlikely(value == NULL)){
		res = 1;
		printk(KERN_ERR "[MYDEV]: Error in allocating value");
		goto r_end;
	}
	/* If empty buffer append reader on device */
	while (kb_isempty(&kb_fifo)){
		printk(KERN_DEBUG "[MYDEV]: Reader Waiting on Empty Device\n");
		mutex_unlock(&dev_mutex);
		wait_event_interruptible(wait_r,(!kb_isempty(&kb_fifo)));
		mutex_lock(&dev_mutex);
	}
	res = kb_pop(value,&kb_fifo);
	if (unlikely(res)){
		printk(KERN_ERR "[MYDEV]: Error in kb_pop");
		goto r_end;
	}
	res = copy_to_user(buf,value,dim);
	if (unlikely(res)){
		res = -EFAULT;
		printk(KERN_ERR "[MYDEV]: Error in copy_to_user");
		goto r_end;
	}
	wake_up_interruptible(&wait_w); /* Awake Writer */
	printk(KERN_DEBUG "[MYDEV]: READ %d byte from fifo: %s\n",dim,value);
	r_end:
	mutex_unlock(&dev_mutex);
	return res;
}

/*	Write to device	*/
static ssize_t my_write(struct file *file, const char __user * buf, size_t dim, loff_t *ppos)
{
	int res, err;
	char *value;

	mutex_lock(&dev_mutex);
	value = kmalloc(sizeof(char)*dim,GFP_KERNEL);
	if (unlikely(value == NULL)){
		res = 1;
		printk(KERN_ERR "[MYDEV]: Error in allocating value");
		goto w_end;
	}
	/* If full buffer append on device */
	while (kb_isfull(&kb_fifo,dev_stat.ds_kb_max)){
		printk(KERN_DEBUG "[MYDEV]: Writer Waiting on Full Device\n");
		mutex_unlock(&dev_mutex);
		wait_event_interruptible(wait_w,(!kb_isfull(&kb_fifo,dev_stat.ds_kb_max)));
		mutex_lock(&dev_mutex);
	}
	err = copy_from_user(value,buf,dim);
	if (unlikely(err)){
		res = -EFAULT;
		printk(KERN_ERR "[MYDEV]: Error in copy_from_user");
		goto w_end;
	}
	res = kb_push(value,&kb_fifo);
	wake_up_interruptible(&wait_r); /* Awake Reader */
	printk(KERN_DEBUG "[MYDEV]: WRITE %d byte into fifo: %s\n",dim,value);

	w_end:
	kfree(value);
	mutex_unlock(&dev_mutex);
	return res;	
}

/*	Module Init	*/
static int my_module_init(void)
{
	int res;
	
	res = misc_register(&my_device);
	if (unlikely(res)){
		printk(KERN_ERR "[MYDEV]: Device error: %d\n",res);
		return res;
	}
	printk(KERN_DEBUG "[MYDEV]: Device Init\n");
	mutex_init(&dev_mutex);
	kb_init(&kb_fifo);
	init_waitqueue_head(&wait_r);
	init_waitqueue_head(&wait_w);

	kt_stat_desc = kthread_run(kt_stat, NULL, "Statistics_Thread");
	if (IS_ERR(kt_stat_desc)) {
		printk("[MYDEV]: Error creating kernel thread!\n");
		return PTR_ERR(kt_stat_desc);
	}
	kt_realloc_desc = kthread_run(kt_realloc, NULL, "Reallocation_Thread");
	if (IS_ERR(kt_realloc_desc)) {
		printk("[MYDEV]: Error creating kernel thread!\n");
		return PTR_ERR(kt_realloc_desc);
	}
	ds_init_alloc(&dev_stat, START_KB_DIM);
	dev_run = 0;
	return 0;
}

/*	Module exit	*/
static void my_module_exit(void)
{
	mutex_destroy(&dev_mutex);
	mutex_destroy(&write_mutex);
	kthread_stop(kt_stat_desc);
	kthread_stop(kt_realloc_desc);
	misc_deregister(&my_device);
	printk(KERN_DEBUG "[MYDEV]: Device Exit\n");
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
