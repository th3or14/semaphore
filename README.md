[![CMake on multiple platforms](https://github.com/th3or14/semaphore/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/th3or14/semaphore/actions/workflows/cmake-multi-platform.yml)

## Subject

The proposed semaphore implementation is the class `Semaphore` defined in the files `semaphore.hpp` and `semaphore.cpp`.

## Features

- Fairness is guaranteed by the ticket lock algorithm (i.e., FIFO order). It prevents a thread from being starved out of execution for a long time due to inability to pass through a semaphore in favor of other threads.

- A queue of condition variables is used for the sake of waking up one certain thread using `notify_one()` instead of waking up all the threads using `notify_all()` in case of one condition variable. Benchmarks registered performance boost against the implementation based on one condition variable. Thus it looks like using an extra queue is reasonable.

- The number of waiting threads is implicitly tracked as the size of the queue of condition variables.

## Requirements

* CMake 3.14+

* A C++14 compliant compiler

## Building and Testing

Use `CMakeLists.txt`. You may refer to `.github/workflows/cmake-multi-platform.yml` as an example.
