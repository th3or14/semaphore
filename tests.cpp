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
    explicit OneCondVarSemaphore(size_t resource_count);
    void wait();
    void signal();
    size_t get_number_of_waiting_threads() const;

private:
    std::condition_variable cond_var;
    mutable std::mutex mtx;
    size_t now_serving;
    size_t next_ticket;
    size_t number_of_waiting_threads;
    size_t resource_count;
};

class UnfairSemaphore
{
public:
    explicit UnfairSemaphore(size_t resource_count);
    void wait();
    void signal();
    size_t get_number_of_waiting_threads() const;

private:
    std::condition_variable cond_var;
    mutable std::mutex mtx;
    size_t number_of_waiting_threads;
    size_t resource_count;
};

} // namespace

template <typename T>
static bool run_fairness_check()
{
    T semaphore(0);
    std::vector<std::thread> threads;
    std::vector<size_t> passing_order;
    static const size_t number_of_threads = 1000;
    for (size_t i = 0; i < number_of_threads; ++i)
    {
        threads.push_back(std::thread([&semaphore, &passing_order, i]
        {
            semaphore.wait();
            passing_order.push_back(i);
            semaphore.signal();
        }));
        while (semaphore.get_number_of_waiting_threads() != (i + 1))
            std::this_thread::yield();
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
    for (auto _ : state)
    {
        T semaphore(0);
        std::vector<std::thread> threads;
        static const size_t number_of_threads = 1000;
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            threads.push_back(std::thread([&semaphore]
            {
                semaphore.wait();
                semaphore.signal();
            }));
        }
        while (semaphore.get_number_of_waiting_threads() != number_of_threads)
            std::this_thread::yield();
        semaphore.signal();
        for (auto &t : threads)
            t.join();
    }
}

OneCondVarSemaphore::OneCondVarSemaphore(size_t resource_count) : now_serving(0), next_ticket(0),
    number_of_waiting_threads(0), resource_count(resource_count) {}

void OneCondVarSemaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    size_t my_ticket = next_ticket;
    ++next_ticket;
    ++number_of_waiting_threads;
    cond_var.wait(ul, [=]() -> bool
    {
        return (my_ticket == now_serving) && (resource_count > 0);
    });
    --number_of_waiting_threads;
    --resource_count;
    ++now_serving;
    cond_var.notify_all();
}

void OneCondVarSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++resource_count;
    cond_var.notify_all();
}

size_t OneCondVarSemaphore::get_number_of_waiting_threads() const
{
    std::unique_lock<std::mutex> ul(mtx);
    return number_of_waiting_threads;
}

UnfairSemaphore::UnfairSemaphore(size_t resource_count) : number_of_waiting_threads(0),
    resource_count(resource_count) {}

void UnfairSemaphore::wait()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++number_of_waiting_threads;
    cond_var.wait(ul, [=]() -> bool
    {
        return resource_count > 0;
    });
    --number_of_waiting_threads;
    --resource_count;
}

void UnfairSemaphore::signal()
{
    std::unique_lock<std::mutex> ul(mtx);
    ++resource_count;
    cond_var.notify_all();
}

size_t UnfairSemaphore::get_number_of_waiting_threads() const
{
    std::unique_lock<std::mutex> ul(mtx);
    return number_of_waiting_threads;
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
