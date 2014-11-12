#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"

void swap_init ();
void swap_read (block_sector_t, void *);
void swap_write (block_sector_t, void *);
size_t swap_acquire ();
void swap_release (size_t);

#endif /* vm/swap.h */