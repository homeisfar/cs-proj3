#ifndef VM_PAGE_H
#define VM_PAGE_H

struct page_entry{
  // uint32_t * page_ptr;
	 //  bool dirty_bit = pagedir_is_dirty (pagedir, page);
  // bool reference_bit bad code
} typedef page_entry;


struct page_dir 
{
    // meta-data
    uint32_t *pd;
    page_entry *pages;
} typedef page_dir;

void vm_init (size_t user_page_limit);
void *vm_get_page (enum palloc_flags);
void *vm_get_multiple (enum palloc_flags, size_t page_cnt);
void vm_free_page (void *);
void vm_free_multiple (void *, size_t page_cnt);

void init_supp_page_dir();

// extern uint32_t **page_dir_supp;
extern page_dir *page_dir_supp;

#endif /* vm/page.h */
