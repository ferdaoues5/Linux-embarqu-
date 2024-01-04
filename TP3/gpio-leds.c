#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>


#define TAILLE 100
int pattern=1;
int dir=0;
int time=1;
struct ensea_leds_dev *dev_g ;
struct proc_dir_entry *proc_file_entry;
struct proc_dir_entry *ensea_parent;
static struct timer_list my_timer;
// Prototypes
static int leds_probe(struct platform_device *pdev);
static int leds_remove(struct platform_device *pdev);
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
ssize_t speed_read(struct file *file, char __user * buffer,size_t count, loff_t * ppos);
ssize_t dir_write(struct file * file, const char __user * buffer,size_t count, loff_t * ppos);
ssize_t dir_read(struct file *file, char __user * buffer,size_t count, loff_t * ppos);
void my_timer_callback(struct timer_list  *timer);

// An instance of this structure will be created for every ensea_led IP in the system
struct ensea_leds_dev {
    struct miscdevice miscdev;
    void __iomem *regs;
    u8 leds_value;
};

// Specify which device tree devices this driver supports
static struct of_device_id ensea_leds_dt_ids[] = {
    {
        .compatible = "dev,ensea"
    },
    { /* end of table */ }
};

// Inform the kernel about the devices this driver supports
MODULE_DEVICE_TABLE(of, ensea_leds_dt_ids);

// Data structure that links the probe and remove functions with our driver
static struct platform_driver leds_platform = {
    .probe = leds_probe,
    .remove = leds_remove,
    .driver = {
        .name = "Ensea LEDs Driver",
        .owner = THIS_MODULE,
        .of_match_table = ensea_leds_dt_ids
    }
};

// The file operations that can be performed on the ensea_leds character file
static const struct file_operations ensea_leds_fops = {
    .owner = THIS_MODULE,
    .read = leds_read,
    .write = leds_write
};

static const struct file_operations proc_speed = {
  .read = speed_read
};

static const struct file_operations proc_dir = {
  .read = dir_read,
  .write = dir_write
};
// Paramètre: vitesse de balayage
static int param;
module_param(param, int, 0);
MODULE_PARM_DESC(param, "Un parametre de ce module");



// Called when the driver is installed
static int leds_init(void)
{
    int ret_val = 0;
	printk(KERN_INFO "Initializing gpio-leds!\n");
    pr_info("Initializing the Ensea LEDs module\n");
	printk(KERN_DEBUG "le parametre est=%d\n", param);
	ensea_parent = proc_mkdir("ensea",NULL);
	if(!ensea_parent)
		{
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
		}
	proc_file_entry = proc_create("speed", 0, ensea_parent, &proc_speed);
	printk(KERN_INFO "proc speed created\n");
	proc_file_entry = proc_create("dir", 0, ensea_parent, &proc_dir);
	printk(KERN_INFO "proc dir created\n");
	
    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&leds_platform);
    if(ret_val != 0) {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }
	printk(KERN_DEBUG "le parametre est=%d\n", param);
	
    pr_info("Ensea LEDs module successfully initialized!\n");

    return 0;
}

// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Ensea LEDs module)
static int leds_probe(struct platform_device *pdev)
{
	int ret_val = -EBUSY;
    struct ensea_leds_dev *dev;
    struct resource *r = 0;

    pr_info("leds_probe enter\n");

    // Get the memory resources for this LED device
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }

    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct ensea_leds_dev), GFP_KERNEL);
	
    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->regs))
        goto bad_ioremap;
	
	
    
	// Turn the LEDs on (access the 0th register in the ensea LEDs module)
    dev->leds_value = 0xFF;
	
    iowrite32(dev->leds_value, dev->regs);

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;    // Dynamically choose a minor number
    dev->miscdev.name = "ensea_leds";
    dev->miscdev.fops = &ensea_leds_fops;

    ret_val = misc_register(&dev->miscdev);
    if(ret_val != 0) {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }
	
	

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void*)dev);
	dev_g=dev;
	/* setup your timer to call my_timer_callback */
	setup_timer(&my_timer, my_timer_callback,0);
	/* setup timer interval to 100 ticks */
	mod_timer(&my_timer, jiffies + param);
	
    pr_info("leds_probe exit\n");

    return 0;

bad_ioremap:
   ret_val = PTR_ERR(dev->regs);
bad_exit_return:
    pr_info("leds_probe bad exit :(\n");
    return ret_val;
}

// This function gets called whenever a read operation occurs on one of the character files
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;
	char message[100];

    /*
    * Get the ensea_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our ensea_leds_dev structure that has the current led value).
    *
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);
	sprintf(message,"%d\n",pattern);
	size_t datalen = strlen(message);
	if (*offset >= datalen) {
        return 0; /* end of file */
    } 
    if(len > datalen - *offset) {
        len = datalen - *offset;
    }
    // Give the user thoe current led value
    success = copy_to_user(buffer, message, len);
	printk(KERN_INFO "pattern : %d\n", pattern);

    // If we failed to copy the value to userspace, display an error message
    if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }

    *offset += len;
    return len;
}

// This function gets called whenever a write operation occurs on one of the character files
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;
	char message[100];
    /*
    * Get the ensea_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our ensea_leds_dev structure that has the current led value).
    *
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);
	
    // Get the new led value (this is just the first byte of the given data)
    success = copy_from_user(message, buffer, len);

    // If we failed to copy the value from userspace, display an error message
    if(success != 0) {
        pr_info("Failed to read led value from userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    } else {
        // We read the data correctly, so update the LEDs
        //iowrite32(dev->leds_value, dev->regs);
		message[len] = '\0';
		if (message[0]=='0')
		{
			pattern=0;
		}
		else if (message[0]=='1')
		{
			pattern=1;
		}
		else if (message[0]=='2')
		{
			pattern=2;
		}
		else if (message[0]=='3')
		{
			pattern=3;
		}
		else 
		{
			pattern=4;
		}
		
		printk(KERN_INFO "New pattern : %d\n", pattern);
    }

    // Tell the user process that we wrote every byte they sent
    // (even if we only wrote the first value, this will ensure they don't try to re-write their data)
    return len;
}

ssize_t speed_read(struct file *file, char __user * buffer,
size_t count, loff_t * ppos) {
    	
	char message[100];
	int errno=0;
	int copy;
	sprintf(message,"%d\n",param);
	size_t datalen = strlen(message);
	if (*ppos >= datalen) {
        return 0; /* end of file */
    } 
    if(count > datalen - *ppos) {
        count = datalen - *ppos;
    }
	if ((copy=copy_to_user(buffer, message, count)))
		errno = -EFAULT;
	printk(KERN_INFO "message read, %d, %p\n", copy, buffer);
	*ppos += count;
    return count;
}


ssize_t dir_read(struct file *file, char __user * buffer,
size_t count, loff_t * ppos) {
    	
	char message[100];
	int errno=0;
	int copy;
	sprintf(message,"%d\n",dir);
	size_t datalen = strlen(message);
	if (*ppos >= datalen) {
        return 0; /* end of file */
    } 
    if(count > datalen - *ppos) {
        count = datalen - *ppos;
    }
	if ((copy=copy_to_user(buffer, message, count)))
		errno = -EFAULT;
	printk(KERN_INFO "message read, %d, %p\n", copy, buffer);
	*ppos += count;
    return count;
}

ssize_t dir_write(struct file * file, const char __user * buffer,
size_t count, loff_t * ppos) {
	char message[100];
	int len = count;
	int size_of_message;
	if (len > TAILLE) len = TAILLE;
	printk(KERN_INFO "Recieving new messag\n");
	if (copy_from_user(message, buffer, count)) 
	{
		return -EFAULT;
	}
	message[count] = '\0';

	if (message[0]=='0')
	{
		dir=0;
	}
	else
	{
		dir=1;
	}
	
	printk(KERN_INFO "New direction : %d\n", dir);
	return count;
}

// Gets called whenever a device this driver handles is removed.
// This will also get called for each device being handled when
// our driver gets removed from the system (using the rmmod command).
static int leds_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    struct ensea_leds_dev *dev = (struct ensea_leds_dev*)platform_get_drvdata(pdev);
	

    // Turn the LEDs off
    iowrite32(0x00, dev->regs);

    // Unregister the character file (remove it from /dev)
    misc_deregister(&dev->miscdev);

    pr_info("leds_remove exit\n");

    return 0;
}

// Called when the driver is removed
static void leds_exit(void)
{
    pr_info("Ensea LEDs module exit\n");
	
	remove_proc_entry("speed", ensea_parent);
	remove_proc_entry("dir", ensea_parent);
	remove_proc_entry("ensea", NULL);
	del_timer(&my_timer);

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "leds_remove" to be called for each connected device
    platform_driver_unregister(&leds_platform);

    pr_info("Ensea LEDs module successfully unregistered\n");
}


void my_timer_callback(struct timer_list  *timer)
{
	 /* do your timer stuff here */
	 
	 
	 dev_g->leds_value=((1<<pattern) -1)<<(time-1);
	 iowrite32(dev_g->leds_value, dev_g->regs);
	 printk(KERN_INFO "LED  0x%x\n",((1<<pattern) -1)<<(time-1)); //(1>>(time-1))
	 if (dir==0)
	 {
		time ++;
	 }
	 else
	 {
		 time--;
	 }
	 mod_timer(&my_timer, jiffies + param);
	 if (dir==0)
	 {
		 if (time>=10-pattern)
		 {
			 time=1;
		 }
	 }
	 else 
	 {
		 if (time<1)
		 {
			 time=9-pattern;
		 }
	 }
	 
}

// Tell the kernel which functions are the initialization and exit functions
module_init(leds_init);
module_exit(leds_exit);

// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Devon Andrade <devon.andrade@oit.edu>");
MODULE_DESCRIPTION("Exposes a character device to user space that lets users turn LEDs on and off");
MODULE_VERSION("1.0");