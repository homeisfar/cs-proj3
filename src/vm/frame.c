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

#define index(x) (((x) - (int) ptov (1024 * 1024)) / PGSIZE - ((int) ptov (init_ram_pages * PGSIZE) - (int) ptov (1024 * 1024)) / PGSIZE / 2)

size_t user_pages;
/* Create a frame table that has 2^20 frame entries,
   or the size of physical memory */ 
frame_entry *frame_table;
bool frame_get_page (uint32_t *, void *, bool , page_entry *);
void *frame_get_multiple (enum palloc_flags, size_t);
void frame_clear_page (void *);
//uintptr_t *frame_evict_page ();


/* Initialize the elements of the frame table allocating and clearing a
   set of memory */
void
init_frame_table ()
{
	uint8_t *free_start = ptov (1024 * 1024);
 	uint8_t *free_end = ptov (init_ram_pages * PGSIZE);
 	size_t free_pages = (free_end - free_start) / PGSIZE;
 	user_pages = free_pages / 2;
	frame_table = calloc (user_pages, sizeof (frame_entry));
}

/* Obtain a single frame */
bool
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
	uint32_t index = index((int) page);
	//supplemental_table[index].index = index;
	// make frame entry point to supplemental page dir entry
	frame_table[index].page = page;
	frame_table[index].page_dir_entry = fault_entry;
	fault_entry->phys_page = page;
	set_in_frame(frame_table[index].page_dir_entry->meta);
	return pagedir_set_page (pd, upage, page, writable);
}




/* Obtain a frame for a stack page */
// should work for valid, non-eviction cases in current state,
// assuming that page_insert_entry_exec and frame_get_page are correctly implemented

void *
frame_get_stack_page (void * vaddr)
{
	void *kpage;
	// uint32_t offset;
	uint32_t *pd = thread_current ()->pagedir; // for pagedir_set_page
	uint8_t *upage = pg_round_down (vaddr); // used in supp.p.t. also
	uint8_t *free_start = ptov (1024 * 1024);
	kpage = palloc_get_page (PAL_USER | PAL_ZERO);


	if (!kpage)
	{
		kpage = frame_evict_page(); // eviction here
		if (!kpage)
			return NULL;
	}

	uint32_t index = index((int) upage);

	// create supplemental page table entry
	ASSERT ( page_insert_entry_stack (upage) == NULL);

	page_entry *fault_entry = page_get_entry (&thread_current ()->page_table_hash, upage);
	

	if (!fault_entry)
	{
		palloc_free_page (kpage);
		return NULL;
	}

	// make frame entry point to supplemental page dir entry
	frame_table[index].page = kpage;
	frame_table[index].page_dir_entry = fault_entry;
	set_in_frame (frame_table[index].page_dir_entry->meta);
	
	return kpage;
}





/* Free page_cnt number of frames from memory */
void
frame_clear_page (void *page) 
{
	uint32_t offset;
	uint8_t *free_start = ptov (1024 * 1024);
		frame_table[((uintptr_t)page - (uintptr_t)free_start)/PGSIZE + offset].page = NULL;
	palloc_free_page (page);
}

/* Free a single frame from memory */
uintptr_t *
frame_evict_page () 
{
	printf("Evicted\n");
	return NULL;
	// eviction algorithm goes here
	// for now, panic/fail
	// only goes into swap if dirty
	/*
	uint32_t *pd = thread_current ()->pagedir;
	void *evict_frame;
	while (evict_frame == NULL)
	{
		if (pagedir_is_accessed(pd, frame_table[clock_hand]))
		{
			pagedir_set_accessed(pd, frame_table[clock_hand], 0);
			clock_hand++;
		} else {
			if (pagedir_is_dirty(pd, frame_table[clock_hand]))
			{
			//swap to swap area
			//if swap table is full, panic
			}
			//updates to frame, supplemental page table, pagedir
			clear_in_frame(frame_table[clock_hand].page_dir_entry->meta);
			pagedir_clear_page();
			frame_table[clock_hand].page = NULL;
			frame_table[clock_hand].page_dir_entry = NULL;
			clock_hand++;
		}
	}
	return evict_frame;
	*/
	// pagedir_clear_page (uint32_t *pd, void *upage) 

}
