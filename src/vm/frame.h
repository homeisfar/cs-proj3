#ifndef VM_FRAME_H
#define VM_FRAME_H

/* Gets an index into the frame table based on the address given by palloc */
#define index(x) (((x) - (int) ptov (1024 * 1024)) / PGSIZE - ((int) \
ptov (init_ram_pages * PGSIZE) - (int) ptov (1024 * 1024)) / PGSIZE / 2 - 1)


/* Create a frame entry that contains the pointer to a page in physical
   memory and contains the page element */
typedef struct frame_entry
{
  page_entry *page_dir_entry;
  void *page;
} frame_entry;

frame_entry *frame_table;

bool frame_get_page (uint32_t *, void *, bool , page_entry *);
void *frame_get_stack_page (void *);
void *frame_get_multiple (enum palloc_flags, size_t);
void frame_free_multiple (void *, size_t);
uintptr_t *frame_evict_page (void);
void frame_clear_page (int, uint32_t *);
#endif /* vm/frame.h */
