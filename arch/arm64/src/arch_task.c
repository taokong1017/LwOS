#include <arch_task.h>
#include <task.h>
#include <cpu.h>
#include <string.h>

extern void task_kernel_entry_point(task_id_t task_id);
extern void task_user_entry_point(task_id_t task_id);
extern void arch_exc_exit();

void arch_task_init(task_id_t task_id) {
	struct arch_esf_context *esf = NULL;
	struct task *task = ID_TO_TASK(task_id);

	/* root任务主动切换调度 */
	memset(task->stack_ptr - task->stack_size, 0xaa, task->stack_size);
	esf = (struct arch_esf_context *)(task->stack_ptr -
									  sizeof(struct arch_esf_context));
	esf->x0 = (uint64_t)task_id;
#ifdef CONFIG_USER_SPACE
	if (task_is_user(task_id)) {
		esf->spsr = SPSR_MODE_EL0T | DAIF_FIQ_BIT | DAIF_IRQ_BIT;
		esf->elr = (uint64_t)task_user_entry_point;
	} else {
#else
	{
#endif
		esf->spsr = SPSR_MODE_EL1H | DAIF_FIQ_BIT | DAIF_IRQ_BIT;
		esf->elr = (uint64_t)task_kernel_entry_point;
	}

	/* 其它任务，在任务切换后，通过异常退出流程进入 */
	task->task_context.callee_context.sp = (uint64_t)esf;
	task->task_context.callee_context.x30 = (uint64_t)arch_exc_exit;
}
