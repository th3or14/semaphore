#ifndef TESTS_HPP
#define TESTS_HPP

#include "semaphore.hpp"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

bool run_fairness_check(int threads_cnt, std::chrono::milliseconds delay_between_threads_creation);

std::chrono::milliseconds run_proposed_impl_performance_benchmark(int threads_cnt);

std::chrono::milliseconds run_alternative_impl_performance_benchmark(int threads_cnt);

#endif // TESTS_HPP
