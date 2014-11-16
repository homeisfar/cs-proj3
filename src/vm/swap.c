#include <debug.h>

#include "vm/page.h"
#include "devices/block.h"
#include "vm/swap.h"

struct bitmap *swap_bitmap;
block_sector_t num_of_swap_pages; 
struct block *b;

/* Initializes the bitmap to reflect our swap space */
void
swap_init (void)
{
	b = block_get_role (BLOCK_SWAP);
	num_of_swap_pages = block_size (b) / 8;
	swap_bitmap = bitmap_create (num_of_swap_pages);
	if (!swap_bitmap)
		PANIC ("Swap bitmap failed to initialize");
}

/* Read data out from swap */
void
swap_read (block_sector_t sector, void *buffer)
{
	sector *= 8;
	int i = 0;
	for (; i < 8; i++)
		block_read (b, sector + i, (char *) buffer + i * 512);
	swap_release (sector/8);
}

/* Write data into the swap */
void
swap_write (block_sector_t sector, void *buffer)
{
	sector *= 8;
	int i = 0;
	for (; i < 8; i++)
		block_write (b, sector + i, (char *) buffer + i * 512);
}

/* Set a bitmap index to tell us that "page" is taken */
size_t
swap_acquire (void)
{
	return bitmap_scan_and_flip (swap_bitmap, 0, 1, 0);
}

/* Unset a bitmap index to tell us that "page" is free */
void
swap_release (size_t bit_idx)
{
	bitmap_set (swap_bitmap, bit_idx, 0);
}
