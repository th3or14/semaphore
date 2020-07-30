#include "tests.hpp"

namespace {

template <typename T>
class SemaphoreInterface
{
public:
    SemaphoreInterface(size_t passing_limit);
    void adjust_passing_limit(size_t limit);
    void wait();
    void signal();

private:
    T impl;
};

template <typename T>
SemaphoreInterface<T>::SemaphoreInterface(size_t passing_limit) : impl(passing_limit) {}

template <typename T>
void SemaphoreInterface<T>::adjust_passing_limit(size_t limit)
{
    impl.adjust_passing_limit(limit);
}

template <typename T>
void SemaphoreInterface<T>::wait()
{
    impl.wait();
}

template <typename T>
void SemaphoreInterface<T>::signal()
{
    impl.signal();
}

class Semaphore2
{
public:
    explicit Semaphore2(size_t passing_limit = 1);
    void adjust_passing_limit(size_t limit);
    void wait();
    void signal();

private:
    size_t now_serving;
    size_t next_ticket;
    size_t passing_cnt;
    size_t passing_limit;
    std::condition_variable cond_var;
    mutable std::mutex mtx;
};

Semaphore2::Semaphore2(size_t passing_limit) : now_serving(0), next_ticket(0), passing_cnt(0),
    passing_limit(passing_limit) {}

void Semaphore2::adjust_passing_limit(size_t limit)
{
    std::unique_lock<std::mutex> ul(mtx);
    passing_limit = limit;
    cond_var.notify_all();
}

void Semaphore2::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    size_t my_ticket = next_ticket;
    ++next_ticket;
    cond_var.wait(ul, [=]() -> bool
    {
        return (my_ticket == now_serving) && (passing_cnt < passing_limit);
    });
    ++passing_cnt;
    ++now_serving;
    cond_var.notify_all();
}

void Semaphore2::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    if (passing_cnt == 0)
        throw std::logic_error("nothing to signal");
    --passing_cnt;
    cond_var.notify_all();
}

template <typename T>
std::chrono::milliseconds run_performance_benchmark(int threads_cnt)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    SemaphoreInterface<T> semaphore(0);
    std::vector<std::thread> threads;
    for (int i = 0; i < threads_cnt; ++i)
    {
        threads.push_back(std::thread([&semaphore]
        {
            semaphore.wait();
            semaphore.signal();
        }));
    }
    semaphore.adjust_passing_limit(1);
    for (auto &t : threads)
        t.join();
    auto t2 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
}

} // namespace

bool run_fairness_check(int threads_cnt, std::chrono::milliseconds delay_between_threads_creation)
{
    Semaphore semaphore(0);
    std::vector<std::thread> threads;
    std::vector<size_t> passing_order;
    for (int i = 0; i < threads_cnt; ++i)
    {
        if (i > 0)
            std::this_thread::sleep_for(delay_between_threads_creation);
        threads.push_back(std::thread([&semaphore, &passing_order, i]
        {
            semaphore.wait();
            passing_order.push_back(i);
            semaphore.signal();
        }));
    }
    semaphore.adjust_passing_limit(1);
    for (auto &t : threads)
        t.join();
    for (size_t i = 0; i < passing_order.size(); ++i)
        if (passing_order.at(i) != i)
            return false;
    return true;
}

std::chrono::milliseconds run_proposed_impl_performance_benchmark(int threads_cnt)
{
    return run_performance_benchmark<Semaphore>(threads_cnt);
}

std::chrono::milliseconds run_alternative_impl_performance_benchmark(int threads_cnt)
{
    return run_performance_benchmark<Semaphore2>(threads_cnt);
}
