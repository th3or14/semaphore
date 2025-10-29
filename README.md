## Features

- Fairness is guaranteed by the ticket lock algorithm (i.e., FIFO order). It prevents a thread from being starved out of execution for a long time due to inability to pass through a semaphore in favor of other threads.

- Instead of just setting the initial resource count value of a counting semaphore you are allowed to adjust the limit of passing threads.

- A queue of condition variables is used for the sake of waking up one certain thread using `notify_one()` instead of waking up all the threads using `notify_all()` in case of one condition variable. Benchmarks registered performance boost against the implementation based on one condition variable. Thus it looks like using an extra queue is reasonable.

## Requirements

* CMake 3.7+

* A C++14 compliant compiler

## Building

Use `CMakeLists.txt`.
