#ifndef VM_FRAME_H
#define VM_FRAME_H

/* Create a frame entry that contains the pointer to a page in physical
   memory and contains the page element */
struct frame_entry
{
  // uint32_t *page_ptr;
  // struct page_entry *page;
  void *phys_page;
  page_entry *page_dir_entry;
  void *page;

  // int index;
  // struct thread * | char *page_dir_base;
} typedef frame_entry;

extern frame_entry *frame_table;
#endif /* vm/frame.h */
