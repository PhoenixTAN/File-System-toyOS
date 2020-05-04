#include <linux/module.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ziqi-Tan U88387934");

/* initialization routine when loading the module */
static int __init initialization_routine(void) {
    printk("<1> Loading module\n");
    
	pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
    // Start create proc entry
    proc_entry = create_proc_entry("discos_module", 0552, NULL);
    if(!proc_entry) {
        printk("<1> Error creating /proc entry.\n");
        return 1;
    }

    proc_entry->proc_fops = &pseudo_dev_proc_operations;

	return 0;
}

/* clean up routine when unloading the module */
static void __exit cleanup_routine(void) {
    printk("<1> Dumping module\n");
    remove_proc_entry("ioctl_kb_driver", NULL);
	return;
}

// ioctl entry
static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {

    return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine);
