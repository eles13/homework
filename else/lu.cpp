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
//#include <dos.h>

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
            delete mat[i];
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
                    sum += mat[i][k] + m1[k][j];
                }
                temp[i][j] = sum;
                sum = 0;
            }
        return temp;
    }

    void out()
    {
        for (size_t i = 0; i < size; i++)
        {
            //cout << 0 << endl;
            for (size_t j = 0; j < size; j++)
                cout << mat[i][j] << ' ';
            cout << endl;
        }
    }

    Matrix &operator=(const Matrix &m1)
    {
        if (size != m1.size)
            throw out_of_range("");
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
        Matrix L(size);

        for (size_t i = 0; i < size; i++)
            for (size_t j = i; j < size; j++)
                L[j][i] = mat[j][i] / mat[i][i];

        for (size_t k = 1; k < size; k++)
        {
            for (size_t i = k - 1; i < size; i++)
                for (size_t j = i; j < size; j++)
                    L[j][i] = mat[j][i] / mat[i][i];

            for (size_t i = k; i < size; i++)
                for (size_t j = k - 1; j < size; j++)
                    mat[i][j] = mat[i][j] - L[i][k - 1] * mat[k - 1][j];
        }
        for (size_t i = 0; i < size; i++)
            for (size_t j = 0; j < size; j++)
                if (i > j)
                {
                    mat[i][j] = L[i][j];
                }
        return *this;
    }

    double det()
    {
        size_t maxl;
        double del, det = 1, d;
        Matrix temp(size);
        temp = *this;
        //приведение матрицы к треугольному виду
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
            if (maxl != k) //если максимальный элемент не на диагонали, тогда переставляем строки с учетом изменения определителя
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
                        temp[i][j] = temp[i][j] - del * temp[k][j]; //вычитание верхней строки от нижних
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        //вычисление определителя путем премножения диагональных элементов
        for (size_t i = 0; i < size; i++)
            det *= temp[i][i];
        return det;
    }

    void ln_change(size_t from, size_t to)
    {
        swap(mat[from],mat[to]);
    }

    Matrix &inverse()
    {
        double det = this->det();
        if (det == 0.0)
            throw "Null determinant!";
        else
        {
            Matrix unit(size);
            for (size_t i = 0; i < size; i++)
                unit[i][i] = 1; //init unit matrix
            double fact;
            for (size_t k = 0; k < size; k++)
            { //outer
                for (size_t j = k + 1; j < size; j++)
                { //inner
                    if (mat[k][k] != 0.0)
                    {
                        fact = mat[j][k] / mat[k][k];
                    } //factor
                    else
                    {
                        for (size_t p = k + 1; p < size; p++)
                        {
                            if (mat[p][k] != 0.0)
                            {
                                ln_change(k, p);      //changing size
                                unit.ln_change(k, p); //changing size in unit
                            }
                        }
                        fact = mat[j][k] / mat[k][k];
                    }
                    for (size_t i = 0; i < size; i++)
                    { //we have to substract unnecessary in primary matrix in order to do the same substraction in unit matrix
                        mat[j][i] = mat[j][i] - mat[k][i] * fact;
                        unit[j][i] = unit[j][i] - unit[k][i] * fact;
                    }
                }
            }
            for (size_t k = size - 1; k > 0; k--)
            { //outer
                for (int j = k - 1; j >= 0; j--)
                { //inner
                    if (mat[k][k] != 0.0)
                    {
                        fact = mat[j][k] / mat[k][k]; //factor
                    }    
                    else
                    {
                        for (size_t p = k - 1; p > 0; p--)
                        {
                            if (mat[p][k] != 0.0)
                            {
                                ln_change(k, p);      //chaging size
                                unit.ln_change(k, p); //chaging size in unit
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
            { //reduction to unit
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

typedef tuple<size_t, double *, Matrix> info;

const Matrix readfile(size_t& size, vector<double>& b)
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
    //return make_tuple(size, b, A);
}

 const vector<double> solution(const vector<double>& b,Matrix& U,const size_t size)
 {
    Matrix L(size);
    for(size_t i = 0; i < size; i++)
    {
        for(size_t j = 0; j < size; j++)
        {
            if (i>j)
            {
                L[i][j] = U[i][j];
                U[i][j] = 0;
            }
        }
        L[i][i]=1;
    }
    L = U.inverse()*L.inverse();
    vector<double> x;
    double sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
            sum+=L[i][j]*b[j];
        x.push_back(sum);
        sum = 0;
    }
    return x;        
 }

int main(int argc, char **argv)
{
    cout << "Choose input option: 0 - file; 1 - formula" << endl;
    int option = 0;
    //cin >> option;
    //if (!option)
    //{
    size_t size;
    vector<double> b;
        auto stored = readfile(size,b);
        b = solution(b,stored,size);
        for (size_t i = 0; i<b.size(); i++)
            cout<<b[i]<<' ';
    //}
    return 0;
}