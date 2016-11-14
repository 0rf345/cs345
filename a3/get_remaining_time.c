#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/t_params.h>

struct task_struct *check_parents_rec(struct task_struct *task, int pid) {
	if(task == NULL) {
		return NULL;
	}
	if(pid == task->pid) {
		return task;
	}else{
		return check_parents_rec(task->real_parent, pid);
	}
}

asmlinkage long sys_get_remaining_time(int pid, struct t_params *t_arguments) {
	struct task_struct *i;
	printk("sys_get_total_c_time running. AM:3285\n");
	if(t_arguments == NULL) {
		printk("t_arguments was NULL\n");
		return EINVAL;
	}
	if(pid <= -1) {
		printk("PID was %d\n", pid);
		return EINVAL;
	}
    struct task_struct *parent;
    parent = NULL;
	for_each_process(i) {
		if(i->pid == pid) {
            parent = check_parents_rec(traverse->real_parent, current_task->pid);
            if(parent == NULL) {
                printk("ERROR. No access to get PID: %d\n", pid);
                return EINVAL;
            }else{
                t_arguments->remaining_time = i->remaining_time;
                t_arguments->infinite = i->infinite;
                printk("Got total_c_time for PID: %d\n", pid);
                return 0;
            }
		}
	}
	printk("PID: %d was not found to be gotten\n", pid);
	return EINVAL;
}
