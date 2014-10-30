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

#define PAGE_DIR_MAX 1 << 10

// uint32_t **page_dir_supp;
page_dir *page_dir_supp;

void init_supp_page_dir();

void init_supp_page_dir()
{
    // page_dir_supp = calloc(PAGE_DIR_MAX, sizeof (uint32_t *));
    page_dir_supp = calloc(1 << 10, sizeof (page_dir));
}
