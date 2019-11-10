#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
int main(int argc, char** argv)
{
  if (argc != 7)
    return -1;
  int a,b,x,n,numthreads;
  double p;
  a = atoi(argv[1]);
  b = atoi(argv[2]);
  x = atoi(argv[3]);
  n = atoi(argv[4]);
  p = strtod(argv[5], NULL);
  numthreads = atoi(argv[6]);
  unsigned int* sds = calloc(p, sizeof(unsigned int));
  assert(sds);
  srand(13);
  for (int i = 0; i < p; i++)
  {
    sds[i] = rand();
  }
  omp_set_num_threads(numthreads);
  double sumt = 0, sump = 0;
  int curtime = time(NULL);
  double whstart = omp_get_wtime();
  #pragma omp parallel
  {
    unsigned int sd = sds[omp_get_thread_num()];
    #pragma omp for
    for (int i = 0; i < n; i++)
    {
      double start = omp_get_wtime();
      int curpos = x;
      while ((curpos != a) && (curpos != b))
      {
        int randnum = rand_r(&sd);
        if ( (float)randnum / (float)RAND_MAX > p)
          curpos--;
        else
          curpos++;
      }
      #pragma omp critical
      {
        sumt += omp_get_wtime()-start;
        if ( curpos == b )
          sump++;
      }
    }
}
  double time = omp_get_wtime() - whstart;
  FILE* fout = fopen("stats.txt", "a");
  if (!fout)
    return -1;
  int check = fprintf(fout,"%f %f %f %d %d %d %d %f %d\n", sump/n, time, sumt/n, a, b, x, n, p, numthreads);
  assert(check);
  free(sds);
  return 0;
}
