#include "allhead.h"

//struct tasklist *myhead; //主线程跟线程池里面的子线程共享的一个全局变量

//任务链表的初始化
struct tasklist *task_init()
{
    struct tasklist *head = malloc(sizeof(struct tasklist));
    head->next = NULL;
    return head;
}

void handler(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

//线程的任务函数--》线程池里面所有的线程共用同一个任务函数
void *routine(void *arg)
{
    //接收传递过来的参数pool
    struct threadpool *mypool = (struct threadpool *)arg;
    //准备辅助指针p
    struct tasklist *p;
    while (1)
    {
        pthread_cleanup_push(handler, (void *)&mypool->poolmutex);
        //上锁
        pthread_mutex_lock(&(mypool->poolmutex));
        while (mypool->threadflag == true && mypool->tasknum <= 0)
        {
            pthread_cond_wait(&(mypool->poolcond), &(mypool->poolmutex));
        }
        //写一个条件，让任务函数可以结束，退出死循环
        if (mypool->threadflag == false && mypool->tasknum <= 0)
        {
            //解锁--》防止死锁
            pthread_mutex_unlock(&(mypool->poolmutex));
            pthread_exit(NULL);
        }
        //所有的线程都是从头结点的下一个位置取出任务
        p = myhead->next;
        myhead->next = p->next;
        p->next = NULL;
        //任务数量更新
        mypool->tasknum--;
        //解锁
        pthread_mutex_unlock(&(mypool->poolmutex));
        pthread_cleanup_pop(0);
        //立马把p指向的节点中的函数指针，去执行
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        (p->taskp)(p->taskarg);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        free(p);
    }
}

//第一个函数：线程池的初始化
// num：表示你的线程池中有多少个线程
struct threadpool *pool_init(int num)
{
    int i;
    struct threadpool *pool = malloc(sizeof(struct threadpool));
    pool->threadnum = num;                        //线程数量
    pool->id = malloc(num * sizeof(pthread_t));   //线程的ID
    pthread_mutex_init(&(pool->poolmutex), NULL); //锁的初始化
    pthread_cond_init(&(pool->poolcond), NULL);   //条件变量的初始化
    pool->tasknum = 0;                            //统计任务链表中任务数量
    pool->threadflag = true;  //漏了这句           //表示线程池处于运行状态
    for (i = 0; i < num; i++)
        pthread_create((pool->id) + i, NULL, routine, pool);
    return pool;
}

//第二个函数：添加任务
int add_task(struct threadpool *pool, void *(*newtask)(void *), void *newarg)
{
    //准备新的节点
    struct tasklist *newnode = malloc(sizeof(struct tasklist));
    newnode->taskp = newtask;
    newnode->taskarg = newarg;
    newnode->next = NULL;

    //找到链表的尾部
    struct tasklist *p = myhead;
    while (p->next != NULL)
        p = p->next;

    //上锁
    pthread_mutex_lock(&(pool->poolmutex));
    //尾插新的任务到任务链表中
    p->next = newnode;
    //任务数量更新
    pool->tasknum++;
    //解锁
    pthread_mutex_unlock(&(pool->poolmutex));
    //唤醒条件变量
    pthread_cond_signal(&(pool->poolcond));
    return 0;
}

//第三个函数：线程池的销毁
int pool_destroy(struct threadpool *pool)
{
    int i;
    //改变标志位，让标志位的值为false
    pool->threadflag = false; //偷偷摸摸让标志位改变，让线程有机会可以退出
    //唤醒所有的线程
    pthread_cond_broadcast(&(pool->poolcond));
    //回收所有的线程
    for (i = 0; i < pool->threadnum; i++)
    {
        pthread_join(*(pool->id + i), NULL);
        //printf("目前回收到的线程是: %ld\n", *(pool->id + i));
    }
    return 0;
}