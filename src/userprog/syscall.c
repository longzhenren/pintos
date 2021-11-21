#include "userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "pagedir.h"
#include "threads/vaddr.h"

void call_halt(void);
void call_exit(struct intr_frame *);
void call_exec(struct intr_frame *);
void call_wait(struct intr_frame *);
void call_create(struct intr_frame *);
void call_remove(struct intr_frame *);
void call_open(struct intr_frame *);
void call_filesize(struct intr_frame *);
void call_read(struct intr_frame *);
void call_write(struct intr_frame *);
void call_seek(struct intr_frame *);
void call_tell(struct intr_frame *);
void call_close(struct intr_frame *);

void halt(void);
void exit(int);
pid_t exec(const char *);
int wait(pid_t pid);
bool create(const char *, unsigned);
bool remove(const char *);
int open(const char *);
int filesize(int fd);
int read(int, void *, unsigned);
int write(int, const void *, unsigned);
void seek(int, unsigned);
unsigned tell(int);
void close(int);

static void syscall_handler(struct intr_frame *);

// bool pointer_valid(void *esp, int num)
// {
//   int i;
//   struct thread *cur = thread_current();
//   for (i = 0; i < num * 4; i++)
//   {
//     if (!is_user_vaddr(esp + i) || pagedir_get_page(cur->pagedir, esp + i) == NULL)
//     {
//       return false;
//     }
//   }
//   return true;
// }

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f)
{
  void *esp = f->esp;
  printf("!!!!");
  if (esp == NULL)
  {
    exit(-1);
  }
  switch (*(int *)esp)
  {
  case SYS_HALT:
    call_halt();
    break; /* Halt the operating system. */
  case SYS_EXIT:
    call_exit(f);
    break; /* Terminate this process. */
  case SYS_EXEC:
    call_exec(f);
    break; /* Start another process. */
  case SYS_WAIT:
    call_wait(f);
    break; /* Wait for a child process to die. */
  case SYS_CREATE:
    call_create(f);
    break; /* Create a file. */
  case SYS_REMOVE:
    call_remove(f);
    break; /* Delete a file. */
  case SYS_OPEN:
    call_open(f);
    break; /* Open a file. */
  case SYS_FILESIZE:
    call_filesize(f);
    break; /* Obtain a file's size. */
  case SYS_READ:
    call_read(f);
    break; /* Read from a file. */
  case SYS_WRITE:
    call_write(f);
    break; /* Write to a file. */
  case SYS_SEEK:
    call_seek(f);
    break; /* Change position in a file. */
  case SYS_TELL:
    call_tell(f);
    break; /* Report current position in a file. */
  case SYS_CLOSE:
    call_close(f);
    break;
  default:
    exit(-1);
    break;
  }
  // printf("system call!\n");
  thread_exit();
}

/* Terminates Pintos*/
void call_halt(void)
{
  halt();
}

void halt(void)
{
  shutdown_power_off();
}

/* Terminates the current user program, returning status to the kernel.  */
void call_exit(struct intr_frame *f)
{
  void *esp = f->esp;
  // if (!pointer_valid(esp + 4, 1))
  // {
  //   exit(-1);
  // }
  int status = *(int *)(esp + 4);
  exit(status);
}

void exit(int status)
{
  struct thread *t = thread_current();
  t->ret = status;
  thread_exit();
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments, 
and returns the new process's program id (pid). */
void call_exec(struct intr_frame *f)
{
}
pid_t exec(const char *cmd_line)
{
}

/* Waits for a child process pid and retrieves the child's exit status. */
void call_wait(struct intr_frame *f)
{
  // if (!pointer_valid(f->esp + 4, 1))
  // {
  //   exit(-1);
  // }
  pid_t pid = *(int *)(f->esp + 4);
  f->eax = wait(pid);
}
int wait(pid_t pid)
{
  return process_wait(pid);
}

/* Creates a new file called file initially initial_size bytes in size. 
 Returns true if successful, false otherwise. */
void call_create(struct intr_frame *f)
{
}
bool create(const char *file, unsigned initial_size)
{
}

/* Deletes the file called file.
 Returns true if successful, false otherwise. */
void call_remove(struct intr_frame *f)
{
}
bool remove(const char *file)
{
}

/* Opens the file called file. Returns fd,
or -1 if the file could not be opened. */
void call_open(struct intr_frame *f)
{
}
int open(const char *file)
{
}

/* Returns the size, in bytes, of the file open as fd. */
void call_filesize(struct intr_frame *f)
{
}
int filesize(int fd)
{
}

/* Reads size bytes from the file open as fd into buffer. 
Returns the number of bytes actually read (0 at end of file), 
or -1 if the file could not be read*/
void call_read(struct intr_frame *f)
{
}
int read(int fd, void *buffer, unsigned size)
{
}

/* Writes size bytes from buffer to the open file fd. 
Returns the number of bytes actually written, which may be less than size if some bytes could not be written. */
void call_write(struct intr_frame *f)
{
  int fd = *(int *)(f->esp + 4);
  void *buffer = *(char **)(f->esp + 8);
  unsigned size = *(unsigned *)(f->esp + 12);
  f->eax = write(fd, buffer, size);
}

int write(int fd, const void *buffer, unsigned size)
{
  putbuf(buffer, size);
  return size;
}

/* Changes the next byte to be read or written in open file fd to position, 
expressed in bytes from the beginning of the file.*/
void call_seek(struct intr_frame *f)
{
}
void seek(int fd, unsigned position)
{
}

/* Returns the position of the next byte to be read or written in open file fd, 
expressed in bytes from the beginning of the file. */
void call_tell(struct intr_frame *f)
{
}
unsigned tell(int fd)
{
}

/* Closes the description fd */
void call_close(struct intr_frame *f)
{
}
void close(int fd)
{
}