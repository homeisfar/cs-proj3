#ifndef VM_FRAME_H
#define VM_FRAME_H

/* Create a frame entry that contains the pointer to a page in physical
   memory and contains the page element */
typedef struct frame_entry
{
  // uint32_t *page_ptr;
  // struct page_entry *page;
  void *phys_page;
  page_entry *page_dir_entry;
  void *page;

  // int index;
  // struct thread * | char *page_dir_base;
} frame_entry;

frame_entry *frame_table;
int frame_get_page (uint32_t *, void *, bool , page_entry *);
void *frame_get_multiple (enum palloc_flags, size_t);
void frame_free_multiple (void *, size_t);
uintptr_t *frame_evict_page ();
#endif /* vm/frame.h */
