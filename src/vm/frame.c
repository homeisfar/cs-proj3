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
#include "threads/synch.h"

/* Student code */
#include "vm/page.h"
#include <bitmap.h>
#include "vm/frame.h"
#include "vm/swap.h"

/* Amount of physical memory in 4kb pages */
#define FRAME_MAX init_ram_pages


size_t user_pages;

static struct lock frame_lock;
/* Create a frame table that has 2^20 frame entries,
   or the size of physical memory */ 
frame_entry *frame_table;
bool frame_get_page (uint32_t *, void *, bool , page_entry *);
void *frame_get_multiple (enum palloc_flags, size_t);
void frame_clear_page (int, uint32_t *);
uintptr_t *frame_evict_page ();

/* Initialize the elements of the frame table allocating and clearing a
   set of memory */
void
init_frame_table ()
{
	uint8_t *free_start = ptov (1024 * 1024);
 	uint8_t *free_end = ptov (init_ram_pages * PGSIZE);
 	size_t free_pages = (free_end - free_start) / PGSIZE;
 	user_pages = free_pages / 2 - 1;
	frame_table = calloc (user_pages, sizeof (frame_entry));
	lock_init (&frame_lock);
}

/* Obtain a single frame */
bool
frame_get_page (uint32_t *pd, void *upage, bool writable, page_entry *fault_entry)
{
	lock_acquire (&frame_lock);
	void *page;
	page = palloc_get_page (PAL_USER);
	if (!page)
	{
		page = frame_evict_page ();
		// check if page is NULL
		if (!page)
			return 0;
	}

	uint32_t index = index ((int) page);
	// make frame entry point to supplemental page dir entry
	frame_table[index].page = page;
	frame_table[index].page_dir_entry = fault_entry;
	fault_entry->phys_page = page;
	set_in_frame (frame_table[index].page_dir_entry->meta);
	bool success = pagedir_set_page (pd, upage, page, writable);
	lock_release (&frame_lock);
	return success;
}

/* Obtain a frame for a stack page */
void *
frame_get_stack_page (void * vaddr)
{
	void *kpage;
	uint8_t *upage = pg_round_down (vaddr);
	lock_acquire (&frame_lock);

	kpage = palloc_get_page (PAL_USER | PAL_ZERO);

	if (!kpage)
	{
		kpage = frame_evict_page ();
		if (!kpage)
			return NULL;
	}

	uint32_t index = index ((int) kpage);

	// create supplemental page table entry
	ASSERT (page_insert_entry_stack (upage) == NULL);
	page_entry *fault_entry = page_get_entry (&thread_current ()->page_table_hash, upage);
	

	if (!fault_entry)
	{
		palloc_free_page (kpage);
		return NULL;
	}

	// make frame entry point to supplemental page dir entry
	frame_table[index].page = kpage;
	frame_table[index].page_dir_entry = fault_entry;
	fault_entry->phys_page = kpage;
	set_in_frame (frame_table[index].page_dir_entry->meta);
	lock_release (&frame_lock);
	return kpage;
}

/* Free page_cnt number of frames from memory */
void
frame_clear_page (int frame_index, uint32_t *pd)
{
	clear_in_frame (frame_table[frame_index].page_dir_entry->meta);
	pagedir_clear_page (pd, frame_table[frame_index].page_dir_entry->upage);
	frame_table[frame_index].page_dir_entry = NULL;
}

/* Free a single frame from memory */
uintptr_t *
frame_evict_page () 
{
	static int clock_hand = 0;
	uint32_t *pd = thread_current ()->pagedir;
	void *evict_frame = NULL;
	while (evict_frame == NULL)
	{
		// check reference bit 
		if (pagedir_is_accessed (pd, frame_table[clock_hand].page))
		{
			// set reference to 0
			pagedir_set_accessed (pd, frame_table[clock_hand].page, 0);
		}
		else 
		{
			// if page is dirty, move to swap
			if (pagedir_is_dirty(pd, frame_table[clock_hand].page))
			{
				size_t index = swap_acquire ();
				if (index == BITMAP_ERROR)
					PANIC ("OUT OF SWAP SPACE");
				
				page_entry *current_page = frame_table[clock_hand].page_dir_entry;
				current_page->swap_index = index;
				 
				 if(is_mmap (current_page->meta))
				 {
					file_write_at (current_page->f, 
						current_page->phys_page, 
						current_page->read_bytes, 
						current_page->ofs); 
				 }

				else
				{
					swap_write (index, frame_table[clock_hand].page);
					set_in_swap (current_page->meta);
				}


			}
			// clear page and update frame, supplemental page table, pagedir
			frame_clear_page (clock_hand, pd);
			evict_frame = frame_table[clock_hand].page;
		}
		clock_hand = (clock_hand + 1) % user_pages;
	}
	return evict_frame;
}
