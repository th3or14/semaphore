## Features

- Fairness is guaranteed by the ticket lock algorithm (i.e., FIFO order). It prevents a thread from being starved out of execution for a long time due to inability to pass through a semaphore in favor of other threads.

- Instead of just setting an initial counter value you are allowed to adjust the limit of passing threads.
