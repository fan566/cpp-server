#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <pthread.h>
#include <signal.h>

struct Task_t
{
    /* 回调函数 handleclientinfo*/ 
    void *(*function)(void *arg);
    /* 回调函数的参数 */
    void *arg;// 在这里是connid
};

class ThreadPool
{
public:
    /* 构造函数 */
    ThreadPool(int minThreads, int maxThreads, int maxQueueCapacity);
    /* 析构函数 */
    ~ThreadPool();

public:
    /* 添加任务, 给客户使用 */
    void addTask(void *(*function)(void *arg), void *arg);

public:
    /* 队列的容量, 响应客户端的数量 */
    int m_queueCapacity;
    /* 队列的任务个数 */
    int m_queueSize;
    /* 队列 */
    Task_t *m_queue;
    /* 队头 */
    int m_queueFront;
    /* 队尾 */
    int m_queueTail;

    /* 工作的线程 */
    pthread_t *m_threads;
    /* 管理者线程 */
    pthread_t manageThread;
    /* 忙碌的线程数 */
    int m_busyThreadNums;
    /* 最大的线程数 */
    int m_maxThreadNums;
    /* 最小的线程数 */
    int m_minThreadNums;
    /* 存活的线程数 */
    int m_aliveThreadNums;

    /* 要减少的线程数 */
    int m_exitNum;

    /* 退出标志位 */


public:
    /* 锁 */
    pthread_mutex_t m_mutex;
    /* 锁2 - 提升效率 — 减少竞争 */
    pthread_mutex_t m_busyMutex;
    /* 条件变量 */
    pthread_cond_t m_queueNotEmpty;
    pthread_cond_t m_queueNotFull;
};

#endif // THREAD_POOL_H