#ifndef VM_PAGE_H
#define VM_PAGE_H

struct page_entry{
  // uint32_t * page_ptr;
	 //  bool dirty_bit = pagedir_is_dirty (pagedir, page);
  // bool reference_bit bad code
};

void vm_init (size_t user_page_limit);
void *vm_get_page (enum palloc_flags);
void *vm_get_multiple (enum palloc_flags, size_t page_cnt);
void vm_free_page (void *);
void vm_free_multiple (void *, size_t page_cnt);

#endif /* vm/page.h */