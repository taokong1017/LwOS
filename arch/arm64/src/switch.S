#include <offsets.h>

.globl arch_context_switch
.type arch_context_switch, function
.section .text, "ax"
arch_context_switch:
	stp	x19, x20, [x1, #__arch_callee_context_t_x19_x20_OFFSET]
	stp	x21, x22, [x1, #__arch_callee_context_t_x21_x22_OFFSET]

