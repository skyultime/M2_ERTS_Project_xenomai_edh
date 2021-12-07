#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/module.h>

#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/timer.h>
#include <linux/power_supply.h>

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>


#define TIMER_LENGTH 5000 /*Call interval*/ 
#define BATTERY_NAME "BAT1"

#define MAX_SIZE_BUF 3*(sizeof(int)+1)

#define XDDP_PORT 0     /* [0..CONFIG-XENO_OPT_PIPE_NRDEV - 1] */
 
#define printd() \
    printk(KERN_INFO "Simple WQ test: %s %d\n", __FUNCTION__, __LINE__); 
 
static void timer_register(struct timer_list* ptimer, signed int timer_length);
static void work_handler(struct work_struct *work);

static int battLinModule_init(void);
static void battLinModule_exit(void);

static int major_num;

static struct timer_list timerBattery;/*Kernel timer*/ 
static struct work_data * data;

static int initialised = 0;

struct file *fd;
mm_segment_t fs; 
 
struct work_data {
    struct work_struct work;
    int data;
};

static void batteryTimerHandler(struct timer_list *t){
    
    printk(KERN_INFO "%s called (%ld)\n", __func__, jiffies);    

    if(schedule_work(&data->work) == 0){
      printk("Failed to schedule work\n");
      //exit(-1);
    } 
}


static void timer_register(struct timer_list* ptimer, signed int timer_length){ //Kernel timer register
    int ret;

    printd();
    timer_setup(&timerBattery, batteryTimerHandler, 0);

    ret = mod_timer(&timerBattery, jiffies + msecs_to_jiffies(timer_length));
    //HZ = number of times jiffies (kernel unit of time) is incremented in one second
    
    if (ret){
    	pr_err("%s: Timer firing failed\n", __func__);
    }
}  


static void work_handler(struct work_struct *work)
{  
    int result = 0;
    char buf[128];
    int n;
    int ret = 0;  
 
    struct power_supply *psy = power_supply_get_by_name("BAT1");
    union power_supply_propval chargenow,chargefull,voltage,intensity,capacity;

    printd();
    #if 0 
      struct work_data * data = (struct work_data *)work;    
      kfree(data);
    #endif    

    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CHARGE_NOW,&chargenow);
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CHARGE_FULL,&chargefull);
    
    printk("\n");    
static int major_num;
    if(!result) {
        printk(KERN_INFO "Charge level : %d / %d\n",chargenow.intval,chargefull.intval);
    }
    
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CAPACITY,&capacity); 
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_VOLTAGE_NOW,&voltage);
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CURRENT_NOW,&intensity);

    if(!result) {
        printk(KERN_INFO "Battery percent: %d%c (%d uV,%d uA)\n\n",capacity.intval,'%',&voltage,&intensity);
    }
    
    #if 0
      /* Get the next message from realtime_thread. */
      xddp_result = read(fd, buf, sizeof(buf));
    
      if (xddp_result <= 0)
        fail("read");
      /* Echo the message back to realtime_thread. */
    #endif
      
    n = sprintf(buf,"[%d,%d,%d]\n",capacity,chargenow,chargefull);

    ret = mod_timer(&timerBattery, jiffies + msecs_to_jiffies(TIMER_LENGTH));

    if (ret){
	    pr_err("%s: Timer firing failed\n", __func__);
    }
}

static int battmod_open(struct inode *pinode, struct file *pfile) {
    struct file *f;
    char devname[12];

		if (sprintf(devname, "/dev/rtp%d", XDDP_PORT) < 0){
				printk(KERN_INFO "Failed using asprintf\n");
		}

		f = filp_open(devname,O_RDWR,0644);


		if (f == NULL){
			printk(KERN_INFO "Failed to open file\n");
		}else {
			printk(KERN_INFO "Open file %s\n",devname);
		} 

    pfile->private_data = f;
    printk(KERN_INFO "Device has been opened\n");
   
    return 0;
}


ssize_t battmod_read(struct file *file, char __user *buf, size_t count, loff_t * ppos) {

    printk(KERN_INFO "Read batmod module\n");

	char *batt_str = "Battery mod\n";
	int len = strlen(batt_str); /* Don't include the null byte. */

	/*
	 * We only support reading the whole string at once.
	 */
	if (count < len)
		return -EINVAL;
	/*
	 * If file position is non-zero, then assume the string has
	 * been read and indicate there is no more data to be read.
	 */
	if (*ppos != 0)
		return 0;
	/*
	 * Besides copying the string to the user provided buffer,
	 * this function also checks that the user has permission to
	 * write to the buffer, that it is mapped, etc.
	 */
	if (copy_to_user(buf, batt_str, len))
		return -EINVAL;
	/*
	 * Tell the user how much data we wrote.
	 */
	*ppos = len;

	return len;
}


ssize_t battmod_write(struct file *filp, const char __user *buff, size_t count, loff_t *pos) {

    printk(KERN_INFO "Write batmod module\n");
    
    char *kbuffer;
	ssize_t bytes_written = 0;

	kbuffer = kmalloc(8, GFP_KERNEL);
	if (!kbuffer)
		return -ENOMEM;

	if (copy_from_user(kbuffer, buff, sizeof(buff))) {
		bytes_written = -EFAULT;
	}

	printk(KERN_INFO "Battery_mod: user have written: >%s<\n", kbuffer);

	kfree(kbuffer);

	return count/*bytes_written*/;

} 

static int  battmod_close(struct inode *pinode, struct file *pfile) {
    struct file *f = pfile->private_data;
    int ret = 0;

    ret = filp_close(f,NULL);  
    
    if (ret == 0){
      printk(KERN_INFO "Device successfully closed\n");
    }else{
      printk(KERN_INFO "Failed to close device\n");   
    }
    
    return 0;
}

struct file_operations battmod_file_operations = { 
    .owner = THIS_MODULE,
    .open = battmod_open,
    .read = battmod_read,
    .write = battmod_write,
    .release = battmod_close,
};

static struct miscdevice battLinModule_misc = { MISC_DYNAMIC_MINOR, "battLinModule", &battmod_file_operations };

static int battLinModule_init(void)
{
    printd();

    data = kmalloc(sizeof(struct work_data), GFP_KERNEL);
    if (misc_register(&battLinModule_misc)) {
		  printk(KERN_ERR "BATTERY : Cannot register misc device battery!\n");
		  return -EBUSY;
	  }

	 if (major_num < 0) {
	   printk(KERN_ALERT "Could not register device: %d\n", major_num);
	 return major_num;
	 } else {
	   printk(KERN_INFO "Module loaded with device major number %d\n", major_num);    
   }
    timer_register(&timerBattery,TIMER_LENGTH);
    if (initialised == 0){
			INIT_WORK(&data->work, work_handler);  	
      initialised = 1;
    }
 
    return 0;
}
 
static void battLinModule_exit(void)
{
    int ret;

    ret = del_timer(&timerBattery);
    if (ret){
      pr_err("%s: Timer still used ...\n", __func__);
    }
    
    flush_scheduled_work();
    printd();
}       

/*################################### WRITE #######################################################*/
/*

int xddp_result = 0;
fs = get_fs();

set_fs(KERNEL_DS);
fd->f_op->write(fd, buf, strlen(buf),&fd->f_pos); 
set_fs(fs);    

if (xddp_result <= 0)
  printk(KERN_INFO "Failed writing value using XDDP prot.\n");

cf. ssize_t kernel_write(struct file *file, const void *buf, size_t count,
            loff_t *pos);


*/
/*#################################################################################################*/

module_init(battLinModule_init);
module_exit(battLinModule_exit);

MODULE_AUTHOR("alexy <alexy.debus@eleves.ec-nantes.fr>");
MODULE_DESCRIPTION("Battery module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
