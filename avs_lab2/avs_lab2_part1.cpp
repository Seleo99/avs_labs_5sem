#include <iostream>
#include <vector>
#include <thread>
#include <pthread.h>
#include <chrono>
#include <mutex>
#include <atomic>

std::mutex mtx;
std::mutex mtx2;

class CounterMutex {
    private:
        int count_;
    public:
        CounterMutex(): count_(-1) {
        }

        int get_value(){
            std::lock_guard<std::mutex> lg(mtx);
            count_++;
            return count_;
        }
};

class CounterAtomic {
private:
    std::atomic<int> count_;
public:
    CounterAtomic(): count_(-1) {
    }

    int get_value(){
        count_++;
        //std::cout<<count_<<" ";
        return count_;
    }
};

class Task1_lab2{
    private:
        size_t num_tasks_;
    public:
        int * array;
        Task1_lab2(int numTasks): num_tasks_(numTasks){
            array = new int[numTasks]();
            for (int i = 0; i < numTasks; ++i){
                array[i] = 0;
            }
        }

        std::string test_array() const{
            std::cout<<std::endl;
            for (size_t i = 0; i < num_tasks_; ++i) {
                if (array[i] == 0) {
                    return "not ok\n";
                }
            }
            return "ok\n";
        }

        size_t size() const{
            std::lock_guard<std::mutex> lg(mtx);
            return num_tasks_;
        }
        Task1_lab2(Task1_lab2 &&) = default;
        int& operator[] (std::size_t index);
        ~Task1_lab2(){
            delete [] array;
        }

};

int& Task1_lab2::operator[] (std::size_t index)
{
    //std::lock_guard<std::mutex> lg(mtx2);
    return array[index];
}

template<typename T>
void start_thread(T &c, Task1_lab2 &task1){
    while(true){
        int tmp = c.get_value();
        if (tmp >= task1.size())
            return;
        //std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        task1[tmp] = 1;
        std::cout<<tmp<<" ";
    }
}

int main() {
    std::vector<uint> nums_threads {4, 8, 16, 32}; //, 8, 16, 32
    std::vector<std::thread> vec_threads;
    constexpr int num_tasks = 1000; //1024*1024
    std::cout<<"using std::mutex: \n";
    for(uint &num: nums_threads){
        Task1_lab2 task1(num_tasks);
        CounterMutex c;
        std::cout<<"for NumThrears = "<<num<<std::endl;
        for (uint i = 0; i < num; ++i) {
            vec_threads.push_back(std::thread(start_thread<CounterMutex>, std::ref(c), std::ref(task1)));
        }
        for (auto& thr: vec_threads){
            thr.join();
        }
        std::cout<<task1.test_array()<<std::endl;
        vec_threads.clear();
    }
    std::cout<<"using std::atomic: \n";
    for(uint &num: nums_threads){
        Task1_lab2 task1(num_tasks);
        CounterAtomic c;
        std::cout<<"for NumThrears = "<<num<<std::endl;
        for (uint i = 0; i < num; ++i) {
            vec_threads.push_back(std::thread(start_thread<CounterAtomic>, std::ref(c), std::ref(task1)));
        }
        for (auto& thr: vec_threads){
            thr.join();
        }
        std::cout<<task1.test_array();
        vec_threads.clear();
    }
    return 0;
}
