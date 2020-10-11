#include <iostream>
#include <vector>
#include <thread>
#include <pthread.h>
#include <chrono>
#include <ctime>
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
            return count_++;
        }
};

class CounterAtomic {
private:
    std::atomic<int> count_;
public:
    CounterAtomic(): count_(-1) {
    }

    int get_value(){
        return count_++;
    }
};

class BigArray{
    private:
        size_t num_tasks_;
    public:
        int * array;
        BigArray(size_t numTasks): num_tasks_(numTasks){
            array = new int[numTasks]();
            for (size_t i = 0; i < numTasks; ++i){
                array[i] = 0;
            }
        }

        std::string test_array() const{
            std::cout<<std::endl;
            for (size_t i = 0; i < num_tasks_; ++i) {
                if (array[i] == 0) {
                    return "test: not ok\n";
                }
            }
            return "test: ok\n";
        }

        size_t size() const{
            //std::lock_guard<std::mutex> lg(mtx);
            return num_tasks_;
        }
        BigArray(BigArray &&) = default;
        int& operator[] (std::size_t index);
        ~BigArray(){
            delete [] array;
        }

};

int& BigArray::operator[] (std::size_t index)
{
    //std::lock_guard<std::mutex> lg(mtx2);
    return const_cast<int &>(array[index]);
}

template<typename T>
void start_thread(T &c, BigArray &task1){

    while(true){
        int tmp = c.get_value();
        if (tmp >= task1.size())
            break;
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        task1[tmp] = 1;
    }

}

int main() {
    std::vector<uint> nums_threads {4, 8, 16, 32}; //, 8, 16, 32
    std::vector<std::thread> vec_threads;
    constexpr int num_tasks = 1024*1024;
    std::cout<<"\nusing std::mutex: \n\n";
    for(uint &num: nums_threads){
        BigArray task1(num_tasks);
        CounterMutex c;
        std::cout<<"for NumThrears = "<<num<<std::endl;
        unsigned int start_time =  clock();
        for (uint i = 0; i < num; ++i) {
            vec_threads.push_back(std::thread(start_thread<CounterMutex>, std::ref(c), std::ref(task1)));
        }

        for (auto& thr: vec_threads){
            thr.join();
        }
        std::cout<<"time: "<<clock() - start_time;
        std::cout<<task1.test_array()<<std::endl;
        vec_threads.clear();
    }
    std::cout<<"\nusing std::atomic: \n";
    for(uint &num: nums_threads){
        BigArray task1(num_tasks);
        CounterAtomic c;
        std::cout<<"for NumThrears = "<<num<<std::endl;

        for (uint i = 0; i < num; ++i) {
            vec_threads.push_back(std::thread(start_thread<CounterAtomic>, std::ref(c), std::ref(task1)));
        }
        unsigned int start_time =  clock();
        for (auto& thr: vec_threads){
            thr.join();
        }
        std::cout<<"time: "<<clock() - start_time<<std::endl;
        std::cout<<task1.test_array();
        vec_threads.clear();
    }
    return 0;
}
