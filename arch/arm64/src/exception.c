#include <arch_regs.h>
#include <esr.h>
#include <operate_regs.h>
#include <stdio.h>
#include <stack_trace.h>

#define PSR_F_BIT 0x00000040
#define PSR_I_BIT 0x00000080
#define PSR_A_BIT 0x00000100
#define PSR_D_BIT 0x00000200
#define PSR_BTYPE_MASK 0x00000c00
#define PSR_SSBS_BIT 0x00001000
#define PSR_PAN_BIT 0x00400000
#define PSR_UAO_BIT 0x00800000
#define PSR_DIT_BIT 0x01000000
#define PSR_TCO_BIT 0x02000000
#define PSR_V_BIT 0x10000000
#define PSR_C_BIT 0x20000000
#define PSR_Z_BIT 0x40000000
#define PSR_N_BIT 0x80000000
#define PSR_BTYPE_SHIFT 10

/* Convenience names for the values of PSTATE.BTYPE */
#define PSR_BTYPE_NONE (0b00 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_JC (0b01 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_C (0b10 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_J (0b11 << PSR_BTYPE_SHIFT)

#define bstr(suffix, str) [PSR_BTYPE_##suffix >> PSR_BTYPE_SHIFT] = str
static const char *const btypes[] = {bstr(NONE, "--"), bstr(JC, "jc"),
									 bstr(C, "-c"), bstr(J, "j-")};

static const char *esr_class_str[] = {
	[0 ... ESR_ELx_EC_MAX] = "UNRECOGNIZED EC",
	[ESR_ELx_EC_UNKNOWN] = "Unknown/Uncategorized",
	[ESR_ELx_EC_WFx] = "WFI/WFE",
	[ESR_ELx_EC_CP15_32] = "CP15 MCR/MRC",
	[ESR_ELx_EC_CP15_64] = "CP15 MCRR/MRRC",
	[ESR_ELx_EC_CP14_MR] = "CP14 MCR/MRC",
	[ESR_ELx_EC_CP14_LS] = "CP14 LDC/STC",
	[ESR_ELx_EC_FP_ASIMD] = "ASIMD",
	[ESR_ELx_EC_CP10_ID] = "CP10 MRC/VMRS",
	[ESR_ELx_EC_PAC] = "PAC",
	[ESR_ELx_EC_CP14_64] = "CP14 MCRR/MRRC",
	[ESR_ELx_EC_BTI] = "BTI",
	[ESR_ELx_EC_ILL] = "PSTATE.IL",
	[ESR_ELx_EC_SVC32] = "SVC (AArch32)",
	[ESR_ELx_EC_HVC32] = "HVC (AArch32)",
	[ESR_ELx_EC_SMC32] = "SMC (AArch32)",
	[ESR_ELx_EC_SVC64] = "SVC (AArch64)",
	[ESR_ELx_EC_HVC64] = "HVC (AArch64)",
	[ESR_ELx_EC_SMC64] = "SMC (AArch64)",
	[ESR_ELx_EC_SYS64] = "MSR/MRS (AArch64)",
	[ESR_ELx_EC_SVE] = "SVE",
	[ESR_ELx_EC_ERET] = "ERET/ERETAA/ERETAB",
	[ESR_ELx_EC_FPAC] = "FPAC",
	[ESR_ELx_EC_SME] = "SME",
	[ESR_ELx_EC_IMP_DEF] = "EL3 IMP DEF",
	[ESR_ELx_EC_IABT_LOW] = "IABT (lower EL)",
	[ESR_ELx_EC_IABT_CUR] = "IABT (current EL)",
	[ESR_ELx_EC_PC_ALIGN] = "PC Alignment",
	[ESR_ELx_EC_DABT_LOW] = "DABT (lower EL)",
	[ESR_ELx_EC_DABT_CUR] = "DABT (current EL)",
	[ESR_ELx_EC_SP_ALIGN] = "SP Alignment",
	[ESR_ELx_EC_MOPS] = "MOPS",
	[ESR_ELx_EC_FP_EXC32] = "FP (AArch32)",
	[ESR_ELx_EC_FP_EXC64] = "FP (AArch64)",
	[ESR_ELx_EC_SERROR] = "SError",
	[ESR_ELx_EC_BREAKPT_LOW] = "Breakpoint (lower EL)",
	[ESR_ELx_EC_BREAKPT_CUR] = "Breakpoint (current EL)",
	[ESR_ELx_EC_SOFTSTP_LOW] = "Software Step (lower EL)",
	[ESR_ELx_EC_SOFTSTP_CUR] = "Software Step (current EL)",
	[ESR_ELx_EC_WATCHPT_LOW] = "Watchpoint (lower EL)",
	[ESR_ELx_EC_WATCHPT_CUR] = "Watchpoint (current EL)",
	[ESR_ELx_EC_BKPT32] = "BKPT (AArch32)",
	[ESR_ELx_EC_VECTOR32] = "Vector catch (AArch32)",
	[ESR_ELx_EC_BRK64] = "BRK (AArch64)",
};

const char *esr_get_class_string(unsigned long esr) {
	return esr_class_str[ESR_ELx_EC(esr)];
}

static void print_pstate(struct arch_regs *regs) {
	uint64_t pstate = regs->pstate;
	const char *btype_str =
		btypes[(pstate & PSR_BTYPE_MASK) >> PSR_BTYPE_SHIFT];

	printf("pstate: %08llx (%c%c%c%c %c%c%c%c %cPAN %cUAO %cTCO %cDIT %cSSBS "
		   "BTYPE=%s)\n",
		   pstate, pstate & PSR_N_BIT ? 'N' : 'n',
		   pstate & PSR_Z_BIT ? 'Z' : 'z', pstate & PSR_C_BIT ? 'C' : 'c',
		   pstate & PSR_V_BIT ? 'V' : 'v', pstate & PSR_D_BIT ? 'D' : 'd',
		   pstate & PSR_A_BIT ? 'A' : 'a', pstate & PSR_I_BIT ? 'I' : 'i',
		   pstate & PSR_F_BIT ? 'F' : 'f', pstate & PSR_PAN_BIT ? '+' : '-',
		   pstate & PSR_UAO_BIT ? '+' : '-', pstate & PSR_TCO_BIT ? '+' : '-',
		   pstate & PSR_DIT_BIT ? '+' : '-', pstate & PSR_SSBS_BIT ? '+' : '-',
		   btype_str);
}

static void show_regs(struct arch_regs *regs) {
	int i = CPU_GPR_COUNT;
	uint64_t lr = regs->lr;
	uint64_t sp = regs->sp;

	print_pstate(regs);
	printf("pc : 0x%016llx\n", regs->pc);
	printf("lr : 0x%016llx\n", lr);
	printf("sp : 0x%016llx\n", sp);

	while (i >= 0) {
		printf("x%-2d: 0x%016llx", i, regs->gprs[i]);

		while (i-- % 3) {
			printf(" x%-2d: 0x%016llx", i, regs->gprs[i]);
		}

		printf("\n");
	}
}

static bool show_Linker(void *cookie, phys_addr_t pc) {
	uint32_t *level = (uint32_t *)cookie;
	printf("%u: 0x%016llx\n", (*level)++, pc);
	return true;
}

static void panic_unhandled(struct arch_regs *regs, const char *vector,
							unsigned long esr) {
	uint32_t level = 0;
	printf("Unhandled %s exception, ESR 0x%016lx -- %s\n", vector, esr,
		   esr_get_class_string(esr));
	show_regs(regs);
	printf("Stack trace:\n");
	arch_stack_walk(show_Linker, &level, NULL, regs);
	while (1) {
		;
	}
}

#define unhandled_handler_define(el, regsize, vector)                          \
	void el##_##regsize##_##vector##_handler(struct arch_regs *regs) {         \
		const char *desc = #regsize "-bit " #el " " #vector;                   \
		panic_unhandled(regs, desc, read_esr_el1());                           \
	}

unhandled_handler_define(el1t, 64, sync);
unhandled_handler_define(el1t, 64, irq);
unhandled_handler_define(el1t, 64, firq);
unhandled_handler_define(el1t, 64, serror);

unhandled_handler_define(el1h, 64, sync);
unhandled_handler_define(el1h, 64, firq);
unhandled_handler_define(el1h, 64, serror);

unhandled_handler_define(el0t, 64, sync);
unhandled_handler_define(el0t, 64, irq);
unhandled_handler_define(el0t, 64, firq);
unhandled_handler_define(el0t, 64, serror);

unhandled_handler_define(el0t, 32, sync);
unhandled_handler_define(el0t, 32, irq);
unhandled_handler_define(el0t, 32, firq);
unhandled_handler_define(el0t, 32, serror);
