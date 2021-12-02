#include "filesys/file.h"
#include "threads/synch.h"

#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
typedef int pid_t;
#define bool _Bool

// struct list file_opened_list;

// 全局文件锁
struct lock g_fd_lock;

// 全局文件描述符标号
int g_fd_num;

/**
 * @brief 文件描述符
 * 
 */
struct file_descriptor
{
    int num;               // 标号
    struct file *file;     // 文件
    struct list_elem elem; // 列表项
};

// 文件描述符
typedef struct file_descriptor file_descriptor_t;
// 文件描述符的指针
typedef struct file_descriptor *file_descriptor_t_ptr;

void syscall_init(void);

// void call_halt(void);
// void call_exit(struct intr_frame *);
// void call_exec(struct intr_frame *);
// void call_wait(struct intr_frame *);
// void call_create(struct intr_frame *);
// void call_remove(struct intr_frame *);
// void call_open(struct intr_frame *);
// void call_filesize(struct intr_frame *);
// void call_read(struct intr_frame *);
// void call_write(struct intr_frame *);
// void call_seek(struct intr_frame *);
// void call_tell(struct intr_frame *);
// void call_close(struct intr_frame *);

// 以下是本次实验要实现的所有系统调用接口
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

#endif /* userprog/syscall.h */
