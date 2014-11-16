#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void sys_exit (int);
struct file *open_helper (const char*);

#endif /* userprog/syscall.h */
