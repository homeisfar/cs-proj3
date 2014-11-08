#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "vm/page.h"

#define PAGE_DIR_MAX 1 << 10

// uint32_t **page_dir_supp;
// page_entry *page_entry_supp;


//still needed to do 4:14PM Nov 1st
/*
Use bitmap to scan for available pages and contiguous regions of memory
Create function to create entry in Supp table (and set bits on bitmap)
Create function to remove entry from supp table and unset bits

//METADATA TO KEEP TRACK OF:
1) Is the memory allocated?
2) Is the memory in frame?
3) Is the memory writeable?

On thread exits:
clear page table pagedir_clear_page
destroy hash table

*/


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

page_entry *
page_get_entry (struct hash *page_table, void *fault_addr)
{
  page_entry pe;
  pe.upage = fault_addr;
  struct hash_elem *elem = hash_find (page_table, &pe.page_elem);
  return elem ? hash_entry(elem, page_entry, page_elem) : NULL;
}

size_t
page_obtain_pages (struct bitmap *page_map, size_t start, size_t cnt)
{
	size_t start_bit;
	while((start_bit = bitmap_scan_and_flip (page_map, start, cnt, 0)) == BITMAP_ERROR)
	{
		//resize bitmap not yet implemented
	}
	return start_bit;
}

struct hash_elem *
page_insert_entry_exec (struct file *file, off_t ofs, uint8_t *upage,
        uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
    page_entry *page_entry_supp = calloc (1 << 10, sizeof (page_entry));
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;

    page_entry_supp->f = file;
    page_entry_supp->ofs = ofs;
    page_entry_supp->upage = upage;
    page_entry_supp->read_bytes = read_bytes;
    page_entry_supp->zero_bytes = zero_bytes;
    if (writable)
      set_writeable (page_entry_supp->meta);
    set_fs (page_entry_supp->meta);

    return hash_insert (&pages, &page_entry_supp->page_elem);
}

struct hash_elem *
page_insert_entry_stack (uint8_t *upage)
{
    page_entry *page_entry_supp = calloc (1 << 10, sizeof (page_entry));
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    set_stack (page_entry_supp->meta);
    set_writeable (page_entry_supp->meta);
    page_entry_supp->upage = upage;

    return hash_insert (&pages, &page_entry_supp->page_elem);
}

struct hash_elem *
page_remove_entry (struct hash_elem *page_elem)
{
    struct thread *t = thread_current ();
    struct hash pages = t->page_table_hash;
    // page_entry *p = t->supp_page_data; //potentially bad
    return hash_delete (&pages, page_elem);
}


//resize bitmap function call ()

// struct hash_elem *
// page_insert (struct hash *, struct hash_elem *);
