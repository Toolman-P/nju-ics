#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <common.h>

#ifndef PGSIZE
#define PGSIZE 4096
#endif

#define PG_ALIGN __attribute((aligned(PGSIZE)))

void* new_page(size_t nr_page);
void* pg_alloc(int n);
void map(AddrSpace *as, void *va, void *pa, int prot);
void protect(AddrSpace *as);
#endif
