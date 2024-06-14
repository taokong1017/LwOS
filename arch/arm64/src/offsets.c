#include <gen_offset.h>
#include <arch_task.h>
#include <task.h>

GEN_ABS_SYM_BEGIN(ARM64_TASK_SYMBOLE)
GEN_NAMED_OFFSET_SYM(struct task, entry, TASK_OF_ENTRY);
GEN_NAMED_OFFSET_SYM(struct task, args, TASK_OF_ARGS);
GEN_NAMED_OFFSET_SYM(struct task, stack_ptr, TASK_OF_STACK_PTR);
GEN_NAMED_OFFSET_SYM(arch_task_context_t, callee_context,
					 TASK_CONTEXT_OF_CALLEE_CONTEXT);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x19, CALLEE_CONTEXT_OF_X19_X20);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x21, CALLEE_CONTEXT_OF_X21_X22);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x23, CALLEE_CONTEXT_OF_X23_X24);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x25, CALLEE_CONTEXT_OF_X25_X26);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x27, CALLEE_CONTEXT_OF_X27_X28);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x29, CALLEE_CONTEXT_OF_X29_SP_EL0);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, sp_elx,
					 CALLEE_CONTEXT_OF_SP_ELx_LR);
#ifdef CONFIG_FPU_ENABLE
GEN_NAMED_OFFSET_SYM(arch_task_context_t, fp_context,
					 TASK_CONTEXT_OF_FP_CONTEXT);
#endif
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x0, ESF_CONTEXT_OF_X0_X1);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x2, ESF_CONTEXT_OF_X2_X3);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x4, ESF_CONTEXT_OF_X4_X5);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x6, ESF_CONTEXT_OF_X6_X7);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x8, ESF_CONTEXT_OF_X8_X9);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x10, ESF_CONTEXT_OF_X10_X11);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x12, ESF_CONTEXT_OF_X12_X13);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x14, ESF_CONTEXT_OF_X14_X15);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x16, ESF_CONTEXT_OF_X16_X17);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, x18, ESF_CONTEXT_OF_X18_LR);
GEN_NAMED_OFFSET_SYM(struct arch_esf_context, spsr, ESF_CONTEXT_OF_SPSR_ELR);
GEN_NAMED_SIZE_SYM(struct arch_esf_context, ESF_CONTEXT);

GEN_ABS_SYM_END
