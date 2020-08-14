#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <future>
#include <queue>

class Semaphore
{
public:
    explicit Semaphore(size_t passing_limit = 1);
    void adjust_passing_limit(size_t limit);
    void wait();
    void signal();

private:
    size_t now_serving;
    size_t next_ticket;
    size_t passing_cnt;
    size_t passing_limit;
    std::queue<std::unique_ptr<std::condition_variable>> cond_vars;
    std::mutex mtx;
};

#endif // SEMAPHORE_HPP
