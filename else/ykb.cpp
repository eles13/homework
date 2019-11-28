#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <cstdlib>
#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <ios>
#include <tuple>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <memory>
#include <unistd.h>
using namespace std;
class Matrix
{
  protected:
  size_t size;
  double **mat;
  class ProxyClass
  {
      double *row;
      size_t col;

  public:
      ProxyClass(double *r, size_t size) : row(r), col(size) {}
      double operator[](size_t n) const
      {
          if (n >= col)
              throw out_of_range("");
          return (row[n]);
      }
      double &operator[](size_t n)
      {
          if (((size_t)n >= col) || (n < 0))
              throw out_of_range("");
          return (row[n]);
      }
  };

  public:
  Matrix() : size(0) {}
  Matrix(size_t s) : size(s)
  {
      mat = new double *[size];
      for (size_t i = 0; i < size; i++)
      {
          mat[i] = new double[size];
          for (size_t j = 0; j < size; j++)
          {
              mat[i][j] = 0;
          }
      }
  }
  ~Matrix()
  {
      for (size_t i = 0; i < size; i++)
          delete[] mat[i];
      delete[] mat;
  }
  const size_t getSize() { return size; }
  ProxyClass operator[](size_t n) const
  {
      if (n >= size)
          throw out_of_range("");
      return ProxyClass(mat[n], size);
  }
  ProxyClass operator[](size_t n)
  {
      if (n >= size)
          throw out_of_range("");
      ProxyClass temp(mat[n], size);
      return temp;
  }
  Matrix &operator*=(const int m2)
  {
      for (size_t i = 0; i < size; i++)
          for (size_t j = 0; j < size; j++)
              mat[i][j] *= m2;
      return *this;
  }
  const bool operator==(const Matrix &m1)
  {
      if ((m1.size != size) || (m1.size != size))
          return false;
      for (size_t i; i < m1.size; i++)
          for (size_t j; j < m1.size; j++)
              if (m1[i][j] != mat[i][j])
                  return false;
      return true;
  }
  const bool operator!=(const Matrix &m1)
  {
      return !(*this == m1);
  }
  Matrix operator*(const Matrix &m1)
  {
      double sum = 0;
      if (m1.size != size)
      {
          throw out_of_range("");
      }
      Matrix temp(size);
      for (size_t i = 0; i < size; i++)
          for (size_t j = 0; j < size; j++)
          {
              for (size_t k = 0; k < size; k++)
              {
                  sum += mat[i][k] * m1[k][j];
              }
              temp[i][j] = sum;
              sum = 0;
          }
      return temp;
  }

  string out() const
  {
      string res = "";
      for (size_t i = 0; i < size; i++)
      {
          for (size_t j = 0; j < size; j++)
              res += to_string(mat[i][j]) + ' ';
          res += '\n';
      }
      return res;
  }

  const Matrix &operator=(const Matrix &m1)
  {
      if ((size != m1.size) && (size != 0))
          throw out_of_range("");
      if (size == 0)
      {
        mat = new double *[m1.size];
        for (size_t i = 0; i < m1.size; i++)
        {
          mat[i] = new double[m1.size];
          for (size_t j = 0; j < m1.size; j++)
          {
              mat[i][j] = 0;
          }
        }
        size = m1.size;
      }
      if (this == &m1)
      {
        return *this;
      }
      for (size_t i = 0; i < size; i++)
          for (size_t j = 0; j < size; j++)
              mat[i][j] = m1[i][j];
      return *this;
  }
  const double sod()
  {
    int sum = 0;
    for (size_t i = 0; i < this->size; i++)
      for (size_t j = 0; j < this->size; j++)
      {
        if (i != j)
          sum+=mat[i][j]*mat[i][j];
      }
    return sum;
  }
};

void choosing(const Matrix& A, size_t& i,size_t& j, const int iter, const size_t size, const int strat, vector<double>& lsum, const size_t previ = 0, const size_t prevj = 0)
{
  double max;
  int it = iter;
  switch (strat)
  {
    case 0 :
    {
      max = 0;
      i = 0;
      j = 0;
      for (size_t p = 0; p < size; p++) {
        for (size_t k = 0; k < size; k++) {
          if ((p != k) && (abs(A[p][k]) > abs(max)))
          {
            max = A[p][k];
            i = p;
            j = k;
          }
        }
      }
      break;
    }
    case 1:
    {
      size_t res;
      int num = size*size - size;
      it  = it % num;
      i = it / (size - 1);
      res = it % (size - 1);
      if(res >= i)
      {
        j = res + 1;
      }
      else
      {
        j = res;
      }
      break;
    }
    case 2:
    {
      i = 0;
      double sum = lsum[0];
      for (size_t p  = 1; p < size; p++)
      {
        if (lsum[p] > sum)
        {
          sum = lsum[p];
          i = p;
        }
      }
      max = 0;
      for (size_t k = 0; k < size; k++)
      {
        if ((k != i) && (abs(A[i][k]) > abs(max)))
        {
          max = abs(A[i][k]);
          j = k;
        }
      }
      break;
    }
    default : {};
  }
}

const double Cos(const double x, const double y)
{
  if (y == 0)
    return sqrt(2) / 2;
  else
    return sqrt(0.5 + abs(y) / (2 * sqrt(pow(x, 2) + pow(y, 2))));
}

const double Sin(const double x, const double y)
{
  if (y == 0)
    return sqrt(2) / 2;
  else
  {
    if (x*y > 0)
      return abs(x) / (2 * Cos(x, y) * sqrt(pow(x, 2) + pow(y, 2)));
    else
      return -Sin(-x,y);
  }
}

Matrix function(size_t& size)
{
  Matrix A(size);
  for(size_t i = 0; i < size; i++)
    for (size_t j = 0; j < size; j++)
    {
      if (i == j)
        A[i][j] = 1;
      if (j == size - 1)
        A[i][j] = i + 1;
      if (i == size - 1)
        A[i][j] = j + 1;
    }
  return A;
}
int ykb(Matrix& A,Matrix& U, const double eps, vector<double>& values, const size_t size)
{
  double sin, cos, ii, jj, t1, t2, t3, t4;
  int strat, iters = 0;
  size_t i = 0,j = 0;
  vector<double> lsum(size);
  cout<<"Select strategy: 0 - max, 1 - cyclic, 2 - optimum"<<endl;
  cin>>strat;
  if (strat == 2)
  {
    for(size_t p = 0; p < size; p++)
    {
      lsum[p] = 0;
      for(size_t k = 0; k < size; k++)
      {
        if (p != k)
          lsum[p]+=abs(A[p][k]);
      }
    }
  }
  double sqsum = A.sod();
  while (sqsum > eps)
  {
    if (strat == 1)
      choosing(A, i, j, iters, size, strat, lsum, i, j);
    else
      choosing(A, i, j, iters, size, strat, lsum);
    cos  = Cos(A[i][j]*(-2),A[i][i] - A[j][j]);
    sin = Sin(A[i][j]*(-2),A[i][i] - A[j][j]);
    ii = cos * cos *A[i][i] + sin * sin * A[j][j] - 2*cos*sin*A[i][j];
    jj =  cos * cos * A[j][j] + sin * sin * A[i][i] + 2*cos*sin*A[i][j];
    for(size_t k = 0; k < size; k++)
    {
      t1 = U[i][k]*cos - sin*U[j][k];
      t2 = U[i][k]*sin + cos*U[j][k];
      if((k!=i) && (k!=j))
      {
        t3 = A[i][k]*cos - sin*A[j][k];
        t4 = A[k][i]*sin + cos*A[k][j];
        sqsum -= A[i][k] * A[i][k] + A[k][i] * A[k][i] + A[k][j] * A[k][j] + A[j][k]* A[j][k];
        if (strat == 2)
        {
          lsum[i] -= abs(A[i][k]);
          lsum[k] -= abs(A[k][j]) + abs(A[k][i]);
          lsum[j] -= abs(A[j][k]);
        }
        A[i][k] = t3;
        A[k][i] = t3;
        A[k][j] = t4;
        A[j][k] = t4;
        sqsum += 2*t3 * t3 + 2*t4 * t4;
        if (strat == 2)
        {
          lsum[i] += abs(t3);
          lsum[j] += abs(t4);
          lsum[k] += abs(t3) + abs(t4);
        }
      }
      U[i][k] = t1;
      U[j][k] = t2;
    }
    A[i][i] = ii;
    A[j][j] = jj;
    sqsum -= A[i][j] * A[i][j] + A[j][i] * A[j][i];
    if (strat == 2)
    {
      lsum[i] -= abs(A[i][j]);
      lsum[j] -= abs(A[j][i]);
    }
    A[i][j] = 0;
    A[j][i] = 0;
    iters++;
  }
  for(size_t i = 0; i < size;i++)
  {
    values[i] = A[i][i];
  }
  return iters;
}

int main(int argc, char** argv)
{
  int option;
  string filename = "text.txt";
  size_t size = 0;
  cout<<"Select input option: 0-file, 1-formula"<<endl;
  cin>>option;
  if (!option)
  {
    cout<<"Enter filename:"<<endl;
    cin>>filename;
    if (filename.size() == 0)
      filename = "test.txt";
    ifstream fsize(filename);
    fsize>>size;
    fsize.close();
  }
  else
  {
    cout<<"Enter size"<<endl;
    cin>>size;
  }
  Matrix A(size);
  Matrix U(size);
  for(size_t i = 0; i < size; i++)
    U[i][i] = 1;
  if (!option)
  {
    ifstream fin(filename);
    fin>>size;
    for (size_t i = 0; i  < size; i++)
      for(size_t j = 0; j < size; j++)
        fin>>A[i][j];
  }
  else
  {
    A = function(size);
  }
  //cout<<A.out()<<endl;
  vector<double> values(size);
  double tme = clock();
  int iters = ykb(A, U, 0.01, values, size);
  tme = clock() - tme;
  //cout<<"res\n"<<A.out()<<endl;
  if (size > 10)
  {
    cout<<"Enter outfile"<<endl;
    string filename;
    cin>>filename;
    ofstream fout(filename);
    fout<<"Values:"<<endl;
    for(size_t i = 0; i < size; i++)
      fout<<values[i]<<' ';
    fout<<endl<<"Vectors:"<<endl;
    for(size_t i = 0; i < size; i++)
    {
      fout<<"vec "<<i<<": ";
      for(size_t j = 0; j < size; j++)
        fout<<U[i][j]<<' ';
      fout<<endl;
    }
    cout<<"Iters: "<<iters<<endl;
    cout<<"Time: "<<tme/CLOCKS_PER_SEC<<endl;
    return 0;
  }
  cout<<"Values:"<<endl;
  for(size_t i = 0; i < size; i++)
    cout<<values[i]<<' ';
  cout<<endl<<"Vectors:"<<endl;
  for(size_t i = 0; i < size; i++)
  {
    cout<<"vec "<<i<<": ";
    for(size_t j = 0; j < size; j++)
      cout<<U[i][j]<<' ';
    cout<<endl;
  }
  cout<<"Iters: "<<iters<<endl;
  cout<<"Time: "<<tme/CLOCKS_PER_SEC<<endl;
  return 0;
}
