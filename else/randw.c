#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
int main(int argc, char** argv)
{
  srand(13);
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
  omp_set_num_threads(numthreads);
  double sumt = 0, sump = 0;
  double whstart = omp_get_wtime();
  #pragma omp parallel for schedule(dynamic,1)
  for (int i = 0; i < n; i++)
  {
    int randnum = clock() % 100;
    double start = omp_get_wtime();
    int curpos = x;
    while ((curpos != a) && (curpos != b))
    {
      randnum = (randnum*11+123) % 10000;
      if ( randnum / 10000.0 > p)
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
  double time = omp_get_wtime() - whstart;
  FILE* fout = fopen("stats.txt", "a");
  if (!fout)
    return -1;
  int check = fprintf(fout,"%f %f %f %d %d %d %d %f %d\n", sump/n, time, sumt/n, a, b, x, n, p, numthreads);
  assert(check);
  return 0;
}
