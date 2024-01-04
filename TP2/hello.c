#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#define TAILLE 100
#define DRIVER_AUTHOR "Christophe BarÃ¨s"
#define DRIVER_DESC "Hello world Module"
#define DRIVER_LICENSE "GPL"
static int param;
int hello_init(void)
{
	printk(KERN_INFO "Hello world!\n");
	printk(KERN_DEBUG "le paramètre est=%d\n", param);
return 0;
}

void hello_exit(void)
{
	printk(KERN_ALERT "Bye bye...\n");
}
struct proc_dir_entry *proc_file_entry;
ssize_t fops_write(struct file * file, const char __user * buffer,
size_t count, loff_t * ppos);
ssize_t fops_read(struct file *file, char __user * buffer,
size_t count, loff_t * ppos);
static const struct file_operations proc_fops = {
.read = fops_read,
.write = fops_write,
};

module_param(param, int, 0);
MODULE_PARM_DESC(param, "Un paramètre de ce module");

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);





