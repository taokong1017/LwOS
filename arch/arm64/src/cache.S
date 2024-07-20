.macro dcache_line_size, reg, tmp
	mrs  \tmp, ctr_el0        // read CTR
	nop
	ubfm \tmp, \tmp, #16, #19 // cache line size encoding
	mov  \reg, #4             // bytes per word
	lsl  \reg, \reg, \tmp     // actual cache line size
.endm

.macro dcache_by_line_op op, domain, start, end, linesz, tmp 
	sub \tmp,   \linesz, #1
	bic \start, \start,  \tmp
.Ldcache_op\@:
	.ifc \op, cvap
	sys 3, c7, c12, 1, \start    // dc cvap
	.else
	dc  \op, \start
	.endif
	add \start, \start, \linesz
	cmp \start, \end
	b.lo .Ldcache_op\@
	dsb \domain
.endm

.macro icache_line_size, reg, tmp
	mrs \tmp, ctr_el0        // read CTR
	nop
	and \tmp, \tmp, #0xf     // cache line size encoding
	mov \reg, #4             // bytes per word
	lsl \reg, \reg, \tmp     // actual cache line size
.endm

.macro invalidate_icache_by_line start, end, tmp1, tmp2
	icache_line_size \tmp1, \tmp2
	sub \tmp2, \tmp1, #1
	bic \tmp2, \start, \tmp2
.Licache_op\@:
	ic  ivau, \tmp2          // invalidate I line PoU
	add \tmp2, \tmp2, \tmp1
	cmp \tmp2, \end
	b.lo .Licache_op\@
	dsb ish
	isb
.endm

/*
 * dcache_inval_poc(start, end)
 */
.text
.globl dcache_inval_poc
.type dcache_inval_poc, function
.section .text, "ax"
dcache_inval_poc:
	dcache_line_size x2, x3
	sub x3, x2, #1
	tst x1, x3     // end cache line aligned?
	bic x1, x1, x3
	b.eq 1f
	dc civac, x1   // clean & invalidate D / U line
1:
	tst x0, x3     // start cache line aligned?
	bic x0, x0, x3
	b.eq 2f
	dc civac, x0   // clean & invalidate D / U line
	b 3f
2:
	dc ivac, x0    // invalidate D / U line
3:
	add x0, x0, x2
	cmp x0, x1
	b.lo 2b
	dsb sy
	ret

/*
 * dcache_clean_poc(start, end)
 */
.text
.globl dcache_clean_poc
.type dcache_clean_poc, function
.section .text, "ax"
dcache_clean_poc:
	dcache_line_size x2, x3
	dcache_by_line_op cvac, sy, x0, x1, x2, x3
	ret

/*
 * dcache_clean_inval_poc(start, end)
 */
.text
.globl dcache_clean_inval_poc
.type dcache_clean_inval_poc, function
.section .text, "ax"
dcache_clean_inval_poc:
	dcache_line_size x2, x3
	dcache_by_line_op civac, sy, x0, x1, x2, x3
	ret

/*
 * dcache_clean_pop(start, end)
 */
.text
.globl dcache_clean_pop
.type dcache_clean_pop, function
.section .text, "ax"
dcache_clean_pop:
	b dcache_clean_poc
	dcache_line_size x2, x3
	dcache_by_line_op cvap, sy, x0, x1, x2, x3
	ret

/*
 * icache_inval_pou(start,end)
 */
.text
.globl icache_inval_pou
.type icache_inval_pou, function
.section .text, "ax"
icache_inval_pou:
	invalidate_icache_by_line x0, x1, x2, x3
	ret
