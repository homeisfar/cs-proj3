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
#include "threads/malloc.h"

/* Student code */
#include "vm/page.h"
#include <bitmap.h>
#include "vm/frame.h"

/* Amount of physical memory in 4kb pages */
#define FRAME_MAX init_ram_pages

/* Create a frame table that has 2^20 frame entries,
   or the size of physical memory */ 
frame_entry *frame_table;
int frame_get_page (uint32_t *, void *, bool , page_entry *);
void *frame_get_multiple (enum palloc_flags, size_t);
void frame_free_multiple (void *, size_t);
//uintptr_t *frame_evict_page ();


/* Initialize the elements of the frame table allocating and clearing a
   set of memory */
void
init_frame_table ()
{
	frame_table = calloc (FRAME_MAX, sizeof (frame_entry));
}

/* Obtain a single frame */
int
frame_get_page (uint32_t *pd, void *upage, bool writable, page_entry *fault_entry)
{
	void *page;
	uint32_t offset;
	uint8_t *free_start = ptov (1024 * 1024);
	page = palloc_get_page (PAL_USER);
	if (!page)
	{
		page = frame_evict_page();//eviction here
		// check if page is NULL
		if (!page)
			return 0;
	}
	// printf ("The value of the page is: %p\n\n", page);
	// printf ("The value of the division is: %d\n\n", ((uintptr_t)page - (uintptr_t)free_start)/PGSIZE);
	uint32_t index = ((uintptr_t) page - (uintptr_t) free_start) / PGSIZE ;
	//supplemental_table[index].index = index;
	// make frame entry point to supplemental page dir entry
	frame_table[index].page = page;
	frame_table[index].page_dir_entry = fault_entry;
	set_frame(frame_table[index].page_dir_entry->meta);
	return pagedir_set_page (pd, upage, page, writable);
}

/* Free page_cnt number of frames from memory */
void
frame_free_multiple (void *pages, size_t page_cnt) 
{
	uint32_t offset;
	uint8_t *free_start = ptov (1024 * 1024);
	for(offset = 0; offset < page_cnt; offset++)
		frame_table[((uintptr_t)pages - (uintptr_t)free_start)/PGSIZE + offset].page = NULL;
	palloc_free_multiple (pages, page_cnt);
}

/* Free a single frame from memory */
uintptr_t *
frame_evict_page () 
{
	return NULL;
	// eviction algorithm goes here
	// for now, panic/fail
	// only goes into swap if dirty
	// pagedir_clear_page (uint32_t *pd, void *upage) 

}

/* Retreive the next free available page from the frame table, starting from
   the passed starting integer */
// int
// next_free_entry (int start)
// {
// 	uint32_t i = start;
// 	while (frame_table[i].page && i < FRAME_MAX)
// 		i++;
// 	if (i >= FRAME_MAX)
// 		return -1;
// 	return i;
// }

