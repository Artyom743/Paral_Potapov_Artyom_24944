#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <cstdlib>

#define VEC_SIZE 10000

void mult(double **vec1, int start, int end, double *vec2, double *res){
    for (int i = start; i < end; i++)
    {
        vec1[i] = new double[VEC_SIZE];
        for(int y = 0; y < VEC_SIZE; y++){
            vec1[i][y] = double(y+1);
            res[i] += vec2[y] * vec1[i][y];
        }
    }
}

int main(int argc, char *argv[])
{
    double **vec1 = new double*[VEC_SIZE];
    double *vec2 = new double[VEC_SIZE];
    double *res = new double[VEC_SIZE];

    auto start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < VEC_SIZE; i++)
    {
        vec2[i] = VEC_SIZE - i;
        res[i] = 0;
    }

    int NUM_THREAD = std::atoi(argv[1]);

    std::vector<std::thread> threads;

    int chunk = VEC_SIZE/NUM_THREAD;
    int start = 0;
    int end = chunk;

    auto end1 = std::chrono::steady_clock::now();

    for(int i = 0; i < NUM_THREAD; i++){
        int end = (i == NUM_THREAD - 1) ? VEC_SIZE : start + chunk;
        
        threads.push_back(std::thread(mult, std::ref(vec1), start, end, vec2, std::ref(res)));
        
        start = end;
    }

    for(auto &i : threads){
        i.join();
    }
    

    auto end2 = std::chrono::steady_clock::now();

    // for (int i = 0; i < VEC_SIZE; i++)
    // {
    //     for(int y = 0; y < VEC_SIZE; y++){
    //         res[i] += vec2[y] * vec1[i][y];
    //     }
    // }

    // std::ofstream file("res.txt", std::ios::out);

    // for (int i = 0; i < VEC_SIZE; i++)
    // {
    //     file << res[i] << std::endl;
    // }

    const std::chrono::duration<double> elapsed_seconds1(end1 - start_time);
    const std::chrono::duration<double> elapsed_seconds2(end2 - end1);

    std::cout << "Init time: " << elapsed_seconds1.count() << std::endl;
    std::cout << "Work time: " << elapsed_seconds2.count() << std::endl;

    delete [] vec1;
    delete [] vec2;
    delete [] res;
}