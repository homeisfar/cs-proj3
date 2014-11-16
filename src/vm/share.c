#include "share.h"
#include <stdlib.h>
#include "threads/malloc.h"
#include "threads/thread.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"

struct hash shared_ro;

static unsigned share_hash (const struct hash_elem *_, void *);
static bool share_less (const struct hash_elem *, const struct hash_elem *, void *);
static ro_entry *find (struct inode *);
static void insert (struct inode *, void *);
static void update_existing (struct inode *, void *);

/* begin code */
unsigned
share_hash (const struct hash_elem *p_, void *aux)
{
    const ro_entry *r = hash_entry (p_, ro_entry, e);
    /* hash the pointer to the inode */
    return hash_bytes (&r->inode, sizeof r->inode);
}

bool
share_less (const struct hash_elem *a_, const struct hash_elem *b_,
        void *aux)
{
    const ro_entry *a = hash_entry (a_, ro_entry, e);
    const ro_entry *b = hash_entry (b_, ro_entry, e);

    return a->inode < b->inode;
}

void 
share_init () 
{
    hash_init(&shared_ro, share_hash, share_less, NULL);
}

ro_entry *
find (struct inode *inode) 
{
    ro_entry tmp;
    tmp.inode = inode;
    struct hash_elem *elem = hash_find(&shared_ro, &tmp.e);
    return hash_entry(elem, ro_entry, e);
}

/* always check if inode exists in table before calling insert */
void
insert (struct inode *inode, void *uaddr) 
{
    /* insert new ro_entry to shared_ro */
    ro_entry *new_entry = calloc(1, sizeof(ro_entry));     
    new_entry->inode = inode;
    list_init(&new_entry->procs);

    /* initialize process list with calling process */
    proc *new_proc = calloc(1, sizeof(proc));
    new_proc->t = thread_current();
    new_proc->uaddr = uaddr;
    list_push_back (&new_entry->procs, &new_proc->list_e);

    hash_insert(&shared_ro, &new_entry->e);
}

void
update_existing (struct inode *inode, void *uaddr)
{
    ro_entry *entry = find(inode);
    ASSERT(entry);

    proc *new_proc = calloc(1, sizeof(proc));
    new_proc->t = thread_current();
    new_proc->uaddr = uaddr;
    list_push_back (&entry->procs, &new_proc->list_e);
}

void 
share_update (struct inode *inode, void *uaddr)
{
    ASSERT(inode);
    ASSERT(uaddr);

    if(find (inode))
        update_existing (inode, uaddr);
    else
        insert(inode, uaddr);
}

void
share_remove (struct inode *inode)
{
    /*  */   
    struct thread *t = thread_current();
    ro_entry *entry = find(inode);
    ASSERT(entry);

    /* linear search */
    proc *p;
    struct list_elem *e;
    for (e = list_begin (&entry->procs); e != list_end (&entry->procs);
            e = list_next (e))
    {
        p = list_entry (e, proc, list_e);
        if (p->t == t)
            break;
    }    
    list_remove(&p->list_e);
    free(p);

    if (list_empty (&entry->procs))
    {
        hash_delete(&shared_ro, &entry->e);
        free(entry);
        /* TODO: clear the frame */
    }
}

/* updates supplemental page metadata and sets the user->kernel page mapping
 * for all other processes that reference the read-only file
 * */
void
share_install_frame (struct inode *inode, void *kpage, off_t ofs)
{
    struct thread *t = thread_current();

    ro_entry *entry = find(inode);
    ASSERT(entry);

    /* linear search */
    proc *p;
    struct list_elem *e;
    void *upage;
    for (e = list_begin (&entry->procs); e != list_end (&entry->procs);
            e = list_next (e))
    {
        p = list_entry (e, proc, list_e);
        /* skip current process */
        if (p->t == t)
            continue;
        
        /* update supplemental page table */
        upage = p->uaddr + ofs; 
        page_entry *pentry = page_get_entry(&p->t->page_table_hash, upage);
        set_in_frame(pentry->meta);
        pagedir_set_page(p->t->pagedir, upage, kpage, false);
    }
}

/* updates supplemental page metadata and clears the user->kernel page mapping
 * for all other processes that reference the read-only file
 * */
void
share_clear_frame (struct inode *inode, off_t ofs)
{
    struct thread *t = thread_current();

    ro_entry *entry = find(inode);
    ASSERT(entry);

    /* linear search */
    proc *p;
    struct list_elem *e;
    void *upage;
    for (e = list_begin (&entry->procs); e != list_end (&entry->procs);
            e = list_next (e))
    {
        p = list_entry (e, proc, list_e);
        /* skip current process */
        if (p->t == t)
            continue;
        
        /* update supplemental page table */
        upage = p->uaddr + ofs; 
        page_entry *pentry = page_get_entry(&p->t->page_table_hash, upage);
        clear_in_frame(pentry->meta);
        pagedir_clear_page(p->t->pagedir, upage);
    }
}

