#include "threads/thread.h"
#include "threads/malloc.h"

#include "vm/page.h"

unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  const struct page_entry *p = hash_entry (p_, struct page_entry, page_elem);
  return hash_bytes (&p->upage, sizeof p->upage);
}

/* Returns true if page a precedes page b. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  const struct page_entry *a = hash_entry (a_, struct page_entry, page_elem);
  const struct page_entry *b = hash_entry (b_, struct page_entry, page_elem);

  return a->upage < b->upage;
}

/* Retrieve an entry from our supp page hash */
page_entry *
page_get_entry (struct hash *page_table, void *fault_addr)
{
  page_entry pe;
  pe.upage = fault_addr;
  struct hash_elem *elem = hash_find (page_table, &pe.page_elem);
  return elem ? hash_entry (elem, page_entry, page_elem) : NULL;
}

/* Insert a page into the supp table if it's an executable */
struct hash_elem *
page_insert_entry_exec (struct file *file, off_t ofs, uint8_t *upage,
        uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
    page_entry *page_entry_supp = calloc (1, sizeof (page_entry));
    struct thread *t = thread_current ();
    struct hash *pages = &t->page_table_hash;

    page_entry_supp->f = file;
    page_entry_supp->ofs = ofs;
    page_entry_supp->upage = upage;
    page_entry_supp->read_bytes = read_bytes;
    page_entry_supp->zero_bytes = zero_bytes;
    if (writable)
      set_writeable (page_entry_supp->meta);
    set_fs (page_entry_supp->meta);

    return hash_insert (pages, &page_entry_supp->page_elem);
}

/* Insert a page into the supp table if it's a stack page */
struct hash_elem *
page_insert_entry_stack (uint8_t *upage)
{
    page_entry *page_entry_supp = calloc (1, sizeof (page_entry));
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    set_stack (page_entry_supp->meta);
    set_writeable (page_entry_supp->meta);
    page_entry_supp->upage = upage;

    return hash_insert (&pages, &page_entry_supp->page_elem);
}

/* For mmap data, we recycle read_bytes from the supp page struct
   to record file size */
struct hash_elem *
page_insert_entry_mmap (uint8_t *upage, struct file *file, off_t file_ofs, 
                        off_t size, bool final)
{
    page_entry *page_entry_supp = calloc (1, sizeof (page_entry));
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    page_entry_supp->ofs = file_ofs;
    page_entry_supp->f = file;
    page_entry_supp->read_bytes = size;
    set_mmap (page_entry_supp->meta);
    set_mmap_final (page_entry_supp->meta);
    set_writeable (page_entry_supp->meta);
    page_entry_supp->upage = upage;

    return hash_insert (&pages, &page_entry_supp->page_elem);
}

/* Removes an entry from the supp page hash */
struct hash_elem *
page_remove_entry (struct hash_elem *page_elem)
{
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    return hash_delete (&pages, page_elem);
}

/* Removes an entry from the supp page hash based on an address */
struct hash_elem *
page_remove_address (void *addr)
{
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    return page_remove_entry (&page_get_entry(&pages, addr)->page_elem);
}


