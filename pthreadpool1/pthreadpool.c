#include"pthreadpool.h"
#include<pthread.h>
#include<time.h>
#include<stdlib.h>
//struct task{
//  void*(*run)(void* arg); //任务执行回调函数
//  void* arg; //回调函数的参数
//  struct task* next; //指向下一个任务节点的指针
//};
//typedef  struct task task_t;
//struct thread_pool
//{
//	pthread_mutex_t mutex; //线程锁
//	pthread_cond_t  cond;  
//	task_t* pre; //任务队列的首地址
//	task_t* last; //任务队列的的尾地址
//	int counter; //当前的任务数量
//	int idle; //当前空闲线程的个数
//	int max_thread; //当前可创建线程的最大个数
//	int quit;   //线程销毁标志
//};
//typedef struct thread_pool pthread_pool_t;
//线程池初始化
void pthread_pool_init(pthread_pool_t* pool,int max)
{
  pthread_mutex_init(&pool->mutex,NULL);
  pthread_cond_init(&pool->cond,NULL);
  pool->counter=0;
  pool->idle=0;
  pool->max_thread= max;
  pool->quit=0; 
  pool->pre=pool->last=NULL;
}
task_t* create_task(void*(*run)(void*),void* arg)
{
   task_t* node=(task_t*)malloc(sizeof(task_t));
   node->run=run;
   node->arg=arg;
}
//线程池添加新线程
void* routine(void* arg)
{
  pthread_pool_t* pool=(pthread_pool_t*) arg;
  printf("%p pthread create\n",pthread_self());
  while(1)
  {
	  int ways=0;
	  pthread_mutex_lock(&pool->mutex);
	  pool->idle++;
	  while(pool->pre==NULL&&pool->quit==0)
	  {
		struct timespec abtime;
        clock_gettime(CLOCK_REALTIME,&abtime);
		abtime.tv_sec+=5;
        int ret=pthread_cond_timedwait(&pool->cond,&pool->mutex,&abtime);
		if(ret==110)
		{
            ways=1;
			break;
		}
	  }
	  pool->idle--;
	  if(pool->pre!=NULL)
	  {
		  task_t* tmp=pool->pre;
		  pool->pre=tmp->next;
		  pthread_mutex_unlock(&pool->mutex);
		  tmp->run(tmp->arg);
		  free(tmp);
		  pthread_mutex_lock(&pool->mutex);
	  }
	  if(pool->pre==NULL&&ways==1)
	  {
		  pool->counter--;
		  printf("becuse the time , %p pthread out!!\n",pthread_self());
		  pthread_mutex_unlock(&pool->mutex);
		  break;
	  }
	  if(pool->pre==NULL&&pool->quit==1)
	  {
         pool->counter--;
		 if(pool->counter==0)
		 {
			 pthread_cond_signal(&pool->cond);
		 }
		 pthread_mutex_unlock(&pool->mutex);
		 printf("%p pthread was killed\n",pthread_self());
		 break;
	  }
	  pthread_mutex_unlock(&pool->mutex);
  }
}
void pthread_pool_add_task(pthread_pool_t* pool,void*(*run)(void*),void* arg)
{
   task_t* node=create_task(run,arg);

   //往任务队列里边扔任务
   pthread_mutex_lock(&pool->mutex);
   if(pool->pre==NULL) 
   {
	   pool->pre=node;
   }
   else 
   {
     pool->last->next=node;	
   }
   pool->last=node;
   //如果有空闲的线程,直接唤醒空闲线程执行当前的任务
   if(pool->idle>0)
   {
	   pthread_cond_signal(&pool->cond);
   }
   else if(pool->idle==0&&pool->counter<pool->max_thread)
   {
	   //如果没有空闲的线程,而且当前的线程的总数未大上线,那么创建新线程
	   pthread_t tid;
	   pthread_create(&tid,NULL,routine,pool);
	   pool->counter++;
   }
   pthread_mutex_unlock(&pool->mutex);
}
//线程池销毁
void pthread_pool_destroy(pthread_pool_t* pool)
{
	printf("I will kill pthread_pool\n");
	if(pool->quit==1)
	{
		return ;
	}
	pthread_mutex_lock(&pool->mutex);
	pool->quit=1;
	if(pool->counter>0)
	{
		if(pool->idle>0)
		pthread_cond_broadcast(&pool->cond);
	    while(pool->counter>0)
		{
          pthread_cond_wait(&pool->cond,&pool->mutex);
		}
	}
	pthread_mutex_unlock(&pool->mutex);
	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
	printf("kill pthread_pool already\n");
}
