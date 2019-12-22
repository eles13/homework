#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;
const double EPS = 0.000000001;

#include "pinverting.h"
#include "input.h"

int prnt(double** a, int n, int m)
{
    int i, j, k;
    k = (n < m)? n: m;
    cout << endl << endl << "Inverse matrix:" << endl;
    for (i = 0; i < k; i++)
    {
        for (j = 0; j < k; j++)
            if (abs((a[i])[j]) < EPS) cout << "        0 ";
            else cout << std::setw(12) << (a[i])[j] << " ";
        cout << endl;
    }
    return 0;
}

int clerror(double** a, double** b, int n)
{
   int i, j, k;
   double v = 0, w;
   for (i = 0; i < n; i++)
   {
      for (j = 0; j < n; j++)
      {
         w = 0;
         for (k = 0; k < n; k++)
            w+=((a[i])[k] * (b[k])[j]);
         if(i == j)
          w-=1;
         v+=w;
      }
   }
   cout<<endl;
   cout.setf(ios::fixed);
   cout << endl << "Nevyazka: " << setprecision(20) << v << endl;
   return 0;
}

int main()
{
   int n;
   size_t n_threads;
   cout<<"Insert numthreads"<<endl;
   cin>>n_threads;
   cout << "Insert the size of the matrix: "; cin >> n;
   double** a = NULL;
   double** b = NULL;
   a = input(n);
   double** copy = NULL;
   if (a != NULL)
   {
     copy = (double**)calloc(n,sizeof(double*));
     for(int i = 0; i < n; i++)
     {
       copy[i] = (double*)calloc(n,sizeof(double));
       for(int j = 0; j < n; j++)
        copy[i][j] = a[i][j];//memcpy probable
     }
   }
   unsigned int start_time =  clock();
   b = QRdec(a, n, EPS, n_threads);
   prnt(b, n, 10);
   clerror(copy, b, n);
   unsigned int end_time = clock();
   unsigned int search_time = end_time - start_time;
   cout << "Time of working: " << search_time * 1e-6 << "s" << endl;
   if(a!=NULL)
   {
    free(a);
    free(copy);
   }
   if(b != NULL)
    free(b);
   return 0;

}
