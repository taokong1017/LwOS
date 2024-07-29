#include <percpu.h>
#include <stack_trace.h>
#include <task.h>
#include <kernel.h>
#include <string.h>
#include <operate_regs.h>
#include <task_sched.h>
#include <msgq.h>

extern char __interrupt_stack_start[];
extern char __interrupt_stack_end[];

struct stack_info irq_stack_info(uint32_t cpu_id) {
	struct stack_info irq_stack;
	struct per_cpu *percpu = percpu_get(cpu_id);

	irq_stack.high = (virt_addr_t)percpu->irq_stack_ptr;
	irq_stack.low = (virt_addr_t)percpu->irq_stack_ptr - percpu->irq_stack_size;

	return irq_stack;
}

void percpu_init(uint32_t cpu_id) {
	int32_t i = 0;
	struct per_cpu *percpu = percpu_get(cpu_id);

	/* initialize all system queues */
	memset((void *)percpu, 0, sizeof(struct per_cpu));
	for (i = 0; i < TASK_PRIORITY_NUM; i++) {
		INIT_LIST_HEAD(&percpu->ready_queue.run_queue.queues[i]);
	}
	INIT_LIST_HEAD(&percpu->timer_queue.queue);

	/* set interrupt stack for percpu */
	percpu->irq_stack_ptr =
		(void *)__interrupt_stack_end - cpu_id * CONFIG_INTERRUPT_STACK_SIZE;
	percpu->irq_stack_size = CONFIG_INTERRUPT_STACK_SIZE;
	memset((void *)(percpu->irq_stack_ptr - percpu->irq_stack_size), 0,
		   percpu->irq_stack_size);

	/* set percpu status */
	write_tpidrro_el0((uint64_t)percpu);
	percpu->pend_sched = false;
	msgq_create(SERVICE_MSGQ_NAME, SERVICE_MSGQ_NUM, SERVICE_MSGQ_SIZE,
				&percpu->msgq_id);

	/* create service task */
	idle_task_create(cpu_id);
	system_task_create(cpu_id);
}
