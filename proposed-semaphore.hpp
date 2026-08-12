#ifndef PROPOSED_SEMAPHORE_HPP
#define PROPOSED_SEMAPHORE_HPP

#include <condition_variable>
#include <queue>

class ProposedSemaphore {
public:
    explicit ProposedSemaphore(size_t resource_count);
    void wait();
    void signal();
    size_t get_number_of_waiting_threads() const;

private:
    std::queue<std::unique_ptr<std::condition_variable>> cond_vars;
    mutable std::mutex mtx;
    size_t now_serving;
    size_t next_ticket;
    size_t resource_count;
};

#endif // PROPOSED_SEMAPHORE_HPP
