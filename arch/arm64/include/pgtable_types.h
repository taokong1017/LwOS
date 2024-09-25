#ifndef __ARM64_PGTABLE_TYPES_H
#define __ARM64_PGTABLE_TYPES_H

#include <types.h>

typedef uint64_t pteval_t;
typedef uint64_t pmdval_t;
typedef uint64_t pudval_t;
typedef uint64_t pgdval_t;

/*
 * These are used to make use of C type-checking
 */
typedef struct {
	pteval_t pte;
} pte_t;
#define pte_val(x) ((x).pte)
#define pte(x) ((pte_t){(x)})

typedef struct {
	pmdval_t pmd;
} pmd_t;
#define pmd_val(x) ((x).pmd)
#define pmd(x) ((pmd_t){(x)})

typedef struct {
	pudval_t pud;
} pud_t;
#define pud_val(x) ((x).pud)
#define pud(x) ((pud_t){(x)})

typedef struct {
	pgdval_t pgd;
} pgd_t;
#define pgd_val(x) ((x).pgd)
#define pgd(x) ((pgd_t){(x)})

typedef struct {
	pteval_t pgprot;
} pgprot_t;
#define pgprot_val(x) ((x).pgprot)
#define pgprot(x) ((pgprot_t){(x)})

#endif
