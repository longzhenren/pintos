#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "pagedir.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include "devices/input.h"

#define EXIT_STATUS_CODE_WHEN_PTR_INVALID -1

static void syscall_handler(struct intr_frame *);

// Check the validity of given pointer, num is the num of args in a call.
bool ptr_valid(void *esp, int num)
{
  if (esp == NULL)
  {
    return false;
  }

  struct thread *cur = thread_current();

  for (int i = 0; i < num * 4; i++)
  {
    if (!is_user_vaddr(esp + i) ||
        pagedir_get_page(cur->pagedir, esp + i) == NULL)
    {
      return false;
    }
  }

  return true;
}

// 若指针指向非法地址，则直接退出进程
void exit_if_ptr_invalid(void *esp, int num)
{
  if (!ptr_valid(esp, num))
  {
    exit(EXIT_STATUS_CODE_WHEN_PTR_INVALID);
  }
}

file_descriptor_t_ptr get_fd(struct thread *t, int fd_num)
{
  struct list_elem *e;

  for (e = list_begin(&t->process_info->fd_list);
       e != list_end(&t->process_info->fd_list);
       e = e->prev)
  {
    file_descriptor_t_ptr fd = list_entry(e, file_descriptor_t, elem);

    if (fd->num == fd_num)
    {
      return fd;
    }
  }

  return NULL;
}

file_descriptor_t_ptr get_fd_for_current(int fd_num)
{
  return get_fd(thread_current(), fd_num);
}

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  g_fd_num = 3;
  // list_init(&file_opened_list);
  lock_init(&g_fd_lock);
}

static void syscall_handler(struct intr_frame *f)
{
  void *esp = f->esp;

  exit_if_ptr_invalid(esp, 1);

  switch (*(int *)esp)
  {
  /* Halt the operating system. */
  case SYS_HALT:
    call_halt();
    break;
  /* Terminate this process. */
  case SYS_EXIT:
    call_exit(f);
    break;
  /* Start another process. */
  case SYS_EXEC:
    call_exec(f);
    break;
  /* Wait for a child process to die. */
  case SYS_WAIT:
    call_wait(f);
    break;
  /* Create a file. */
  case SYS_CREATE:
    call_create(f);
    break;
  /* Delete a file. */
  case SYS_REMOVE:
    call_remove(f);
    break;
  /* Open a file. */
  case SYS_OPEN:
    call_open(f);
    break;
  /* Obtain a file's size. */
  case SYS_FILESIZE:
    call_filesize(f);
    break;
  /* Read from a file. */
  case SYS_READ:
    call_read(f);
    break;
  /* Write to a file. */
  case SYS_WRITE:
    call_write(f);
    break;
  /* Change position in a file. */
  case SYS_SEEK:
    call_seek(f);
    break;
  /* Report current position in a file. */
  case SYS_TELL:
    call_tell(f);
    break;
  /* Close a file. */
  case SYS_CLOSE:
    call_close(f);
    break;
  default:
    thread_exit();
    break;
  }
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
  exit_if_ptr_invalid(esp + 4, 1);

  int status = *(int *)(esp + 4);
  exit(status);
}
void exit(int status)
{
  struct thread *t = thread_current();

  lock_acquire(&(t->process_info->lock));
  t->process_info->ret = status;
  lock_release(&(t->process_info->lock));

  thread_exit();
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments, 
and returns the new process's program id (pid). */
void call_exec(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  char *cmd_line = *(char **)(esp + 4);
  exit_if_ptr_invalid(cmd_line, 1);

  f->eax = exec(cmd_line);
}
pid_t exec(const char *cmd_line)
{
  return process_execute(cmd_line);
}

/* Waits for a child process pid and retrieves the child's exit status. */
void call_wait(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

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
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 2);

  char *file = *(char **)(esp + 4);
  unsigned initial_size = *(int *)(esp + 8);
  exit_if_ptr_invalid(file, 1);

  lock_acquire(&g_fd_lock);
  f->eax = create(file, initial_size);
  lock_release(&g_fd_lock);
}
bool create(const char *file, unsigned initial_size)
{
  return filesys_create(file, initial_size);
}

/* Deletes the file called file.
 Returns true if successful, false otherwise. */
void call_remove(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  char *file = *(char **)(esp + 4);
  exit_if_ptr_invalid(file, 1);

  lock_acquire(&g_fd_lock);
  f->eax = remove(file);
  lock_release(&g_fd_lock);
}
bool remove(const char *file)
{
  return filesys_remove(file);
}

/* Opens the file called file. Returns fd,
or -1 if the file could not be opened. */
void call_open(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  char *file_name = *(char **)(esp + 4);
  exit_if_ptr_invalid(file_name, 1);

  lock_acquire(&g_fd_lock);
  f->eax = open(file_name);
  lock_release(&g_fd_lock);
}
int open(const char *file_name)
{
  struct file *file = filesys_open(file_name);
  if (file == NULL)
    return -1;

  // 文件关闭之后一定记得释放FD避免内存泄漏！！！
  file_descriptor_t_ptr FD = malloc(sizeof(file_descriptor_t));
  if (FD == NULL)
    return -1;

  FD->file = file;
  FD->num = g_fd_num;
  list_push_back(&thread_current()->process_info->fd_list, &FD->elem);
  g_fd_num++;
  return FD->num;
}

/* Returns the size, in bytes, of the file open as fd. */
void call_filesize(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  int fd = *(int *)(esp + 4);
  lock_acquire(&g_fd_lock);
  f->eax = filesize(fd);
  lock_release(&g_fd_lock);
}
int filesize(int fd)
{
  file_descriptor_t_ptr FD = get_fd_for_current(fd);
  if (FD == NULL)
    return -1;

  return file_length(FD->file);
}

/* Reads size bytes from the file open as fd into buffer. 
Returns the number of bytes actually read (0 at end of file), 
or -1 if the file could not be read*/
void call_read(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 3);

  int fd = *(int *)(esp + 4);
  void *buffer = *(char **)(esp + 8);
  unsigned size = *(unsigned *)(esp + 12);
  exit_if_ptr_invalid(buffer, 1);

  lock_acquire(&g_fd_lock);
  f->eax = read(fd, buffer, size);
  lock_release(&g_fd_lock);
}
int read(int fd, void *buffer, unsigned size)
{
  if (fd == 0)
  {
    char *buf = *(char **)buffer;

    for (int i = 0; i < size; i++)
    {
      buf[i] = input_getc();
    }

    return size;
  }
  else
  {
    file_descriptor_t_ptr FD = get_fd_for_current(fd);
    if (FD == NULL)
      return -1;

    return file_read(FD->file, buffer, size);
  }
}

/* Writes size bytes from buffer to the open file fd. 
Returns the number of bytes actually written, which may be less than size if some bytes could not be written. */
void call_write(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 3);

  int fd = *(int *)(esp + 4);
  void *buffer = *(char **)(esp + 8);
  unsigned size = *(unsigned *)(esp + 12);
  exit_if_ptr_invalid(buffer, 1);

  lock_acquire(&g_fd_lock);
  f->eax = write(fd, buffer, size);
  lock_release(&g_fd_lock);
}
int write(int fd, const void *buffer, unsigned size)
{
  if (fd == 1)
  {
    putbuf(buffer, size);

    return size;
  }
  else
  {
    file_descriptor_t_ptr FD = get_fd_for_current(fd);
    if (FD == NULL)
      return -1;

    return file_write(FD->file, buffer, size);
  }
}

/* Changes the next byte to be read or written in open file fd to position, 
expressed in bytes from the beginning of the file.*/
void call_seek(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 2);

  int fd = *(int *)(f->esp + 4);
  unsigned position = *(unsigned *)(f->esp + 8);

  lock_acquire(&g_fd_lock);
  seek(fd, position);
  lock_release(&g_fd_lock);
}
void seek(int fd, unsigned position)
{
  file_descriptor_t_ptr FD = get_fd_for_current(fd);
  if (FD == NULL)
    return;

  file_seek(FD->file, position);
}

/* Returns the position of the next byte to be read or written in open file fd, 
expressed in bytes from the beginning of the file. */
void call_tell(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  int fd = *(int *)(f->esp + 4);

  lock_acquire(&g_fd_lock);
  f->eax = tell(fd);
  lock_release(&g_fd_lock);
}
unsigned tell(int fd)
{
  file_descriptor_t_ptr FD = get_fd_for_current(fd);
  if (FD == NULL)
    return -1;

  return file_tell(FD->file);
}

/* Closes the description fd */
void call_close(struct intr_frame *f)
{
  void *esp = f->esp;
  exit_if_ptr_invalid(esp + 4, 1);

  int fd = *(int *)(f->esp + 4);

  lock_acquire(&g_fd_lock);
  close(fd);
  lock_release(&g_fd_lock);
}
void close(int fd)
{
  file_descriptor_t_ptr FD = get_fd_for_current(fd);
  if (FD == NULL)
    return -1;

  file_close(FD->file);
  list_remove(&FD->elem);
  free(FD);
}
