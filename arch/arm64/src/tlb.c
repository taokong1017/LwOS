#include <types.h>
#include <tlb.h>
#include <compiler.h>
#include <menuconfig.h>
#include <mmu.h>

#define tlbi_0(op, arg) __asm__("tlbi " #op "\n" ::)
#define tlbi_1(op, arg) __asm__("tlbi " #op ", %0\n" : : "r"(arg))
#define tlbi_N(op, arg, n, ...) tlbi_##n(op, arg)
#define tlbi(op, ...) tlbi_N(op, ##__VA_ARGS__, 1, 0)

void tlb_local_flush() {
	dsb(nshst);
	tlbi(vmalle1);
	dsb(nsh);
	isb();
}

void tlb_all_flush() {
	dsb(ishst);
	tlbi(vmalle1is);
	dsb(ish);
	isb();
}

void tlb_range_flush(virt_addr_t start, virt_addr_t end) {
	virt_addr_t addr = start;

	dsb(ishst);
	for (; addr < end; addr += 1 << (PAGE_SHIFT - 12)) {
		tlbi(vaale1is, addr);
	}
	dsb(ish);
	isb();
}
