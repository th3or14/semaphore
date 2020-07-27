#include "semaphore.hpp"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

void run_sample()
{
    static const int threads_cnt = 10;
    static const std::chrono::milliseconds extra_delay_between_threads_creation = 1s;
    // 0 forbids any proceeding beyond wait()
    Semaphore semaphore(0);
    std::vector<std::thread> threads;
    std::vector<size_t> proceeding_order;
    for (int i = 0; i < threads_cnt; ++i)
    {
        if (i > 0)
            std::this_thread::sleep_for(extra_delay_between_threads_creation);
        threads.push_back(std::thread([&semaphore, &proceeding_order, i]
        {
            semaphore.wait();
            proceeding_order.push_back(i);
            semaphore.signal();
        }));
    }
    // allows to proceed beyond wait() sequentially
    semaphore.adjust_passing_limit(1);
    for (auto &t : threads)
        t.join();
    for (size_t i = 0; i < proceeding_order.size(); ++i)
        if (proceeding_order.at(i) != i)
            throw std::runtime_error("detected proceeding order violation: " +
                                     std::to_string(proceeding_order.at(i)) + " != " +
                                     std::to_string(i));
    std::cout << "no order violation detected\n";
}

int main()
{
    run_sample();
    return 0;
}
