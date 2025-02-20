#include <gen_offset.h>
#include <arch_task.h>
#include <task.h>
#include <kernel.h>
#include <percpu.h>
#include <smccc.h>
#include <arch_smp.h>
#include <mem_domain.h>

GEN_ABS_SYM_BEGIN(ARM64_OFFSET_SYMBOLE)
GEN_NAMED_OFFSET_SYM(struct task, status, TASK_OF_STATUS);
GEN_NAMED_OFFSET_SYM(struct task, entry, TASK_OF_ENTRY);
GEN_NAMED_OFFSET_SYM(struct task, args, TASK_OF_ARGS);
GEN_NAMED_OFFSET_SYM(struct task, flag, TASK_OF_FLAG);
GEN_NAMED_OFFSET_SYM(struct task, stack_ptr, TASK_OF_STACK_PTR);
GEN_NAMED_OFFSET_SYM(struct task, task_context, TASK_OF_ARCH_TASK_CONTEXT);
GEN_NAMED_OFFSET_SYM(struct task, mem_domain, TASK_OF_MEM_DOMAIN);
GEN_NAMED_OFFSET_SYM(arch_task_context_t, callee_context,
					 TASK_CONTEXT_OF_CALLEE_CONTEXT);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x19, CALLEE_CONTEXT_OF_X19);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x20, CALLEE_CONTEXT_OF_X20_X21);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x22, CALLEE_CONTEXT_OF_X22_X23);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x24, CALLEE_CONTEXT_OF_X24_X25);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x26, CALLEE_CONTEXT_OF_X26_X27);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x28, CALLEE_CONTEXT_OF_X28_X29);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, x30, CALLEE_CONTEXT_OF_X30_SP);
GEN_NAMED_OFFSET_SYM(arch_callee_context_t, daif, CALLEE_CONTEXT_OF_DAIF_NZCV);

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

GEN_NAMED_OFFSET_SYM(struct per_cpu, irq_nested_cnt, PER_CPU_OF_IRQ_NESTED_CNT);
GEN_NAMED_OFFSET_SYM(struct per_cpu, irq_stack_ptr, PER_CPU_OF_IRQ_STACK_PTR);
GEN_NAMED_OFFSET_SYM(struct per_cpu, irq_stack_size, PER_CPU_OF_IRQ_STACK_SIZE);

GEN_NAMED_OFFSET_SYM(struct arch_smccc, a0, ARCH_SMMCC_A0_A1);
GEN_NAMED_OFFSET_SYM(struct arch_smccc, a2, ARCH_SMMCC_A2_A3);
GEN_NAMED_OFFSET_SYM(struct arch_smccc, a4, ARCH_SMMCC_A4_A5);
GEN_NAMED_OFFSET_SYM(struct arch_smccc, a6, ARCH_SMMCC_A6_A7);

GEN_NAMED_OFFSET_SYM(struct boot_params, cpu_id, BOOT_PARAMS_OF_CPU_ID);
GEN_NAMED_OFFSET_SYM(struct boot_params, mp_id, BOOT_PARAMS_OF_MP_ID);
GEN_NAMED_OFFSET_SYM(struct boot_params, func, BOOT_PARAMS_OF_FUNC);
GEN_NAMED_OFFSET_SYM(struct boot_params, arg, BOOT_PARAMS_OF_ARG);

GEN_NAMED_OFFSET_SYM(struct mem_domain, arch_mem_domain,
					 MEM_DOMAIN_OF_ARCH_MEM_DOMAIN);
GEN_NAMED_OFFSET_SYM(struct arch_mem_domain, pgtable,
					 ARCH_MEM_DOMAIN_OF_PGTABLE);
GEN_NAMED_OFFSET_SYM(struct mmu_pgtable, ttbr0, MMU_PGTABLE_OF_TTBR0);

GEN_ABS_SYM_END
