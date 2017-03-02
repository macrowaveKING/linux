#include "linux/module.h"
//#include "linux/init.h"
#include "linux/fs.h"
#include "linux/cdev.h"
#include "linux/semaphore.h"
#include "asm/uaccess.h"
struct fake_device {
  char data[100];
  struct semaphore sem;
}virtual_device;
struct cdev *mcdev;
int major_number;
int ret;
dev_t dev_num;
#ifndef DEVICE_NAME
#define DEVICE_NAME "solidusbdevice"
#endif
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SUXING FACTORY");
MODULE_VERSION("2:1.0");
//CONFIG_MODULE_SIG

int device_open(struct inode *inode,struct file *filp){
  if ((down_interruptible(&virtual_device.sem))!=0) {
    printk(KERN_ALERT "solidusbcode:could not lock device during open");
    return -1;
  }
  printk(KERN_INFO "solidusbcode:opened device");
  return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufCount, loff_t* curOffset){
  printk(KERN_INFO "solidusbcode:Reading from device");
  ret=copy_to_user(bufStoreData,virtual_device.data,bufCount);
  return ret;
}
ssize_t device_write(struct file *filp,const char* bufStoreData,size_t bufCount,loff_t* curOffset) {
  /* code */
  printk(KERN_INFO "solidusbcode:write device");
  ret=copy_from_user(virtual_device.data,bufStoreData,bufCount);

  return ret;
}
int device_close(struct inode *inode,struct file* filp) {
  /* code */
  up(&virtual_device.sem);
  printk(KERN_INFO "solidusbcode:closed device");
  return 0;
}

struct file_operations fops= {
  .owner=THIS_MODULE,
  .open=device_open,
  .release=device_close,
  .write=device_write,
  .read=device_read
};


static int driver_enter(void){
  printk(KERN_ALERT "hello driver_enter");
  ret=alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
  if (ret<0) {
    printk(KERN_ALERT "solidusbcode:fail to allocate a major number");
    return ret;
  }
  major_number=MAJOR(dev_num);
  printk(KERN_INFO "solidusbcode major number is %d",major_number);
  printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file",DEVICE_NAME,major_number);
  mcdev=cdev_alloc();
  mcdev->ops=&fops;
  mcdev->owner=THIS_MODULE;
  ret=cdev_add(mcdev,dev_num,1);
  if (ret<0) {
    printk(KERN_ALERT "solidusbcode:unable to add device to kernel");
    return ret;
  }
  sema_init(&virtual_device.sem,1);
  return 0;
}
static void driver_exit(void){
  printk(KERN_ALERT "goodbye driver_exit");
  cdev_del(mcdev);
  unregister_chrdev_region(dev_num,1);
  return;
}

module_init(driver_enter);
module_exit(driver_exit);
