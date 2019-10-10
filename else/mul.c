#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <papi.h>
#define NUM_EVENTS 1
int main(int argc, char** argv)
{
    long long values[NUM_EVENTS];
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    assert(retval == PAPI_VER_CURRENT);
    int events[NUM_EVENTS] = {
        PAPI_L1_DCM,
      };
    if (argc != 5)
        return -1;
    FILE* fina = fopen(argv[1],"rb+");
    FILE* finb = fopen(argv[2],  "rb+");
    FILE* fout = fopen(argv[3],"wb+");
    FILE* fpapi = fopen("papis.txt", "a");
    if (!(fina&&finb&&fout))
      return -1;
    char ca,cb;
    int check;
    check = fread(&ca,sizeof(char),1,fina);
    if (!check)
      return -1;
    check = fread(&cb, sizeof(char),1,finb);
    if (!check)
      return -1;
    if (ca!=cb)
        return -1;
    int32_t sizea,sizeb;
    check = fread(&sizea,sizeof(int32_t),1,fina);
    if (!check)
      return -1;
    check = fread(&sizeb,sizeof(int32_t),1,finb);
    if (!check)
      return -1;
    if (sizea != sizeb)
        return -1;
    fwrite(&ca, sizeof(char),1,fout);
    fwrite(&sizea, sizeof(int32_t),1,fout);
    int mode = (int)argv[4][0] - (int)'0';
    time_t start,end;
    FILE* timings = fopen("timings.txt","a");
    double passed;
    sizea = sizeb = 100;
    if (ca == 'd')
    {
        int32_t** m32a = (int32_t**)calloc(sizeof(int32_t*),sizea);
        int32_t** m32b = (int32_t**)calloc(sizeof(int32_t*), sizeb);
        int32_t** m32c = (int32_t**)calloc(sizeof(int32_t*), sizea);
        assert(m32a);
        assert(m32b);
        assert(m32c);
        for(int32_t i = 0; i< sizea; i++)
        {
            m32a[i] = (int32_t*)calloc(sizeof(int32_t),sizea);
            m32b[i] = (int32_t*)calloc(sizeof(int32_t),sizeb);
            m32c[i] = (int32_t*)calloc(sizeof(int32_t),sizeb);
            assert(m32a[i]);
            assert(m32b[i]);
            assert(m32c[i]);
            check = fread(&m32a[i],sizeof(int32_t),sizea,fina);
            if (!check)
              return -1;
            check = fread(&m32b[i],sizeof(int32_t),sizea,finb);
            if (!check)
              return -1;
        }
        start = clock();
        retval = PAPI_start_counters((int *)events, NUM_EVENTS);
        assert(retval == 0);
        switch (mode)
        {
            case 0:
            {
                for(int32_t i = 0; i<sizea; i++)
                {
                    for (int32_t j = 0; j<sizea; j++)
                    {
                        int32_t temp = 0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            temp+=m32a[i][k]*m32b[k][j];
                        }
                        m32c[i][j] = temp;
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
        end = clock();
        retval = PAPI_read_counters(values, NUM_EVENTS);
        assert(retval == 0);
        fprintf(fpapi,"mode32: %d, L1 miss: %lf; L2 miss: %lf\n", mode, (double)values[0] , (double)values[1]);
        retval = PAPI_stop_counters(values, NUM_EVENTS);
        assert(retval == 0);
        passed = end - start;
        fprintf(timings,"%.30f mode32: %d\n",passed/CLOCKS_PER_SEC, mode);
        for (int32_t i = 0; i< sizea; i++)
        {
            for (int32_t j = 0; j<sizea; j++)
            {
                check = fwrite(&m32c[i][j],sizeof(int32_t),1,fout);
                if (!check)
                  return -1;
            }
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
        assert(m64a);
        assert(m64b);
        assert(m64c);
        for(int32_t i = 0; i< sizea; i++)
        {
            m64a[i] = (int64_t*)calloc(sizeof(int64_t),sizea);
            m64b[i] = (int64_t*)calloc(sizeof(int64_t),sizeb);
            m64c[i] = (int64_t*)calloc(sizeof(int64_t),sizea);
            assert(m64a[i]);
            assert(m64b[i]);
            assert(m64c[i]);
            check = fread(&m64a[i],sizeof(int64_t),sizea,fina);
            if (!check)
              return -1;
            check = fread(&m64b[i],sizeof(int64_t),sizea,finb);
            if (!check)
              return -1;
        }
        start = clock();
        retval = PAPI_start_counters((int *)events, NUM_EVENTS);
        assert(retval == 0);
        switch (mode)
        {
            case 0:
            {
                for(int32_t i = 0; i<sizea; i++)
                {
                    for (int32_t j = 0; j<sizea; j++)
                    {
                        int64_t temp = 0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            temp+=m64a[i][k]*m64b[k][j];
                        }
                        m64c[i][j] = temp;
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
                        int64_t temp = 0;
                        for (int32_t k = 0; k<sizea; k++)
                        {
                            temp+=m64a[i][k]*m64b[k][j];
                        }
                        m64c[i][j] = temp;
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
        end = clock();
        passed = end - start;
        retval = PAPI_read_counters(values, NUM_EVENTS);
        assert(retval == 0);
        check = fprintf(fpapi,"mode32: %d, L1 miss: %lf; L2 miss: %lf\n", mode, (double)values[0] , (double)values[1]);
        if (!check)
          return -1;
        retval = PAPI_stop_counters(values, NUM_EVENTS);
        assert(retval == 0);
        check = fprintf(timings,"%.30f mode64: %d\n",passed/CLOCKS_PER_SEC, mode);
        if (!check)
          return -1;
        for (int32_t i = 0; i< sizea; i++)
        {
            for (int32_t j = 0; j<sizea; j++)
            {
                check = fwrite(&m64c[i][j],sizeof(int64_t),1,fout);
                if (!check)
                  return -1;
            }
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
