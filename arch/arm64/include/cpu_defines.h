#ifndef __ARM64_FP_CONTEXT_H__
#define __ARM64_FP_CONTEXT_H__

/* Field offsets for struct arch_regs */
#define ARM_ARCH_REGS_GPR0		0x0
#define ARM_ARCH_REGS_GPR1		0x8
#define ARM_ARCH_REGS_GPR2		0x10
#define ARM_ARCH_REGS_GPR3		0x18
#define ARM_ARCH_REGS_GPR4		0x20
#define ARM_ARCH_REGS_GPR5		0x28
#define ARM_ARCH_REGS_GPR6		0x30
#define ARM_ARCH_REGS_GPR7		0x38
#define ARM_ARCH_REGS_GPR8		0x40
#define ARM_ARCH_REGS_GPR9		0x48
#define ARM_ARCH_REGS_GPR10		0x50
#define ARM_ARCH_REGS_GPR11		0x58
#define ARM_ARCH_REGS_GPR12		0x60
#define ARM_ARCH_REGS_GPR13		0x68
#define ARM_ARCH_REGS_GPR14		0x70
#define ARM_ARCH_REGS_GPR15		0x78
#define ARM_ARCH_REGS_GPR16		0x80
#define ARM_ARCH_REGS_GPR17		0x88
#define ARM_ARCH_REGS_GPR18		0x90
#define ARM_ARCH_REGS_GPR19		0x98
#define ARM_ARCH_REGS_GPR20		0xa0
#define ARM_ARCH_REGS_GPR21		0xa8
#define ARM_ARCH_REGS_GPR22		0xb0
#define ARM_ARCH_REGS_GPR23		0xb8
#define ARM_ARCH_REGS_GPR24		0xc0
#define ARM_ARCH_REGS_GPR25		0xc8
#define ARM_ARCH_REGS_GPR26		0xd0
#define ARM_ARCH_REGS_GPR27		0xd8
#define ARM_ARCH_REGS_GPR28		0xe0
#define ARM_ARCH_REGS_GPR29		0xe8
#define ARM_ARCH_REGS_GPR30		0xf0
#define ARM_ARCH_REGS_GPR31		0xf8
#define ARM_ARCH_REGS_LR		0xf0
#define ARM_ARCH_REGS_SP		0xf8
#define ARM_ARCH_REGS_PC		0x100
#define ARM_ARCH_REGS_PSTATE	0x108
#define ARM_ARCH_REGS_SIZE		0x110

/* GPR related macros & defines */
#define CPU_GPR_COUNT	30

/* Interrupt or Exception related macros & defines */
#define EXC_OS_SYNC_SP0		0
#define EXC_OS_IRQ_SP0		1
#define EXC_OS_FIQ_SP0		2
#define EXC_OS_SERROR_SP0	3
#define EXC_OS_SYNC_SPx		4
#define EXC_OS_IRQ_SPx		5
#define EXC_OS_FIQ_SPx		6
#define EXC_OS_SERROR_SPx	7
#define EXC_APP_SYNC_A64	8
#define EXC_APP_IRQ_A64		9
#define EXC_APP_FIQ_A64		10
#define EXC_APP_SERROR_A64	11
#define EXC_APP_SYNC_A32	12
#define EXC_APP_IRQ_A32		13
#define EXC_APP_FIQ_A32		14
#define EXC_APP_SERROR_A32	15

#endif