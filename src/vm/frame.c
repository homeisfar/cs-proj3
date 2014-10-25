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
#include <bitmap.h>


#define FRAME_MAX 1 << 20
/* create a frame table that has 2^20 frame entries,
   or the size of physical memory */ 
static frame_entry frame_table[FRAME_MAX];

//TODO:
//create a hash table for every process. This hash table
//will be reflective of all of virtual memory per process.
//Since the table grows dynamically, it won't take up too much
//space. But we may need to figure out how to store that data
//in the user process stack. Or it may need to exist elsewhere
//based on this PDE -> PTE -> P madness!


/* initialize the elements of the frame table */
static void
init_frame_table (){

}

int
next_free_entry () {
	int i = 0;
	while (frame_table[i].page && i < FRAME_MAX)
		i++;
	if (i >= FRAME_MAX)
		return -1;
	return i;
}

