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
    uint8_t size_;
    uint8_t count_; //num of elements
    std::condition_variable cv_not_full_;
    std::condition_variable cv_empty_; //if empty
    std::mutex mtx_;
public:
    std::atomic_int count_all_;
    queue(uint capacity): size_(capacity), head_(0), tail_(0), count_(0), count_all_(0) {
        buffer_ = new uint8_t[capacity];
    }
    ~queue() {
        delete [] buffer_;
    }

    bool pop(uint8_t &value) {
        std::unique_lock<std::mutex> un_lock(mtx_);
        if (cv_empty_.wait_for(un_lock,
                               std::chrono::milliseconds(1),
                               [this]() { return count_ != 0; }))
        {
            value = buffer_[head_];
            head_ = (head_ + 1) % size_;
            count_--;
            cv_not_full_.notify_all();
            return true;
        }
        return false;
    }

    void push(uint8_t value) {
        std::unique_lock<std::mutex> un_lock(mtx_);
        cv_not_full_.wait(un_lock, [this](){return count_ < size_;});
        buffer_[tail_] = value;
        tail_ = (tail_ + 1) % size_;
        count_++;
        count_all_++;
        cv_empty_.notify_all();
    }


};

class Producer{
private:
    queue *q_ = nullptr;
    std::thread *ptr_thr_ = nullptr;
    int tasks_;
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
    std::thread *ptr_thr_ = nullptr;
public:
    queue *q_ = nullptr;
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
    void join(){ ptr_thr_->join(); }
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
        if (con.q_->count_all_ >= *tasksNum && !is_pop) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> delta_t = end - start;
            std::lock_guard<std::mutex> lk(mtx_cout);
            std::cout<<delta_t.count()<<" ms for consumer\n";
            return;
        }
    }
});

int main() {
    constexpr uint taskNum = 4*1024*1024;
    std::vector<int> producerNum{1, 2, 4}, consumerNum{1, 2, 4};
    std::vector<uint> vec_capacity {1, 4, 16};
    for (uint& capacity: vec_capacity) {
        std::cout<<"for capacity = "<<capacity<<std::endl;
        for (int prod = 0, con = 0; prod < producerNum.size() && con < consumerNum.size(); ++prod, ++con) {
            std::vector<Producer> prods(producerNum[prod]);
            std::vector<Consumer> cons(consumerNum[con]);
            queue q(capacity);
            std::cout<<"Producers amount: "<<producerNum.at(prod)<<" Consumers amount: "<<consumerNum.at(con)<<std::endl;
            for (Producer &pr: prods) {
                pr = Producer(taskNum / producerNum[prod], &q);
                pr.start(start_creating);
            }
            for (Consumer &consumer: cons) {
                consumer = Consumer(&q);
                consumer.start(start_receiving, &taskNum);
            }
            for (Producer &pr: prods) {
                pr.join();
            }
            for (Consumer &consumer: cons) {
                consumer.join();
            }
        }
    }
    return 0;
}
