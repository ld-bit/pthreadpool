#include"pthreadpool.h"
#include<stdio.h>
#include<stdlib.h>
void* rutiun(void* p)
{
	int id=*(int*)p;
	printf("id: %d pthreadid: %p  切入\n",id+1,pthread_self());
	sleep(1);
	printf("id: %d pthreadid: %p 退出\n",id+1,pthread_self());
}
int main()
{
	pthread_pool_t pool;
	pthread_pool_init(&pool,5);
	int i;
	for(i=0;i<5;i++)
	{
		int *p=(int*)malloc(sizeof(int));
		*p=i;
		pthread_pool_add_task(&pool,rutiun,p);
	}
	pthread_pool_destroy(&pool);
}

