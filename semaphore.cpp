#include "semaphore.hpp"

Semaphore::Semaphore(size_t resource_count) : now_serving(0), next_ticket(0),
    resource_count(resource_count) {}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    size_t my_ticket = next_ticket;
    ++next_ticket;
    cond_vars.push(std::make_unique<std::condition_variable>());
    cond_vars.back()->wait(ul, [=]() -> bool
    {
        // in spite of condition variables are notified one by one in the right order in the queue,
        // this predicate is still needed for protecting from spurious wakeups
        return (my_ticket == now_serving) && (resource_count > 0);
    });
    cond_vars.pop();
    --resource_count;
    ++now_serving;
}

void Semaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++resource_count;
    if (!cond_vars.empty())
        cond_vars.front()->notify_one();
}

size_t Semaphore::get_number_of_waiting_threads() const
{
    std::unique_lock<std::mutex> ul(mtx);
    return cond_vars.size();
}
