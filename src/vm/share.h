#include "lib/kernel/hash.h"
#include "lib/kernel/list.h"


extern struct hash shared_ro;

typedef struct _ro_entry {
    struct hash_elem e;
    struct inode *inode;
    struct list procs;
} ro_entry;

typedef struct _proc {
    struct list_elem list_e;
    struct thread *t;
    void *uaddr; /* start of file in process's virtual address space */
} proc;
