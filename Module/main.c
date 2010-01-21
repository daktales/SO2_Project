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
static struct mutex bmutex; /*	Buffer's mutex	*/
static char *my_data;
static int count;
static struct kb kbuffer;

struct buf_node {
	struct list_head list;
	char *data;
};

static struct list_head kbuf;

static int my_open(struct inode *inode, struct file *file)
{
	int *flag;

	flag = kmalloc(sizeof(int),GFP_USER);
	if (flag == NULL){
		return -1;
	}

	*flag = 0;
	file->private_data = flag;
	
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	
	return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t dim, loff_t *ppos)
{/*
	int res, err, *flag;
	//struct buf_node *tmp;
	
	err = 0;
	mutex_lock(&bmutex);
	flag = file->private_data;
	if (*flag == 1){
		res = 0;
		goto r_end;
	}
	if (dim > my_len){
		res = my_len;
	} else {
		res = dim;
	}
	err = copy_to_user(buf,my_data, res);
	if(err){
		res = -EFAULT;
		goto r_end;
	}
	kfree(my_data);
	my_data = NULL;
	*flag = 1;
	/*tmp = &(kbuf.prev);
	err = copy_to_user(buf,tmp->data,res);
		if (err){
		res = -EFAULT;
		mutex_unlock(&bmutex);
		return res;}
	//list_del (&tmp);
	printk(KERN_DEBUG "Extract: %s into buffer",tmp->data);
	r_end:
	mutex_unlock(&bmutex);
	return res;*/
}
static ssize_t my_write(struct file *file, const char __user * buf, size_t dim, loff_t *ppos)
{
	/*int res, err;
	char *value;
	struct buf_node *node = kmalloc(sizeof(struct buf_node), GFP_USER);
	value = kmalloc(dim,GFP_USER);
	mutex_lock(&bmutex);
	err = copy_from_user(value,buf,dim);
	printk(KERN_DEBUG "Dim = %d\n",dim);
	res = dim;
	if (err){
		res = -EFAULT;
		goto w_end;
	}
	node->data = value;
	list_add (&(node->list), &kbuf);
	//printk(KERN_DEBUG "Insert: %s into buffer\n",value);
	
	mutex_lock(&bmutex);
	my_data = kmalloc(dim, GFP_USER);
	my_len = dim;
	err = copy_from_user(my_data,buf,dim);
	res = dim;
	
	w_end:
	mutex_unlock(&bmutex);
	return res;*/
	int res, err;
	char *value;
	value = kmalloc(dim,GFP_USER);
	err = copy_from_user(value,buf,dim);
	kb_push(value,&kbuffer);
	count++;
	
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
	mutex_init(&bmutex);
	kb_init(&kbuffer);
	count = 0;
	return res;
}

static void my_module_exit(void)
{
	mutex_destroy(&bmutex);
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
