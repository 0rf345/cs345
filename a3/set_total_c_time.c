#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

/* pid should be that of current process or 0 or of child
 * total time in ms 
 */
asmlinkage long sys_set_total_c_time(int pid, unsigned int total_time) {
	struct task_struct *traverse;
	int success;
	printk("sys_set_total_c_time running. AM:3285\n");
	
	traverse = get_current();
	success = 0;

	if(pid == -1 || traverse->pid == pid) {
		printk("Set own PID: %d\n", pid);
		traverse->total_computation_time = total_time;
		traverse->remaining_time = total_time;
		traverse->infinite = 0;
		success = 1;
	}else{
		struct list_head *list;
		struct task_struct *i;
		list_for_each(list, &traverse->children) {
			i = list_entry(list, struct task_struct, sibling);
			printk("Child with PID: %d is being inspected\n", i->pid);
			if(i->pid == pid) {
				printk("Set child PID: %d\n", pid);
				i->total_computation_time = total_time;
				i->remaining_time = total_time;
				i->infinite = 0;
				success = 1;
			}
		}
	}
	if(success == 1) return 0;
	else {
		printk("Didn't find PID: %d in children list\n", pid);
		return EINVAL;
	}
}
