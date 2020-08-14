#include "tests.hpp"

int main()
{
    // if this delay is not sufficient then some newer thread may acuqire a semaphore before
    // another thread created earlier which results in race condition and fariness check violation
    static const std::chrono::milliseconds delay_between_threads_creation = 100ms;
    static const int fairness_check_threads_cnt = 10;
    std::cout << (run_fairness_check(fairness_check_threads_cnt, delay_between_threads_creation) ?
                      "fairness check passed" :
                      "fairness check failed") << "\n";
    static const int performance_benchmark_threads_cnt = 1000;
    std::cout << "proposed semaphore benchmark (queue of condition variables is used): " <<
                 run_proposed_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    std::cout << "alternative semaphore benchmark (single condition variable is used): " <<
                 run_alternative_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    return EXIT_SUCCESS;
}
