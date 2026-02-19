#include <iostream>
#include <fstream>
#include <chrono>
#include <omp.h>
#include <cmath>

#define N 10000000
#define BEGIN -5.0
#define END 5.0

double func(double x){
    return sin(x) * cos(x) * exp(-x*x) * log(x + 2);
}

double integrate_omp(double a, double b, int n){
    double w = (b-a)/n;
    double sum = 0.0;
    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int size_per_thread = n / num_threads;
        int thread_id = omp_get_thread_num();
        int start = thread_id * size_per_thread;
        int end = thread_id == num_threads - 1 ? n : (thread_id + 1) * size_per_thread;
        double local_sum = 0.0;
        for (int i = start; i < end; i++)
        {
            double h = func(a + i*w + (w/2.0));
            local_sum+= h*w;
        }
        #pragma omp atomic
        sum+=local_sum;
    }
    return sum; 
}

int main(int argc, char **argv){
    auto start = std::chrono::steady_clock::now();
    integrate_omp(BEGIN, END, N);
    auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed_seconds1(end - start);

    std::cout << "Work time: " << elapsed_seconds1.count() << std::endl;
}