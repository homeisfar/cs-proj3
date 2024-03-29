#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "lib/user/syscall.h"
#include "pagedir.h"
#include "devices/shutdown.h"
#include "process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "lib/kernel/console.h"
#include "threads/synch.h"
#include "devices/input.h"
#include "vm/page.h"
#include "vm/frame.h"

static void syscall_handler (struct intr_frame *);
/* Project created methods */
void sys_exit (int);
static pid_t sys_exec (const char *);
static bool sys_create (const char *, uint32_t);
static int sys_wait (pid_t);
static bool sys_remove (const char *);
static int sys_open (const char *);
static int sys_filesize (int);
static int sys_read (int, void *, uint32_t);
static int sys_write (int, const void *, uint32_t);
static void sys_seek (int, uint32_t);
static uint32_t sys_tell (int);
static void sys_close (int);
static mapid_t sys_mmap (int, void *);
static void sys_unmmap (mapid_t);
static void valid_ptr (const void*);
static int valid_index (int);


static off_t filesize_helper (struct file*);
static off_t read_helper (struct file *, void *, off_t);
static off_t write_helper (struct file *, const void *, off_t); 
static void seek_helper (struct file *, off_t);
static off_t tell_helper (struct file *);
static void close_helper (struct file *);
void valid_buf_ptr (const void *);
mapid_t sys_mmap (int, void *);
void sys_unmmap (mapid_t);

static struct lock fs_lock;

/* Quick helper function to test if a pointer points to user
   memory. If the pointer is bad, exit the user program. */
void 
valid_ptr (const void *usrdata) 
{
  struct thread *t = thread_current ();
  void *usrdata_rounded = pg_round_down (usrdata);
  
	if (!(usrdata && is_user_vaddr (usrdata)))
  {
   sys_exit (-1);
  }

  if (!(page_get_entry (&t->page_table_hash, usrdata_rounded) 
    || ((uintptr_t) usrdata >= t->esp - 32) && (usrdata < PHYS_BASE)))
    sys_exit (-1);
}

/* Special helper function to see if a buffer is valid */
void
valid_buf_ptr (const void *usrdata)
{
  struct thread *t = thread_current ();
  if (page_get_entry (&t->page_table_hash, pg_round_down (usrdata)) != NULL && 
    !is_writeable (page_get_entry (&t->page_table_hash, 
    pg_round_down (usrdata))->meta))
     sys_exit (-1);

}

/* Initialize the system call handler and the file system lock. */
void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&fs_lock);
}

/* Syscall_handler will get the interrupt value, and 
   based on the switch case, call the appropriate system call
   function. */
static void
syscall_handler (struct intr_frame *f UNUSED)
{
  int sys_call_num;
  void *default_esp = f->esp;
  valid_ptr (f->esp);
  struct thread *t = thread_current ();
  t->esp = f->esp;


  f->esp = pop (f->esp, (void *) &sys_call_num, sizeof (int));
  valid_ptr (f->esp);

  switch (sys_call_num){
    case SYS_HALT:{ // 0 args
      shutdown_power_off ();
      break;
    }

    case SYS_EXIT:{ // 1 arg
      int arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      sys_exit (arg0);
      break;
    }

    case SYS_EXEC:{ // 1 arg
      char *arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (char *));
      valid_ptr (f->esp);
      valid_ptr (arg0);
      f->eax = sys_exec (arg0);
      break;
    }

    case SYS_WAIT:{ // 1 arg
      pid_t arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (pid_t));
      valid_ptr (f->esp);
      f->eax = sys_wait (arg0);
      break;
    }

    case SYS_CREATE:{ // 2 args
      char *arg0;
      uint32_t arg1;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (char *));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *) &arg1, sizeof (uint32_t));
      valid_ptr (f->esp);
      valid_ptr (arg0);
      f->eax = sys_create (arg0, arg1);
      break;
    }

    case SYS_REMOVE:{ // 1 arg
      char *arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (char *));
      valid_ptr (f->esp);
      valid_ptr (arg0);
      f->eax = sys_remove (arg0);
      break;
    }

    case SYS_OPEN:{ // 1 arg
      char *arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (char *));
      valid_ptr (f->esp);
      valid_ptr (arg0);
      f->eax = sys_open (arg0);

      break;
    }

    case SYS_FILESIZE:{ // 1 arg
      int arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      f->eax = sys_filesize (arg0);
      break;
    }

    case SYS_READ:{ // 3 args
      int arg0;
      void *arg1;
      uint32_t arg2;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *) &arg1, sizeof (void *));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *) &arg2, sizeof (uint32_t));
      valid_ptr (f->esp);
      valid_ptr (arg1);
      valid_buf_ptr (arg1);
      f->eax = sys_read (arg0, arg1, arg2);
      break;
    }

    case SYS_WRITE:{ // 3 args
      int arg0;
      void *arg1;
      uint32_t arg2;
      f->esp = pop (f->esp, (void *)&arg0, sizeof (int));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *)&arg1, sizeof (void *));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *)&arg2, sizeof (uint32_t));
      valid_ptr (f->esp);
      valid_ptr (arg1);
      f->eax = sys_write (arg0, arg1, arg2);
      break;
    }

    case SYS_SEEK:{ // 2 args
      int arg0;
      uint32_t arg1;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *) &arg1, sizeof (uint32_t));
      valid_ptr (f->esp);
      sys_seek (arg0, arg1);
      break;
    }

    case SYS_TELL:{ // 1 arg
      int arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      f->eax = sys_tell (arg0);
      break;
    }

    case SYS_CLOSE:{ // 1 arg
      int arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      sys_close (arg0);
      break;
    }

    case SYS_MMAP:{ // 2 arg
      int arg0;
      void *arg1;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      f->esp = pop (f->esp, (void *) &arg1, sizeof (void *));
      valid_ptr (f->esp);
      f->eax = sys_mmap (arg0, arg1);
      break;
    }
    case SYS_MUNMAP:{
      int arg0;
      f->esp = pop (f->esp, (void *) &arg0, sizeof (int));
      valid_ptr (f->esp);
      sys_unmmap (arg0);
      break;
    }
  }
  f->esp = default_esp;
  t->esp = NULL;
}

/* When the user process attempts to exit, set the thread
   exit status accordingly. We also have a function, set_orphan,
   available to then mark all of the exiting process's children
   as orphans. Furthermore, we take care to close every open
   file that an exiting process has opened. */
void
sys_exit (int status)
{
  lock_acquire (&fs_lock);
  struct thread *t;
  t = thread_current ();
  t->exit_status = status;
  uint32_t i;

  for (i = 0; i < FDMAX; i++)
    if (t->fds[i])
      file_close (t->fds[i]);

  set_orphan (t);

  char output[32];
  int num_bytes = snprintf (output, 32, "%s: exit(%d)\n", 
    t->name, t->exit_status);
  putbuf(output, num_bytes);
  file_close (t->self_executable);
  lock_release (&fs_lock);
  thread_exit ();
}

/* Call for user process to exec, which works similar
   to the UNIX fork(). */
pid_t
sys_exec (const char *cmdline)
{
  return process_execute (cmdline);
}

/* Call for the user process to wait on a child process.
   No other kinds of processes can be waited upon, and 
   no child process can be waited upon more than once. 
   We return -1 immediately if we cannot wait on the 
   child process. If the child process has already terminated,
   this function will immediately return the exit status of
   the child process. */
int
sys_wait (pid_t pid)
{
  return process_wait (pid);
}

/* Create a file in the file system. All system
   calls which use the file system are considered
   atomic operations. */
bool
sys_create (const char *file, uint32_t initial_size)
{
  lock_acquire (&fs_lock);
  bool create_return = filesys_create (file, (off_t) initial_size);
  lock_release (&fs_lock);
  return create_return;
}

/* If a file is open when it is removed, its blocks are 
   not deallocated and it may still be accessed by any 
   threads that have it open, until the last one closes it. */
bool
sys_remove (const char *file)
{
  lock_acquire (&fs_lock);
  bool create_return = filesys_remove (file);
  lock_release (&fs_lock);
  return create_return;
}

/* Open a file for the user process. Add the file to the
   process's file descriptor table. If the FDT is full,
   return -1 (but do not terminate the process). */
int
sys_open (const char *file)
{
  struct file *f;
  struct thread *t;
  uint32_t i;
  t = thread_current();

  if(t->fd_size >= FDMAX)
    return -1;

  f = open_helper(file);
  if (!f)
    return -1;

  for(i = 0; i < FDMAX && t->fds[i]; i++);
  t->fds[i] = f;
  t->fd_size++;
  return i+2;
}

/* Helper function to open files atomically. */
struct file * 
open_helper (const char *name)
{
  struct file *f;
  lock_acquire (&fs_lock);
  f= filesys_open (name);
  lock_release (&fs_lock);
  return f;
}

/* Return the file size. Canot use STDIN/STDOUT. */
int
sys_filesize (int fd)
{
  struct thread *t;
  struct file *f;
  int retval;
  t = thread_current ();
  retval = -1;
  fd = valid_index (fd);
  if (fd < 0)
    sys_exit (-1);

  f = t->fds[fd];
  if (f)
    retval = (int) filesize_helper (t->fds[fd]);
  else
    sys_exit (-1);
  return retval;
}

/* Helper function to open files atomically. */
off_t
filesize_helper (struct file *f)
{
  lock_acquire (&fs_lock);
  off_t size;
  size = file_length (f);
  lock_release (&fs_lock);
  return size;
}

/* Reads size bytes from the file into buffer. We
always read as much as was requested, and we do not
read from STDOUT. If STDIN, read from the keyboard. */
int 
sys_read (int fd, void *buffer, unsigned size)
{
	struct thread *t;
	t = thread_current ();
	uint32_t bytes_read = 0;
  if (fd == 0)
    {
      char *strbuf = buffer;
      while (bytes_read < size)
        strbuf[bytes_read++] = input_getc ();
    }
    else if (fd == 1)
      sys_exit (-1);
    else if ((fd = valid_index (fd)) >= 0)
      bytes_read = (int) read_helper (t->fds[fd], buffer, size);
    else
    {
      sys_exit (-1);
      return -1;
    }
  return bytes_read;
}

/* Helper function to open files atomically. */
off_t
read_helper (struct file *file, void *buffer, off_t size)
{
  lock_acquire (&fs_lock);
  off_t read_size;
  read_size = file_read (file, buffer, size);
  lock_release (&fs_lock);
  return read_size;
}

/* Write from buffer into the file described by fd, or
   write out to STDOUT (the console). If we write to
   STDOUT, we first break up the output into 256 byte
   sized chunks. */
int
sys_write (int fd, const void *buffer, uint32_t size)
{
  struct thread *t;
  struct file *f;
  t = thread_current ();
  int bytes_out = 0;

  if (fd == 1)
  {
    while ((int) size > bytes_out + 256)
    {
      putbuf (buffer, (size_t) 256);
      bytes_out += 256;
    }
    putbuf (buffer, (size_t) (size - bytes_out));
    bytes_out = size;
  }
  else if (fd == 0)
    sys_exit (-1);
  else
  {
    fd = valid_index (fd);
    if (fd < 0)
      sys_exit (-1);
    f = t->fds[fd];
    if (f)
    {
      bytes_out = write_helper (f, buffer, (off_t) size);
      return bytes_out;
    }
    else
    {
      sys_exit (-1);
      return -1;
    }
  }
  return bytes_out;
}

/* Helper function to open files atomically. */
off_t
write_helper (struct file *file, const void *buffer, off_t size) 
{
  lock_acquire (&fs_lock);
  off_t bytes_out;
  bytes_out = file_write (file, buffer, size);
  lock_release (&fs_lock);
  return bytes_out;
}

/* Seek in a file until position. */
void
sys_seek (int fd, uint32_t position)
{
  struct thread *t;
  struct file *f;
  t = thread_current ();
  fd = valid_index (fd);
  if (fd < 0)
    sys_exit (-1);
  f = t->fds[fd];
  if (f)
    seek_helper (f, position);
  else
    sys_exit (-1);
}

/* Helper function to open files atomically. */
void
seek_helper (struct file *file, off_t new_pos)
{
  lock_acquire (&fs_lock);
  file_seek (file, new_pos);
  lock_release (&fs_lock);
}

/* Returns the position of the next call to be written or
   read from in bytes from the beginning of the file. */
uint32_t
sys_tell (int fd)
{
  struct thread *t;
  struct file *f;
  t = thread_current ();
  fd = valid_index (fd);
  if (fd < 0)
  {
    sys_exit (-1);
    return (uint32_t) -1;
  }
  f = t->fds[fd];
  if (f)
    return (uint32_t) tell_helper (f);
  else
  {
    sys_exit (-1);
    return -1;
  }
}

/* Helper function to open files atomically. */
off_t
tell_helper (struct file *file)
{
  lock_acquire (&fs_lock);
  off_t pos;
  pos = file_tell (file);
  lock_release (&fs_lock);
  return pos;
}

/* Close a file that has been opened by the user process.
   If the file doesn't exist, exit user program. */
void
sys_close (int fd)
{
  struct thread *t;
  uint32_t size;

  t = thread_current ();
  size = t->fd_size;

  // if fd-2 is a valid index and the file descriptor corresponds
  // to an open file
  if (fd > 1 && (fd-2) < (int) size && t->fds[fd-2])
  {
    close_helper (t->fds[fd-2]);
    t->fds[fd-2] = NULL;
    t->fd_size--;
  }
  else
    sys_exit(-1);
}

/* Helper function to open files atomically. */
void
close_helper (struct file *file)
{
  lock_acquire (&fs_lock);
  file_close (file);
  lock_release (&fs_lock);
}

/* Memmory maps a file pointed to by addr */
mapid_t
sys_mmap (int fd, void *addr)
{
  struct thread *t;
  struct file *f;
  int i, j;
  int mapid;
  off_t size;
  t = thread_current ();
  fd = valid_index (fd);
  bool safe = true;
  if (fd < 0 || !addr || !is_user_vaddr (addr))
    return -1;
  if (!t->fds[fd])
    return -1;
  lock_acquire (&fs_lock);
  f = file_reopen (t->fds[fd]);
  size = file_length (f);
  lock_release (&fs_lock);
  if (size == 0)
    return -1;
  if (addr != pg_round_down (addr))
    return -1;

  for (i = 0; i < MAPPINGSMAX && t->mappings[i]; i++);
  if (i >= MAPPINGSMAX)
    return -1;
  t->mappings[i] = addr;
  
  mapid = i;

  off_t size_temp = size / 4096;

  for (i = 0; i < size_temp && safe; i++)
  {
    safe = page_insert_entry_mmap (addr + i * 4096, f, i * 4096,
                                   4096, false) == NULL;
  }
  if (!safe || page_insert_entry_mmap (addr + i * 4096, f, i * 4096, 
                                       size % 4096, true) != NULL)
  { 
    for (j = 0; j < i; j++)
    {
      page_remove_address (addr + j * 4096);
    }
    mapid = -1;
  }
  return mapid;
}

/* Unmaps a file. If not explicitly called, the file is unmapped
 * when a process exits */
void
sys_unmmap (mapid_t mapping)
{
  struct thread *t = thread_current ();
  void *mmap = t->mappings[mapping];
  page_entry *mmap_page;
  
  if (mmap == NULL) {
    return;
  }

  do
  {
    /* get page entry from vaddr */
    mmap_page = page_get_entry (&t->page_table_hash, mmap);
    /* if in a frame */
    if (is_in_frame (mmap_page->meta)) 
    { /* if dirty */
      if (pagedir_is_dirty (t->pagedir, mmap_page->upage)) 
      {
          /* write back */
          lock_acquire (&fs_lock);
          file_write_at (mmap_page->f, mmap_page->phys_page, 
                         mmap_page->read_bytes, mmap_page->ofs);
          lock_release (&fs_lock);
      }
      /* clear frame */
      frame_clear_page (index ((uintptr_t) mmap_page->phys_page), t->pagedir);
    }
    page_remove_address (mmap);
    mmap += 4096;
  } while (!is_mmap_final (mmap_page->meta));
    t->mappings[mapping] = NULL;
  return;
}

/* Valid_index is a helper function designed to 
   match the file descriptor to its index in the
   FDT. This is necessary because fd = 0, 1 are 
   both reserved for STDIN, STDOUT, respectively. */
int
valid_index (int fd)
{
  fd -= 2;
  if( fd < (int)FDMAX && fd >= 0 )
    return fd;
  else
    return -1;
}
