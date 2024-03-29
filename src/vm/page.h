#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"
#include "threads/palloc.h"
#include "lib/kernel/bitmap.h"
#include <stdint.h>
#include <filesys/file.h>

/* The following bits are used in the "meta" entry in the page_entry
    struct from below. We use this to tell us a little bit of information
    while trying to remain memory efficient. */
#define is_in_frame(x) ((x) & 1)		/* Tells us if it is in frame */
#define is_in_fs(x) ((x) & 2)			/* Tells us if it is in filesystem */
#define is_in_swap(x) ((x) & 4)         /* Tells us if is in swap */
#define is_zero_pages(x) ((x) & 8)		/* User is requesting all 0 pages */
#define is_writeable(x) ((x) & 16)		/* Page can be written to. Useful for shared memory */
#define is_stack(x) ((x) & 32)			/* Is the page a stack page? */
#define is_mmap(x) ((x) & 64)           /* Is the page an mmap page? */
#define is_mmap_final(x) ((x) & 128)    /* Is the page the final mapped page for mmap? */

#define set_in_frame(x) ((x) |= 1)
#define set_fs(x) ((x) |= 2)
#define set_in_swap(x) ((x) |= 4)
#define set_zero_pages(x) ((x) |= 8)
#define set_writeable(x) ((x) |= 16)
#define set_stack(x) ((x) |= 32)
#define set_mmap(x) ((x) |= 64)
#define set_mmap_final(x) ((x) |= 128)

#define clear_in_frame(x) ((x) &= ~1)
#define clear_fs(x) ((x) &= ~2)
#define clear_in_swap(x) ((x) &= ~4)
#define clear_zero_pages(x) ((x) &= ~8)
#define clear_writeable(x) ((x) &= ~16)
#define clear_stack(x) ((x) &= ~32)
#define clear_mmap(x) ((x) &= ~64)
#define clear_mmap_final(x) ((x) &= ~128)

typedef struct page_entry {
	struct hash_elem page_elem;
	uint8_t meta;
	struct file *f;
	off_t ofs;
	uint8_t *upage;
    uint32_t read_bytes; 
    uint32_t zero_bytes;
    void *phys_page;
    size_t swap_index;
} page_entry;

void vm_init (size_t user_page_limit);
void *vm_get_page (enum palloc_flags);
void *vm_get_multiple (enum palloc_flags, size_t page_cnt);
void vm_free_page (void *);
void vm_free_multiple (void *, size_t page_cnt);
void init_supp_page_dir (void);

page_entry *page_get_entry (struct hash *, void *);
unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED);
/* Returns true if page a precedes page b. */
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_,
                void *aux UNUSED);
struct hash_elem *page_insert_entry_exec (struct file *, off_t, uint8_t *,
                                          uint32_t, uint32_t, bool);
struct hash_elem *page_insert_entry_mmap (uint8_t *, 
                struct file *, off_t, off_t, bool);
struct hash_elem *page_remove_entry (struct hash_elem *);
struct hash_elem *page_remove_address (void *);
extern page_entry *page_entry_supp;

#endif /* vm/page.h */
