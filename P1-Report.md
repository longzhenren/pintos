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
2. [Kaist's gitbook](https://casys-kaist.github.io/pintos-kaistl)

## 实验要求

通过修改操作系统中的代码，使得操作系统可以正常运行并通过测试点：

### Mission1

在实验过程中，线程进入睡眠的方式会造成忙等待，修改代码消除忙等待。

### Mission2.1

在实验过程中，实现线程优先队列，并解决线程生命周期中的优先级排序问题。

### Mission2.2

在实验过程中，实现线程优先级捐赠，重点是要解决多次捐赠和递归捐赠的问题。

### Mission3

在实验过程中，实现一个类似于4.4BSD调度器的多级反馈队列调度器，以减少系统上运行作业的平均响应时间

## 需求分析

### Mission1

在实验过程中，线程进入睡眠的方式会造成忙等待，修改代码消除忙等待。

### Mission2.1

设计排序算法保证每次线程有序地插入list中、保证线程能够正确地交出权限。在实验过程中，实现线程优先队列，并解决线程生命周期中的优先级排序问题。

### Mission2.2

在实验过程中，实现线程优先级捐赠，重点是要解决多次捐赠和递归捐赠的问题。

### Mission3

平衡线程的不同调度需求，根据recent_cpu、等若干参数，来更新线程的priority，使得线程以更合理的方式先后执行。

在实验过程中，需要实现一个类似于4.4BSD调度器的多级反馈队列调度器（MLFQS），以减少系统上运行作业的平均响应时间。

## 设计思路

### Mission1

在Pintos中，线程一共有运行态，就绪态，阻塞态和销毁态四个状态，如果想要“忙等待”使得线程不占用CPU资源，可以在线程睡眠的时候处于**阻塞状态**。利用时钟终端，检查线程的状态（检查时间片），如果时间片用完，就唤醒这个线程。

### Mission2.1

在Pintos中，线程加入队列时直接使用list_push_back 函数直接放在队列队尾。而线程为一个链表list结构，相较于动态维护，为每一个ele指定顺序，实现时选择了在每次更改链表时确保拥有正确的前后优先级关系，即可维持一个正确的优先级关系列表。
而维持一个优先队列需要比较两个线程之间的优先级，因此实现一个比较函数后，就可以实现在有序队列中插入，进而在创建和唤醒以及挂起等函数中保证每次加入队列都维持队列的有序性，就可以保证优先级队列的实现了。而由于是抢占式，当一个线程更改完优先级之后，需要立即重新考虑所有线程的执行顺序，因而调用yield，进入就绪态，重新调用。

### Mission2.2

通过阅读Pintos线程部分的实验代码，可以发现此实验任务要修改的地方集中在 `synch.c`、`synch.h`、`thread.c`、`thread.h` 中，前二者和信号量、锁、条件变量有关，后二者和线程有关。

若要实现优先级捐赠，就要考虑优先级何时捐赠、何时恢复、何时更新的问题。可以想到，在线程使用 `lock_acquire()` 获取锁时，需要进行优先级捐赠，其中就包含了优先级更新的过程，在此过程中，就要解决多次捐赠和递归捐赠的问题；而在线程获取锁之后，即锁的 `holder` 为当前线程时，我们也需要更新线程优先级；在线程使用 `lock_release()` 释放锁时，如果被捐赠了优先级，那么显然就要恢复原来的优先级，其中就包含了优先级更新的过程；另外，在使用 `thread_set_priority()` 设置线程的优先级时，显然也是要对线程的优先级进行更新的。

在初步分析了优先级捐赠的过程后，我们可以发现线程优先级的更新是一个重要的过程，可以将其单独写出一个函数。针对优先级恢复的要求，可以想到必须要保留线程优先级被捐赠之前的优先级，否则就不能恢复了；针对多次捐赠的要求，即当线程持有多个锁时，需要将线程的优先级设置为其被捐赠的优先级中最大的，可以想到必须要记录线程目前持有的锁以及被捐赠的优先级，而自然地，被捐赠的优先级可以和锁进行绑定，当要查找被捐赠的优先级中最大的优先级时，只要找线程持有的锁关联的被捐赠的优先级即可；针对递归捐赠的要求，可以想到必须要记录线程要申请的，即将要持有的锁，通过将要持有的锁，获取到它的 `holder`，即当前持有它的线程，才能形成一个优先级捐赠链，更新锁关联的被捐赠的优先级。

### Mission3

首先，为了保证效率，pintos的内核不支持浮点数运算，只支持整数运算。实验文档中提供了一种用整形运算代替浮点型运算（17.14定点数）的方法。根据实验要求对代码进行扩展，编写一系列浮点运算相关操作。为了保证节约内存空间，使用宏定义的方式进行运算的处理。

根据题意，我们需要在合适的时机实现定时动态更新recent_cpu、load_avg等值。

## 重难点讲解

### Mission1

在实验中，阅读代码时发现，`timer_sleep`就是在传入函数的时间`ticks`内，如果线程处于运行状态就把该线程放入就绪队列中。也就是说线程不断在CPU就绪状态队列和运行状态之间来回，一直占用CPU的资源。于是决定使用线程阻塞的方法来实现线程睡眠的方式，在时间片结束时不再对每一个thread进行轮询，而是将thread放入`block_list`中，每个时间片检查`block_list`中的线程是否到了唤醒的时间。如果thread没有到规定的唤醒时间，则等待下一次时间片的查询。若是已经到了规定的唤醒时间，那么就将该线程从`block_list`中取出，放入到`ready_list`当中。

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

### Mission2.1

在尝试修改函数本身结构体结果发现队列顺序在每次更改后都需要修改每个函数中的顺序十分繁琐，发现调用*list_entry*取线程在线程队列中的指针可以比较队列的先后，从而维持优先级只需要比较两个队列之间的大小，找到合适的插入位置就可以。

### Mission2.2

因为优先级捐赠部分的实验任务应该是一个整体，不好单独拿出某个点进行讲解，所以这里就将整体的实现过程叙述一遍。

根据前面在设计思路中的描述，我们就可以先在 `thread` 和 `lock` 结构体中添加需要的成员变量。

```c++
// thread.h
struct thread
{
  ...

  /* Members for project1 mission2 priority donate */
  // 记录原来的优先级，没有的话优先级在捐赠后就没法恢复原来的优先级了
  int original_priority;
  // 当前将要持有的锁，用于优先级捐赠，对于递归捐赠来说显然是需要的
  struct lock *desired_lock;
  // 线程持有的锁，多捐赠时优先级要设为最大的那个
  struct list holding_locks;

  ...
};

// synch.h
struct lock 
{
  ...

  /* Members for project1 mission2 priority donate */
  // 锁记录的最大优先级，用于优先级捐赠
  int donated_priority;
 
  /* Shared between thread.c and synch.c. */
  // 这主要是为了锁能够添加到 thread->holding_locks 中
  struct list_elem elem;      /* List element. */

  ...
};
```

然后就可以考虑实现线程优先级更新的过程。首先，若线程的优先级比被捐赠的优先级小，那么线程的优先级需要设置为其被捐赠的优先级中最大的，否则线程的优先级自然还是原来的优先级 `original_priority`，不需要被捐赠，这是多次捐赠的一个要求。而如何查找被捐赠的优先级中最大的呢？显然，在线程记录的持有的锁 `holding_locks` 中，因为是按顺序插入的，所以找到首个具有 `max(lock->donated_priority)` 值的锁即可，这个过程可以使用Pintos中已经提供的列表 `list` 操作的函数（`list_entry`、`list_max`）来完成，如下所示，其中 `cmp_lock_donated_priority__thread` 表示一个在 `thread.c` 文件中的被捐赠优先级比较函数。

```c++
// 找到持有锁中的最大优先级
max_donated_priority = list_entry(
  list_max(&t->holding_locks, cmp_lock_donated_priority__thread, NULL),
    struct lock, elem)->donated_priority;
```

然后最终要设置的优先级就是 `max(thread->original_priority, max_donated_priority)`，其中 `max_donated_priority` 为 `max(lock->donated_priority)`。在设置好新的优先级后，我们就要确保线程就绪队列的优先级顺序是正确的，因为线程可能处于 `THREAD_READY` 状态，在优先级被修改后，如果不确保线程就绪队列的优先级顺序，那么就可能会导致错误的线程调度。然后可以想到，这个过程只要简单地进行一次列表排序即可，相关的函数（`list_sort`）已经在Pintos中给出了，如下所示，其中 `thread_cmp_priority` 表示线程优先级比较函数。

```c++
// 确保线程优先级顺序
void thread_guard_priority_order(void)
{
  list_sort(&ready_list, thread_cmp_priority, NULL);
}
```

这样，线程优先级的更新过程就大致完成了。然后就可以考虑实现线程优先级捐赠的过程。根据前面的分析，可以确定需要修改的地方就是 `lock_acquire()`，通过阅读其代码，就能发现具体要在 `sema_down()`，即P操作之前完成优先级捐赠。首先，我们就需要记录当前线程将要持有的锁 `desired_lock` 为要申请的锁，以为可能的递归捐赠做准备。然后，若当前线程要申请的锁没有被持有，即 `holder` 为 `NULL`，那么自然就没有捐赠的过程；若锁被其他线程持有，那么就进行优先级捐赠，因为递归捐赠包含了简单捐赠，所以可以直接考虑递归捐赠的过程，若当前线程的优先级比锁记录的被捐赠优先级大，就要捐赠，即将锁记录的被捐赠优先级设为当前线程的优先级，否则，低优先级的线程显然不需要给高优先级的线程进行捐赠，以上是一个递归循环的过程，最终代码大致如下所示。

```c++
struct lock *tmp_lock = lock;

// 递归捐赠，只要当前线程的优先级比锁记录的最大优先级大，就要捐赠，此处要维护好锁记录的被捐赠优先级
// 比如 H->M->L，锁最大优先级要设为 H，同时提升 M 和 L 优先级
while (tmp_lock != NULL && tmp_lock->donated_priority < cur->priority)
{
  // 优先级捐赠
  tmp_lock->donated_priority = cur->priority;

  // 更新锁持有线程的优先级
  thread_update_priority(tmp_lock->holder);

  // 下一个锁，在 H->M->L 中，就是 M 申请 L 持有的锁
  tmp_lock = tmp_lock->holder->desired_lock;
}
```

这样，线程优先级捐赠的主要过程就大致完成了。之后考虑在 `lock_acquire()` 中 `sema_down()`，即P操作之后的运行逻辑。在P操作之后，表明当前线程不再被阻塞，成功获取到了要持有的锁，这从 `lock->holder = thread_current();` 中也可以看出。而持有了锁之后，那么线程自然就要记录这个持有的锁，并且之前设置的线程将要持有的锁 `desired_lock` 就要清空了，最后就需要更新当前线程优先级，确保线程优先级正确，最终代码大致如下所示。

```c++
struct thread *cur = thread_current();

// 把锁加到线程持有锁列表里
list_push_back(&cur->holding_locks, &lock->elem);

// 终于持有了锁，于是把当前将要持有的锁设为空
cur->desired_lock = NULL;

// 更新当前线程优先级
thread_update_priority(cur);
```

之后考虑线程释放锁时优先级恢复的过程。当前线程释放锁，自然要把锁从持有的锁中移除，而锁在被移除后，它就和线程的优先级的调整和更新无关了，也就是说之前记录的被捐赠优先级就要重置成 `PRI_MIN`，然后优先级的恢复可以直接通过之前的优先级更新过程完成，因为它里面已经包含了对原来优先级和被捐赠优先级的综合更新逻辑，如下所示。

```c++
struct thread *cur = thread_current();

// 将锁从持有锁列表中移除
list_remove(&lock->elem);

// 重置锁记录的被捐赠优先级
lock->donated_priority = PRI_MIN;

// 更新一下优先级
thread_update_priority(cur);
```

再考虑线程优先级设置的过程。优先级设置过程在 `thread_set_priority()` 中，其原本是直接设置了线程优先级，这是不符合优先级调度的，所以首先，线程要将原来的优先级记录为新的优先级，以进行可能的优先级更新，若线程没有持有锁或新的优先级比线程当前的优先级大，那么自然就要更新线程当前的优先级，在持有锁且新优先级比当前的小时，我们是不能去修改线程的优先级的。而若成功设置了新的优先级，那么当前线程就要进行重新调度，确保优先级调度顺序的正确，如下所示。

```c++
struct thread *cur = thread_current();

// 先设一下原来的优先级
cur->original_priority = new_priority;

// 没有持有锁或者新的优先级比当前优先级高，那就重设线程优先级并重新调度
// 我们需要保持线程优先级为被捐赠优先级和实际优先级中的最大值，即 max(donated, original)，这样才符合优先级捐赠部分的要求
if (list_empty(&cur->holding_locks) ||
   new_priority > cur->priority)
{
 thread_update_priority(cur);
 thread_yield();
}
```

这样，总体上线程优先级捐赠部分的实验任务就完成了。

**附加：信号量与条件变量相关的优先级调度**

这部分本应该是Mission2.1的任务内容，但在最初任务分配时没有分配好，使这一部分移到了Mission2.2里去，为了突出Mission2.2是实现优先级捐赠的过程，所以就将这一部分移到最后来叙述（其实也比较简单）。

首先可以知道信号量与条件变量相关的优先级调度的要求是“信号量唤醒时，唤醒信号量等待队列中优先级最高的线程”和“条件变量唤醒时，唤醒条件变量等待队列中优先级最高的线程”。而信号量唤醒的过程在 `sema_up()` 函数中，条件变量唤醒的过程在 `cond_signal()` 中，于是基本可以确定只要修改这两个函数就可以实现要求。

那么就先看一下信号量 `sema_up()`，它就相当于一个V操作，其中关键部分就是当等待此信号量的线程队列不为空时，调用 `thread_unblock()` 函数，如下所示，主要做的就是取出队列的头部线程，然后取消阻塞。

```c++
if (!list_empty (&sema->waiters)) 
 thread_unblock (list_entry (list_pop_front (&sema->waiters),
                             struct thread, elem));
```

这样的话思路就挺清晰了，只要把从队首取线程改成取最大优先级的线程就可以了，如下所示。

```c++
// 找出最大优先级的线程，并取消阻塞，转入就绪队列中
struct list_elem *max_priority_thread = list_max(&sema->waiters, cmp_thread_priority__synch, NULL);
list_remove(max_priority_thread);

thread_unblock(list_entry(max_priority_thread,
                        struct thread, elem));
```

然后考虑到抢占式调度，所以在最后加上 `thread_yield()` 进行重新调度。

然后再看一下条件变量 `cond_signal()`，其中关键部分就是当等待此条件变量的信号量元素队列不为空时，调用 `sema_up()` 函数，唤醒队首信号量元素包含的信号量，从而唤醒线程。

```c++
if (!list_empty (&cond->waiters)) 
 sema_up (&list_entry (list_pop_front (&cond->waiters),
                       struct semaphore_elem, elem)->semaphore);
```

这个形式和信号量的很像，可以想到这应该也是 `list_max()`、`list_remove()` 的改写形式。然后我们可以发现信号量元素是在 `cond_wait()` 函数中生成的，它主要就是包含一个初值为0的临时信号量，通过配合释放锁、P操作、获取锁实现，在P操作时临时信号量的线程等待队列中就新增了当前线程，也就是说实际上这个临时信号量的线程等待队列中只有一个线程，然后经过这样的分析，我们就能够写出需要的线程优先级比较函数了，如下所示。

```c++
bool cmp_cond_semaphore_thread_priority__synch(const struct list_elem *a, const struct list_elem *b, void *aux)
{
  // 对应上面的 list_push_back(&cond->waiters, &waiter.elem);
  // 这样就使和信号量有关的线程能够分开处理（唤醒）
  struct semaphore_elem *semaphore_a = list_entry(a, struct semaphore_elem, elem);
  struct semaphore_elem *semaphore_b = list_entry(b, struct semaphore_elem, elem);
  
  // 这里的优先级比较，实际上这里的信号量的线程等待队列里只有 1 个线程
  // 这从 cond_wait() 里的 sema_down(&waiter.semaphore); 可以得知，所以直接用一个 list_front() 取出线程进行优先级比较就可以了
  struct thread *thread_a = list_entry(list_front(&semaphore_a->semaphore.waiters), struct thread, elem);
  struct thread *thread_b = list_entry(list_front(&semaphore_b->semaphore.waiters), struct thread, elem);

  return thread_a->priority < thread_b->priority;
}
```

然后也就可以完成对 `cond_signal()` 的修改，主要部分如下所示。

```c++
if (!list_empty(&cond->waiters))
{
 // 唤醒优先级最大的线程
 struct list_elem *max_priority_thread_semaphore = list_max(&cond->waiters, cmp_cond_semaphore_thread_priority__synch, NULL);
 list_remove(max_priority_thread_semaphore);

 sema_up(&list_entry(max_priority_thread_semaphore,
                     struct semaphore_elem, elem)
              ->semaphore);
}
```

这样，信号量与条件变量相关的优先级调度也就基本完成了。

### Mission3

定义核心算法函数

```c
// 更新指定线程的recent_cpu值
void thread_update_recent_cpu_mlfqs(struct thread *t)
{
  if (t == idle_thread)
    return;
  t->recent_cpu = IADD(MUL(DIV(IMUL(load_avg, 2), IADD(IMUL(load_avg, 2), 1)), t->recent_cpu), t->nice);
}
// 更新当前系统的负载load_avg
void thread_update_load_avg(void)
{
  // load_avg = (59/60)*load_avg + (1/60)*ready_threads
  load_avg = ADD(IDIV(IMUL(load_avg, 59), 60), IDIV(CONST(ready_threads_count(thread_current())), 60));
  // load_avg = ADD(IMUL(load_avg, IDIV(CONST(59), 60)), IDIV(CONST(ready_threads_count(thread_current())), 60));
}

// 更新当前就绪队列中正在排队的线程总数
fp_t ready_threads_count(struct thread *t)
{
  fp_t ready_thread = list_size(&ready_list);
  if (t != idle_thread)
    ready_thread++;
  return ready_thread;
}

// 更新指定线程的优先级
void thread_update_priority_mlfqs(struct thread *t)
{
  if (t == idle_thread)
    return;
  t->priority = ROUND(SUB(SUB(CONST(PRI_MAX), IDIV(t->recent_cpu, 4)), IMUL(CONST(2), t->nice)));
  if (t->priority > PRI_MAX)
    t->priority = PRI_MAX;
  if (t->priority < PRI_MIN)
    t->priority = PRI_MIN;
}

```

任务三的难点在于核心算法函数调用的时机，应该在何时更新哪些变量、调用那些函数，才能符合题目要求。

- 依靠时钟中断，在`thread_tick`函数中调用，根据题目描述，每`TIMER_FREQ`个ticks更新一次负载和每个线程的recent_cpu值、每个时间片（4 ticks）更新一次全部线程的优先级并重新进行调度。

```c
/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void thread_tick(void)
{
  struct thread *t = thread_current();

  if (t != idle_thread)
    t->recent_cpu = IADD(t->recent_cpu, 1);

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;
  if (thread_mlfqs)
  {
    /* re-schedule every 4 ticks (a time slice) */
    if (timer_ticks() % 4 == 0) //当前时间片（4 ticks）用完
    {
      thread_foreach(&thread_update_priority_mlfqs, NULL);// 对全部线程进行更新优先级操作
      list_sort(&ready_list, &thread_cmp_priority, NULL);//维护等待队列，重新根据更新后的优先级进行排序
      intr_yield_on_return();// 中断返回时，直接将当前进程推到就绪队列中
    }
    /* Update load_avg and recent_cpu globally */
    if (timer_ticks() % TIMER_FREQ == 0)
    {
      // load_avg = (59/60)*load_avg + (1/60)*ready_threads */
      thread_update_load_avg();// 更新系统负载
      thread_foreach(&thread_update_recent_cpu_mlfqs, NULL);// 更新每个线程的recent_cpu值
    }
  }
  else
  {
    /* Enforce preemption. */
    if (++thread_ticks >= TIME_SLICE)
      intr_yield_on_return();// 中断返回时，直接将当前进程推到就绪队列中
  }
}
```

- 在创建新线程时候，如果开启了高级调度，则在创建线程时进行一次优先级更新

```c
void thread_init(void)
{
  ......
      
  /* M3: Update priority in mlfqs when initializing a new thread */
  if (thread_mlfqs)
    thread_update_priority_mlfqs(initial_thread);
}
```

- 在重新设定nice值的时候，对当前线程进行一次优先级更新

```c
void thread_set_nice(int nice)
{
  ASSERT(nice >= -20 && nice <= 20);
  ASSERT(thread_mlfqs);

  struct thread *t = thread_current();
  t->nice = nice;
  thread_update_priority_mlfqs(t);
  thread_yield();
}
```



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

**张智博：**在实验过程中我了解了pintos的基本代码架构和进行项目管理的基本要求。从现在看来，我们在项目的前期在沟通上存在一定的欠缺，导致走了一些弯路。另外我还发现我在Git的使用上还存在一些不足，多次误操作导致代码被覆盖。

### Student2

**王彦婷**：在实验过程中阅读代码的时候发现，在函数中，会反复验证条件是不是符合，并且保证原子操作。在实验过程中加深了对于操作系统知识的理解，尤其是关于进程忙等待，阻塞等方面的知识。通过阅读代码，更加规范了写代码的风格。

### Student3

**陈哲**：实验中需要和其他小组成员进行沟通，使我们对于实验的递进关系有更加清晰的认识，有一些代码也可以复用。在具体的代码实验过程中，重要的是做好分析工作，以免走弯路，浪费时间，同时良好的分析工作也能使之后debug也更加快速。

### Student4

**姜海洋**：在实验过程中，发现操作系统的内容十分繁琐且耦合性很高，锻炼了我在大量代码中搜寻所需函数的能力，而实现的过程中，维持优先级队列本不是什么很繁琐的需求，但因为一开始的实现方式过于繁琐导致绕了很多弯路。由此体会到在开始动手前大量思考规划出合理方案的必要性。

## 其他你认为有必要的内容 (Optional)

- 在进行代码测试时请连接笔记本电脑的电源，否则可能因为一些玄学问题出现有测试点不通过的现象。





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

主要就是 `thread` 和 `lock` 结构体中新增的成员变量，具体见下面的代码和注释。

```c++
// thread.h
struct thread
{
  ...

  /* Members for project1 mission2 priority donate */
  // 记录原来的优先级，没有的话优先级在捐赠后就没法恢复原来的优先级了
  int original_priority;
  // 当前将要持有的锁，用于优先级捐赠，对于递归捐赠来说显然是需要的
  struct lock *desired_lock;
  // 线程持有的锁，多捐赠时优先级要设为最大的那个
  struct list holding_locks;

  ...
};

// synch.h
struct lock 
{
  ...

  /* Members for project1 mission2 priority donate */
  // 锁记录的最大优先级，用于优先级捐赠
  int donated_priority;
 
  /* Shared between thread.c and synch.c. */
  // 这主要是为了锁能够添加到 thread->holding_locks 中
  struct list_elem elem;      /* List element. */

  ...
};
```

> B2: Explain the data structure used to track priority donation. Use ASCII art to diagram a nested donation. (Alternately, paste an image.)



### ALGORITHMS

> B3: How do you ensure that the highest priority thread waiting for a lock, semaphore, or condition variable wakes up first?

锁、信号量、条件变量在唤醒线程时，本质上都是调用了 `sema_up()`，然后只要将原来从线程等待队列的队首取线程改成使用 `list_max()` 函数找出最大优先级的线程进行唤醒就可以了，这样就可以确保最大优先级的线程首先被唤醒，具体可见上面重难点讲解中的分析过程。

> B4: Describe the sequence of events when a call to lock_acquire() causes a priority donation. How is nested donation handled?

具体可下面的代码和注释，更详细的分析可见上面的设计思路和重难点讲解。

```c++
void lock_acquire(struct lock *lock)
{
  struct thread *cur;
  struct lock *tmp_lock;

  // 确保锁不为空
  ASSERT(lock != NULL);
  // 确保不在中断上下文
  ASSERT(!intr_context());
  // 确保锁不被当前线程持有
  ASSERT(!lock_held_by_current_thread(lock));

  // 任务3不使用
  if (!thread_mlfqs)
  {
    // 获取当前线程
    cur = thread_current();
    // 设置线程当前将要持有的锁
    cur->desired_lock = lock;

    // 当前锁没有持有线程的话，那就不用捐赠了
    if (lock->holder != NULL)
    {
      tmp_lock = lock;

      // 递归捐赠，只要当前线程的优先级比锁记录的最大优先级大，就要捐赠，此处要维护好锁记录的被捐赠优先级
      // 比如 H->M->L，锁最大优先级要设为 H，同时提升 M 和 L 优先级
      while (tmp_lock != NULL && tmp_lock->donated_priority < cur->priority)
      {
        // 优先级捐赠
        tmp_lock->donated_priority = cur->priority;

        // 更新锁持有线程的优先级
        thread_update_priority(tmp_lock->holder);

        // 下一个锁，在 H->M->L 中，就是 M 申请 L 持有的锁
        tmp_lock = tmp_lock->holder->desired_lock;
      }
    }
  }

  // P 操作，锁被其它线程持有时，当前线程就会阻塞
  sema_down(&lock->semaphore);

  // P 操作过了，然后线程持有锁
  cur = thread_current();

  // 锁的持有者成为当前线程
  lock->holder = cur;

  // 任务3不使用
  if (!thread_mlfqs)
  {
    // 把锁加到线程持有锁列表里
    list_push_back(&cur->holding_locks, &lock->elem);

    // 终于持有了锁，于是把当前将要持有的锁设为空
    cur->desired_lock = NULL;
    
    // 更新当前线程优先级
    thread_update_priority(cur);
  }
}
```

> B5: Describe the sequence of events when lock_release() is called on a lock that a higher-priority thread is waiting for.

具体可下面的代码和注释，更详细的分析可见上面的设计思路和重难点讲解。

```c++
void lock_release(struct lock *lock)
{
  struct thread *cur;

  // 确保锁不为空
  ASSERT(lock != NULL);
  // 确保锁被当前线程持有
  ASSERT(lock_held_by_current_thread(lock));

  // 任务3不使用
  if (!thread_mlfqs)
  {
    // 获取当前线程
    cur = thread_current();

    // 将锁从持有锁列表中移除
    list_remove(&lock->elem);

    // 重置锁记录的被捐赠优先级
    lock->donated_priority = PRI_MIN;
    
    // 更新一下当前线程优先级
    thread_update_priority(cur);
  }

  // 锁的持有者重置为空
  lock->holder = NULL;
  // V操作
  sema_up(&lock->semaphore);
}
```

### SYNCHRONIZATION

> B6: Describe a potential race in thread_set_priority() and explain how your implementation avoids it. Can you use a lock to avoid this race?



### RATIONALE

> B7: Why did you choose this design? In what ways is it superior to another design you considered?

考虑了更改结构体加入顺序次序变量，但由于每次更改都需更新整个队列中元素的次序，因而过于繁琐效率过低。而链表本身的顺序性使得只需要在更改时维持好元素的前后顺序就可以维持整个链表的有序性。

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

如果CPU在计算recent_cpu、load_avg和priority上花费了太多的时间，那么它会占用线程被调度之前的时间，使这个线程运行时间延长，导致优先级降低和提高系统负载，影响当前线程的调度情况，降低性能。同时不在内核中进行中断处理，避免了降低响应速度影响性能的情况发生。

### RATIONALE

> C5: Briefly critique your design, pointing out advantages and disadvantages in your design choices. If you were to have extra time to work on this part of the project, how might you choose to refine or improve your design?

系统内置了基于时钟的中断处理函数以及使用链表方式实现的线程队列和相关的系统函数。基于这些的方式进行实现。优点是思路相对简洁，编写代码相对简单，缺点是调度方式不够精确，占用系统资源相对较多，且对于中断处理的相关细节了解不够深入。

在本次任务中，由于作为课程配套设计，我们以“通过全部测试点”为驱动进行编写，只要测试点通过，那么就假定我们的实现是正确且合理的。如果有更加充足的时间，我们会尝试更多的调度方式，比如修改内核，新建专门的调度进程、对于各个不同的优先级创建单独的就绪队列等方式，提高系统的效率和结构性。
