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
GEN_ABS_SYM_END
