#pragma once
#include<pthread.h>
struct task{
  void*(*run)(void* arg); //任务执行回调函数
  void* arg; //回调函数的参数
  struct task* next; //指向下一个任务节点的指针
};
typedef  struct task task_t;
struct thread_pool
{
	pthread_mutex_t mutex; //线程锁
	pthread_cond_t  cond;  
	task_t* pre; //任务队列的首地址
	task_t* last; //任务队列的的尾地址
	int counter; //当前的任务数量
	int idle; //当前空闲线程的个数
	int max_thread; //当前可创建线程的最大个数
	int quit;   //线程销毁标志
};
typedef struct thread_pool pthread_pool_t;
//线程池初始化
void pthread_pool_init(pthread_pool_t* pool,int max_thread);
//线程池添加新线程
void pthread_pool_add_task(pthread_pool_t* pool,void*(*run)(void*),void* arg);
//线程池销毁
void pthread_pool_destroy(pthread_pool_t* pool);


