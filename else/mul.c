#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char** argv)
{
    if (argc != 5)
        return -1;
    FILE* fina = fopen(argv[1],"rb+");
    FILE* finb = fopen(argv[2], "rb+");
    char ca,cb;
    fread(&ca,sizeof(char),1,fina);
    fread(&cb, sizeof(char),1,finb);
    if (ca!=cb)
        return -1;
    int32_t sizea,sizeb;
    fread(&sizea,sizeof(int32_t),1,fina);
    fread(&sizeb,sizeof(int32_t),1,finb);
    if (sizea != sizeb)
        return -1;
    int32_t** m32a = (int32_t**)calloc(sizeof(int32_t*),sizea);
    int32_t** m32b = (int32_t**)calloc(sizeof(int32_t*), sizeb);
    int64_t** m64a = (int64_t**)calloc(sizeof(int64_t*),sizea);
    int64_t** m64b = (int64_t**)calloc(sizeof(int64_t*),sizeb);
    int mode32 = 0;
    if (ca == 'd')
    {
        for(int32_t i = 0; i< sizea; i++)
        {
            m32a[i] = (int32_t*)calloc(sizeof(int32_t),sizea);
            m32b[i] = (int32_t*)calloc(sizeof(int32_t),sizeb);
            for (int32_t j = 0; j<sizea; j++)
            {
                fread(&m32a[i][j],sizeof(int32_t),1,fina);
                fread(&m32b[i][j],sizeof(int32_t),1,finb);
            }
        }
        mode32 = 1;
    }
    else 
    {
        for(int32_t i = 0; i< sizea; i++)
        {
            m64a[i] = (int64_t*)calloc(sizeof(int64_t),sizea);
            m64b[i] = (int64_t*)calloc(sizeof(int64_t),sizeb);
            for (int32_t j = 0; j<sizea; j++)
            {
                fread(&m64a[i][j],sizeof(int64_t),1,fina);
                fread(&m64b[i][j],sizeof(int64_t),1,finb);
            }
        }
    }

    #if mode32
        #define A m32a
        #define B m32b
        #define C m32c
        #define type int32_t
        int32_t** m32c = (int32_t**)calloc(sizeof(int32_t*),sizea);
    #else
        #define A m64a
        #define B m64b
        #define C m64c
        #define type int64_t
        int64_t** m64c = (int64_t**)calloc(sizeof(int64_t*),sizea);
    #endif
    switch ((int)argv[4][0]) 
    {
        case 0:
        {
            for(int32_t i = 0; i<sizea; i++)
            { 
                for (int32_t j = 0; j<sizea; j++)
                {
                    C[i][j]=0;
                    for (int32_t k = 0; k<sizea; k++)
                    {
                        C[i][j]+=A[i][k]*B[k][j];
                    }                    
                }
            }
            break;
        }
        case 1:
        {
            for(int32_t i = 0; i<sizea; i++)
            {
                for (int32_t k = 0; k<sizea; k++)
                {
                    type r = A[i][k];
                    for (int32_t j = 0; j<sizea; j++)
                        C[i][j] += r*B[k][j];
                }
            }
            break;    
        }
        case 4:
        {
            for(int32_t j = 0; j<sizea; j++)
            {
                for (int32_t k = 0; k<sizea; k++)
                {
                    type r = B[k][j];
                    for(int32_t i = 0; i<sizea; i++)
                        C[i][j] += A[i][k] * r;
                }
            } 
            break;
        }
        case 3:
        {
            for(int32_t j = 0; j<sizea; j++)
            { 
                for (int32_t i = 0; i<sizea; i++)
                {
                    C[i][j] = 0;
                    for (int32_t k = 0; k<sizea; k++)
                    {
                        C[i][j]+=A[i][k]*B[k][j];
                    }                    
                }
            }
            break;   
        }
    }
    for (int32_t i = 0; i< sizea; i++)
    {
        free(C[i]);
        free(m32a[i]);
        free(m32b[i]);
        free(m64a[i]);
        free(m64b[i]);
    }
    free(m32a);
    free(m32b);
    free(m64a);
    free(m64b);
    free(C);
}