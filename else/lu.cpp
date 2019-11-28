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

using namespace std;

class Calc
{
    private:
    string f;
    public:
    Calc(const string s): f(s) {}
    void setexpr(const string s)
    {
		    f = s;
	  }
    double number()
    {
        int res = 0;
        for (;;)
        {
            char c = f[0];
            f.erase(0,1);
            if (c >= '0' && c <= '9')
                res = res * 10 + c - '0';
            else
            {
                f.insert(f.begin(),c);
                return res;
            }
        }
    }

    double ckobki()
    {
        char c = f[0];
        f = f.substr(1, f.size()-1);
        if (c == '(')
        {
            double x = expr();
            f.erase(0,1);
            return x;
        }
        else
        {
            f.insert(f.begin(),c);
            return number();
        }
    }

    double factor()
    {
        int x = ckobki();
        for (;;)
        {
            char c = f[0];
            f.erase(0,1);
            switch (c)
            {
            case '*':
                x *= ckobki();
                break;
            case '/':
                x /= ckobki();
                break;
            default:
                f.insert(f.begin(), c);
                return x;
            }
        }
    }

    double expr()
    {
        int x = factor();
        for (;;)
        {
            char c = f[0];
            f.erase(0,1);
            switch (c)
            {
            case '+':
                x += factor();
                break;
            case '-':
                x -= factor();
                break;
            default:
                f.insert(f.begin(), c);
                return x;
            }
        }
    }
};

class Matrix
{
protected:
    size_t size;
    double **mat;
    bool islu;
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
    size_t* order;
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
        islu = false;
        order = new size_t[size];
        for (size_t i = 0; i<size; i++)
          order[i] = i;
    }
    ~Matrix()
    {
        for (size_t i = 0; i < size; i++)
            delete[] mat[i];
        delete[] mat;
        delete[] order;
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

    Matrix &lu()
    {

        double max = mat[0][0];
        size_t index = 0;
        for (size_t i = 1; i < size; i++)
        {
          if (abs(mat[i][0]) > abs(max))
          {
            max = mat[i][0];
            index = i;
          }
        }
        order = new size_t[size];
        for (size_t i = 0; i<size; i++)
          order[i] = i;
        order[0] = index;
        order[index] = 0;
        for (size_t j = 0; j < size; j++)
        {
          for (size_t k = j + 1; k < size; k++)
          {
            for (size_t i = 0; i < j; i++)
                mat[order[j]][k] -= mat[order[j]][i] * mat[order[i]][k];
            mat[order[j]][k] /= max;
          }
          for (size_t k = j + 1; k < size; k++)
            for (size_t i = 0; i < j + 1; i++)
                mat[order[k]][j + 1] -= mat[order[k]][i] * mat[order[i]][j+1];
          if (j != size-1)
            max = mat[order[j + 1]][j + 1];
          index = j + 1;
          for (size_t i = j + 2; i < size; i++)
          {
            if (abs(mat[order[i]][j + 1]) > abs(max))
            {
                max = mat[order[i]][j + 1];
                index = i;
            }
          }
          if (j != size - 1)
            swap(order[j + 1], order[index]);
        }
        islu = true;
        return *this;
    }

    double det() const
    {
        size_t maxl;
        double del, det = 1, d;
        Matrix temp(size);
        temp = *this;
        for (size_t k = 0; k < size - 1; k++)
        {
            maxl = k;
            for (size_t i = k + 1; i < size; i++)
            {
                if (abs(temp[i][k]) > abs(temp[k][k]))
                {
                    maxl = i;
                }
            }
            if (maxl != k)
            {
                det = det * (-1);
                for (size_t j = 0; j < size; j++)
                {
                    d = temp[k][j];
                    temp[k][j] = temp[maxl][j];
                    temp[maxl][j] = d;
                }
            }
            if (temp[k][k] != 0)
            {
                for (size_t i = k + 1; i < size; i++)
                {
                    del = temp[i][k] / temp[k][k];
                    for (size_t j = 0; j < size; j++)
                    {
                        temp[i][j] = temp[i][j] - del * temp[k][j];
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        for (size_t i = 0; i < size; i++)
            det *= temp[i][i];
        return det;
    }

    void ln_change(size_t from, size_t to)
    {
        swap(mat[from], mat[to]);
    }

    Matrix &inverse()
    {
		double det = 1;
    if (islu)
    {
		    for (size_t i = 0; i< size; i++)
		    det*=mat[i][i];
		}
		else
			det = this->det();
        if (det == 0.0)
            throw "Null determinant!";
        else
        {
            Matrix unit(size);
            for (size_t i = 0; i < size; i++)
                unit[i][i] = 1;
            double fact;
            for (size_t k = 0; k < size; k++)
            {
                for (size_t j = k + 1; j < size; j++)
                {
                    if (mat[k][k] != 0.0)
                    {
                        fact = mat[j][k] / mat[k][k];
                    }
                    else
                    {
                        for (size_t p = k + 1; p < size; p++)
                        {
                            if (mat[p][k] != 0.0)
                            {
                                ln_change(k, p);
                                unit.ln_change(k, p);
                            }
                        }
                        fact = mat[j][k] / mat[k][k];
                    }
                    for (size_t i = 0; i < size; i++)
                    {
                        mat[j][i] = mat[j][i] - mat[k][i] * fact;
                        unit[j][i] = unit[j][i] - unit[k][i] * fact;
                    }
                }
            }
            for (size_t k = size - 1; k > 0; k--)
            {
                for (int j = k - 1; j >= 0; j--)
                {
                    if (mat[k][k] != 0.0)
                    {
                        fact = mat[j][k] / mat[k][k];
                    }
                    else
                    {
                        for (size_t p = k - 1; p > 0; p--)
                        {
                            if (mat[p][k] != 0.0)
                            {
                                ln_change(k, p);
                                unit.ln_change(k, p);
                            }
                        }
                        fact = mat[j][k] / mat[k][k];
                    }
                    for (int i = k; i >= 0; i--)
                    {
                        mat[j][i] = mat[j][i] - mat[k][i] * fact;
                        unit[j][i] = unit[j][i] - unit[k][i] * fact;
                    }
                }
            }
            for (size_t i = 0; i < size; i++)
            {
                for (size_t j = 0; j < size; j++)
                {
                    unit[i][j] /= mat[i][i];
                }
            }
            *this = unit;
        }
        return *this;
    }
};

const Matrix readfile(size_t &size, vector<double> &b)
{
    cout << "Input filename" << endl;
    string filename;
    cin >> filename;
    ifstream fin(filename);
    fin >> size;
    Matrix A(size);
    double temp;
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            fin >> A[i][j];
        }
        fin >> temp;
        b.push_back(temp);
    }
    return A;
}

const vector<double> solution(const vector<double> &b, const Matrix &A, const size_t size)
{
  vector<double> y(size),x(size);
  for (size_t i = 0; i < size; i++)
  {
      y[A.order[i]] = b[A.order[i]];
      for (size_t j = 0; j < i; j++)
          y[A.order[i]] -= y[A.order[j]] * A[A.order[i]][j];
      y[A.order[i]] /= A[A.order[i]][i];
  }
  for (int i = (int)size - 1; i >= 0; i--) {
      x[A.order[i]] = y[A.order[i]];
      for (size_t j = (size_t)i + 1; j < size; j++) {
          x[A.order[i]] -= A[A.order[i]][j] * x[A.order[j]];
      }
  }
  return x;
}


const string outresult(const size_t size, const vector<double> b, const Matrix &A)
{
    cout<<endl;
    string res = "X = \n";
    for (size_t i = 0; i < size; i++)
    {
        res += to_string(b[A.order[i]]) + ' ';
    }
    res += '\n';
    res+="L = \n";
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            if (i <= j)
            {
              res+=to_string(A[A.order[i]][j])+ ' ';
            }
            else
              res+="0 ";
        }
        res+="\n";
    }
    res+="U=\n";
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            if (i > j)
            {
              res+=to_string(A[A.order[i]][j])+' ';
            }
            else if (i == j)
            {
              res+="1 ";
            }
            else
            {
              res+="0 ";
            }
        }
        res+="\n";
    }
    return res;
}

const double discrepancy_rmse(const vector<double> &solution, const vector<double> &b,const Matrix &A)
{
  vector<double> x(b.size());
  for (size_t i = 0; i < b.size() ; i++)
  {
      double elem = 0;
      x[i] = 0;
      for (size_t j = 0; j < b.size(); j++)
      {
        if (i>j)
          elem = 0;
        else if (i == j)
          elem = 1;
        else
          elem = A[A.order[i]][j];
        x[i]+=elem*solution[A.order[j]];
      }
    }
    vector<double> y(b.size());
    for (size_t i = 0; i < b.size(); i++)
    {
        double elem = 0;
        y[i] = 0;
        for (size_t j = 0; j < b.size(); j++)
        {
          if (i<j)
            elem = 0;
          else
            elem = A[A.order[i]][j];
          y[i]+=elem*x[j];
        }
      }
      double sum = 0;
      for(size_t i = 0; i < b.size(); i++)
      {
        sum+=pow(y[i] - b[A.order[i]],2);
      }
      return sqrt(sum);
  }

const Matrix fromformula(size_t &size, vector<double> &b)
{
    cout << "Enter size:" << endl;
    cin >> size;
    cout << "Enter formula:" << endl;
    string formula;
    cin >> formula;
    int m;
    Calc c("");
    int ipos = formula.find('i');
    int jpos = formula.find('j');
    int npos = formula.find('n');
    Matrix tempmat(size);
    string tempf;
    if (npos != -1)
    {
      formula.erase(ipos,1);
      formula.insert(ipos, to_string(size));
    }
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            tempf = formula;
            if (ipos != -1)
            {
              tempf.erase(ipos,1);
              tempf.insert(ipos, to_string(i));
            }
            if (jpos != -1)
            {
              tempf.erase(jpos,1);
              tempf.insert(jpos, to_string(j));
            }
			      c.setexpr(tempf);
			      m = c.expr();
			      tempmat[i][j] = m;
        }
    }
    double tempvar;
    cout<<"Enter vec b:"<<endl;
    for( size_t i = 0; i < size; i++)
    {
		cin>>tempvar;
		b.push_back(tempvar);
	}
    return tempmat;
}

Matrix form(size_t& size, vector<double>& b)
{
  cin>>size;
  Matrix A(size);
  for(size_t i = 0; i < size; i++)
  {
    double sum = 0;
    for(size_t j = 0; j < size; j++)
    {
      A[i][j] = 1./(2*size - i - j + 1);
      sum+= A[i][j];
    }
    b.push_back(sum);
  }
  cout<<A.out();
  return A;
}

int main(int argc, char **argv)
{
    cout << "Choose input option: 0 - file; 1 - formula" << endl;
    int option = 0;
    cin >> option;
    size_t size;
    vector<double> b;
    Matrix stored;
    if (!option)
    {
        stored = readfile(size, b);
    }
    else
    {
        stored = form(size, b);
    }
    try
    {
		auto sol = solution(b, stored.lu(), size);
		if (b.size() > 10)
		{
			cout << "Input out file" << endl;
			string ofilename;
			cin >> ofilename;
			ofstream fout(ofilename);
			fout << outresult(size, sol, stored);
			fout << "rmse:" << endl;
			fout << discrepancy_rmse(sol, b, stored) << endl;
		}
		else
		{
			cout<<outresult(size, sol, stored);
			cout << "rmse:";
			cout << discrepancy_rmse(sol, b, stored) << endl;
		}
	}
  catch(char const* err)
	{
		cout<<err<<endl;
		return -1;
	}
    return 0;
}
