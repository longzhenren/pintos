#include "userprog/process.h"
#include "userprog/syscall.h"
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
#include "devices/timer.h"
#include "threads/synch.h"

#define ESP_PUSH(ESP, VALUE) *(--ESP) = VALUE

#define ESP_PUSH_BATCH(ESP, ARRAY, START, END) \
  for (int i = START; i >= END; i--)           \
  ESP_PUSH(ESP, ARRAY[i])

#define ESP_WORD_ALIGN(ESP) \
  while ((int)ESP % 4 != 0) \
  ESP_PUSH(ESP, '\0')

/**
 * @brief 进程初始化数据
 * 
 */
struct process_init_data
{
  char *args;              /* 传入的参数*/
  struct semaphore sema;   /* 父进程等待子进程的信号量 */
  bool load_status;        /* 是否成功开启进程*/
  process_info_t_ptr info; /* 保存进程的进程信息 */
};

// 进程初始化数据
typedef struct process_init_data process_init_data_t;
// 进程初始化数据的指针
typedef struct process_init_data *process_init_data_t_ptr;

// 对进程进行遍历时要执行的函数
typedef void process_action_func(process_info_t_ptr info, void *aux);
static bool init_process_info(process_info_t_ptr *info_ptr);
static void start_process(void *init_data_);
static bool load(const char *cmdline, void (**eip)(void), void **esp);
void main_init(void);
void process_foreach(struct list *list, process_action_func *func, void *aux);
void close_all_for_current(void);

/**
 * @brief 关闭当前线程所有打开的文件
 * 
 */
void close_all_for_current(void)
{
  struct thread *t = thread_current();

  lock_acquire(&g_fd_lock);

  while (!list_empty(&t->process_info->fd_list))
  {
    struct list_elem *e = list_pop_front(&t->process_info->fd_list);
    file_descriptor_t_ptr fd = list_entry(e, file_descriptor_t, elem);

    file_close(fd->file);
    free(fd);
  }

  lock_release(&g_fd_lock);
}

/**
 * 在thread/init.c中补充初始化主进程的信息
 * 主进程没有父进程
 * 
 */
void main_init(void)
{
  process_info_t_ptr main_info;

  bool result = init_process_info(&main_info);

  main_info->is_main = true;
  struct thread *t = thread_current();
  t->process_info = main_info;
}
/**
 * 进程初始化信息
 * 
 * @param init_data 
 * @param args_copy 
 */
void process_set_init_data(process_init_data_t_ptr init_data, char *args_copy)
{
  init_data->args = args_copy;
  /**
   * 初始化的时候锁住
   * 
   */
  sema_init(&(init_data->sema), 0);
  init_data->load_status = false;
}

/**
 * @brief 初始化进程信息
 * 
 * @param info_ptr 
 * @return true 
 * @return false 
 */
bool init_process_info(process_info_t_ptr *info_ptr)
{
  process_info_t_ptr info;
  // 在进程退出时释放分配的内存
  info = malloc(sizeof(process_info_t));
  if (info == NULL)
    return false;

  // initialize file descriptor list
  list_init(&(info->fd_list));

  info->is_alive = true;
  info->is_waited = false;
  info->is_main = false;

  lock_init(&(info->lock));
  cond_init(&(info->cond));
  list_init(&(info->children_list));

  info->ret = 0;
  info->pid = -1;
  *info_ptr = info;

  return true;
}
/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t process_execute(const char *file_name)
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page(0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy(fn_copy, file_name, PGSIZE);

  char tmp[strnlen(fn_copy, PGSIZE)];
  strlcpy(tmp, fn_copy, PGSIZE);

  char *thread_name, *save_ptr;
  thread_name = strtok_r(tmp, " ", &save_ptr);
  ASSERT(thread_name != NULL);

  /* setup the data struct to pass to start_process */
  process_init_data_t init_data;
  process_set_init_data(&init_data, fn_copy);

  /* 进行有关父子进程相关信息创建维护 */
  process_info_t_ptr child_info;
  if (init_process_info(&child_info) == false)
  {
    return -1;
  }
  init_data.info = child_info;

  process_info_t_ptr this_info = thread_current()->process_info;
  //将子进程列表放入父进程的子进程列表中
  list_push_back(&(this_info->children_list), &(child_info->child_elem));

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create(thread_name, PRI_DEFAULT, start_process, (void *)&init_data);
  if (tid != TID_ERROR)
  {
      // 等待进程开始函数完成加载可执行文件操作
    sema_down(&(init_data.sema));
  }

  // 在进程开始函数中完成加载可执行文件后再执行下面的语句
  // 一定要把这部分占用的一页的内存释放掉
  palloc_free_page(fn_copy);

  // 未加载成功
  if (!init_data.load_status)
  {
    return TID_ERROR;
  }

  return tid;
}

/* A thread function that loads a user process and starts it
   running. */
static void start_process(void *init_data_)
{
  process_init_data_t_ptr init_data = (process_init_data_t_ptr)init_data_;
  char *args = init_data->args;

  struct intr_frame if_;
  bool success;

  /* Initialize interrupt frame and load executable. */
  memset(&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  char *tmp = malloc(strlen(args) + 1);
  strlcpy(tmp, args, strlen(args) + 1);
  char *save_ptr = NULL;
  tmp = strtok_r(tmp, " ", &save_ptr);

  success = load(tmp, &if_.eip, &if_.esp);

  /**
   * 初始化进程信息
   * 
   */
  init_data->load_status = success;
  struct thread *t = thread_current();
  init_data->info->pid = (pid_t)t->tid;
  t->process_info = init_data->info;

  /* 初始化完成后再释放掉可以保证主进程不中断 */
  sema_up(&(init_data->sema));

  /* If load failed, quit. */
  if (!success)
  {
    exit(-1);
  }

  /* 参数传递 */
  /*
  Address     Name	          Data	      Type
  0xbffffffc	argv[3][...]	  bar\0	      char[4]
  0xbffffff8	argv[2][...]	  foo\0	      char[4]
  0xbffffff5	argv[1][...]	  -l\0	      char[3]
  0xbfffffed	argv[0][...]	  /bin/ls\0	  char[8]
  0xbfffffec	word-align	    0	          uint8_t
  0xbfffffe8	argv[4]	        0	          char *
  0xbfffffe4	argv[3]	        0xbffffffc	char *
  0xbfffffe0	argv[2]	        0xbffffff8	char *
  0xbfffffdc	argv[1]	        0xbffffff5	char *
  0xbfffffd8	argv[0]	        0xbfffffed	char *
  0xbfffffd4	argv	          0xbfffffd8	char **
  0xbfffffd0	argc	          4	          int
  0xbfffffcc	return address	0	          void (*) ()
  */
  char *esp = (char *)if_.esp; // 维护栈顶，转为char*就可以直接复制字符串进去了
  int argv[128], argc = 0;     // argv:存储的参数地址 argc:参数数量 len:tmp长度
  for (; tmp != NULL; tmp = strtok_r(NULL, " ", &save_ptr))
  {
    ESP_PUSH_BATCH(esp, tmp, strlen(tmp), 0); //'(tmp)\0'
    argv[argc++] = (int)esp;                  // 记录参数地址
  }
  ESP_WORD_ALIGN(esp);                  // word-align
  int *p = (int *)esp;                  // 接下来存argv地址，指针4个字节
  ESP_PUSH(p, 0);                       // argv[argc + 1]
  ESP_PUSH_BATCH(p, argv, argc - 1, 0); // argv[0:argc + 1]
  ESP_PUSH(p, (int)(p + 1));            // argv
  ESP_PUSH(p, argc);                    // argc;
  ESP_PUSH(p, 0);                       // return address
  if_.esp = p;                          // 栈更新

  free(tmp); // 释放临时字符串占有的内存

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */

  asm volatile("movl %0, %%esp; jmp intr_exit"
               :
               : "g"(&if_)
               : "memory");
  NOT_REACHED();
}
/**
 * @brief 获取进程信息
 * 
 * @param parent_info 
 * @param child_pid 
 * @return struct process_info* 
 */
process_info_t_ptr process_get_info(process_info_t_ptr parent_info, pid_t child_pid)
{
  struct list_elem *e;
  struct list *list = &(parent_info->children_list);

  for (e = list_begin(list);
       e != list_end(list);
       e = list_next(e))
  {
    process_info_t_ptr info = list_entry(e, process_info_t, child_elem);

    if (info->pid == child_pid)
    {
      return info;
    }
  }

  return NULL;
}

/**
 * @brief 获取当前进程信息
 * 
 * @param child_pid 
 * @return struct process_info* 
 */
process_info_t_ptr process_get_info_for_current(pid_t child_pid)
{
  return process_get_info(thread_current()->process_info, child_pid);
}

/**
 * @brief 释放掉所有已退出子进程的信息，如果没退出，就等待
 * 
 * @param parent_info 
 */
void free_dead_children(process_info_t_ptr parent_info)
{
  struct list *list = &(parent_info->children_list);
  struct list_elem *e;
  struct list_elem *ne;

  e = list_begin(list);
  while (e != list_end(list))
  {
    process_info_t_ptr child_info = list_entry(e, process_info_t, child_elem);

    ne = list_next(e);

    lock_acquire(&(child_info->lock));
    if (!child_info->is_alive)
    {
      list_remove(e);
      free(child_info);
    }
    else
    {
      lock_release(&(child_info->lock));
    }

    e = ne;
  }
}
/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */
int process_wait(tid_t child_tid UNUSED)
{
  process_info_t_ptr child_info = process_get_info_for_current(child_tid);

  if (child_info == NULL)
    return -1;

  int result = -1;

  /*  等待这个子进程结束 */
  lock_acquire(&(child_info->lock));
  //如果子进程还未退出
  if (!child_info->is_waited && child_info->is_alive)
  {
    //等待退出
    cond_wait(&(child_info->cond), &(child_info->lock));
  }
  //退出之后
  result = child_info->ret;
  child_info->is_waited = true;
  child_info->ret = -1;

  lock_release(&(child_info->lock));

  return result;
}
void wait_children(process_info_t_ptr info, void *aux UNUSED)
{
  process_wait(info->pid);
}
/**
 * @brief 遍历列表里所有进程
 * 
 * 
 * @param list 
 * @param func 
 * @param aux 
 */
void process_foreach(struct list *list, process_action_func *func, void *aux)
{
  struct list_elem *e;

  for (e = list_begin(list);
       e != list_end(list);
       e = list_next(e))
  {
    process_info_t_ptr info = list_entry(e, process_info_t, child_elem);

    func(info, aux);
  }
}

// /**
//  * @brief 释放掉持有的所有子进程的锁
//  *
//  * @param info
//  * @param UNUSED
//  */
// void release_children_locks(process_info_t_ptr info, void *aux UNUSED)
// {
//   if (lock_held_by_current_thread(&(info->lock)))
//     lock_release(&(info->lock));
// }

// /**
//  * @brief 设定子进程的父进程状态为结束
//  *
//  * @param info
//  * @param UNUSED
//  */
// void set_parent_dead(process_info_t_ptr info, void *aux UNUSED)
// {
//   lock_acquire(&(info->lock));
//   info->is_parentalive = false;
//   lock_release(&(info->lock));
// }

/* Free the current process's resources. */
void process_exit(void)
{

  struct thread *cur = thread_current();
  uint32_t *pd;

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL)
  {
    /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */

    cur->pagedir = NULL;
    pagedir_activate(NULL);
    pagedir_destroy(pd);
  }
  process_info_t_ptr info = cur->process_info;

  if (!lock_held_by_current_thread(&info->lock))
    lock_acquire(&(info->lock));

  //等待所有子进程退出以后
  process_foreach(&(info->children_list), &wait_children, NULL);

  /* 更改进程状态为已经退出 */
  info->is_alive = false;

  /* 释放掉持有的子进程的锁 */
  //process_foreach(&(info->children_list), &release_children_locks, NULL);

  /* 父进程已经结束 */
  // process_foreach(&(info->children_list), &set_parent_dead, NULL);

  /* 释放掉所有已结束的子进程*/
  free_dead_children(info);

  /* 关闭所有打开的文件 */
  close_all_for_current();

  /* allow writes to the executable */
  if (cur->exec_file != NULL)
  {
    file_allow_write(cur->exec_file);
    file_close(cur->exec_file);
    cur->exec_file = NULL;
  }

  /* 打印退出信息 */
  printf("%s: exit(%d)\n", cur->name, info->ret);

  /* 如果是主进程，则没有父进程，直接释放 */
  if (info->is_main)
  {
    lock_release(&(info->lock));
    free(info);
    info = NULL;
  }
  else
  {
    /* 如果父进程在等自己结束，告诉父进程自己结束了，结束父进程的等待 */
    cond_signal(&(info->cond), &(info->lock));
    lock_release(&(info->lock));
  }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void process_activate(void)
{
  struct thread *t = thread_current();

  /* Activate thread's page tables. */
  pagedir_activate(t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32 /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32 /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32 /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16 /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
{
  unsigned char e_ident[16];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
};

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
{
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
};

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL 0           /* Ignore. */
#define PT_LOAD 1           /* Loadable segment. */
#define PT_DYNAMIC 2        /* Dynamic linking info. */
#define PT_INTERP 3         /* Name of dynamic loader. */
#define PT_NOTE 4           /* Auxiliary info. */
#define PT_SHLIB 5          /* Reserved. */
#define PT_PHDR 6           /* Program header table. */
#define PT_STACK 0x6474e551 /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1 /* Executable. */
#define PF_W 2 /* Writable. */
#define PF_R 4 /* Readable. */

static bool setup_stack(void **esp);
static bool validate_segment(const struct Elf32_Phdr *, struct file *);
static bool load_segment(struct file *file, off_t ofs, uint8_t *upage,
                         uint32_t read_bytes, uint32_t zero_bytes,
                         bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool load(const char *file_name, void (**eip)(void), void **esp)
{
  struct thread *t = thread_current();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create();
  if (t->pagedir == NULL)
    goto done;
  process_activate();

  /* Open executable file. */
  lock_acquire(&g_fd_lock);
  file = filesys_open(file_name);
  if (file == NULL)
  {
    printf("load: %s: open failed\n", file_name);
    goto done;
  }

  /* Read and verify executable header. */
  if (file_read(file, &ehdr, sizeof ehdr) != sizeof ehdr || memcmp(ehdr.e_ident, "\177ELF\1\1\1", 7) || ehdr.e_type != 2 || ehdr.e_machine != 3 || ehdr.e_version != 1 || ehdr.e_phentsize != sizeof(struct Elf32_Phdr) || ehdr.e_phnum > 1024)
  {
    printf("load: %s: error loading executable\n", file_name);
    goto done;
  }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++)
  {
    struct Elf32_Phdr phdr;

    if (file_ofs < 0 || file_ofs > file_length(file))
      goto done;
    file_seek(file, file_ofs);

    if (file_read(file, &phdr, sizeof phdr) != sizeof phdr)
      goto done;
    file_ofs += sizeof phdr;
    switch (phdr.p_type)
    {
    case PT_NULL:
    case PT_NOTE:
    case PT_PHDR:
    case PT_STACK:
    default:
      /* Ignore this segment. */
      break;
    case PT_DYNAMIC:
    case PT_INTERP:
    case PT_SHLIB:
      goto done;
    case PT_LOAD:
      if (validate_segment(&phdr, file))
      {
        bool writable = (phdr.p_flags & PF_W) != 0;
        uint32_t file_page = phdr.p_offset & ~PGMASK;
        uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
        uint32_t page_offset = phdr.p_vaddr & PGMASK;
        uint32_t read_bytes, zero_bytes;
        if (phdr.p_filesz > 0)
        {
          /* Normal segment.
                     Read initial part from disk and zero the rest. */
          read_bytes = page_offset + phdr.p_filesz;
          zero_bytes = (ROUND_UP(page_offset + phdr.p_memsz, PGSIZE) - read_bytes);
        }
        else
        {
          /* Entirely zero.
                     Don't read anything from disk. */
          read_bytes = 0;
          zero_bytes = ROUND_UP(page_offset + phdr.p_memsz, PGSIZE);
        }
        if (!load_segment(file, file_page, (void *)mem_page,
                          read_bytes, zero_bytes, writable))
          goto done;
      }
      else
        goto done;
      break;
    }
  }

  /* Set up stack. */
  if (!setup_stack(esp))
    goto done;

  /* Start address. */
  *eip = (void (*)(void))ehdr.e_entry;

  success = true;

done:
  /* We arrive here whether the load is successful or not. */
  if (success)
  {
    t->exec_file = file;
    //printf("%d\n", t->tid);
    // t->process_info->exec_file = file;
    // file_deny_write(t->process_info->exec_file);
    file_deny_write(file);
  }
  else
  {
    file_close(file);
  }

  lock_release(&g_fd_lock);

  return success;
}

/* load() helpers. */

static bool install_page(void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment(const struct Elf32_Phdr *phdr, struct file *file)
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
    return false;

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off)file_length(file))
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz)
    return false;

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;

  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr((void *)phdr->p_vaddr))
    return false;
  if (!is_user_vaddr((void *)(phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment(struct file *file, off_t ofs, uint8_t *upage,
             uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  ASSERT((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT(pg_ofs(upage) == 0);
  ASSERT(ofs % PGSIZE == 0);

  file_seek(file, ofs);
  while (read_bytes > 0 || zero_bytes > 0)
  {
    /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
    size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
    size_t page_zero_bytes = PGSIZE - page_read_bytes;

    /* Get a page of memory. */
    uint8_t *kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL)
      return false;

    /* Load this page. */
    if (file_read(file, kpage, page_read_bytes) != (int)page_read_bytes)
    {
      palloc_free_page(kpage);
      return false;
    }
    memset(kpage + page_read_bytes, 0, page_zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page(upage, kpage, writable))
    {
      palloc_free_page(kpage);
      return false;
    }

    /* Advance. */
    read_bytes -= page_read_bytes;
    zero_bytes -= page_zero_bytes;
    upage += PGSIZE;
  }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack(void **esp)
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page(PAL_USER | PAL_ZERO);
  if (kpage != NULL)
  {
    success = install_page(((uint8_t *)PHYS_BASE) - PGSIZE, kpage, true);
    if (success)
      *esp = PHYS_BASE - 12;
    else
      palloc_free_page(kpage);
  }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page(void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page(t->pagedir, upage) == NULL && pagedir_set_page(t->pagedir, upage, kpage, writable));
}
