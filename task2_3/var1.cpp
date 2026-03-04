#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <cmath>

#define N 1000

int main(int argc, char **argv)
{
    double **A = new double*[N];
    double *b = new double[N];
    double *res = new double[N];
    double *res_new = new double[N];

    auto start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < N; i++)
    {
        A[i] = new double[N];
        for(int y = 0; y < N; y++){
            if(i == y){
                A[i][y] = 2.0;
            }
            else{
                A[i][y] = 1.0;
            }
        } 
        b[i] = N+1;
        res[i] = 0.0;
        res_new[i] = 0.0;
    }

    auto end1 = std::chrono::steady_clock::now();

    for(int z = 0; z < N; z++){
        #pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            double sum = 0.0;
            for(int y = 0; y < N; y++){
                if(y!=i){
                    sum += (A[i][y] * res[y]);
                }
            }
            res_new[i] = (1 - 0.0001) * res[i] + 0.0001 *((b[i]-sum)/(A[i][i])); 
        }
        double sum_error = 0.0;
        #pragma omp parallel for reduction(+:sum_error)
        for (int i = 0; i < N; i++)
        {
            // #pragma omp atomic
            sum_error+=(pow(res_new[i] - res[i], 2));
        }
        sum_error = sqrt(sum_error);
        #pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            res[i] = res_new[i];
        }
    }

    auto end2 = std::chrono::steady_clock::now();
    
    for(int i = 0; i < N; i++){
        std::cout << res[i] << std::endl;
    }

    const std::chrono::duration<double> elapsed_seconds1(end1 - start_time);
    const std::chrono::duration<double> elapsed_seconds2(end2 - end1);

    std::cout << "Init time: " << elapsed_seconds1.count() << std::endl;
    std::cout << "Work time: " << elapsed_seconds2.count() << std::endl;

    delete [] A;
    delete [] b;
    delete [] res;
    delete [] res_new;
}
