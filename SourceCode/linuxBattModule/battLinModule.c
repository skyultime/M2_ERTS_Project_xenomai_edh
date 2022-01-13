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


#define TIMER_LENGTH 5000 /*Call interval*/ 

#define BATTERY_NAME "BAT1"

#define BATTERY_SIZE 100
#define PRODUCED_ENERGY 5

#define MAX_SIZE_BUF 3*(sizeof(int)+1)

#define XDDP_PORT 0     /* [0..CONFIG-XENO_OPT_PIPE_NRDEV - 1] */
 
#define printd() \
    printk(KERN_INFO "Simple WQ test: %s %d\n", __FUNCTION__, __LINE__); 
 
static void timer_register(struct timer_list* ptimer, signed int timer_length);
static void work_handler(struct work_struct *work);

static int wq_init(void);
static void wq_exit(void);

static struct timer_list timerBattery;/*Kernel timer*/ 
static struct work_data * data;
struct file *fd;

static int initialised = 0;
 
struct work_data {
    struct work_struct work;
    struct file *fd;
};

static void batteryTimerHandler(struct timer_list *t){
    
    printk(KERN_INFO "%s called (%ld)\n", __func__, jiffies);    

    if(schedule_work(&data->work) == 0){
      printk("Failed to schedule work\n");
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
    int ret = 0;    
    int result = 0;
    int xddp_result = 0;
    char buf[128];
    int n;

    mm_segment_t fs; 
    struct power_supply *psy = power_supply_get_by_name("BAT1");
    union power_supply_propval chargenow,chargefull,voltage,intensity,capacity;

    printd();   

    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CHARGE_NOW,&chargenow);
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CHARGE_FULL,&chargefull);
    
    printk("\n");    

    if(!result) {
        printk(KERN_INFO "Charge level : %d / %d\n",chargenow.intval,chargefull.intval);
    }
    
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CAPACITY,&capacity); 
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_VOLTAGE_NOW,&voltage);
    result|= power_supply_get_property(psy,POWER_SUPPLY_PROP_CURRENT_NOW,&intensity);

    if(!result) {
        printk(KERN_INFO "Battery percent: %d%c (%d uV,%d uA)\n\n",capacity.intval,'%',&voltage,&intensity);
    }

    //Pass parameters through workqueue
    struct work_data *data = container_of(work, struct work_data, work);
      
    //Write
    n = sprintf(buf,"[%d,%d,%d,%d,%d]\n",capacity,chargenow,chargefull,BATTERY_SIZE,PRODUCED_ENERGY);
    
    fs = get_fs();
    set_fs(KERNEL_DS);
    data->fd->f_op->write(data->fd, buf, strlen(buf),&(data->fd->f_pos)); 
    printk(KERN_INFO "Write buf %s with len %d\n",buf,strlen(buf));
    set_fs(fs);      

    #if 0
      kfree(data);
    #endif

    //Relaunch timer
    ret = mod_timer(&timerBattery, jiffies + msecs_to_jiffies(TIMER_LENGTH));
    
    if (ret){
    	pr_err("%s: Timer firing failed\n", __func__);
    }

}

static int wq_init(void)
{
    char devname[12];
    printd();
    
    mm_segment_t fs; 

    data = kmalloc(sizeof(struct work_data), GFP_KERNEL);
    timer_register(&timerBattery,TIMER_LENGTH);

    //Init here and pass fd through arguments (cf pthread example)    

    if (initialised == 0) {

 	//Open
	if (sprintf(devname, "/dev/rtp%d", XDDP_PORT) < 0){
	printk(KERN_INFO "Failed using asprintf\n");
	}

        fs = get_fs();
        set_fs(KERNEL_DS);

        fd = filp_open(devname,O_WRONLY, 0);
	set_fs(fs);

        printk(KERN_ALERT "1 Attempt to open file with result %d\n",fd);     

	if (IS_ERR(fd)) {
	printk(KERN_INFO "Failed to open file\n");
	}else {
	printk(KERN_INFO "Open file %s\n",devname);
}      
         
        data->fd = fd;	    
        INIT_WORK(&data->work, work_handler);
 
        initialised = 1;
    }
 
    return 0;
}
 
static void wq_exit(void)
{
    int ret;
    mm_segment_t fs; 
    
    ret = del_timer(&timerBattery);
    if (ret){
      pr_err("%s: Timer still used ...\n", __func__);
    }

   //Close
    fs = get_fs();
    set_fs(KERNEL_DS);
    
    
    if(fd)
      filp_close(fd,NULL);
    
    set_fs(fs);
    

    flush_scheduled_work();  

    printd();
}
 
module_init(wq_init);
module_exit(wq_exit);

MODULE_AUTHOR("alexy <alexy.debus@eleves.ec-nantes.fr>");
MODULE_DESCRIPTION("Battery module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

