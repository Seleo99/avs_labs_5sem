#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <condition_variable>

std::mutex mtx_cout;

class queue
{
private:
    uint8_t *buffer_;
    uint8_t head_;
    uint8_t tail_;
    int capacity_;
    int count_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_empty; //if empty
    std::mutex mtx_;
public:
    queue(int capacity): capacity_(capacity), head_(0), tail_(0), count_(0) {
        buffer_ = new uint8_t[capacity];
    }
    ~queue() {
        delete [] buffer_;
    }

    bool pop(uint8_t &value) {
        std::unique_lock<std::mutex> u_lock(mtx_);
        return true;
    }

    void push(uint8_t value) {
        std::unique_lock<std::mutex> un_lock(mtx_);
    }


};

class Producer{
private:
    queue *q_ = nullptr;
    std::thread *ptr_thr_ = nullptr;
    uint tasks_;
public:
    Producer(): tasks_(0){}
    explicit Producer(uint tasksNum, queue *q): tasks_(tasksNum), q_(q){}
    void start(std::function<void(Producer&)> func){
        ptr_thr_ = new std::thread(func, std::ref(*this));
    }
    Producer(Producer && obj) noexcept : ptr_thr_(obj.ptr_thr_), tasks_(std::move(obj.tasks_)), q_(std::move(obj.q_)){
    }
    Producer & operator=(Producer && obj){
        if (this == &obj) {
            return *this;
        }
        ptr_thr_ = obj.ptr_thr_;
        tasks_ = obj.tasks_;
        q_ = obj.q_;
        return *this;
    }
    void join(){
        ptr_thr_->join();
        while(tasks_!=0){}
    }
    uint get_tasks() const{
        return tasks_;
    }
    void push_back(){
        q_->push(1);
        tasks_--;
    }
};

class Consumer{
private:
    uint counter_;
    queue *q_ = nullptr;
    std::thread *ptr_thr_ = nullptr;
public:
    Consumer(): counter_(0) {}
    explicit Consumer(queue *q): counter_(0), q_(q){}
    void start(std::function<void(Consumer&, const uint*)> func, const uint *tasksNum){
        ptr_thr_ = new std::thread(func, std::ref(*this), tasksNum);
    }
    Consumer(Consumer && obj) : ptr_thr_(obj.ptr_thr_), counter_(std::move(obj.counter_)), q_(std::move(obj.q_)){}
    Consumer & operator=(Consumer && obj){
        ptr_thr_ = obj.ptr_thr_;
        counter_ = obj.counter_;
        q_ = obj.q_;
        return *this;
    }
    void join(){
        ptr_thr_->join();
    }
    bool pop_front(uint8_t& val){
        counter_++;
        return q_->pop(val);
    }
};

std::function<void(Producer&)> start_creating = ([](Producer &pr) {
    auto start = std::chrono::high_resolution_clock::now(); //type?
    while(pr.get_tasks() > 0){
        pr.push_back();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> delta_t = end - start;
    std::lock_guard<std::mutex> lk(mtx_cout);
    std::cout<<delta_t.count()<<" ms for producer\n";
});

std::function<void(Consumer&, const uint*)> start_receiving = ([](Consumer& con, const uint *tasksNum) {
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        uint8_t value = 0;
        bool is_pop = con.pop_front(value);
        if (is_pop) counter++;
        if (!is_pop && counter == *tasksNum) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> delta_t = end - start;
            std::lock_guard<std::mutex> lk(mtx_cout);
            std::cout<<delta_t.count()<<" ms for consumer\n";
            return;
        }
    }
});

int main() {
    std::vector<int> producerNum = {1, 2, 4}, consumerNum = {1, 2, 4};
    queue q{};
    constexpr uint taskNum = 4*1024*1024;
    for (int prod = 0, con = 0; prod < producerNum.size() && con < consumerNum.size(); ++prod, ++con){
        counter = 0;
        std::vector<Producer> prods(producerNum[prod]);
        std::vector<Consumer> cons(consumerNum[con]);
        std::cout<<"Producers amount: "<<producerNum.at(prod)<<std::endl<<"Consumers amount: "<<consumerNum.at(con)<<std::endl;
        for (Producer& pr: prods){
            pr = Producer(taskNum/producerNum[prod], &q);
            pr.start(start_creating);
        }
        for (Consumer& consumer: cons){
            consumer = Consumer(&q);
            consumer.start(start_receiving, &taskNum);
        }
        for (Producer& pr: prods){
            pr.join();
        }
        for (Consumer& consumer: cons){
            consumer.join();
        }
        //while(counter!=taskNum){}
    }
    return 0;
}
