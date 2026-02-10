#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#define N 10000000

int main(int argc, char* argv[]){
    int a = std::atoi(argv[1]);
    if(a == 1){
        std::vector<float> sini(N);
        float summ = 0;
        for(int i = 0; i < N-1; i++){
            sini[i] = std::sin(((2*M_PI*i)/N));
            summ+=sini[i];
        }
        std::cout << summ;
    }
    else if(a == 2){
        std::vector<double> sini(N);
        double summ = 0;
        for(int i = 0; i < N-1; i++){
            sini[i] = std::sin(((2*M_PI*i)/N));
            summ+=sini[i];
        }
        std::cout << summ;
    }
}