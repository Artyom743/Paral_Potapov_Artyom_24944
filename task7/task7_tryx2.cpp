#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <cublas_v2.h>

#define IDX(i, j, size) ((long long)(i) * (size) + (j))

int main(int argc, char* argv[]) {
    int N = 128;
    double eps = 1e-6;
    int max_iter = 1000000;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            N = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--eps") == 0 && i + 1 < argc) {
            eps = atof(argv[++i]);
        } else if (strcmp(argv[i], "--max_iter") == 0 && i + 1 < argc) {
            max_iter = atoi(argv[++i]);
        }
    }

    int size = N + 2;
    long long n = (long long)size * size;

    double* u = new double[n];
    double* un = new double[n];
    double* diff_buf = new double[n];

    for (long long i = 0; i < n; i++) {
        u[i] = 0.0;
    }

    u[IDX(0, 0, size)] = 10.0;
    u[IDX(0, size-1, size)] = 20.0;
    u[IDX(size-1, 0, size)] = 30.0;
    u[IDX(size-1, size-1, size)] = 20.0;

    for (int j = 1; j < size - 1; j++) {
        double t = (double)j / (size - 1);
        u[IDX(0, j, size)] = 10.0 + 10.0 * t;
        u[IDX(size-1, j, size)] = 30.0 - 10.0 * t;
    }
    for (int i = 1; i < size - 1; i++) {
        double t = (double)i / (size - 1);
        u[IDX(i, 0, size)] = 10.0 + 20.0 * t;
        u[IDX(i, size-1, size)] = 20.0;
    }

    for (long long i = 0; i < n; i++) {
        un[i] = u[i];
    }

    cublasHandle_t handle;
    cublasCreate(&handle);

    auto start = std::chrono::high_resolution_clock::now();

    int iter = 0;
    double error = 1.0;

    #pragma acc data copy(u[0:n], un[0:n]) create(diff_buf[0:n])
    {
        for (iter = 0; iter < max_iter; iter++) {

            #pragma acc parallel loop collapse(2) present(u, un)
            for (int i = 1; i < size-1; i++) {
                for (int j = 1; j < size-1; j++) {
                    un[IDX(i,j,size)] = (u[IDX(i-1,j,size)] + u[IDX(i+1,j,size)] +
                                         u[IDX(i,j-1,size)] + u[IDX(i,j+1,size)]) / 4.0;
                }
            }

            #pragma acc parallel loop collapse(2) present(u, un, diff_buf)
            for (int i = 1; i < size-1; i++) {
                for (int j = 1; j < size-1; j++) {
                    long long k = IDX(i, j, size);
                    double val = un[k];
                    diff_buf[k] = fabs(val - u[k]);
                    u[k] = val;
                }
            }

            int max_idx = 1;
            #pragma acc host_data use_device(diff_buf)
            {
                cublasIdamax(handle, n, diff_buf, 1, &max_idx);
            }

            #pragma acc host_data use_device(diff_buf)
            {
                cudaMemcpy(&error, diff_buf + (max_idx - 1), sizeof(double), cudaMemcpyDeviceToHost);
            }

            if (error < eps) break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Размер: " << N << "x" << N << std::endl;
    std::cout << "Время: " << duration.count() << " мс" << std::endl;
    std::cout << "Итерации: " << iter << std::endl;
    std::cout << "Ошибка: " << error << std::endl;

    cublasDestroy(handle);
    delete[] u;
    delete[] un;
    delete[] diff_buf;

    return 0;
}