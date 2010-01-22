#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/list.h> /*	Kernel List	*/
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include "kbuf.h"

MODULE_AUTHOR("Walter Da Col");
MODULE_DESCRIPTION("Sistemi Operativi 2 - Module");
MODULE_LICENSE("GPL");

static struct miscdevice my_device;

static struct kb kb_fifo; /*	Buffer fifo	*/

static struct mutex kb_mutex; /*	Buffer's mutex	*/


static int my_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{

	return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t dim, loff_t *ppos)
{
	int res,len;
	char* value;
	
	
	mutex_lock(&kb_mutex);
	value = kmalloc(dim,GFP_USER);
	res = kb_pop(value,&kb_fifo);
	printk(KERN_ERR "After kb_pop\n");
	printk(KERN_ERR "print value: %s\n",value);
	if (res){
		res = 1;
		goto r_end;
	}
	//len = strlen(*value)+1; /* strlen() do not count \0, so +1	*/
	len = dim;
	printk(KERN_ERR "After strlen: %d\n",len);
	res = copy_to_user(buf,value,len);
	if (res){
		res = -EFAULT;
		goto r_end;
	}
	r_end:
	mutex_unlock(&kb_mutex);
	
	return res;
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t dim, loff_t *ppos)
{
	int res, err;
	char *value;

	value = kmalloc(sizeof(char)*dim,GFP_KERNEL);
	if (value == NULL){
		res = 1;
		printk(KERN_ERR "Error in allocating value");
		goto w_end;
	}
	mutex_lock(&kb_mutex);
	err = copy_from_user(value,buf,dim);
	if (err){
		res = -EFAULT;
		printk(KERN_ERR "Error in copy_from_user");
		goto w_end;
	}
	res = kb_push(value,&kb_fifo);
	printk(KERN_DEBUG "Write %d byte (remember \\0) into fifo: %s\n",dim,value);
	w_end:
	mutex_unlock(&kb_mutex);
	return res;	
}

static int my_module_init(void)
{
	int res;

	res = misc_register(&my_device);
	if (res){
		printk(KERN_ERR "**Device error: %d\n",res);
		return res;
	}
	printk(KERN_DEBUG "**Device Init\n");
	mutex_init(&kb_mutex);
	kb_init(&kb_fifo);
	return res;
}

static void my_module_exit(void)
{
	mutex_destroy(&kb_mutex);
	misc_deregister(&my_device);
	printk(KERN_DEBUG "*Device Exit\n");
}

int my_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param){
	return 0;
}


static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
  .write =        my_write,
  .ioctl=         my_ioctl,
};

static struct miscdevice my_device = {
	MISC_DYNAMIC_MINOR, "mydev", &my_fops
};

module_init(my_module_init);
module_exit(my_module_exit);
