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
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/loader.h"

#include "vm/page.h"
#include <bitmap.h>
#include "vm/frame.h"

#define FRAME_MAX init_ram_pages
/* create a frame table that has 2^20 frame entries,
   or the size of physical memory */ 
static frame_entry *frame_table;
void *frame_get_page(enum palloc_flags);
void *frame_get_multiple(enum palloc_flags, size_t);
void frame_free_multiple (void *, size_t);
void frame_free_page (void *);

//TODO:
//create a hash table for every process. This hash table
//will be reflective of all of virtual memory per process.
//Since the table grows dynamically, it won't take up too much
//space. But we may need to figure out how to store that data
//in the user process stack. Or it may need to exist elsewhere
//based on this PDE -> PTE -> P madness!


/* initialize the elements of the frame table */
void
init_frame_table ()
{
	frame_table = calloc (FRAME_MAX, sizeof (frame_entry));
}

void *
frame_get_page (enum palloc_flags flags)
{
	return frame_get_multiple (flags, 1);
}

void *
frame_get_multiple (enum palloc_flags flags, size_t size)
{
	void *page;
	uint32_t offset;
	uint8_t *free_start = ptov (1024 * 1024);
	page = palloc_get_multiple (flags, size);
	// printf ("The value of the page is: %p\n\n", page);
	// printf ("The value of the division is: %d\n\n", ((uintptr_t)page - (uintptr_t)free_start)/PGSIZE);
	for(offset = 0; offset < size; offset++)
		frame_table[((uintptr_t)page - (uintptr_t)free_start)/PGSIZE + offset].page = page;
	return page;
}

void
frame_free_multiple (void *pages, size_t page_cnt) 
{
	uint32_t offset;
	uint8_t *free_start = ptov (1024 * 1024);
	for(offset = 0; offset < page_cnt; offset++)
		frame_table[((uintptr_t)pages - (uintptr_t)free_start)/PGSIZE + offset].page = NULL;
	palloc_free_multiple (pages, page_cnt);
}

void
frame_free_page (void *page) 
{
	frame_free_multiple (page, 1);
}

int
next_free_entry (int start)
{
	int i = start;
	while (frame_table[i].page && i < FRAME_MAX)
		i++;
	if (i >= FRAME_MAX)
		return -1;
	return i;
}

