# Project 1 实验文档

[TOC]

## 团队

### 基本信息

| 姓名   | 学号     |
| ------ | -------- |
| 张智博 | 19373042 |
| 王彦婷 | 18241081 |
| 陈哲   | 19373305 |
| 姜海洋 | 19373614 |

### 每位组员的主要工作内容

| 姓名   | 工作内容                           |
| ------ | ---------------------------------- |
| 张智博 | Mission3, 项目管理, 代码整合与测试 |
| 王彦婷 | Mission1, 调研和资料收集,代码审核  |
| 陈哲   | Mission2-2, 代码结构分析, 报告整理 |
| 姜海洋 | Mission2-1, 文档整理, 风格检查     |

### Git 相关

[Pintos 的 Git 项目地址](https://gitee.com/buaa-os/os)

#### 项目网络图

![image-20211109125915465](http://longzhen.ren:12580/images/2021/11/09/image-20211109125915465.png)

### 参考资料

1. [Pintos Guide by  Stephen Tsung-Han Sher, Indiana University Bloomington](https://static1.squarespace.com/static/5b18aa0955b02c1de94e4412/t/5b85fad2f950b7b16b7a2ed6/1535507195196/Pintos+Guide)
2. [Official Pintos documentation](https://web.stanford.edu/class/cs140/projects/pintos/pintos_1.html)

## 实验要求

通过修改操作系统中的代码，使得操作系统可以正常运行并通过测试点：

**Mission1**:在实验过程中，线程进入睡眠的方式会造成忙等待，修改代码消除忙等待。

**Mission2.1**:在实验过程中，实现线程优先队列，并解决线程生命周期中的优先级排序问题

## 需求分析

**Mission1**：修改代码使得线程睡眠时不占据CPU，这样就可以消除“忙等待”。

**Mission2.1**:在实验过程中，实现线程优先队列，并解决线程生命周期中的优先级排序问题

## 设计思路

**Mission1**：在Pintos中，线程一共有运行态，就绪态，阻塞态和销毁态四个状态，如果想要“忙等待”使得线程不占用CPU资源，可以在线程睡眠的时候处于**阻塞状态**。利用时钟终端，检查线程的状态（检查时间片），如果时间片用完，就唤醒这个线程。

**Mission2.1**:在pintos中，线程加入队列时直接使用list_push_back 函数直接放在队列队尾，而维持一个优先队列需要比较两个线程之间的优先级，因此实现一个比较函数后，就可以实现在有序队列中插入，进而在创建和唤醒以及挂起等函数中保证每次加入队列都维持队列的有序性，就可以保证优先级队列的实现了

## 重难点讲解

**Mission1**：在实验中，阅读代码时发现，`timer_sleep`就是在传入函数的时间`ticks`内，如果线程处于运行状态就把该线程放入就绪队列中。也就是说线程不断在CPU就绪状态队列和运行状态之间来回，一直占用CPU的资源。

在实验过程中，比较复杂的就是阅读源代码，在实验过程中很多函数调用的非常复杂，其中有的函数通过阅读注解就可以轻易地理解函数的作用和返回结果，但是有的函数描述得并不清楚，需要对函数进行深入地解读。了解家源代码如何**屏蔽时钟中断**，如何**实现原子操作**，在重新实现的时候，如果有这些需要，可以仿照源代码的方式进行书写。

在源代码中，`intr_disable()`将标志寄存器的IF位置为0, IF=0时将不响应可屏蔽中断。

```c
enum intr_level old_level = intr_disable ();
...
intr_set_level (old_level);
```

获取了当前的中断状态， 然后将当前中断状态改为不能被中断， 然后返回执行之前的中断状态。

根据设计思路，除了对原来实现方法做出了解之外，还需要查找源代码中关于线程阻塞的函数，了解其作用和效果，为重新实现的时候打好基础。经过查找发现，在`thread.h`和`thread.c`中有以下两个函数：

```c
/* thread.h */
void thread_block (void);
void thread_unblock (struct thread *);
```

```c
/* thread.c */

void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  list_push_back (&ready_list, &t->elem);
  t->status = THREAD_READY;
  intr_set_level (old_level);
}
```

因此如果想要实现线程的阻塞的话，可以调用这两个函数。

**Mission2.1**:在尝试修改函数本身结构体结果发现队列顺序在每次更改后都需要修改每个函数中的顺序十分繁琐之后，发现调用*list_entry*取线程在线程队列中的指针可以比较队列的先后，从而维持优先级只需要比较两个队列之间的大小，找到合适的插入位置就可以。

## 用户手册

测试命令如下:

```bash
cd /pintos/src/threads
make clean
make check
```

## 测试报告

可以成功通过全部测试点

![image-20211109130440039](http://longzhen.ren:12580/images/2021/11/09/image-20211109130440039.png)

## 各成员的心得体会

### Student1

### Student2

**王彦婷**：在实验过程中阅读代码的时候发现，在函数中，会反复验证条件是不是符合，并且保证原子操作。在实验过程中加深了对于操作系统知识的理解，尤其是关于进程忙等待，阻塞等方面的知识。通过阅读代码，更加规范了写代码的风格。

### Student3

### Student4

**姜海洋**：在实验过程中，发现操作系统的内容十分繁琐且耦合性很高，锻炼了我在大量代码中搜寻所需函数的能力，而实现的过程中，维持优先级队列本不是什么很繁琐的需求，但因为一开始的实现方式过于繁琐导致绕了很多弯路。由此体会到在开始动手前大量思考规划出合理方案的必要性

## 其他你认为有必要的内容 (Optional)

# Project 1 Design Document

## QUESTION 1: ALARM CLOCK

### DATA STRUCTURES

> A1: Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

```c
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
    int64_t ticks_blocked;					/* Ticks blocked by sleep timer. */
  };
```

备注：在线程的属性中加入一个用来记录被阻塞时间的变量，当线程被阻塞的之后就通过修改这个变量的值，来确定被阻塞的时间。

### ALGORITHMS

> A2: Briefly describe what happens in a call to timer_sleep(), including the effects of the timer interrupt handler.

#### A2：

```c
/* timer.c */
void
timer_sleep (int64_t ticks) 
{
  if(ticks <= 0){
    return;
  }
  ASSERT (intr_get_level () == INTR_ON);
  enum intr_level old_level = intr_disable();
  struct thread *current_thread = thread_current();
  current_thread->ticks_blocked = ticks;
  thread_block ();
  intr_set_level(old_level);
}

/* timer.c */
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  ticks++;
  thread_tick ();
  thread_foreach (blocked_thread_check, NULL);
}
/* thread.c */
void
blocked_thread_check (struct thread *t, void *aux UNUSED)
{
  if (t->status == THREAD_BLOCKED && t->ticks_blocked > 0)
  {
    t->ticks_blocked--;
    if(t->ticks_blocked == 0)
    {
      thread_unblock(t);
    }
  }
}
```

A2问题涉及的代码主要涉及以上三个函数：

- 先判断传入的`ticks`，如果其值小于0，则返回到`static voidreal_time_sleep (int64_t num, int32_t denom) `函数，否则通过函数`thread_current()`获得当前正在运行的进程，并将线程属性`ticks_blocked`设置成为传入的时间。
- 在时钟中断的时候，需要通过函数`void thread_foreach (thread_action_func *func, void *aux)`对所有的函数进行检查。
- 检查的内容为检查阻塞的时间，如果阻塞的时间大于0，那么需要将阻塞的时间进行减一操作。此时，如果时间为零，则将这个线程取消阻塞状态，放入到就绪队列中。



> A3: What steps are taken to minimize the amount of time spent in the timer interrupt handler?

#### A3:

- 修改线程的结构体，加入了记录还需要阻塞的时间，将进程阻塞，不需要一直在就绪队列和CPU进行切换，从而消除了忙等待，此时CPU可以运行其他进程，节约了时间。

### SYNCHRONIZATION

> A4: How are race conditions avoided when multiple threads call timer_sleep() simultaneously?

#### A4:

```c
enum intr_level old_level = intr_disable();
struct thread *current_thread = thread_current();
current_thread->ticks_blocked = ticks;
thread_block ();
intr_set_level(old_level);
```

- 通过上述的原子操作，避免竞争。

通过原子操作

> A5: How are race conditions avoided when a timer interrupt occurs during a call to timer_sleep()?

#### A5:

- 通过`intr_disable()`将标志寄存器的IF位置为0, IF=0时将不响应可屏蔽中断。

```c
enum intr_level old_level = intr_disable ();
...
intr_set_level (old_level);
```

- 获取了当前的中断状态， 然后将当前中断状态改为不能被中断， 然后返回执行之前的中断状态。



### RATIONALE

> A6: Why did you choose this design? In what ways is it superior to another design you considered?

#### A6:

另外一种设计方法：增加一个阻塞的队列`blocked_list`。当进程阻塞时就将进程放入阻塞队列中，每次时钟中断的时候对这个队列中的进程阻塞时间进行更改，当阻塞完成的时候，将次进程放入就绪队列。

上述的这种方法需要将进程在不同的队列中进行移动，会造成一定开销。

## QUESTION 2: PRIORITY SCHEDULING

### DATA STRUCTURES

> B1: Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

> B2: Explain the data structure used to track priority donation. Use ASCII art to diagram a nested donation. (Alternately, paste an image.)

### ALGORITHMS

> B3: How do you ensure that the highest priority thread waiting for a lock, semaphore, or condition variable wakes up first?

> B4: Describe the sequence of events when a call to lock_acquire() causes a priority donation. How is nested donation handled?

> B5: Describe the sequence of events when lock_release() is called on a lock that a higher-priority thread is waiting for.

### SYNCHRONIZATION

> B6: Describe a potential race in thread_set_priority() and explain how your implementation avoids it. Can you use a lock to avoid this race?

### RATIONALE

> B7: Why did you choose this design? In what ways is it superior to another design you considered?

## QUESTION 3: ADVANCED SCHEDULER

### DATA STRUCTURES

> C1: Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.
>
> ```c
> /* thread.h */
> struct thread
> {
> /* Members for project1 mission3 mlfqs */
> int nice;        /* Nice Value in MLFQS. */
> fp_t recent_cpu; /* Recent CPU Value. */
> };
> 
> /* thread.c */
> fp_t load_avg; /* loag_avg Value of the whole system. */
> ```

### ALGORITHMS

> C2: Suppose threads A, B, and C have nice values 0, 1, and 2. Each has a recent_cpu value of 0. Fill in the table below showing the scheduling decision and the priority and recent_cpu values for each thread after each given number of timer ticks:

| timer ticks | `recent_cpu` A | `recent_cpu` B | `recent_cpu` C | `priority` A | `priority` B | `priority` C | thread to run |
| ----------- | -------------- | -------------- | -------------- | ------------ | ------------ | ------------ | ------------- |
| 0           | 0              | 1              | 2              | 63           | 61           | 59           | A             |
| 4           | 4              | 1              | 2              | 62           | 61           | 59           | A             |
| 8           | 8              | 1              | 2              | 61           | 61           | 59           | B             |
| 12          | 8              | 5              | 2              | 61           | 60           | 59           | A             |
| 16          | 12             | 5              | 2              | 60           | 60           | 59           | B             |
| 20          | 12             | 9              | 2              | 60           | 59           | 59           | A             |
| 24          | 16             | 9              | 2              | 59           | 59           | 59           | C             |
| 24          | 16             | 9              | 6              | 59           | 59           | 58           | B             |
| 32          | 16             | 13             | 6              | 59           | 58           | 58           | A             |
| 36          | 20             | 13             | 6              | 58           | 58           | 58           | C             |

> C3: Did any ambiguities in the scheduler specification make values in the table uncertain? If so, what rule did you use to resolve them? Does this match the behavior of your scheduler?

由于在浮点数运算的Round（实数舍入到整数）的过程中采取了以下的实现方式：

```c
#define ROUND(A) (A >= 0 ? ((A + (1 << (14)) / 2) >> 14) : ((A - (1 << (14)) / 2) >> 14))
t->priority = ROUND(SUB(SUB(CONST(PRI_MAX), IDIV(t->recent_cpu, 4)), IMUL(CONST(2), t->nice)));
```

因此可能存在两个不同的priority经过舍入后，仅保留整数部分，造成优先级的相同的现象。

另外由于我们采取的实现方式是：在任务三高级调度中的所有更改和运算最终都都体现到线程的priority属性上，并依靠任务2中已经实现的优先级调度方式进行处理，即动态修改等待队列中的全部线程的优先级顺序，并重新进行调度。因此在更改优先级之后，可能出现原来的优先级不同、经过高级调度之后优先级相同的现象，导致了算法结果的不确定性。

由于使用了任务二中实现的线程队列，因此对于相同优先级的调度，执行顺序依赖于线程队列的实现算法，即将线程推进就绪队列中，根据优先级队列的特性，先进入的进程会先被执行，因此就可能出现先进入的优先级为54.5的线程比后进入的优先级为55.2的线程先执行的情况发生。

此外，由于计算recent_cpu、 load_avg、priority的时间不能确定，导致我们认为的ticks略大于实际的ticks，可能会导致不确定性。

> C4: How is the way you divided the cost of scheduling between code inside and outside interrupt context likely to affect performance?

### RATIONALE

> C5: Briefly critique your design, pointing out advantages and disadvantages in your design choices. If you were to have extra time to work on this part of the project, how might you choose to refine or improve your design?

系统内置了基于时钟的中断处理函数以及使用链表方式实现的线程队列和相关的系统函数。基于这些的方式进行实现。优点是思路相对简洁，编写代码相对简单，缺点是调度方式不够精确，占用系统资源相对较多，且对于中断处理的相关细节了解不够深入。

在本次任务中，由于作为课程配套设计，我们以“通过全部测试点”为驱动进行编写，只要测试点通过，那么就假定我们的实现是正确且合理的。如果有更加充足的时间，我们会尝试更多的调度方式，比如修改内核，新建专门的调度进程、对于各个不同的优先级创建单独的就绪队列等方式，提高系统的效率和结构性。
