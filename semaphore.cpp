#include "semaphore.hpp"

Semaphore::Semaphore(size_t passing_limit) : now_serving(0), next_ticket(0), passing_cnt(0),
    passing_limit(passing_limit) {}

void Semaphore::set_passing_limit(size_t limit)
{
    std::unique_lock<std::mutex> ul(mtx);
    passing_limit = limit;
    if (!cond_vars.empty())
        cond_vars.front()->notify_one();
}

size_t Semaphore::get_passing_limit() const
{
    std::unique_lock<std::mutex> ul(mtx);
    return passing_limit;
}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    size_t my_ticket = next_ticket;
    next_ticket++;
    cond_vars.push(std::make_unique<std::condition_variable>());
    cond_vars.back()->wait(ul, [=]
    {
        return my_ticket == now_serving && passing_cnt < passing_limit;
    });
    cond_vars.pop();
    passing_cnt++;
    now_serving++;
    if (!cond_vars.empty())
        cond_vars.front()->notify_one();
}

void Semaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    passing_cnt--;
    if (!cond_vars.empty())
        cond_vars.front()->notify_one();
}
