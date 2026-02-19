#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <cmath>

#define VEC_SIZE 100000000

int main(int argc, char **argv)
{
    double *vec1 = new double[VEC_SIZE];
    double res = 0;

    auto start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < VEC_SIZE; i++)
    {
        vec1[i] = i;
    }

    auto end1 = std::chrono::steady_clock::now();

    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int size_per_thread = VEC_SIZE / num_threads;
        int thread_id = omp_get_thread_num();
        int start = thread_id * size_per_thread;
        int end = thread_id == num_threads - 1 ? VEC_SIZE : (thread_id + 1) * size_per_thread;
        double sum = 0;
        for (int i = start; i < end; i++)
        {
            // #pragma omp critical
            sum += vec1[i];
        }
        #pragma omp critical // atomic на процессор
        {
            res += sum;
        }
    }

    auto end2 = std::chrono::steady_clock::now();
    
    std::cout << res << std::endl;

    const std::chrono::duration<double> elapsed_seconds1(end1 - start_time);
    const std::chrono::duration<double> elapsed_seconds2(end2 - end1);

    std::cout << "Init time: " << elapsed_seconds1.count() << std::endl;
    std::cout << "Work time: " << elapsed_seconds2.count() << std::endl;

    delete [] vec1;
}
