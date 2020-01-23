#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
int N,M,K;
pthread_mutex_t f_mutex;
int f = 0;
struct Riders{
	pthread_t riders_thread;
	pthread_mutex_t mutex;
	int rider_idx;
	int type_of_cab;
	int ride_time;
	int cab_number;
	int status;
	int max_wait_time;
	int arrived;
};
struct Drivers{
	pthread_t cabs_thread;
	pthread_mutex_t mutex;
	int driver_idx;
	int wait;
	int pool;
	int prime;
};
struct payment{
	pthread_t payment_thread;
	pthread_mutex_t mutex;
	int status;
	int server_idx;
	int rider_index;
};
struct Riders ** rider;
struct Drivers ** cab;
struct payment **pay;
void paymnt(struct Riders *a)
{	
	while(1)
	{
		int c = 0;
		for(int z = 0; z < K; z++)
		{
			pthread_mutex_lock(&(pay[z]->mutex));
			if(pay[z]->status == 0 && a->status == 1)
			{
				pay[z]->rider_index = a->rider_idx;
				pay[z]->status = 1;
				a->status = 3;
				printf("Rider %d will pay through server %d\n",pay[z]->rider_index,pay[z]->server_idx);
				pthread_mutex_unlock(&(pay[z]->mutex));
				c = 1;
				break;
			}
			else 
			{
				pthread_mutex_unlock(&(pay[z]->mutex));
			}

		}
		if(c ==  1)
			break;
	}

}
void *Cabs(void *args)
{
	sleep(rand()%2);
	struct Riders *rider2 = (struct Riders *)args;
	while(1)
	{
		if(rider2->type_of_cab == 1 && rider2->status == 0)
		{
			for(int q = 0; q < N;q++)
			{
				pthread_mutex_lock(&(cab[q]->mutex));
				if(cab[q]->wait == 1 && rider2->status == 0 && cab[q]->pool == 0)
				{
					rider2->status = 1;
					cab[q]->prime = 1;
					rider2->cab_number = cab[q]->driver_idx;
					cab[q]->wait = 0;
					pthread_mutex_unlock(&(cab[q]->mutex));
					printf("Cab %d has picked rider %d in a prime\n",cab[q]->driver_idx,rider2->rider_idx);
					fflush(stdout);
					sleep(rider2->ride_time);
					cab[q]->wait = 1;
					cab[q]->prime = 0;
					cab[q]->pool = 0;
					printf("Cab %d has dropped rider %d\n",cab[q]->driver_idx,rider2->rider_idx);
					fflush(stdout);
					paymnt(rider2);

				}
				else {
					pthread_mutex_unlock(&(cab[q]->mutex));

				}


			}
		}
		else if(rider2->type_of_cab == 2 && rider2->status == 0)
		{
			for( int q = 0 ; q < N;q ++)
			{
				pthread_mutex_lock(&(cab[q]->mutex));
				if(cab[q]->wait == 1 && rider2->status == 0 && cab[q]->pool == 1 && f > 0)
				{
					pthread_mutex_lock(&(f_mutex));
					cab[q]->pool++;
					if(cab[q]->pool == 2)
					{
						cab[q]->wait = 0;
						f--;
					}
					pthread_mutex_unlock(&(f_mutex));
					rider2->status = 1;
					rider2->cab_number = cab[q]->driver_idx;
					printf("Cab %d has picked rider %d in a pool\n",cab[q]->driver_idx,rider2->rider_idx);
					pthread_mutex_unlock(&(cab[q]->mutex));
					fflush(stdout);
					sleep(rider2->ride_time);
					cab[q]->pool--;
					pthread_mutex_lock(&(f_mutex));
					if(cab[q]->pool == 0)
					{	
						f--;
						cab[q]->wait = 1;
					}
					else {
						f++;
						cab[q]->wait = 1;
					     }
					pthread_mutex_unlock(&(f_mutex));
					printf("Cab %d has dropped rider %d\n",cab[q]->driver_idx,rider2->rider_idx);
					fflush(stdout);
					paymnt(rider2);

				}
				else if(cab[q]->wait == 1 && rider2->status == 0 && cab[q]->pool == 0 && f == 0)
				{
					pthread_mutex_lock(&(f_mutex));
					f++;
					pthread_mutex_unlock(&(f_mutex));
					cab[q]->pool++;
					rider2->status = 1;
					rider2->cab_number = cab[q]->driver_idx;
					pthread_mutex_unlock(&(cab[q]->mutex));
					printf("Cab %d has picked rider %d in a pool\n",cab[q]->driver_idx,rider2->rider_idx);
					fflush(stdout);
					sleep(rider2->ride_time);
					cab[q]->pool--;
					pthread_mutex_lock(&(f_mutex));
					if(cab[q]->pool == 0)
					{
						f--;    
						cab[q]->wait = 1;
					}
					else {
						f++;
						cab[q]->wait = 1;
					     }
					pthread_mutex_unlock(&(f_mutex));
					printf("Cab %d has dropped rider %d\n",cab[q]->driver_idx,rider2->rider_idx);
					fflush(stdout);
					paymnt(rider2);
				}

				else{
					pthread_mutex_unlock(&(cab[q]->mutex));
				}
			}	
		}

	}
}
void *Rider(void *args)
{
	sleep(rand()%5);
	struct Riders *rider1 = (struct Riders *)args;
	int cab_type = 1 + rand()%2;
	rider1->type_of_cab = cab_type;
	rider1->ride_time = 1 + rand()%20;
	rider1->max_wait_time = 20 + rand()%10;
	rider1->arrived = 1;
	if(rider1->type_of_cab  == 1)
	{
		printf("Rider %d needs a prime ride of duration %d\n",rider1->rider_idx,rider1->ride_time);
		fflush(stdout);
	}
	else 
	{
		printf("Rider %d needs a pool ride of duration %d\n",rider1->rider_idx,rider1->ride_time);
		fflush(stdout);
	}
	sleep(rider1->max_wait_time);
	if(rider1->cab_number == 0)
	{
		printf("Rider %d did not get a cab due to time out\n",rider1->rider_idx);
		fflush(stdout);
		rider1->status = 2;
	}
}
void *Payment(void *args)
{
	struct payment * pay1 = (struct payment *)args ;
	while(1)
	{

		if(pay1->status == 1)
		{
			sleep(2);
			printf("Rider %d paid\n",pay1->rider_index);		
			rider[pay1->rider_index - 1]->status = 2;
			pay1->status = 0;
			pay1->rider_index = 0;
		}
		int c = 0;
		for(int x = 0;x < M;x++)
		{
			if(rider[x]->status == 2)
			{
				c++;
			}
		}	
		if(c == M)
		{
			break;
		}
	}
}

int main()
{
	srand(time(0));
	int i;
	scanf("%d %d %d",&N,&M,&K);
	cab = (struct Drivers **)malloc(N*sizeof(struct Drivers *));
	for(i = 0;i < N;i++)
	{
		cab[i] = (struct Drivers *)malloc(1*sizeof(struct Drivers));
		cab[i]->driver_idx = i + 1;
		cab[i]->wait = 1;
		cab[i]->pool = 0;
		cab[i]->prime = 0;
		pthread_mutex_init(&(cab[i]->mutex), NULL);
	}
	rider = (struct Riders **)malloc(M*sizeof(struct Riders *));
	for(i = 0;i < M;i++)
	{
		rider[i] = (struct Riders *)malloc(1*sizeof(struct Riders));
		rider[i]->rider_idx = i + 1;
		rider[i]->type_of_cab = 0;
		rider[i]->ride_time = 0;
		rider[i]->cab_number = 0;
		rider[i]->status = 0;
		rider[i]->max_wait_time = 0;
		rider[i]->arrived = 0;
		pthread_mutex_init(&(rider[i]->mutex), NULL);
	}
	pay = (struct payment**)malloc(K *sizeof(struct payment *));
	for(i = 0;i < K; i++)
	{
		pay[i] = (struct payment*)malloc(1*sizeof(struct payment));
		pthread_mutex_init(&(pay[i]->mutex), NULL);
		pay[i]->status = 0;
		pay[i]->server_idx = i + 1;	
		pay[i]->rider_index = 0;
	}
	for(i = 0;i < M; i++)
	{
		pthread_create(&(rider[i]->riders_thread), NULL, Rider, rider[i]);
	}
	for(i = 0;i < M; i++)
	{
		pthread_create(&(rider[i]->riders_thread), NULL,Cabs, rider[i]);
	}
	for(i = 0;i < K; i++)
	{
		pthread_create(&(pay[i]->payment_thread), NULL, Payment, pay[i]);
	}
	for(i = 0;i < K; i++)
	{
		pthread_join(pay[i]->payment_thread, 0);
	}
	printf("Simulation Over\n");
}
