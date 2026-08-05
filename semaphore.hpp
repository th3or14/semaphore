#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <future>
#include <queue>

class Semaphore
{
public:
    explicit Semaphore(size_t resource_count = 1);
    void wait();
    void signal();

private:
    std::queue<std::unique_ptr<std::condition_variable>> cond_vars;
    std::mutex mtx;
    size_t now_serving;
    size_t next_ticket;
    size_t resource_count;
};

#endif // SEMAPHORE_HPP
