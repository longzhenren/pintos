# Project 1 实验文档

[TOC]

## 团队

### 基本信息

| 姓名   | 学号     |
| ------ | -------- |
| 张智博 | 19373042 |
|        |          |
|        |          |
|        |          |

### 每位组员的主要工作内容

### Git 相关

[Pintos 的 Git 项目地址](https://gitee.com/buaa-os/os)

### 参考资料

1. [Pintos Guide by  Stephen Tsung-Han Sher, Indiana University Bloomington](https://static1.squarespace.com/static/5b18aa0955b02c1de94e4412/t/5b85fad2f950b7b16b7a2ed6/1535507195196/Pintos+Guide)
2. [Official Pintos documentation](https://web.stanford.edu/class/cs140/projects/pintos/pintos_1.html)

## 实验要求

## 需求分析

## 设计思路

## 重难点讲解

## 用户手册

## 测试报告

## 各成员的心得体会

### Student1

### Student2

### Student3

### Student4

## 其他你认为有必要的内容 (Optional)

# Project 1 Design Document

## QUESTION 1: ALARM CLOCK

### DATA STRUCTURES

> A1: Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or enumeration. Identify the purpose of each in 25 words or less.

### ALGORITHMS

> A2: Briefly describe what happens in a call to timer_sleep(), including the effects of the timer interrupt handler.

> A3: What steps are taken to minimize the amount of time spent in the timer interrupt handler?

### SYNCHRONIZATION

> A4: How are race conditions avoided when multiple threads call timer_sleep() simultaneously?

> A5: How are race conditions avoided when a timer interrupt occurs during a call to timer_sleep()?

### RATIONALE

> A6: Why did you choose this design? In what ways is it superior to another design you considered?

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

