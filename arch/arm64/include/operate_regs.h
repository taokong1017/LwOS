#include <types.h>

#define declare_reg_operation(reg) \
uint64_t read_##reg(void); 		\
void write_##reg(uint64_t val);	\
void zero_##reg(void);

#define declare_el123_reg_operation(reg) \
declare_reg_operation(reg##_el1) \
declare_reg_operation(reg##_el2) \
declare_reg_operation(reg##_el3)

/* define special register operations */
declare_reg_operation(cntfrq_el0)
declare_reg_operation(cnthctl_el2)
declare_reg_operation(cnthp_ctl_el2)
declare_reg_operation(cntv_ctl_el0)
declare_reg_operation(cntv_cval_el0)
declare_reg_operation(cntvoff_el2)
declare_reg_operation(csselr_el1)
declare_reg_operation(daif)
declare_reg_operation(hcr_el2)
declare_reg_operation(par_el1)
declare_reg_operation(scr_el3)
declare_reg_operation(tpidrro_el0)
declare_reg_operation(vmpidr_el2)
declare_reg_operation(sp_el0)

declare_el123_reg_operation(actlr)
declare_el123_reg_operation(elr)
declare_el123_reg_operation(esr)
declare_el123_reg_operation(far)
declare_el123_reg_operation(mair)
declare_el123_reg_operation(sctlr)
declare_el123_reg_operation(spsr)
declare_el123_reg_operation(tcr)
declare_el123_reg_operation(ttbr0)
declare_el123_reg_operation(vbar)
