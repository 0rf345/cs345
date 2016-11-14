#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

/* pid should be that of current process or 0 or of child
 * total time in ms 
 */

// Returns the task_struct* on success, NULL on failure
struct task_struct *check_parents_rec(struct task_struct task, int pid) {
	if(task == NULL) {
		return NULL;
	}
	if(pid == task->pid) {
		return task;
	}else{
		return check_parents_rec(task->p_pptr, pid);
	}
}

// Returns 0 on success, EINVAL on failure
asmlinkage long sys_set_total_c_time(int pid, unsigned int total_time) {
	printk("AM:3285 sys_set_total_c_time trying to set pid %d\n", pid);
	struct task_struct *current_task;
	current_task = get_current();
	if(pid == 0 || pid < -1) {
		printk("ERROR pid was %d\n");
		return EINVAL;
	}
	if(pid == -1 || current_task->pid == pid) {
		current_task->total_computation_time = total_time;
		current_task->remaining_time = total_time;
		current_task->infinite = 0;
		printk("Set own pid: %d\n", current_task->pid);
		return 0;
	}else{
		struct task_struct *parent, *traverse;
		parent = NULL;
		for_each_process(traverse) {
			if(traverse->pid == pid) { 
				parent = check_parents_rec(traverse->p_pptr, current_task->pid);
				if(parent == NULL) {
					printk("ERROR. No access to set PID: %d\n", pid);
					return EINVAL;
				}else{
					traverse->total_computation_time = total_time;
					traverse->remaining_time = total_time;
					traverse->infinite = 0;
					printk("Set child PID: %d\n", pid);
					return 0;
				}
			}
		}
		return EINVAL;
	}
}

/*
asmlinkage long sys_set_total_c_time(int pid, unsigned int total_time) {
	struct task_struct *traverse;
	int success;
	printk("sys_set_total_c_time running. AM:3285\n");
	
	traverse = get_current();
	success = 0;

	if(pid == -1 || traverse->pid == pid) {
		printk("Set own PID: %d\n", traverse->pid);
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
*/

