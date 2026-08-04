#include "semaphore.hpp"

#include <gtest/gtest.h>
#include <benchmark/benchmark.h>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

using ProposedSemaphore = Semaphore;

namespace {

class OneCondVarSemaphore
{
public:
    explicit OneCondVarSemaphore(size_t passing_limit = 1);
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

class UnfairSemaphore
{
public:
    explicit UnfairSemaphore(size_t passing_limit = 1);
    void adjust_passing_limit(size_t limit);
    void wait();
    void signal();

private:
    size_t passing_cnt;
    size_t passing_limit;
    std::condition_variable cond_var;
    mutable std::mutex mtx;
};

} // namespace

template <typename T>
static bool run_fairness_check()
{
    T semaphore(0);
    std::vector<std::thread> threads;
    std::vector<size_t> passing_order;
    static const int fairness_check_threads_cnt = 30;
    for (int i = 0; i < fairness_check_threads_cnt; ++i)
    {
        threads.push_back(std::thread([&semaphore, &passing_order, i]
        {
            semaphore.wait();
            passing_order.push_back(i);
            semaphore.signal();
        }));
        // if this delay is not sufficient, then some newer thread may call semaphore.wait() before
        // another thread created earlier, which results in a race condition making this check fail
        static const std::chrono::milliseconds delay_between_threads_creation = 100ms;
        std::this_thread::sleep_for(delay_between_threads_creation);
    }
    semaphore.adjust_passing_limit(1);
    for (auto &t : threads)
        t.join();
    for (size_t i = 0; i < passing_order.size(); ++i)
        if (passing_order.at(i) != i)
            return false;
    return true;
}

template <typename T>
static void run_performance_benchmark(benchmark::State& state)
{
    for (auto _ : state) {
        T semaphore(0);
        std::vector<std::thread> threads;
        static const int performance_benchmark_threads_cnt = 1000;
        for (int i = 0; i < performance_benchmark_threads_cnt; ++i)
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
    }
}

OneCondVarSemaphore::OneCondVarSemaphore(size_t passing_limit) : now_serving(0), next_ticket(0),
    passing_cnt(0), passing_limit(passing_limit) {}

void OneCondVarSemaphore::adjust_passing_limit(size_t limit)
{
    std::unique_lock<std::mutex> ul(mtx);
    bool is_new_limit_greater = limit > passing_limit;
    passing_limit = limit;
    if (is_new_limit_greater)
        cond_var.notify_all();
}

void OneCondVarSemaphore::wait()
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
}

void OneCondVarSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    if (passing_cnt == 0)
        throw std::logic_error("nothing to signal");
    --passing_cnt;
    cond_var.notify_all();
}

UnfairSemaphore::UnfairSemaphore(size_t passing_limit) : passing_cnt(0),
    passing_limit(passing_limit) {}

void UnfairSemaphore::adjust_passing_limit(size_t limit)
{
    std::unique_lock<std::mutex> ul(mtx);
    bool is_new_limit_greater = limit > passing_limit;
    passing_limit = limit;
    if (is_new_limit_greater)
        cond_var.notify_all();
}

void UnfairSemaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    cond_var.wait(ul, [=]() -> bool
    {
        return passing_cnt < passing_limit;
    });
    ++passing_cnt;
}

void UnfairSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    if (passing_cnt == 0)
        throw std::logic_error("nothing to signal");
    --passing_cnt;
    cond_var.notify_all();
}

TEST(FairnessCheck, PassesForProposedSemaphore)
{
    EXPECT_TRUE(run_fairness_check<ProposedSemaphore>());
}

TEST(FairnessCheck, PassesForOneCondVarSemaphore)
{
    EXPECT_TRUE(run_fairness_check<OneCondVarSemaphore>());
}

TEST(FairnessCheck, FailsForUnfairSemaphore)
{
    EXPECT_FALSE(run_fairness_check<UnfairSemaphore>());
}

BENCHMARK(run_performance_benchmark<ProposedSemaphore>);
BENCHMARK(run_performance_benchmark<OneCondVarSemaphore>);
BENCHMARK(run_performance_benchmark<UnfairSemaphore>);

TEST(PerformanceBenchmark, RunAllRegisteredBenchmarks)
{
    benchmark::RunSpecifiedBenchmarks();
}
