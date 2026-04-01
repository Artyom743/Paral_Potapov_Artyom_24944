#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <fstream>
#include <condition_variable>
#include <unordered_map>
#include <vector>

double fun_sin(double arg){
    return std::sin(arg);
}

double fun_sqrt(double arg){
    return std::sqrt(arg);
}

double fun_pow(double arg1, double arg2){
    return std::pow(arg1, arg2);
}


template <typename T>
class Server {
    private:
    std::queue<std::pair<size_t, std::packaged_task <T()>>> tasks;
    size_t count_task = 0;
    std::condition_variable cond_var;
    std::mutex mut;
    std::unordered_map<size_t, T> results;
    std::vector<std::jthread> thread_pool;
    size_t num_threads;

    public:
    Server(size_t num_thread = 1) : num_threads(num_thread){}

    void start(){
        for(size_t i = 0; i < num_threads; i++){
            thread_pool.emplace_back(std::jthread([this](std::stop_token stoken){
                std::unique_lock<std::mutex> lock_res{mut, std::defer_lock}; // Лок для защиты ресурсов
                size_t id_task;
    
                while (!stoken.stop_requested()){
                    lock_res.lock();
                    cond_var.wait(lock_res, [&stoken, this] { return !tasks.empty() || stoken.stop_requested(); });
                    if (!tasks.empty()){
                        id_task = tasks.front().first;
                        std::future<T>res = (tasks.front().second).get_future();
                        auto task = std::move(tasks.front().second);
                        tasks.pop();
                        lock_res.unlock();
                        task();
                        lock_res.lock();
                        results[id_task] =  res.get();
                        cond_var.notify_all();
                    }
            
                    lock_res.unlock();
                }
    
            }));
        }
    }

    void stop(){
        for(auto& i : thread_pool){
            i.request_stop();
        }
        cond_var.notify_all();
    }
    
    template <typename F, typename... Args>
    size_t add_task(F&& func, Args&&... arg){
        size_t id;
        std::packaged_task<T()> task(std::bind(std::forward<F>(func), std::forward<Args>(arg)...));

        {
            std::lock_guard<std::mutex> lock(mut);
            count_task++;
            id = count_task;
            tasks.push(std::make_pair(id, std::move(task)));
        }

        cond_var.notify_one();

        return id;
    }

    T request_result(size_t id_res) {
        std::unique_lock<std::mutex> lock(mut);
        cond_var.wait(lock, [this, id_res] { 
        return results.find(id_res) != results.end(); 
        });
        return results[id_res];
    }
};

int main(){
    Server<double> server(4);

    auto start = std::chrono::steady_clock::now();

    server.start();

    std::thread client1([&server] () {
        std::ofstream out("client_sin.txt");
        std::vector<size_t> ids;
        std::vector<double> params;
        
        for(double i = 0.0; i < 100000.0; i++){
            ids.push_back(server.add_task(fun_sin, i * 10.0));
            params.push_back(i * 10.0);
        }
        
        for(size_t j = 0; j < ids.size(); j++){
            double res = server.request_result(ids[j]);
            out << ids[j] << " ( " << params[j] << " ) = " << res << std::endl;
        }
    });

    std::thread client2([&server] () {
        std::ofstream out("client_sqrt.txt");
        std::vector<size_t> ids;
        std::vector<double> params;
        
        for(double i = 0.0; i < 100000.0; i++){
            ids.push_back(server.add_task(fun_sqrt, i * 10.0));
            params.push_back(i * 10.0);
        }
        
        for(size_t j = 0; j < ids.size(); j++){
            double res = server.request_result(ids[j]);
            out << ids[j] << " ( " << params[j] << " ) = " << res << std::endl;
        }
    });

    std::thread client3([&server] () {
        std::ofstream out("client_pow.txt");
        std::vector<size_t> ids;
        std::vector<std::pair<double, double>> params;
        
        for(double i = 0.0; i < 100000.0; i++){
            ids.push_back(server.add_task(fun_pow, i * 2.0, i));
            params.push_back({i * 2.0, i});
        }
        
        for(size_t j = 0; j < ids.size(); j++){
            double res = server.request_result(ids[j]);
            out << ids[j] << ": ( " << params[j].first << ", " << params[j].second << ") = " << res << std::endl;
        }
    });

    client1.join();
    client2.join();
    client3.join();
    
    server.stop();

    auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed_seconds(end - start);
    std::cout << "Work time: " << elapsed_seconds.count() << std::endl;
    
    return 0;
}