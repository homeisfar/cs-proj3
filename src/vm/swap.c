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

#include "vm/page.h"
#include "devices/block.h"

struct bitmap *swap_bitmap;
size_t num_of_swap_pages;

void
swap_init ()
{
	swap_bitmap = bitmap_create(/* STILL NEED A VALUE HERE */);
	if (!swap_bitmap)
		PANIC ("Swap bitmap failed to initialize");


	// block_get_role
}

void
swap_read ()
{

}

void
swap_write ()
{

}

size_t
swap_acquire ()
{
	return bitmap_scan_and_flip (swap_bitmap, 0, 1, 0);
}

void
swap_release (size_t bit_idx)
{
	bitmap_flip (swap_bitmap, bit_idx);
}

/*

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

*/