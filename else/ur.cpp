#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
using namespace std;

double geta() { return 2.; }

double u(double x, double t) {
  return (1 + sin(t)) * cos(M_PI * x) * sin(2 * M_PI * x);
}

double f(double x, double t) { return cos(t) * cos(M_PI * x) * sin(2 * M_PI* x) - M_PI * M_PI * (1 + sin(t)) * (-5*cos(M_PI * x) * sin(2 * M_PI * x) - 4 * cos(2 * M_PI * x) * sin(M_PI * x)); }

double u_0(double x, double t) { return cos(M_PI * x) * sin (2 * M_PI * x); }

double fi_1(double x, double t) { return 0.0; }

double fi_2(double x, double t) { return 0.0; }

double psi_1(double x, double t) { return 2 * M_PI * (1 + sin(t)); }

double psi_2(double x, double t) { return -2 * M_PI * (1 + sin(t)); }

void getlayer(double* prev, double* cur, const double h, const double t, const double a, const int hs, const double prevt)
{
  for(int i = 1; i < hs; i++)
  {
    cur[i] = prev[i] + f(i * h, prevt) * t + a* a * t / (h * h) * (prev[i - 1] + prev[i + 1] - 2 * prev[i]);
  }
}

const double getexp(const int i, double * cur, double * psi, const int point, const double h)
{
  int z = 1;
  if(point)
    z = -1;
  return z * (2 * psi[i] * h + z * cur[abs(point - 2)] - z * 4 * cur[abs(point - 1)]) / (-3);
}

void expl(const double t, const double h, const int ts, const int hs,  \
   const double a, const int left, const int right,  \
  double* leftf, double* rightf, double* curlayer, double* prevlayer, const int render)
{
  ofstream fout;
  string filename = "./results/exp";
  for(int i = 1; i < ts + 1; i++)
  {
    getlayer(prevlayer, curlayer, h, t, a, hs, i * t);
    if (left == 1)
      curlayer[0] = leftf[i];
    else
      curlayer[0] = getexp(i, curlayer, leftf, 0, h);
    if (right == 1)
      curlayer[hs] = rightf[i];
    else
      curlayer[hs] = getexp(i, curlayer, rightf, hs, h);
    if (render && (i % (ts/4) == 0))
    {
      string toconcat = to_string(i);
      fout.open(filename + toconcat);
      for(int j = 0; j < hs + 1; j++)
      {
        fout<<j*h<<' '<<curlayer[j]<<' '<<u(j*h,t*i)<<endl;
      }
      fout.close();
    }
    swap(prevlayer, curlayer);
  }
}

void impl(const double t, const double h, const int ts, const int hs, const double a,\
   const int left, const int right, double* leftf, double* rightf, \
    double* curlayer, double* prevlayer, const int render)
{
  ofstream fout;
  string filename = "./results/impl";
  double A = -a * a * t / (h * h);
  double B = A;
  double C = 2 * A - 1;
  double hi1 = 0;
  double hi2 = 0;
  double mu1, mu2;
  if (left != 1)
    hi1 = 1;
  if (right != 1)
    hi2 = 1;
  double al[hs + 1];
  double bt[hs + 1];
  al[1] = hi1;
  for(int i = 1; i < ts + 1; i++)
  {
    mu1 = leftf[i];
    mu2 = rightf[i];
    if(left != 1)
      mu1 *= -h;
    if(right != 1)
      mu2 *= h;
    bt[1] = mu1;
    for(int j = 2; j < hs + 1; j ++)
    {
      double temp = (C - al[j - 1] * A);
      al[j] = B / temp;
      bt[j] = (A * bt[j - 1] - prevlayer[j-1] - f((j - 1) * h, i * t) * t) / temp;
    }
    if (right == 1)
      curlayer[hs] = rightf[i];
    else
      curlayer[hs] = (mu2 + hi2 * bt[hs])/(1 - hi2 * al[hs]);
    for(int j = hs - 1; j >= 0; j--)
    {
      curlayer[j] = al[j+1] * curlayer[j+1] + bt[j+1];
    }
    if (render && (i % (ts/4) == 0))
    {
      string toconcat = to_string(i);
      fout.open(filename + toconcat);
      for(int j = 0; j < hs + 1; j++)
      {
        fout<<j*h<<' '<<curlayer[j]<<endl;
      }
      fout.close();
    }
    swap(prevlayer, curlayer);
  }

}

void init(int& left, int& right, double* leftf, double* rightf, \
  double& t, int& ts, double& h, int& hs, double* prevlayer, double* curlayer)
{
  if (left == 1)
  {
    for(int i = 0; i < ts + 1; i++)
        leftf[i] = fi_1(0,i * t);
  }
  else
  {
    for(int i = 0; i < ts + 1; i++)
        leftf[i] = psi_1(0, i * t);
  }
  if (right == 1)
  {
    for(int i = 0; i < ts + 1; i++)
        rightf[i] = fi_2(1, i * t);
  }
  else
  {
    for(int i = 0; i < ts + 1; i++)
        rightf[i] = psi_2(1, i * t);
  }
  for(int i = 0; i < hs + 1; i++)
  {
    prevlayer[i] = u_0(i*h, 0);
  }
}

const double abs_Cn(double* prevlayer,const double h,const double M) {
    double max = -1, temp;
    for (int i = 0; i < M+1; i++) {
        temp = abs(u(h*i,1) - prevlayer[i]);
        if (temp > max) {
            max = temp;
        }
    }
    return max;
}

const double abs_I2n(double* prevlayer,const double h,const double M)
{
    double sum = 0;
    for (int i = 0; i < M+1; i++) {
        sum += pow(u(h*i,1) - prevlayer[i], 2);
    }
    sum = sqrt(sum * h);
    return sum;
}

const double rel_Cn(double* prevlayer,const double h,const double M)
{
    double val1, val2 = -1;
    val1 = abs_Cn(prevlayer, h, M);
    for (int i = 0; i < M+1; i++) {
        if (abs(u(h*i,1)) > val2) {
            val2 = abs(u(h*i,1));
        }
    }
    return (val1 / val2);
}

const double rel_I2n(double* prevlayer,const double h,const double M)
{
    double val1, val2 = 0;
    val1 = abs_I2n(prevlayer, h, M);
    for (int i = 0; i < M+1; i++) {
        val2 += pow(u(h*i,1), 2);
    }
    val2 = sqrt(val2 * h);
    return (val1 / val2);
}

int main(int argc, char** argv)
{
  int ts, hs, left, right, mode, render = 0;
  if (argc == 2)
    render = 1;
  double a = 1.;
  cout<<"Mode: 0-ex, 1-im\n";
  cin>>mode;
  cout<<"Time"<<endl;
  cin>>ts;
  cout<<"Points"<<endl;
  cin>>hs;
  cout<<"Left"<<endl;
  cin>>left;
  cout<<"Right"<<endl;
  cin>>right;
  double t = 1./ts;
  double h = 1./hs;
  double* leftf = new double[ts + 1];
  double* rightf = new double[ts + 1];
  double* curlayer = new double[hs + 1];
  double* prevlayer = new double[hs + 1];
  init(left, right, leftf, rightf, t, ts, h, hs, prevlayer, curlayer);
  if(!mode)
    expl(t, h, ts, hs, a, left, right, leftf, rightf, curlayer, prevlayer, render);
  else
    impl(t, h, ts, hs, a, left, right, leftf, rightf, curlayer, prevlayer, render);
  if (render)
  {
    ofstream fout;
    fout.open("results.dat");
    for (int i = 0; i < hs + 1; i++)
    {
      fout<<i*h<<' '<<prevlayer[i]<<endl;
    }
    fout.close();
  }
  cout<<abs_Cn(prevlayer, h, hs)<<endl;
  cout<<abs_I2n(prevlayer, h, hs)<<endl;
  cout<<rel_Cn(prevlayer, h, hs)<<endl;
  cout<<rel_I2n(prevlayer, h, hs)<<endl;
  delete[] curlayer;
  delete[] prevlayer;
  delete[] leftf;
  delete[] rightf;
  return 0;
}
