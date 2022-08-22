#ifndef _ALLHEAD_H
#define _ALLHEAD_H

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dirent.h>

#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include <time.h>

//定义结构体表示任务链表
struct tasklist
{
	void *(*taskp)(void *); //指向你要存放的任务
	void *taskarg;			//传递给函数值的参数
	struct tasklist *next;
};

//定义结构体来表示线程池
struct threadpool
{
	int threadnum;
	pthread_t *id;
	pthread_mutex_t poolmutex;
	pthread_cond_t poolcond;
	int tasknum;
	bool threadflag;
};

struct twopath
{
	char now_path[512];
	char target_path[512];
};

extern struct tasklist *myhead;
extern int time_all;
extern int allfile_num;
extern int ready_num;
extern int allfile_size_MB;
extern int ready_size_MB;

extern struct tasklist *task_init();
extern void handler(void *arg);
extern void *routine(void *arg);
extern struct threadpool *pool_init(int num);
extern int add_task(struct threadpool *pool, void *(*newtask)(void *), void *newarg);
extern int pool_destroy(struct threadpool *pool);
extern void *thread_task(void *arg);
extern int copy_file(char *path, char *target_path);
extern int mkdir_act(struct threadpool *mythreadpool, char *path, char *target, char *type);
extern int mkdir_tree(char *path);
extern int count_file(char *path);
extern int count_allfile(char *path, char *type);
extern int tree_directory(char *destpath,int k);
extern int count_num(char *path,int *filenum,int *docnum);


#endif
