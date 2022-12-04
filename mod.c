
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/namei.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/ptrace.h>
#include <asm/syscall.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/types.h>
#include <linux/types.h>
#include <linux/sched/cputime.h>
#include <linux/path.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/netdevice.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("AgeDim");
MODULE_DESCRIPTION("IOCTL module");
MODULE_VERSION("0.01");
#define MYMAJOR 27
#define WR_AVALUE _IOW('a','a',struct message*)
#define WR_SVALUE _IOW('a','c',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)
#define BUF_LENGHT 1024

dev_t dev = 0;
int pid = 0;

struct message {
        struct syscall_info si;
	struct task_cputime ctime;
};

struct message* msg;
struct path path;
struct syscall_info* si;
struct task_struct* tsk;
struct task_cputime* ctime;

static void fill_structs(void){
 printk(KERN_INFO "Filling......");
 tsk = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
 
 ctime = vmalloc(sizeof(struct task_cputime));
 ctime->stime = tsk->stime;
 ctime->utime = tsk->utime;
 ctime->sum_exec_runtime = tsk->stime + tsk->utime;

 unsigned long args[6]={ };
 si = vmalloc(sizeof(struct syscall_info));
 struct pt_regs* regs = task_pt_regs(tsk);
 si->sp = user_stack_pointer(regs);
 si->data.instruction_pointer = instruction_pointer(regs);
 si->data.nr = syscall_get_nr(tsk, regs);
 if(si->data.nr != -1L) syscall_get_arguments(tsk, regs, args);
 msg = vmalloc(sizeof(struct message));
 msg->si = *si;
 msg->ctime=*ctime;
}

static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}
/*
** This function will be called when we write IOCTL on the Device file
*/
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        char path_arg[BUF_LENGHT];
         switch(cmd) {
              case WR_SVALUE:
                       if( copy_from_user(&pid ,(int*) arg, sizeof(pid)) )
                       {
                              pr_err("Data Write : Err!\n");
                                                    }
                       pr_info("Pid = %d\n", pid);
                       break;
                case WR_AVALUE:
                        copy_from_user(path_arg, (char*) arg, BUF_LENGHT);
kern_path(path_arg, LOOKUP_FOLLOW, &path) ;
                        break;
                case RD_VALUE:
                         fill_structs();
                        if( copy_to_user((struct message*) arg, msg, sizeof(struct message)) )
                        {
                                pr_err("Data Read : Err!\n");
                        }
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
	vfree(ctime);
	vfree(si);
	vfree(msg);
        return 0;
}

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};


static int __init lkm_example_init(void) {
 printk(KERN_INFO "ioctl_mod: module loaded.\n");

 int retval;
    retval = register_chrdev(MYMAJOR, "etc_driver", &fops);
    if (0 == retval) {
        printk("et_driver device number Major:%d , Minor:%d\n", MYMAJOR, 0);
    } else if (retval > 0) {
        printk("etc_driver device number Major:%d , Minor:%d\n", retval >> 20, retval & 0xffff);
    } else {
        printk("Couldn't register device number!\n");
        return -1;
    }
    return 0;
}


static void __exit lkm_example_exit(void) {
  unregister_chrdev(MYMAJOR, "etc_driver");
        pr_info("Device Driver Remove...Done!!!\n");
}
module_init(lkm_example_init);
module_exit(lkm_example_exit);
