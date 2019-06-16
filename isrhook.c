#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "pagehelper.h"
#include "isrhelper.h"

#define DEVNAME "isrhook"

static int major = 0;
module_param(major, int, 0);
MODULE_AUTHOR("Tiny");
MODULE_LICENSE("Dual BSD/GPL");

static struct cdev cdevice;
dev_t devno;
struct class *my_class;


static int isrhook_open(struct inode *pinode, struct file *pflie)
{
    printk(KERN_ALERT "here is open\n");
    return 0;
}

static int isrhook_release(struct inode *pinode, struct file *pfile)
{
    printk(KERN_ALERT "here is release\n");
    return 0;
}

long isrhook_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    //copy_from_user
    //copy_to_user
    return 0;
}

static struct file_operations isr_remap_ops = {
    .owner = THIS_MODULE,
    .open = isrhook_open,
    .release = isrhook_release,
    .unlocked_ioctl = isrhook_ioctl,
    .compat_ioctl = isrhook_ioctl,
};

static int isrhook_init(void)
{
    int result;

    /**/
    struct tagMyPageHelper page_helper;
    uint64_t lapic_base_pa = 0xfee00000;
    uint32_t data;
    uint32_t cpuidinfo[4];
    /**/
    /*
    uint64_t cr4;
    
    cr4 = __read_cr4();
    printk("cr4 is 0x%x \n", cr4);
    cr4 = cr4 & (~0x200000); // if support smap, need disab smap, every core need disable
    __write_cr4(cr4);
    cr4 = __read_cr4();
    printk("cr4 is 0x%x \n", cr4);
*/
    devno = MKDEV(major, 0);
    if (major)
    {
        result = register_chrdev_region(devno, 1, DEVNAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, 0, 1, DEVNAME);
        major = MAJOR(devno);
    }
    if (result < 0)
    {
        printk(KERN_ALERT "register faill\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return 0;
    }
    if (major == 0)
    {
        major = result;
    }
    devno = MKDEV(major, 0);
    cdev_init(&cdevice, &isr_remap_ops);
    cdevice.owner = THIS_MODULE;
    cdevice.ops = &isr_remap_ops;
    if (cdev_add(&cdevice, devno, 1))
    {
        printk(KERN_ALERT "cdev_add fail \n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return 0;
    }
    my_class = class_create(THIS_MODULE, DEVNAME);
    if (IS_ERR(my_class))
    {
        printk(KERN_ALERT "create class fail\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return 0;
    }
    device_create(my_class, NULL, devno, NULL, DEVNAME);
    printk(KERN_ALERT "isrhook init\n");
    


    page_helper.order = 0;
    if(allocate_kernel_continuous_4k_pages(&page_helper))
    {
        return 1;
    }
    data =  read_mmio_32bit_by_pagehelper(0xfed00010, page_helper);
    printk(KERN_ALERT "isrhook data by mypage is 0x%016llx\n", (uint64_t)data);
    //set_bit(data,17);
    //write_mmio_32bit_by_pagehelper(lapic_base_pa + 0x320,data, page_helper);
    free_continuous_page(page_helper);

    //data = read_mmio_32bit_by_ioremap(lapic_base_pa + 0x320);
    //printk(KERN_ALERT "isrhook data by ioremap is 0x%016llx\n", (uint64_t)data);
    //clear_bit(data,17);
    //printk(KERN_ALERT "isrhook data is 0x%016llx\n",(uint64_t)data);

    //write_mmio_32bit_by_ioremap(lapic_base_pa + 0x320,data);
   
    //data = read_mmio_32bit_by_ioremap(lapic_base_pa + 0x320);
    //printk(KERN_ALERT "isrhook data by ioremap is 0x%016llx\n", (uint64_t)data);
    
    //cpuid_helper(1, cpuidinfo);
    //
    backup_idt_gate_descriptors(0);
    return 0;
}

static void isrhook_exit(void)
{
    cdev_del(&cdevice);
    device_destroy(my_class, devno);
    class_destroy(my_class);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    printk(KERN_ALERT "isrhook exit\n");

}

module_init(isrhook_init);
module_exit(isrhook_exit);
