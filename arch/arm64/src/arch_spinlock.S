.globl arch_spin_lock
.type arch_spin_lock, function
.section .text, "ax"
arch_spin_lock:
	mov w2, #1
	sevl
1:
	wfe
	ldaxr w1, [x0]
	cbnz w1, 1b
	stxr w1, w2, [x0]
	cbnz w1, 1b
	ret

.globl arch_spin_try_lock
.type arch_spin_try_lock, function
.section .text, "ax"
arch_spin_try_lock:
	mov w2, #1
	ldaxr w1, [x0]
	cbnz w1, 1f
	stxr w1, w2, [x0]
1:
	mov w0, w1
	ret

.globl arch_spin_unlock
.type arch_spin_unlock, function
.section .text, "ax"
arch_spin_unlock:
	stlr xzr, [x0]
	ret
