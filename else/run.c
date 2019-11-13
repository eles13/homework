#include <assert.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "payload.h"

static int count_zeroes(const unsigned char *hash, const int len)
{
	int cnt = 0;
	for (int i = 0; i < len; i++) {
		cnt += hash[i] == 0;
	}
	return cnt;
}

static int run_omp(const char *str, const int target_count, const int n_threads)
{
	omp_set_num_threads(n_threads);
	int ret = 0;
	#pragma omp parallel reduction(+ : ret)
	{
		struct payload_t *p = payload_gen(str);
		assert(p);

		unsigned char hash[32];
		#pragma omp for schedule(dynamic)
		for (long i = 0; i < MAX_MAGIC; i++) {
			payload_set_magic(p, i);
			payload_checksum(p, hash);
			int cnt = count_zeroes(hash, sizeof(hash));
			ret += cnt == target_count;
		}
		payload_free(p);
	}
	return ret;
}

struct list {
  long value;
  struct list *next;
};

struct queue {
  struct list *head, *tail;
};

struct queue* qinit(){
	struct queue* q = calloc(1,sizeof(struct queue));
	assert(q);
  q->head = NULL;
  q->tail = NULL;
	return q;
}

int isempty(struct queue *q) {
  if(q->tail == NULL)
    return(1);
  else
    return(0);
}

void insert(struct queue *q, long x) {
	struct list * new = calloc(1,sizeof(struct list));
	assert(new);
	new->value = x;
  if((q->head==NULL) && (q->tail==NULL)) {
    new->next = NULL;
		q->tail = new;
    q->head = q->tail;
  }
	else
	{
		q->tail->next = new;
		q->tail = q->tail->next;
	}
}

long pop(struct queue *q) {
  struct list *temp;
  long x;
  if(isempty(q)==1)
    return(-1);
  x = q->head->value;
  temp = q->head;
  q->head = q->head->next;
  free(temp);
  return x;
}

struct tpool{
	pthread_t* thrs;
	struct queue* thq;
	pthread_mutex_t qmutex;
	pthread_mutex_t smutex;
	int ret;
	int shutdown;
	const char* str;
	int target_count;
};

void* Handler(void* pl)
{
	struct tpool* pool = (struct tpool*)pl;
	long curarg;
	struct payload_t *p = payload_gen(pool->str);
	assert(p);
	unsigned char hash[32];
	while(1)
	{
		if(pool->shutdown)
		{
			payload_free(p);
			pthread_exit(0);
		}
		pthread_mutex_lock(&(pool->qmutex));
		if(!isempty(pool->thq))
		{
			curarg = pop(pool->thq);
			if (curarg == -1)
			{
				pool->shutdown = 1;
				payload_free(p);
				pthread_exit(0);
			}
			pthread_mutex_unlock(&(pool->qmutex));
		}
		else
		{
			pthread_mutex_unlock(&(pool->qmutex));
			continue;
		}
		//if(curarg % 10000 == 0)
		//	printf("round %ld\n", curarg);
		payload_set_magic(p, curarg);
		payload_checksum(p, hash);
		int cnt = count_zeroes(hash, sizeof(hash));
		pthread_mutex_lock(&(pool->smutex));
		pool->ret+= cnt == pool->target_count;
		pthread_mutex_unlock(&(pool->smutex));
	}
}

static int run_pthreads(const char *str, const int target_count, const int n_threads)
{
	int ret = 0;
	struct tpool* pool = calloc(1, sizeof(struct tpool));
	assert(pool);
	pool->thrs = calloc(n_threads, sizeof(pthread_t));
	assert(pool->thrs);
	pthread_mutex_init(&(pool->qmutex),NULL);
	pthread_mutex_init(&(pool->smutex), NULL);
	pool->thq = qinit();
	pool->shutdown = 0;
	pool->str = str;
	pool->target_count = target_count;
	for(int i = 0; i < n_threads; i++)
	{
		if(pthread_create(&(pool->thrs[i]),NULL, Handler, pool))
			return -1;
	}
	pool->ret = 0;
	/////////////
	for (long i = 0; i < MAX_MAGIC; i++) {
		insert(pool->thq, i);
	}
	insert(pool->thq, -1);
	for(int i = 0; i < n_threads; i++)
	{
		int rc = pthread_join(pool->thrs[i], NULL);
		assert(rc == 0);
	}
	pthread_mutex_destroy(&pool->qmutex);
	pthread_mutex_destroy(&pool->smutex);
	ret = pool->ret;
	free(pool->thrs);
	return ret;
}

typedef int (*cb)(const char *str, const int target_count, const int n_threads);

struct result_t {
	double elapsed;
	int cnt;
};

static struct result_t timer(const cb f, const char *str, const int target_count, const int n_threads)
{
	struct result_t res;
	double start = omp_get_wtime();
	res.cnt = f(str, target_count, n_threads);
	res.elapsed = omp_get_wtime() - start;
	return res;
}

static int check(const char *str, const int target_count, const int n_threads) {
	struct result_t r1 = timer(run_omp, str, target_count, n_threads);
	printf("OpenMP: cnt = %d, elapsed = %lfs\n", r1.cnt, r1.elapsed);
	struct result_t r2 = timer(run_pthreads, str, target_count, n_threads);
	printf("pthreads: cnt = %d, elapsed = %lfs\n", r2.cnt, r2.elapsed);
	if (r1.cnt != r2.cnt){
		printf("Unexpected count: got %d, want %d\n", r2.cnt, r1.cnt);
		return -1;
	}
 	if (0.9 * r2.elapsed > r1.elapsed){
	 	printf("pthreads version is %lf times slower than OpenMP one\n", r2.elapsed / r1.elapsed);
	 	return -2;
	 }
	return 0;
}

int main(int argc, char **argv)
{
	char *data = argv[1];
	int target_count = atoi(argv[2]);
	int n_threads = atoi(argv[3]);
	return check(data, target_count, n_threads);
}
