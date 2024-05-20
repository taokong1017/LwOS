#include <gen_offset.h>
#include <task_context.h>

GEN_ABS_SYM_BEGIN(ARM64_TASK_SYMBOLE)
GEN_NAMED_OFFSET_SYM(arch_task_context_t, callee_context, TASK_CONTEXT_OF_CALLEE_CONTEXT);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x19, CALLEE_CONTEXT_OF_X19_X20);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x21, CALLEE_CONTEXT_OF_X21_X22);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x23, CALLEE_CONTEXT_OF_X23_X24);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x25, CALLEE_CONTEXT_OF_X25_X26);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x27, CALLEE_CONTEXT_OF_X27_X28);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, fp, CALLEE_CONTEXT_OF_FP_LR);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, sp, CALLEE_CONTEXT_OF_SP);
GEN_NAMED_OFFSET_SYM(arch_task_context_t, fp_context, TASK_CONTEXT_OF_FP_CONTEXT);
GEN_ABS_SYM_END
