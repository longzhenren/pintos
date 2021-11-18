#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler(struct intr_frame *f)
{

  void *esp = f->esp;
  char a[6] = "!!!!!";
  struct thread *t = thread_current();
  putbuf((char *)esp, 16);
  switch (*(int *)esp)
  {
  case SYS_HALT:

    break; /* Halt the operating system. */
  case SYS_EXIT:

    break; /* Terminate this process. */
  case SYS_EXEC:
    break; /* Start another process. */
  case SYS_WAIT:
    break; /* Wait for a child |]process to die. */
  case SYS_CREATE:
    break; /* Create a file. */
  case SYS_REMOVE:
    break; /* Delete a file. */
  case SYS_OPEN:
    break; /* Open a file. */
  case SYS_FILESIZE:
    break; /* Obtain a file's size. */
  case SYS_READ:
    break; /* Read from a file. */
  case SYS_WRITE:
    break; /* Write to a file. */
  case SYS_SEEK:
    break; /* Change position in a file. */
  case SYS_TELL:
    break; /* Report current position in a file. */
  case SYS_CLOSE:
    break;
  default:
    break;
  }
  printf("!!!");
  putbuf(a, 6);
  printf("system call!\n");
  thread_exit();
}

/* Terminates Pintos*/
void halt(void)
{
  shutdown_power_off();
}

/* Terminates the current user program, returning status to the kernel.  */
void exit(int status)
{
  // struct thread *t = thread_current();
  // t->ret = status;
  // printf("%s: exit(%d)\n", t->name, t->ret);
  // thread_exit();
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments, 
and returns the new process's program id (pid). */
pid_t exec(const char *cmd_line)
{
}

/* Waits for a child process pid and retrieves the child's exit status. */
int wait(pid_t pid)
{
}

/* Creates a new file called file initially initial_size bytes in size. 
 Returns true if successful, false otherwise. */
bool create(const char *file, unsigned initial_size)
{
}

/* Deletes the file called file.
 Returns true if successful, false otherwise. */
bool remove(const char *file)
{
}

/* Opens the file called file. Returns fd,
or -1 if the file could not be opened. */
int open(const char *file)
{
}

/* Returns the size, in bytes, of the file open as fd. */
int filesize(int fd)
{
}

/* Reads size bytes from the file open as fd into buffer. 
Returns the number of bytes actually read (0 at end of file), 
or -1 if the file could not be read*/
int read(int fd, void *buffer, unsigned size)
{
}

/* Writes size bytes from buffer to the open file fd. 
Returns the number of bytes actually written, which may be less than size if some bytes could not be written. */
int write(int fd, const void *buffer, unsigned size)
{
}

/* Changes the next byte to be read or written in open file fd to position, 
expressed in bytes from the beginning of the file.*/
void seek(int fd, unsigned position)
{
}

/* Returns the position of the next byte to be read or written in open file fd, 
expressed in bytes from the beginning of the file. */
unsigned
tell(int fd)
{
}

/* Closes the description fd */
void close(int fd)
{
}