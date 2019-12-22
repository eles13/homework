#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;
double** input (int n)
{
    int o, i, j;
    double **a = (double**)calloc(n,sizeof(double*));
    for(int i = 0; i < n;i++)
      a[i] = (double*)calloc(n,sizeof(double));
    cout << "If you want to fill from the file press '1'" << endl << "If you want to fill by the formula press '2'" << endl;
    cin >> o;
    if (o == 2)
    {
       cout << "By formula" << endl;
       for (i = 0; i < n; i++)
       {
           for (j = 0; j < n; j++)
              a[i][j] = (i == j);
       }
    }
    else
    if (o == 1) // �� �����
    {
        cout << "From file" << endl;
        ifstream f("matrix.txt");
        if (!f.is_open())
        {
            cout << "File not opened" << endl;
            return NULL;
        }
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
        {
            if (f.eof())
            {
                cout << "Short file" << endl;
                return 0;
            }
            f >> a[i][j];
        }
        f.close();

    }
    else
    {
        cout << "Incorrect answer" << endl;
        return NULL;
    }
    return a;
}



#endif // INPUT_H_INCLUDED
