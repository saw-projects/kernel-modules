#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Developer");
MODULE_DESCRIPTION("A simple module to list processes with state and memory info");
MODULE_VERSION("0.1");

static const char *get_task_state(long state)
{
    switch (state) {
        case TASK_RUNNING:
            return "RUNNING";
        case TASK_INTERRUPTIBLE:
            return "SLEEPING(INTERRUPTIBLE)";
        case TASK_UNINTERRUPTIBLE:
            return "SLEEPING(UNINTERRUPTIBLE)";
        case __TASK_STOPPED:
            return "STOPPED";
        case __TASK_TRACED:
            return "TRACED";
        case TASK_DEAD:
            return "DEAD";
        case TASK_WAKEKILL:
            return "WAKEKILL";
        case TASK_WAKING:
            return "WAKING";
        case TASK_PARKED:
            return "PARKED";
        case TASK_NOLOAD:
            return "NOLOAD";
        case TASK_NEW:
            return "NEW";
        default:
            return "UNKNOWN";
    }
}

static int __init ps_list_init(void)
{
    struct task_struct *task;
    unsigned long vm_size;

    printk(KERN_INFO "ps_list: Module loaded\n");
    printk(KERN_INFO "ps_list: Process list:\n");
    
    for_each_process(task) {
        if (task->mm) {  // Handle user processes
            vm_size = task->mm->total_vm << (PAGE_SHIFT - 10); // Convert to KB
        } else {  // Handle kernel threads
            vm_size = 0;
        }
        
        printk(KERN_INFO "ps_list: Process: %s [PID: %d] State: %s VM: %lu KB\n",
               task->comm, task->pid, 
               get_task_state(task->state),
               vm_size);
    }

    return 0;
}

static void __exit ps_list_exit(void)
{
    printk(KERN_INFO "ps_list: Module unloaded\n");
}

module_init(ps_list_init);
module_exit(ps_list_exit);
