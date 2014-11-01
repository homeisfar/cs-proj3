#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"
#include "threads/palloc.h"

#define is_alloc(x) (x & 1)
#define is_in_frame(x) (x & 2)
#define is_writeable(x) (x & 4)

#define set_frame(x) (x |= 2)
/*
	
*/

typedef struct page_entry{
  // uint32_t * page_ptr;
	 //  bool dirty_bit = pagedir_is_dirty (pagedir, page);
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
get_page_entry (struct hash *, void *);

unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED);

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED);
// extern uint32_t **page_dir_supp;
extern page_dir *page_dir_supp;

#endif /* vm/page.h */
