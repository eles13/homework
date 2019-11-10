#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void out(int* m, const int n)
{
	for (int i = 0; i<n; i++)
		printf("%d\n", m[i]);
}

static int cmp(const void *_a, const void *_b)
{
	return *((int *)_a) - *((int *)_b);
}

static int equal(const int *a, const int *b, const int n)
{
	for (int i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			return 0;
		}
	}
	return 1;
}

static void baseline(int *m, const int n, const int numthreads)
{
	qsort(m, n, sizeof(int), cmp);
}

static void merge(int* m1,int* m2, int size, int pos, int* res,int part, const int oddsize)
{
		if (!part)
		{
			size++;
			pos++;
		}
		else
		if (oddsize)
		{ pos++;}
		//printf("size %d pos %d thr %d\n", size, pos, omp_get_thread_num());
    int a = 0, b = 0, i = 0;
    for (; i < size+pos; i++)
    {
				//printf("a %d b %d , thr %d\n", a, b, omp_get_thread_num());
				if ((a >= size) && (b < pos))
				{
					//printf("from 2 %d thr %d fs\n", m2[b], omp_get_thread_num());
					res[i] = m2[b];
					b++;
					//printf("/\n");
					continue;
				}
				else if ((a < size) && (b >= pos))
				{
					//printf("from 1 %d thr %d fs\n", m1[a], omp_get_thread_num());
					res[i] = m1[a];
					a++;
					//printf("/\n");
					continue;
				}
        if (m1[a] < m2[b])
        {
						//printf("from 1 %d thr %d alt %d\n", m1[a], omp_get_thread_num(), m2[b]);
            res[i] = m1[a];
						a++;
						//printf("/\n");
        }
        else
        {
						//printf("from 2 %d thr %d alt %d\n", m2[b], omp_get_thread_num(), m1[a]);
            res[i] = m2[b];
						b++;
						//printf("/\n");
        }
    }
}

static int binsearch(int* buf, int size, int* keybuf, int pos)
{
	int key = keybuf[pos];
	//printf("key %d %d\n", key, buf[0]);
	int r = 0;
	int l = size-1;
	if (buf[l] <= key)
		return l;
	if (buf[r] >= key)
	{
		int i = 0;
		while (keybuf[pos + i + 1] < buf[i])
			{
				int temp = keybuf[pos + i + 1];
				keybuf[pos+i+1] = buf[i];
				buf[i] = temp;
				i++;
			}
		if (i > 0)
			return i-1;
		return 0;
	}
	while (!((buf[(l+r)/2 - 1] < key)&&(buf[(l+r)/2 + 1] >key)))
	{
		//printf("%d %d\n", r, l);
		if (l == r)
			break;
		if (key > buf[(l+r)/2])
		{
			r = (l+r)/2;
		}
		else
		{
			l = (l+r)/2;
		}
	}
	// if (buf[(l+r)/2] < key)
	// 	return (l + r)/2 + 1;
	 int res  = (l+r) / 2;
	 while (buf[res] > keybuf[pos + 1])
	 	res--;
	return res;
}

static void sort(int *m, const int n, const int numthreads)
{

	omp_set_num_threads(numthreads);
	int size = n / numthreads;
	printf("--start--\n");
	out(m, n);
	printf("--start--\n");
	#pragma omp parallel firstprivate(size)
	{
		int oddsize = 0;
	 	int thrnum = omp_get_thread_num();
		if ((n%2 == 1) && (thrnum == numthreads - 1))
		{
			oddsize++;
		}
		qsort(&m[size*thrnum],size + oddsize,sizeof(int),cmp);
		#pragma omp barrier
		oddsize = 0;
		#pragma omp single
		{
		out(m, n);
	  }
		for (int pow = 2; pow <= numthreads; pow*=2)
		{
			//printf("ITER %d\n", pow);
			if (thrnum < numthreads / pow)
			{
				//#pragma omp critical
			//	{
				printf("num %d\n", omp_get_thread_num());
				if (thrnum == numthreads/pow - 1)
				{
					oddsize = 1;
				}
				int j = 0;
				int* buf = calloc(size * pow / 2 , sizeof(int));
				assert(buf);
				for (int i = size * pow * thrnum ; i < size * pow * thrnum  + size * pow / 2; i++)
					{
						buf[j] = m[i];
						j++;
					}
				///after-initiaalization
				if (thrnum == 1)
				{
					printf("/////////\nbuf1\n");
					out(buf, size*pow/2);
					printf("/////////\n");
				}
				int* buf2 = calloc(size * pow / 2 + oddsize, sizeof(int));
				assert(buf2);
				j = 0;
				for (int i = size * thrnum * pow + size * pow / 2; i < size * thrnum * pow + size * pow + oddsize; i++)
				{
					buf2[j] = m[i];
					if (thrnum == 1)
					{
						printf("adding %d on %d from %d\n", m[i], j, i);
					}
					j++;
				}
				if (thrnum == 1)
				{
					printf("/////////\nbuf2\n");
					if (oddsize)
						out(buf2, size*pow/2 + 1);
					else
						out(buf2, size*pow/2);
					printf("/////////\n");
				}
				//#pragma omp critical
				//{

				int defpos = size*pow / 4;
				//#pragma omp task firstprivate(buf,buf2, size, oddsize, defpos, pow)
				int pos = binsearch(buf2,size*pow / 2 + oddsize, buf, defpos);

				#pragma omp task firstprivate(buf,buf2, defpos,pow,size, oddsize)
				{
					//thrnum = omp_get_thread_num() % (pow * 2);
					//printf("thn %d\n", thrnum);
					merge(buf, buf2, defpos, pos, &m[size * thrnum * pow], 0, oddsize);
 			}
//
// 									//}
// //printf("start 1 %d start 2 %d size1 %d size2 %d\n", buf[size * pow/2 + 1], buf2[pos+1], pow * (size - size/2) - 1, size * pow - pos-2);
 				#pragma omp task firstprivate(buf,buf2, defpos,pow,size, oddsize)
 				{
					//printf("start 1 %d start 2 %d size 1 %d size 2 %d start %d\n",defpos + 1, pos+1, pow * (size - size/2) / 2 - 1, size * pow / 2 - pos - 1, size * pow * thrnum  + size*pow/4 + pos + 2);
					merge(&buf[defpos + 1], &buf2[pos+1], pow * (size - size/2) / 2 - 1, size * pow / 2 - pos - 1,\
					 &m[size * pow * thrnum  + defpos + pos + 2], 1, oddsize);
				}
				#pragma omp taskwait
			  free(buf);
			  free(buf2);
			}
			printf("num %d iter\n", omp_get_thread_num());
			#pragma omp barrier
		}
	}
	{
	printf("--------\n");
	out(m, n);
	printf("--------\n");
	}
}

typedef void (*cb)(int *m, const int n, const int numthreads);

static double timer(const cb f, int *m, const int n, const int numthreads)
{
	double start = omp_get_wtime();
	f(m, n, numthreads);
	return omp_get_wtime() - start;
}

static int do_check(int *a, int *b, const int n, const int numthreads)
{
	double t1 = timer(baseline, a, n, numthreads);
	double t2 = timer(sort, b, n, numthreads);
	if (!equal(a, b, n)) {
		printf("`b' is not sorted\n");
		return -1;
	}
	FILE* fout = fopen("out.txt", "a");
	fprintf(fout,"t1 = %lfs, t2 = %lfs; s = %lf\n", t1, t2, t1 / t2);
	if (t2 >= t1) {
		printf("sort is slower than qsort\n");
		return -2;
	}
	return 0;
}

static int check(const int n, const int numthreads)
{
	int *a = malloc(n * sizeof(int));
	assert(a);
	int *b = malloc(n * sizeof(int));
	assert(b);

	for (int i = 0; i < n; i++) {
		int v = rand() % 1048576;
		a[i] = v;
		b[i] = v;
	}
	// b[0] = 16;
	// b[1] = 15;
	// b[2] = 53;
	// b[3] = 19;
	// b[4] = 55;
	// b[5] = 28;
	// b[6] = 29;
	// b[7] = 22;
	// b[8] = 37;
	// b[9] = 33;
	// b[10] = 80;
	// b[11] = 55;
	// b[12] = 77;
	// b[13] = 73;
	// b[14] = 31;
	// b[15] = 36;
	int ret = do_check(a, b, n, numthreads);
	out(b, n);
	free(a);
	free(b);
	return ret;
}

int main(int argc, char **argv)
{
	// parse cmd line args here
	srand(time(NULL));
	return check(atoi(argv[1]), atoi(argv[2]));
}
