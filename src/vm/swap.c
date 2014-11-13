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
	b = block_get_role (BLOCK_SWAP);
	num_of_swap_pages = block_size (b) / 8;
	swap_bitmap = bitmap_create (num_of_swap_pages);
	if (!swap_bitmap)
		PANIC ("Swap bitmap failed to initialize");
}

void
swap_read (block_sector_t sector, void *buffer)
{
	sector *= 8;
	int i = 0;
	for (; i < 8; i++)
		block_read (b, sector + i, (char *) buffer + i * 512);
	swap_release (sector/8);
}

void
swap_write (block_sector_t sector, void *buffer)
{
	sector *= 8;
	int i = 0;
	for (; i < 8; i++)
		block_write (b, sector + i, (char *) buffer + i * 512);
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
