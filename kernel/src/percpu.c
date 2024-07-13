#include <percpu.h>
#include <stack_trace.h>
#include <task.h>
#include <kernel.h>

struct stack_info irq_stack_info(uint32_t cpu_id) {
	struct stack_info irq_stack;
	struct per_cpu *percpu = percpu_get(cpu_id);

	irq_stack.high = (phys_addr_t)percpu->irq_stack_ptr;
	irq_stack.low = (phys_addr_t)percpu->irq_stack_ptr - percpu->irq_stack_size;

	return irq_stack;
}
