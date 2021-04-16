#include <iostream>
#include "mpi.h"
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstring>

int rank, size;

double formula_Mat(size_t i, size_t j){
    return 1.0/(i+j + 1.0);
}

double* form_x(size_t n){
    auto x_true = (double*)calloc(n, sizeof(double));
    for(size_t i = 0; i < n; i++){
        x_true[i] = i*i - 2;
    }
    return x_true;
}

double* calc_x(size_t iter, const double* col, size_t n){
    auto x = (double*) calloc(n - iter, sizeof(double));
    double s = 0;
    for(size_t i = iter+1; i < n; i++){
        s += col[i]*col[i];
        x[i-iter] = col[i];
    }
    x[0] = col[iter] - sqrt(s + col[iter] * col[iter]);
    auto norm_x = sqrt(s + x[0] * x[0]);
    if (norm_x != 0) {
        for (size_t i = 0; i < n - iter; i++) {
            x[i] = x[i] / norm_x;
        }
    }
    return x;
}

void matByVec(const double* x, double* a, size_t n, size_t iter){
    auto rez = (double*)calloc(n, sizeof(double));
    double elem = 0.0;
    for(size_t i = 0; i < n - iter; i++){
        elem += x[i] * a[i + iter];
    }
    elem *= 2;
    for(size_t i = 0; i < n - iter; i++){
        rez[i+iter] = a[i+iter] - elem * x[i];
    }
    std::memcpy(&a[iter], &rez[iter], (n - iter) * sizeof(double));
    free(rez);
}

double gen_b_row(size_t i, double** vec, size_t s){
    double sum = 0.0;
    for(size_t idx = 0; idx < s; idx++) {
        sum += vec[idx][i];
    }
    double gsum;
    MPI_Reduce(&sum, &gsum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(&gsum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return gsum;
}

double norm(const double* v1, const double*v2, size_t n){
    double sum = 0;
    for(size_t i = 0; i < n; i++){
        sum += (v1[i]-v2[i])*(v1[i]-v2[i]);
    }
    return sqrt(sum);
}

int main(int argc, char** argv) {
    //std::cout << std::fixed << std::setprecision(8);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (argc < 3){
        MPI_Finalize();
        return -1;
    }
    int mode = atoi(argv[2]);
    auto n = (size_t)atoi(argv[1]);
    double* x_true;
    if(mode){
        x_true = form_x(n);
    }
    std::vector<int> my_cols;
    my_cols.reserve((int)(n / size));
    for(int i = 0; i < (int)(n / size); i++) {
        my_cols.push_back(rank + size * i);
    }
    if (size_t(n/size) * size < n and size_t(rank) < n - int(n/size) * size){
        my_cols.push_back(size * int(n/size) + rank);
    }
    auto locmat = (double**) calloc(my_cols.size(), sizeof(double*));
    auto copy_locmat = (double**) calloc(my_cols.size(), sizeof(double*));
    auto solution = (double*) calloc(my_cols.size(), sizeof(double));
    for(size_t i = 0; i < my_cols.size(); i++){
        locmat[i] = (double*)calloc(n, sizeof(double));
        copy_locmat[i] = (double*)calloc(n, sizeof(double));
        for(size_t j = 0; j < n; j++){
            locmat[i][j] = formula_Mat(my_cols[i],j);
            copy_locmat[i][j] = locmat[i][j];
        }
    }
    auto b = (double*)calloc(n, sizeof (double));
    auto copy_b = (double*)calloc(n, sizeof (double));
    for(size_t j = 0; j < n; j++){
        b[j] = gen_b_row(j, locmat, my_cols.size());
        copy_b[j] = b[j];
    }
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    for(size_t iter = 0; iter < n; iter++){
        double* x;
        if((int)iter % size == rank){
            x = calc_x(iter, locmat[(int)(iter/size)], n);
        }
        else{
            x = (double*) calloc(n - iter, sizeof(double));
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(x, n-iter, MPI_DOUBLE, (int)iter%size, MPI_COMM_WORLD);

        for(size_t i = 0; i < my_cols.size(); i++){
            matByVec(x, locmat[i], n, iter);
        }
        matByVec(x, b, n, iter);
        free(x);
    }
    MPI_Barrier(MPI_COMM_WORLD);
//    for(size_t i = 0; i < n; i++){
//        std::cout<<rank<<' '<<i<<' ';
//        for(size_t j = 0; j < my_cols.size(); j++){
//            std::cout<<locmat[j][i]<<' ';
//        }
//        std::cout<<'\n';
//    }
    double t1 = MPI_Wtime() - start;
    start = MPI_Wtime();
    auto csum = (double *) calloc(n, sizeof(double));
    //auto x = (double *) calloc(n, sizeof(double));
    for(int iter = n-1; iter >= 0; iter--){
        if((int)iter%size == rank){
            //std::cout<<iter<<' '<<b[iter]<<' '<<csum[iter]<<' '<<locmat[int(iter/size)][iter]<<'\n';
            solution[int(iter/size)] = (b[iter] - csum[iter]) / locmat[int(iter/size)][iter];
            for(int lociter = 0; lociter < iter; lociter++) {
                csum[lociter] += solution[int(iter / size)] * locmat[int(iter / size)][lociter];
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(csum, n, MPI_DOUBLE, (int)iter%size, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
//    for(size_t i = 0; i < my_cols.size(); i++){
//        std::cout<<rank<<' '<<solution[i]<<'\n';
//    }
    MPI_Barrier(MPI_COMM_WORLD);
    double t2 = MPI_Wtime() - start;
    free(csum);
    auto newb  = (double*)calloc(n, sizeof(double));
    for(size_t i = 0; i < n; i++){
        for(size_t j = 0; j < my_cols.size(); j++) {
            newb[i] += copy_locmat[j][i] * solution[j];
        }
    }
    auto b_b  = (double*)calloc(n, sizeof(double));
    auto x_x =  (double*)calloc(n, sizeof(double));
    MPI_Reduce(newb, b_b, n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    //MPI_Gather(solution, my_cols.size(), MPI_DOUBLE, x, my_cols.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if(mode){
        for(size_t i = 0; i < my_cols.size(); i++){
            x_true[my_cols[i]] -= solution[i];
            x_true[my_cols[i]] = x_true[my_cols[i]] * x_true[my_cols[i]];
        }
        MPI_Reduce(x_true, x_x, n, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        free(x_true);
    }

    if(rank == 0){
        std::cout<<t1<<' '<<t2<<' '<<t1+t2<<std::endl;
        std::cout<<norm(copy_b, b_b, n)<<std::endl;
//        for(size_t i = 0; i < n; i++){
//            std::cout<<x[i]<<'\n';
//        }
        if(mode){
            double norm_x  = 0;
            for(size_t i = 0; i < n; i++){
                norm_x += x_x[i];
            }
            std::cout<<sqrt(norm_x)<<std::endl;
        }
    }
    free(x_x);
    free(copy_b);
    free(b_b);
    free(newb);
    free(b);
    free(solution);
    for(size_t i = 0; i < my_cols.size(); i++){
        free(locmat[i]);
    }
    free(locmat);
    MPI_Finalize();
    return 0;
}
