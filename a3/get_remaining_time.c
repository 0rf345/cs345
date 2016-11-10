#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/t_params.h>
asmlinkage long sys_get_remaining_time(int pid, struct t_params *t_arguments) {
	struct task_struct *i;
	printk("sys_get_total_c_time running. AM:3285\n");
	if(t_arguments == NULL) {
		printk("t_arguments was NULL\n");
		return EINVAL;
	}
	if(pid == -1) {
		printk("PID was -1\n");
		return EINVAL;
	}
	for_each_process(i) {
		if(i->pid == pid) {
			t_arguments->remaining_time = i->remaining_time;
			t_arguments->infinite = i->infinite;
			printk("Got total_c_time for PID: %d\n", pid);
			return 0;
		}
	}
	printk("PID: %d was not found to be gotten\n", pid);
	return EINVAL;
}

