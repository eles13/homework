#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <ios>

using namespace std;

class Matrix
{
  protected:
    size_t rows;
    size_t cols;
    int **mat;
    class ProxyClass
    {
        int *row;
        size_t col;

      public:
        ProxyClass(int *r, size_t c) : row(r), col(c) {}
        int operator[](int n) const
        {
            if ((size_t)n >= col)
                throw out_of_range("");
            return (row[n]);
        }
        int &operator[](int n)
        {
            if (((size_t)n >= col)||((size_t)n<0))
                throw out_of_range("");
            return (row[n]);
        }
    };

  public:
    Matrix(int r, int c) : rows(r), cols(c)
    {
        mat = new int*[rows];
        for (size_t i = 0; i < rows; i++)
        {
            mat[i] = new int[cols];
            for (size_t j = 0; j < cols; j++)
            mat[i][j]=0;
        }    
    }
    ~Matrix()
    {
        for (size_t i = 0; i < rows; i++)
            delete mat[i];
    }
    const size_t getRows() { return rows; }
    const size_t getColumns() { return cols; }
    ProxyClass operator[](int n) const
    {
        if ((size_t)n >= rows)
            throw out_of_range("");
        ProxyClass temp(mat[n], cols);
        return temp;
    }
    ProxyClass operator[](int n)
    {
        if (((size_t)n >= rows)||((size_t)n<0))
            throw out_of_range("");
        ProxyClass temp(mat[n], cols);
        return temp;
    }
    Matrix& operator*=(const int m2)
    {
        for (size_t i = 0; i < rows; i++)
            for (size_t j = 0; j < cols; j++)
                mat[i][j] *= m2;
        return *this;        
    }
    const bool operator==(Matrix m1)
    {
        if ((m1.rows != rows) || (m1.cols != cols))
            return false;
        for (size_t i; i < m1.rows; i++)
            for (size_t j; j < m1.cols; j++)
                if (m1[i][j] != mat[i][j])
                    return false;
        return true;
    }
    const bool operator!=(Matrix m1)
    {   
        return !(*this==m1);
    }
};
