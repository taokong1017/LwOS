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
	percpu->is_idle = false;

	/* create service task */
	idle_task_create(cpu_id);
}

uint32_t percpu_idle_mask_get() {
	struct per_cpu *per_cpu = NULL;
	int32_t i = 0;
	uint32_t mask = 0;

	for (i = 0; i < CONFIG_CPUS_MAX_NUM; i++) {
		per_cpu = percpu_get(i);
		if (per_cpu->is_idle) {
			mask |= TASK_CPU_AFFI(i);
		}
	}

	return mask;
}