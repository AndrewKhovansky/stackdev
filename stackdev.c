#include <linux/atomic.h>  
#include <linux/cdev.h>
 
#include <linux/delay.h>  
#include <linux/device.h>  
#include <linux/fs.h>
 
#include <linux/init.h>  
#include <linux/kernel.h> 
 
#include <linux/module.h>  
#include <linux/printk.h>  
#include <linux/types.h>
 
#include <linux/uaccess.h>
#include <linux/version.h>  
 
#include <asm/errno.h>  

#define STACK_SIZE 16
  
static int device_open(struct inode *, struct file *);
 
static int device_release(struct inode *, struct file *);
 
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
 
static ssize_t device_write(struct file *, const char __user *, size_t,  
                            loff_t *);
 
  
#define DEVICE_NAME "stackdev" 
 
static int major; 
  
enum {  
    CDEV_NOT_USED,
    CDEV_EXCLUSIVE_OPEN,  
};  
  
/* Is the device open? Used to prevent multiple access to device */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);  


//Device stack
static uint8_t deviceStack[STACK_SIZE * sizeof(int)];
static int deviceStackPointer = 0;



static struct class *cls;  
  
static struct file_operations chardev_fops = 
{
    .read = device_read,  
    .write = device_write,  
    .open = device_open,
    .release = device_release,  
};  
  
//Load module
static int __init chardev_init(void)  
{
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops);  
  
    if (major < 0) 
	{
        pr_alert("Registering char device failed with %d\n", major);
        return major;  
    }  
  
    pr_info("Assigned major number %d.\n", major);  

 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)  
    cls = class_create(DEVICE_NAME);
#else  
    cls = class_create(THIS_MODULE, DEVICE_NAME);  
#endif
 
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);  

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);  
  
	deviceStackPointer = 0;
  
    return 0;  
}  

 
//Unload module
static void __exit chardev_exit(void)  
{  
    device_destroy(cls, MKDEV(major, 0));
 
    class_destroy(cls);  
  
    /* Unregister the device */  
    unregister_chrdev(major, DEVICE_NAME);
}  


//Open device
static int device_open(struct inode *inode, struct file *file)  
{
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;  

    return 0;  
}  
  
//Close device 
static int device_release(struct inode *inode, struct file *file)  
{
    atomic_set(&already_open, CDEV_NOT_USED);
    return 0;  
}  


//Read from device
static ssize_t device_read(struct file *filp, 
                           char __user *buffer, 
                           size_t length,
                           loff_t *offset)  
{  
	if ((length % sizeof(int)) != 0)
	{
		pr_alert("Byte count should be a multiple of INT size.\n");  
		return -ENOSYS;
	}
	
	length = (length > deviceStackPointer)?(deviceStackPointer):(length);

	if (length == 0)
	{
		pr_alert("Buffer is empty.\n");  
		return -EINVAL;
	}
	
	unsigned long result;
	for(int i=0; i<length; i += sizeof(int))
	{
		result = copy_to_user(buffer, &deviceStack[deviceStackPointer - sizeof(int)], sizeof(int));
		
		if(result != 0)
		{
			return -EADDRNOTAVAIL;
		}
		
		buffer += sizeof(int);
		deviceStackPointer -= sizeof(int);
	}

	return length;
}  
  
//Write to the device
static ssize_t device_write(struct file *filp, const char __user *buffer,
 
                            size_t length, loff_t *offset)  
{
	if ((length % 4) != 0)
	{
		pr_alert("Invalid write length.\n");  
		return -EINVAL;
	}
	
	int space = (STACK_SIZE*sizeof(int) - deviceStackPointer);
	
	length = (length > space)?(space):(length);
	
	
	if (length == 0)
	{
		pr_alert("No space in buffer.\n");  
		return -ENOMEM;
	}
	

	unsigned long result;
	for(int i=0; i<length; i += sizeof(int))
	{
		result = copy_from_user(&deviceStack[deviceStackPointer], buffer, sizeof(int));
		
		if(result != 0)
		{
			return -EADDRNOTAVAIL;
		}
		
		buffer += sizeof(int);
		deviceStackPointer += sizeof(int);
	}
	
	return length;
}  

 
module_init(chardev_init);  
module_exit(chardev_exit);  
 
MODULE_LICENSE("GPL");