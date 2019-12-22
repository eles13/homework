#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <pthread.h>
using namespace std;
pthread_mutex_t gmutex;
pthread_cond_t jdone;
pthread_barrier_t barrier;
struct list {
  int value;
  struct list *next;
};

struct queue {
  struct list *head, *tail;
};

struct queue* qinit(){
	struct queue* q = (struct queue*)calloc(1,sizeof(struct queue));
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
	struct list * nw = (struct list*)calloc(1,sizeof(struct list));
	assert(nw);
	nw->value = x;
  nw->next = NULL;
  if((q->head==NULL) && (q->tail==NULL)) {
    nw->next = NULL;
		q->tail = nw;
    q->head = q->tail;
  }
	else
	{
		q->tail->next = nw;
		q->tail = q->tail->next;
	}
}

int pop(struct queue *q) {
  struct list *temp;
  int x;
  if (q->tail == NULL)
  {
    return -2;
  }
  x = q->head->value;
  temp = q->head;
  if (q->head == q->tail)
  {
    q->head = NULL;
    q->tail = NULL;
  }
  else
    q->head = q->head->next;
  free(temp);
  return x;
}

struct tpool{
	pthread_t* thrs;
	struct queue* thq;
	pthread_mutex_t qmutex;
	pthread_mutex_t smutex;
	int target_count_second;
	int shutdown;
	int mode;
	int target_count_first;
  int counter;
  int i;
  int j;
  double** a;
  double** q;
  double r;
};

void* Handler(void* pl)
{
	struct tpool* pool = (struct tpool*)pl;
	int k = 0, i = 0, j = 0;
  double aii,aik,ajk,aji,qki,qkj,v,w,r;
	while(1)
	{
		if(pool->shutdown)
		{
			pthread_exit(0);
		}
		pthread_mutex_lock(&(pool->qmutex));
		if(!isempty(pool->thq))
		{
			k = pop(pool->thq);
      pthread_mutex_unlock(&(pool->qmutex));
      pthread_mutex_lock(&(pool->smutex));
      i = pool->i;
      j = pool->j;
      r = pool->r;
      pthread_mutex_unlock(&(pool->smutex));
      if(pool->mode == 1)
      {
        aji = (pool->a)[j][i];
        aii = (pool->a)[i][i];
        aik = (pool->a)[i][k];
        ajk = (pool->a)[j][k];
      }
      else
      {
        aji = (pool->a)[j][i];
        aii = (pool->a)[i][i];
        qki = (pool->q)[k][i];
        qkj = (pool->q)[k][j];
      }
		}
		else
		{
			pthread_mutex_unlock(&(pool->qmutex));
			continue;
		}
    if (pool->mode == 1)
    {
      w = (aik*aii + ajk*aji) / r;
      v = (-aik*aji + ajk*aii) / r;
      //cout<<w<<' '<<v<<' '<<(pthread_self() % 100)<<endl;
      (pool->a)[i][k] = w;
      (pool->a)[j][k] = v;
      pthread_mutex_lock(&(pool->smutex));
      pool->counter++;
      if(pool->counter == pool->target_count_first)
      {
        pool->mode = 2;
        pool->counter = 0;
        pthread_barrier_wait(&barrier);
        //pthread_cond_signal(&jdone);
      }
    }
    else
    {
      w = (qki*aii + qkj*aji) / r;
      v = (-qki*aji + qkj*aii) / r;
      //cout<<w<<' '<<v<<' '<<(pthread_self() % 100)<<endl;
      (pool->q)[k][i] = w;
      (pool->q)[k][j] = v;
      pthread_mutex_lock(&(pool->smutex));
      pool->counter++;
      if(pool->counter == pool->target_count_second)
      {
        pool->mode = 1;
        pool->counter = 0;
        pthread_barrier_wait(&barrier);
        //pthread_cond_signal(&jdone);
      }
    }
    pthread_mutex_unlock(&(pool->smutex));
	}
}

double** QRdec(double** a, int n, const double EPS, const size_t n_threads)
{
   double** q = (double**)calloc(n,sizeof(double*));
   int i, j, k;
   for (i = 0; i < n; i++)
   {
       q[i] =(double*) calloc(n,sizeof(double));
       q[i][i] = 1;
    }
    double r2, r;
  	struct tpool* pool = (struct tpool*)calloc(1, sizeof(struct tpool));
  	assert(pool);
  	pool->thrs = (pthread_t*)calloc(n_threads, sizeof(pthread_t));
  	assert(pool->thrs);
  	pthread_mutex_init(&(pool->qmutex),NULL);
  	pthread_mutex_init(&(pool->smutex), NULL);
    pthread_mutex_init(&(gmutex), NULL);
    pthread_barrier_init(&barrier,NULL, 2);
  	pool->thq = qinit();
  	pool->shutdown = 0;
  	pool->mode = 1;
  	pool->target_count_first = -1;
    pool->counter = 0;
    pool->a = a;
    pool->q = q;
  	for(i = 0; i < n_threads; i++)
  	{
  		if(pthread_create(&(pool->thrs[i]),NULL, Handler, pool))
  			return NULL;
  	}
  	pool->target_count_second = n;
    for (i = 0; i < n - 1; i++)
    {
        r2 = a[i][i] * a[i][i];
        for (j = i+1; j < n; j++)
        {
            r2+= ((a[j])[i]*(a[j])[i]);
            if (abs(r2) < EPS) continue;
            r = sqrt(r2);
            pthread_mutex_lock(&(pool->smutex));
            pool->r = r;
            pool->target_count_first = n - i - 1;
            pool->i = i;
            pool->j = j;
            pthread_mutex_unlock(&(pool->smutex));
            for (int k = i+1; k < n; k++) {
                pthread_mutex_lock(&(pool->qmutex));
		            insert(pool->thq, k);
                pthread_mutex_unlock(&(pool->qmutex));
	          }
            //pthread_cond_wait(&jdone, &gmutex);
            pthread_barrier_wait(&barrier);
            for (int k = 0; k < n; k++) {
              pthread_mutex_lock(&(pool->qmutex));
              insert(pool->thq, k);
              pthread_mutex_unlock(&(pool->qmutex));
	          }
            pthread_barrier_wait(&barrier);
            //pthread_cond_wait(&jdone, &gmutex);
            (a[i])[i] = r;
            (a[j])[i] = 0;
        }
    }
    pool->shutdown = 1;
    for(int i = 0; i < n_threads; i++)
    {
      int rc = pthread_join(pool->thrs[i], NULL);
      assert(rc == 0);
    }
    pthread_mutex_destroy(&pool->qmutex);
	  pthread_mutex_destroy(&pool->smutex);
    pthread_mutex_destroy(&gmutex);
    free(pool->thrs);
    free(pool);
    if ((a[n-1])[n-1] < 0)
    {
        (a[n-1])[n-1] = - (a[n-1])[n-1];
        for (i = 0; i < n; i++)
        {
            (q[i])[n-1] = - (q[i])[n-1];
        }
    }
    for (i = 0; i < n; i++)
        if (abs((a[i])[i]) < EPS)
        {
            cout << "Uninvertible matrix" << endl;
            return NULL;
        }
    for (i = 1; i < n; i++)
    {
        for (j = 0; j < n - i; j++)
        {
            for (k = 1; k < i; k++)
            {
                (a[i+j])[j]-= ((a[j])[j+k] * (a[i+j])[j+k]);
            }
            (a[i+j])[j]-= ((a[j])[j+i] / (a[i+j])[i+j]);
            (a[i+j])[j]/= (a[j])[j];
        }
    }
    cout << endl << "R^(-1):" << endl;
    double** b = (double**)calloc(n,sizeof(double*));
    for (i = 0; i < n; i++)
    {
       b[i] = (double*)calloc(n, sizeof(double));
       for (j = 0; j < n; j++)
        b[i][j] = 0;
    }

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            (b[i])[j] = (q[j])[i] / (a[i])[i];
            for (k = i+1; k < n; k++)
            {
                (b[i])[j]+= (a[k])[i]*(q[j])[k];
            }
        }
    }
    for (i = 0; i < n; i++)
      free(q[i]);
    free(q);
    return b;
}
