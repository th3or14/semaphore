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
    explicit OneCondVarSemaphore(size_t resource_count = 1);
    void wait();
    void signal();

private:
    std::condition_variable cond_var;
    std::mutex mtx;
    size_t now_serving;
    size_t next_ticket;
    size_t resource_count;
};

class UnfairSemaphore
{
public:
    explicit UnfairSemaphore(size_t resource_count = 1);
    void wait();
    void signal();

private:
    std::condition_variable cond_var;
    std::mutex mtx;
    size_t resource_count;
};

} // namespace

template <typename T>
static bool run_fairness_check()
{
    T semaphore(0);
    std::vector<std::thread> threads;
    std::vector<size_t> passing_order;
    static const int number_of_threads = 30;
    for (int i = 0; i < number_of_threads; ++i)
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
    semaphore.signal();
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
        static const int number_of_threads = 1000;
        for (int i = 0; i < number_of_threads; ++i)
        {
            threads.push_back(std::thread([&semaphore]
            {
                semaphore.wait();
                semaphore.signal();
            }));
        }
        semaphore.signal();
        for (auto &t : threads)
            t.join();
    }
}

OneCondVarSemaphore::OneCondVarSemaphore(size_t resource_count) : now_serving(0), next_ticket(0),
    resource_count(resource_count) {}

void OneCondVarSemaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    size_t my_ticket = next_ticket;
    ++next_ticket;
    cond_var.wait(ul, [=]() -> bool
    {
        return (my_ticket == now_serving) && (resource_count > 0);
    });
    --resource_count;
    ++now_serving;
}

void OneCondVarSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++resource_count;
    cond_var.notify_all();
}

UnfairSemaphore::UnfairSemaphore(size_t resource_count) : resource_count(resource_count) {}

void UnfairSemaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    cond_var.wait(ul, [=]() -> bool
    {
        return resource_count > 0;
    });
    --resource_count;
}

void UnfairSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++resource_count;
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
