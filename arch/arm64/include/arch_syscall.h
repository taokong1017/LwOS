#ifndef __ARM64_ARCH_SYSCALL_H__
#define __ARM64_ARCH_SYSCALL_H__

#define SVC_CALL_SYSTEM_CALL 0

#ifndef __ASSEMBLY__
#include <types.h>
#include <syscall.h>

static inline uintptr_t arch_syscall_invoke6(uintptr_t arg1, uintptr_t arg2,
											 uintptr_t arg3, uintptr_t arg4,
											 uintptr_t arg5, uintptr_t arg6,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r1 __asm__("x1") = arg2;
	register uint64_t r2 __asm__("x2") = arg3;
	register uint64_t r3 __asm__("x3") = arg4;
	register uint64_t r4 __asm__("x4") = arg5;
	register uint64_t r5 __asm__("x5") = arg6;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r1),
					   "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r8)
					 : "memory", "cc");

	return ret;
}

static inline uintptr_t arch_syscall_invoke5(uintptr_t arg1, uintptr_t arg2,
											 uintptr_t arg3, uintptr_t arg4,
											 uintptr_t arg5,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r1 __asm__("x1") = arg2;
	register uint64_t r2 __asm__("x2") = arg3;
	register uint64_t r3 __asm__("x3") = arg4;
	register uint64_t r4 __asm__("x4") = arg5;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r1),
					   "r"(r2), "r"(r3), "r"(r4), "r"(r8)
					 : "memory", "cc");

	return ret;
}

static inline uintptr_t arch_syscall_invoke4(uintptr_t arg1, uintptr_t arg2,
											 uintptr_t arg3, uintptr_t arg4,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r1 __asm__("x1") = arg2;
	register uint64_t r2 __asm__("x2") = arg3;
	register uint64_t r3 __asm__("x3") = arg4;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r1),
					   "r"(r2), "r"(r3), "r"(r8)
					 : "memory", "cc");

	return ret;
}

static inline uintptr_t arch_syscall_invoke3(uintptr_t arg1, uintptr_t arg2,
											 uintptr_t arg3,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r1 __asm__("x1") = arg2;
	register uint64_t r2 __asm__("x2") = arg3;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r1),
					   "r"(r2), "r"(r8)
					 : "memory", "cc");

	return ret;
}

static inline uintptr_t arch_syscall_invoke2(uintptr_t arg1, uintptr_t arg2,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r1 __asm__("x1") = arg2;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r1),
					   "r"(r8)
					 : "memory", "cc");

	return ret;
}

static inline uintptr_t arch_syscall_invoke1(uintptr_t arg1,
											 enum syscall_id call_id) {
	register uint64_t ret __asm__("x0") = arg1;
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r8)
					 : "memory", "cc");
	return ret;
}

static inline uintptr_t arch_syscall_invoke0(enum syscall_id call_id) {
	register uint64_t ret __asm__("x0");
	register uint64_t r8 __asm__("x8") = call_id;

	__asm__ volatile("svc %[svid]\n"
					 : "=r"(ret)
					 : [ svid ] "i"(SVC_CALL_SYSTEM_CALL), "r"(ret), "r"(r8)
					 : "memory", "cc");

	return ret;
}

#endif
#endif
