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
block_sector_t num_of_swap_pages; 
struct block *b;

void
swap_init ()
{
	num_of_swap_pages = block_size (b = block_get_role (BLOCK_SWAP)) / 8;
	swap_bitmap = bitmap_create(num_of_swap_pages);
	if (!swap_bitmap)
		PANIC ("Swap bitmap failed to initialize");
}

void
swap_read (block_sector_t sector, void *buffer)
{
	block_read (b, sector, buffer);
}

void
swap_write (block_sector_t sector, void *buffer)
{
	block_write (b, sector, buffer);
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