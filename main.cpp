#include "tests.hpp"

int main()
{
    // if this delay is not sufficient, then some newer thread may acuqire a semaphore before
    // another thread created earlier, which results in race condition and fariness check violation
    static const std::chrono::milliseconds delay_between_threads_creation = 100ms;
    static const int fairness_check_threads_cnt = 10;
    static const int performance_benchmark_threads_cnt = 1000;
    std::cout << "proposed semaphore (ticket lock algorithm, queue of condition variables) tests:\n";
    std::cout << (run_proposed_impl_fairness_check(fairness_check_threads_cnt, delay_between_threads_creation) ?
                      "\t1) fairness check passed\n" :
                      "\t1) fairness check failed\n");
    std::cout << "\t2) benchmark took " <<
                 run_proposed_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    std::cout << "alternative semaphore (ticket lock algorithm, single condition variable) tests:\n";
    std::cout << (run_alternative_impl_fairness_check(fairness_check_threads_cnt, delay_between_threads_creation) ?
                      "\t1) fairness check passed\n" :
                      "\t1) fairness check failed\n");
    std::cout << "\t2) benchmark took " <<
                 run_alternative_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    std::cout << "unfair semaphore (no ticket lock algorithm) tests:\n";
    std::cout << (run_unfair_impl_fairness_check(fairness_check_threads_cnt, delay_between_threads_creation) ?
                      "\t1) fairness check passed\n" :
                      "\t1) fairness check failed (expectedly for unfair semaphore)\n");
    std::cout << "\t2) benchmark took " <<
                 run_unfair_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    return EXIT_SUCCESS;
}
