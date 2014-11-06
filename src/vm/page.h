#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"
#include "threads/palloc.h"
#include "lib/kernel/bitmap.h"

#define is_alloc(x) (x & 1)		/* Tells us if it's in frame or swap */
#define is_in_frame(x) (x & 6)	/* More specifically, if it is in frame or swap, or zero page. We need 2 bits */
#define is_writeable(x) (x & 8)	/* Page can be written to. Useful for shared memory */
#define is_stack(x) (x & 16)	/* Is the page a stack page? For stack growth */

#define set_frame(x) (x |= 2)
/*
	We also want to keep track in our supp table of:

	Files that are open || struct array?
	Offsets in said files || uint32 array?
	Does this information need to be malloc'd and free?

*/

typedef struct page_entry{
  // uint32_t * page_ptr;
	 // bool dirty_bit = pagedir_is_dirty (pagedir, page);
  // bool reference_bit bad code
	void *vaddr;
	struct hash_elem page_elem;
	uint8_t meta;
	/*
		swap meta data
	*/
} page_entry;


typedef struct page_dir
{
    // meta-data
    uint32_t *pd;
    page_entry *pages;
} page_dir;



void vm_init (size_t user_page_limit);
void *vm_get_page (enum palloc_flags);
void *vm_get_multiple (enum palloc_flags, size_t page_cnt);
void vm_free_page (void *);
void vm_free_multiple (void *, size_t page_cnt);

void init_supp_page_dir();

page_entry *
page_get_entry (struct hash *, void *);

unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED);

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED);

size_t
page_obtain_pages (struct bitmap *, size_t, size_t);
// extern uint32_t **page_dir_supp;
extern page_dir *page_dir_supp;

#endif /* vm/page.h */
