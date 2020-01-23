#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
struct robot
{
	pthread_t robot_thread;
	pthread_mutex_t mutex;	
	int robot_idx;
	int num_vessel;
	int num_student;
	int status;
};
struct tables 
{
	pthread_t table_thread;
	pthread_mutex_t mutex;
	int table_idx;
	int total_no_of_servings;
	int no_people_added;
	int no_of_slots;
	int status;
	int refill;
};
struct stdnt 
{
	pthread_t stdnt_thread;
	int stdnt_idx;
	int service_table;
	int slot_taken; 
	int eaten;
};
int N, M, K, cnt;
struct robot ** chef;
struct tables ** serving;
struct stdnt ** student;
int min(int a , int b)
{
	if(a < b && a != 0)
		return a;
	if(a < b && a == 0)
		return b;
	if(b < a && b != 0)
		return b;
	else return a;
}
void *Biryani_Preparation(void *args)
{
	struct robot *chef1 = (struct robot *)args;
	while(1)
	{
		if(chef1->status == 0)
		{
		int w = 2 + rand()%4;
		sleep(w);
		int r = 1 + rand()%10;
		int p = 25 + rand()%26;
		chef1->num_vessel = r;
		chef1->num_student = p;
		printf("Number of vessels prepared by chef %d is %d\nEach vessel serves %d students\n",chef1->robot_idx,r,p);
		fflush(stdout);
		chef1->status = 1;
		}
	}
}
void *Serve_Table(void *args)
{
	struct tables *serving1 = (struct tables *)args;
	while(1)
	{	

		if(cnt == K)
			break;

		int j = 0;
		for( j; j<M; j++)
		{
			pthread_mutex_lock(&(chef[j]->mutex));
			if(chef[j]->status == 1 && serving1->status == 0)
			{
				sleep(2);
				serving1->total_no_of_servings = chef[j]->num_student;
				chef[j]->num_vessel --;
				serving1->no_of_slots = min(1 + rand()%10,serving1->total_no_of_servings);
				serving1->status = 1;
				if(serving1->refill == 0)
				{printf("Serving Table %d has taken a vessel from chef %d and is ready to serve %d students in %d slots\n",serving1->table_idx,chef[j]->robot_idx,serving1->total_no_of_servings,serving1->no_of_slots);
					fflush(stdout);
				}
				if(serving1->refill == 1)
				{
					printf("Serving Table %d has refilled a vessel from chef %d and is ready to serve %d students in %d slots\n",serving1->table_idx,chef[j]->robot_idx,serving1->total_no_of_servings,serving1->no_of_slots);
					fflush(stdout);
				}

				if(chef[j]->status == 1 && chef[j]->num_vessel == 0)
				{
					printf("All vessels prepared by Robot Chef %d are emptied. Resuming cooking now\n",chef[j]->robot_idx);
					chef[j]->status = 0;
					chef[j]->num_vessel = 0;
					chef[j]->num_student = 0;
				}
			}
			pthread_mutex_unlock(&(chef[j]->mutex));
		}
	}

}
void *Wait_For_Slot(void *args)
{
	sleep(rand()%10);
	struct stdnt* student1 = (struct stdnt *)args;
	printf("Student %d has arrived\n",student1->stdnt_idx);
	fflush(stdout);
	int c = 0;
	while(1)
	{
		int k = 0;
		for(k; k < N;k++)
		{
			pthread_mutex_lock(&(serving[k]->mutex));
			if(serving[k]->status == 1 && serving[k]->no_of_slots != 0 && student1->slot_taken == 0)
			{
				sleep(2);
				student1->slot_taken = 1; 		
				serving[k]->no_people_added++;
				student1->slot_taken = 1;		
				printf("Student %d took a seat in table %d\n",student1->stdnt_idx,serving[k]->table_idx);
				cnt++;
				fflush(stdout);
				student1->service_table = serving[k]->table_idx;
			}
			if(serving[k]->no_people_added == serving[k] -> no_of_slots)
			{
				int p = 0,q = 0;
				for(p;p<K;p++)
				{
					if(student[p]->service_table == k + 1 && student[p]->eaten != 1)
					{
						student[p]->eaten = 1;
						printf("Student %d has eaten from table %d\n",student[p]->stdnt_idx,k+1);
						fflush(stdout);
						serving[k]->total_no_of_servings =- serving[k]->no_people_added;
						if(serving[k]->total_no_of_servings == 0 && q == 0)
						{
							q++;
							printf("Serving Container of %d is empty and has to be refilled\n",serving[k]->table_idx);	
							serving[k]->status = 0;
							serving[k]->refill = 1;
						}
						serving[k]->no_people_added = 0;
						serving[k]->no_of_slots = 0;
					}
				}
			}
			  else if ( cnt == K)
			  {
				  int p = 0, q = 0;
				  for(p;p<K;p++)
				  {
					  if(student[p]->eaten == 0)
					  {
						  printf("Student %d has eaten from table %d\n",student[p]->stdnt_idx,student[p]->service_table);
						  fflush(stdout);
						  student[p]->eaten = 1;
					  }
				  }
			  }
			  pthread_mutex_unlock(&(serving[k]->mutex));
		}
		if(student1->eaten ==  1)
			break;
	}
}
int main()
{
	srand(time(0));
	int i;
	scanf("%d %d %d",&M,&N,&K);

	chef = (struct robot **)malloc(M*sizeof(struct robot *));

	for(i = 0;i < M;i++)
	{

		chef[i] = (struct robot *)malloc(sizeof(struct robot));
		chef[i]->status = 0;
		chef[i]->num_vessel = 0;
		chef[i]->num_student = 0;
		chef[i]->robot_idx = i + 1;
		pthread_mutex_init(&(chef[i]->mutex), NULL);
	}
	serving = (struct tables **)malloc(N*sizeof(struct tables *));
	for(i = 0;i < N;i++)
	{
		serving[i] = (struct tables *)malloc(sizeof(struct tables));
		serving[i]->status = 0;
		serving[i]->total_no_of_servings = 0;
		serving[i]->table_idx = i + 1;
		serving[i]->no_people_added = 0 ; 
		serving[i]->refill = 0;
		pthread_mutex_init(&(serving[i]->mutex), NULL);
	}
	student = (struct stdnt **)malloc(K*sizeof(struct stdnt *));
	for(i = 0;i < K;i++)
	{
		student[i] = (struct stdnt *)malloc(sizeof(struct stdnt));
		student[i]->eaten = 0;		
		student[i]->slot_taken = 0;
		student[i]->stdnt_idx = i + 1;
		student[i]->service_table = 0;
	}
	for(i = 0;i < M; i++)
	{
		pthread_create(&(chef[i]->robot_thread), NULL,Biryani_Preparation, chef[i]);
	}
	for(i = 0;i < N; i++)
	{
		pthread_create(&(serving[i]->table_thread), NULL, Serve_Table, serving[i]);
	}
	for(i = 0;i < K; i++)
	{
		pthread_create(&(student[i]->stdnt_thread), NULL, Wait_For_Slot, student[i]);
	}
	for(i = 0;i < K; i++)
	{
		pthread_join(student[i]->stdnt_thread, 0);
	}
	printf("Simulation Over\n");
	fflush(stdout);
}
