#include "tests.hpp"

#include <iostream>

using namespace std::chrono_literals;

int main()
{
    // if this delay is not sufficient, then some newer thread may acuqire a semaphore before
    // another thread created earlier, which results in race condition and fariness check violation
    static const std::chrono::milliseconds delay_between_threads_creation = 100ms;
    static const int fairness_check_threads_cnt = 10;
    static const int performance_benchmark_threads_cnt = 1000;
    std::cout << "proposed semaphore implementation tests:\n";
    if (run_proposed_impl_fairness_check(fairness_check_threads_cnt,
                                         delay_between_threads_creation))
        std::cout << "\t1) fairness check passed\n";
    else
    {
        std::cout << "\t1) fairness check failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "\t2) benchmark took " <<
                 run_proposed_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    std::cout << "alternative semaphore implementation tests:\n";
    if (run_alternative_impl_fairness_check(fairness_check_threads_cnt,
                                            delay_between_threads_creation))
        std::cout << "\t1) fairness check passed\n";
    else
    {
        std::cout << "\t1) fairness check failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "\t2) benchmark took " <<
                 run_alternative_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    std::cout << "unfair semaphore implementation tests:\n";
    if (run_unfair_impl_fairness_check(fairness_check_threads_cnt,
                                       delay_between_threads_creation))
    {
        std::cout << "\t1) fairness check passed (may happen by accident, worth retrying)\n";
        return EXIT_FAILURE;
    }
    else
        std::cout << "\t1) fairness check failed (expectedly for unfair semaphore)\n";
    std::cout << "\t2) benchmark took " <<
                 run_unfair_impl_performance_benchmark(
                     performance_benchmark_threads_cnt).count() << "ms\n";
    return EXIT_SUCCESS;
}
