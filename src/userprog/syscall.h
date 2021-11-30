#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
typedef int pid_t;
#define bool _Bool

struct lock fd_lock;
// struct list file_opened_list;
int fd_num;

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
