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

[Pintos 的 Git 项目地址]()

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
> 
> ```

### ALGORITHMS

> C2: Suppose threads A, B, and C have nice values 0, 1, and 2. Each has a recent_cpu value of 0. Fill in the table below showing the scheduling decision and the priority and recent_cpu values for each thread after each given number of timer ticks:

| timer ticks | `recent_cpu` A | `recent_cpu` B | `recent_cpu` C | `priority` A | `priority` B | `priority` C | thread to run |
| ----------- | -------------- | -------------- | -------------- | ------------ | ------------ | ------------ | ------------- |
| 0           | 0.00           | 0.00           | 0.00           | 63.00        | 61.00        | 59.00        | A             |
| 4           | 4.00           | 1.00           | 2.00           | 62.00        | 60.75        | 58.50        | A             |
| 8           | 8.00           | 1.00           | 2.00           | 61.00        | 60.75        | 58.50        | A             |
| 12          | 12.00          | 1.00           | 2.00           | 60.00        | 60.75        | 58.50        | B             |
| 16          | 12.00          | 5.00           | 2.00           | 60.00        | 59.75        | 58.50        | A             |
| 20          | 2.36           | 1.45           | 2.18           | 62.40        | 60.63        | 58.45        | A             |
| 24          | 6.36           | 1.45           | 2.18           | 61.40        | 60.63        | 58.45        | A             |
| 28          | 10.36          | 1.45           | 2.18           | 60.40        | 60.63        | 58.45        | B             |
| 32          | 10.36          | 5.45           | 2.18           | 60.40        | 59.63        | 58.45        | A             |
| 36          | 14.36          | 5.45           | 2.18           | 59.40        | 59.63        | 58.45        | B             |

> C3: Did any ambiguities in the scheduler specification make values in the table uncertain? If so, what rule did you use to resolve them? Does this match the behavior of your scheduler?

> C4: How is the way you divided the cost of scheduling between code inside and outside interrupt context likely to affect performance?

### RATIONALE

> C5: Briefly critique your design, pointing out advantages and disadvantages in your design choices. If you were to have extra time to work on this part of the project, how might you choose to refine or improve your design?

