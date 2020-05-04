/*
 *  RAMDISK
 */
#include "discos.h"

static int discos_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations discos_proc_operations;

static struct proc_dir_entry *proc_entry;

// static spinlock_t my_lock = SPIN_LOCK_UNLOCKED;

/* ioctl entry point */
static int discos_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int size, left, flag;
	char *pathname, *data, *from;
	ioctl_args_t* args = (ioctl_args_t *)vmalloc(sizeof(ioctl_args_t));
	if(copy_from_user(args, (ioctl_args_t *)arg, sizeof(ioctl_args_t))) {
		printk("<1> Error copy from user\n");
	}
	if (args->pathname != NULL) {
		size = strnlen_user(args->pathname, 14);
		pathname = (char *)kmalloc(size, GFP_KERNEL);
		copy_from_user(pathname, args->pathname, size);
	}
	/*printk("Got user path %s\n", args->pathname);*/
	switch (cmd) {
		case RD_INIT:
			// init_fd_table();		
			ret = init_file_sys();
			// vfree(args);
			/*spin_unlock(&my_lock);*/
			return ret;
		// case RD_OPEN:
		// 	printk("<1> Opening %s\n", args->pathname);
		// 	// Send fd back to user space
		// 	ret = open(args->pid, args->pathname);
		// 	vfree(args);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
		// case RD_CLOSE:
		// 	printk("<1> Switch case close\n");
		// 	ret = close(args->pid, args->fd_num);
		// 	vfree(args);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
		// case RD_READ:
		// 	ret = read(args->fd_num, args->address, args->num_bytes, args->pid);
		// 	vfree(args);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
		// case RD_WRITE:
		// 	data = vmalloc(args->num_bytes);
		// 	flag = 1;
		// 	left = args->num_bytes;
		// 	from = args->address;
		// 	while (flag) {
		// 		if (left > 4096) {
		// 			ret = copy_from_user(data, from, 4096);
		// 			left -= 4096;
		// 			data += 4096;
		// 			from += 4096;
		// 			printk("<1> ret: %d left:%d\n", ret, left);
		// 		} else {
		// 			ret = copy_from_user(data, from, left);
		// 			data += left;
		// 			printk("<1> ret: %d left:%d\n", ret, left);
		// 			flag = 0;
		// 		}
		// 	}
		// 	printk("<1> Here!\n");
		// 	/*copy_from_user(data, args->address, args->num_bytes);*/
		// 	/*printk("<1> Got write data from user: %s\n",data);*/
		// 	ret = write(args->fd_num, data - args->num_bytes, args->num_bytes, args->pid);
		// 	vfree(data);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
 		case RD_CREATE:
			// spin_lock(&my_lock);
			ret = rd_create(pathname, "reg\0", 1);
			vfree(pathname);
			vfree(args);
			// spin_unlock(&my_lock);
			return ret;
			break;
		// case RD_LSEEK:
		// 	ret = lseek(args->pid, args->fd_num, args->offset);
		// 	vfree(args);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
		case RD_MKDIR:
			printk("<1> Mkdir %s\n", pathname);
			ret = rd_mkdir(pathname);
			kfree(pathname);
			vfree(args);
			/*spin_unlock(&my_lock);*/
			return ret;
			break;
		case RD_UNLINK:
			ret = rd_unlink(pathname);
			kfree(pathname);
			vfree(args);
			/*spin_unlock(&my_lock);*/
			return ret;
			break;
		// case RD_READDIR:
		// 	ret = readdir(args->pid, args->fd_num, args->address);
		// 	vfree(args);
		// 	/*spin_unlock(&my_lock);*/
		// 	return ret;
		default:
			printk("<1> hitting default case \n");
			vfree(args);
			/*spin_unlock(&my_lock);*/
			return -EINVAL;
			break;			
	}

	/*spin_unlock(&my_lock);*/
	return 0;
}

static int __init initialization_routine(void)
{
	printk("<1> Loading Module\n");

	discos_proc_operations.ioctl = discos_ioctl;

	/* create proc entry */
	proc_entry = create_proc_entry("discos", 0444, NULL);
	if (!proc_entry)
	{
		printk("<1> Error creating /proc entry\n");
		return 1;
	}

	proc_entry->proc_fops = &discos_proc_operations;

	/*spin_lock_init(&my_lock);*/

	return 0;
}

static void __exit cleanup_routine(void)
{
	printk("<1> Dumping module\n");
	remove_proc_entry("discos", NULL);
	cleanup_fs();
	return;
}

module_init(initialization_routine);
module_exit(cleanup_routine);
