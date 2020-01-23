#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

long long int * shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (long long int*)shmat(shm_id, NULL, 0);
}
struct arg{
    long long int l;
    long long int r;
    long long int* arr;
};
void insertionsort(long long int l,long long int r,long long int *arr)
{
	long long int i,t,j;
	for( i = l; i <= r;i++)
	{
		t = arr[i];
		j = i - 1;
		while(j >= 0 && arr[j] > t)
		{
			arr[j+1] = arr[j];
			j--;
		}
		arr[j+1] = t;
	}

}
long long int partition(long long int *arr,long long int l,long long int r)
{
	long long int pi = arr[r];
	long long int i = l - 1 , temp,j = l;
	while(j < r)
	{
		if(arr[j] < pi )
		{
			i++;
			temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	j++;
	}
	temp = arr[r];
	arr[r] = arr[i + 1];
	arr[i + 1] = temp;
	return i + 1;
}
void normal_quicksort(long long int l,long long int r,long long int *arr)
{
	if(l > r)
	return;
	if(r - l + 1 <= 5 )
	{
	insertionsort(l,r,arr);
	return ;
	}
	else {
		long long int pivot = partition(arr, l, r);
		normal_quicksort(l,pivot - 1,arr);
		normal_quicksort(pivot + 1, r,arr);
	      }
}
void quicksort(long long int *arr, long long int l, long long int r){
    if(l>r) _exit(1);

    else if(r-l+1<=5){
        	insertionsort(l,r,arr);
	    return;
    }

    int pid1 = fork();
    int pid2;
   long long int pi = partition(arr,l , r);
    if(pid1==0){
        quicksort(arr, l, pi - 1);
        _exit(1);
    }
    else
    {
        pid2 = fork();
        if(pid2==0){
            //sort right half array
            quicksort(arr,pi + 1,r);
            _exit(1);
        }
        else{
            //wait for the right and the left half to get sorted
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
    }
    return;
}

void *threaded_quicksort(void* a){
    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg*) a;

    long long int l = args->l;
    long long int r = args->r;
    long long int *arr = args->arr;
    if(l>r) return NULL;

    //insertion sort
    if(r-l+1<=5){
        	insertionsort(l,r,args->arr);
	  return NULL;
    }

    //sort left half array
   long long int pi = partition(arr,l,r);
    struct arg a1;
    a1.l = l;
    a1.r = pi -  1;
    a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_quicksort, &a1);

    //sort right half array
    struct arg a2;
    a2.l = pi + 1;
    a2.r = r;
    a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_quicksort, &a2);
    //wait for the two halves to get sorted
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

}
void runsort(long long int N,long long int *arr)
{
    long long int brr[N+1];
    for(long long int i=0;i<N;i++) 
    {
	   brr[i] = arr[i];
    }
  
	    struct timespec ts;

    printf("Running concurrent_quicksort for N = %lld\n", N);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;
    quicksort(arr, 0 ,N - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;
    for(int x = 0; x < N; x++)
    printf("%lld\n",arr[x]);
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = N-1;
    a.arr = brr;
    printf("Running threaded_concurrent_quicksort for N = %lld\n", N);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    //multithreaded quicksort
    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL);
    for(int x = 0; x < N; x++)
    printf("%lld\n",a.arr[x]);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;

    printf("Running normal_quicksort for N = %lld\n", N);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    // normal quicksort
    normal_quicksort(0, N - 1, brr);
    for(int x = 0; x < N; x++)
    printf("%lld\n",brr[x]);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;

    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n\n", t1/t3, t2/t3);
    shmdt(arr);
    return;

}
int main()
{
	long long int N,i;
	scanf("%lld" , &N);
	long long int *arr = shareMem(sizeof(long long int)*(N+1));

	for(i = 0; i < N; i++)
	{
		scanf("%lld",&arr[i]);
	}
	runsort(N,arr);
}
