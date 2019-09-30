#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char** argv)
{
    if (argc != 5)
        return -1;
    FILE* fina = fopen(argv[1],"rb+");
    FILE* finb = fopen(argv[2], "rb+");
    FILE* fout = fopen(argv[3],"wb+");
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
    fwrite(&ca, sizeof(char),1,fout);
    fwrite(&sizea, sizeof(int32_t),1,fout);
    int mode = (int)argv[4][0] - (int)'0'; 
    time_t start,end;
    FILE* timings = fopen("timings.txt","a");
    double passed;
    if (ca == 'd')
    {
        int32_t** m32a = (int32_t**)calloc(sizeof(int32_t*),sizea);
        int32_t** m32b = (int32_t**)calloc(sizeof(int32_t*), sizeb);
        int32_t** m32c = (int32_t**)calloc(sizeof(int32_t*), sizea);
        for(int32_t i = 0; i< sizea; i++)
        {
            m32a[i] = (int32_t*)calloc(sizeof(int32_t),sizea);
            m32b[i] = (int32_t*)calloc(sizeof(int32_t),sizeb);
            m32c[i] = (int32_t*)calloc(sizeof(int32_t),sizeb);
            for (int32_t j = 0; j<sizea; j++)
            {
                fread(&m32a[i][j],sizeof(int32_t),1,fina);
                fread(&m32b[i][j],sizeof(int32_t),1,finb);
                m32c[i][j] = 0;
            }
        }
        time(&start);
        switch (mode) 
        {
            case 0:
            {
                for(int32_t i = 0; i<sizea; i++)
                {    
                    for (int32_t j = 0; j<sizea; j++)
                    {
                        m32c[i][j]=0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            m32c[i][j]+=m32a[i][k]*m32b[k][j];
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
                        int32_t r = m32a[i][k];
                        for (int32_t j = 0; j<sizea; j++)
                            m32c[i][j] += r*m32b[k][j];
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
                        int32_t r = m32b[k][j];
                        for(int32_t i = 0; i<sizea; i++)
                            m32c[i][j] += m32a[i][k] * r;
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
                        m32c[i][j] = 0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            m32c[i][j]+=m32a[i][k]*m32b[k][j];
                        }                    
                    }
                }
                break;   
            }
            case 2:
            {
                for(int32_t k = 0; k<sizea; k++)
                {
                    for(int32_t i = 0; i<sizea; i++)
                    {
                        for(int32_t j = 0; j<sizea; j++)
                        {
                            m32c[i][j]+=m32a[i][k]*m32b[k][j];
                        }
                    }
                }
                break;
            }
            case 5:
            {   
                for(int32_t k=0; k<sizea; k++)
                {
                    for(int32_t j=0; j<sizea; j++)
                    {
                        for(int32_t i=0; i<sizea; i++)
                            m32c[i][j]+=m32a[i][k]*m32b[k][j];
                    }
                }
                break;
            }
        }
        time(&end);
        passed = difftime(end,start);
        fprintf(timings,"%.15f mode32: %d\n",passed, mode);
        for (int32_t i = 0; i< sizea; i++)
        {
            for (int32_t j = 0; j<sizea; j++)
                fwrite(&m32c[i][j],sizeof(int32_t),1,fout);
        }
        for (int32_t i = 0; i< sizea; i++)
        {
            free(m32a[i]);
            free(m32b[i]);
            free(m32c[i]);
        }
        free(m32a);
        free(m32b);
        free(m32c);
    }
    else 
    {
        int64_t** m64a = (int64_t**)calloc(sizeof(int64_t*),sizea);
        int64_t** m64b = (int64_t**)calloc(sizeof(int64_t*),sizeb);
        int64_t** m64c = (int64_t**)calloc(sizeof(int64_t*),sizea);
        for(int32_t i = 0; i< sizea; i++)
        {
            m64a[i] = (int64_t*)calloc(sizeof(int64_t),sizea);
            m64b[i] = (int64_t*)calloc(sizeof(int64_t),sizeb);
            m64c[i] = (int64_t*)calloc(sizeof(int64_t),sizea);
            for (int32_t j = 0; j<sizea; j++)
            {
                fread(&m64a[i][j],sizeof(int64_t),1,fina);
                fread(&m64b[i][j],sizeof(int64_t),1,finb);
                m64c[i][j] = 0;
            }
        }
        time(&start);
        switch (mode) 
        {
            case 0:
            {
                for(int32_t i = 0; i<sizea; i++)
                {    
                    for (int32_t j = 0; j<sizea; j++)
                    {
                        m64c[i][j]=0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            m64c[i][j]+=m64a[i][k]*m64b[k][j];
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
                        int64_t r = m64a[i][k];
                        for (int32_t j = 0; j<sizea; j++)
                            m64c[i][j] += r*m64b[k][j];
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
                        int64_t r = m64b[k][j];
                        for(int32_t i = 0; i<sizea; i++)
                            m64c[i][j] += m64a[i][k] * r;
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
                        m64c[i][j] = 0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            m64c[i][j]+=m64a[i][k]*m64b[k][j];
                        }                    
                    }
                }
                break;   
            }
            case 2:
            {
                for(int32_t k = 0; k<sizea; k++)
                {
                    for(int32_t i = 0; i<sizea; i++)
                    {
                        for(int32_t j = 0; j<sizea; j++)
                        {
                            m64c[i][j]+=m64a[i][k]*m64b[k][j];
                        }
                    }
                }
                break;
            }
            case 5:
            {
                for(int32_t k=0; k<sizea; k++)
                {
                    for(int32_t j=0; j<sizea; j++)
                    {
                        for(int32_t i=0; i<sizea; i++)
                            m64c[i][j]+=m64a[i][k]*m64b[k][j];
                    }
                }
                break;
            }
        }
        time(&end);
        passed = difftime(end,start);
        fprintf(timings,"%.15f mode64: %d\n",passed, mode);
        for (int32_t i = 0; i< sizea; i++)
        {
            for (int32_t j = 0; j<sizea; j++)
                fwrite(&m64c[i][j],sizeof(int64_t),1,fout);
        }
        for (int32_t i = 0; i< sizea; i++)
        {
            free(m64a[i]);
            free(m64b[i]);
            free(m64c[i]);
        }
        free(m64a);
        free(m64b);
        free(m64c);
    }
    fclose(fina);
    fclose(finb);
    fclose(fout);
    fclose(timings);
    return 0;
}