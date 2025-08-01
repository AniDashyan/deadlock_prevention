# Synchronization and Deadlock Prevention

## Overview

This project demonstrates different multithreading strategies in C++ for handling shared resources using mutexes. It explores:

* Safe access to a single resource.
* A deadlock scenario using naive locking.
* Deadlock avoidance using `std::scoped_lock`.
* Deadlock avoidance using consistent lock ordering.

## Build & Run

Clone and build the project:

```bash
# Clone the repo
git clone https://github.com/AniDashyan/deadlock_prevention
cd deadlock_prevention

# Build the project
cmake -S . -B build
cmake --build build

# Run the executable
./build/deadlock --method [method] --iters [iters]
```

**Options:**

* `--method`:
  * `1` - Safe access to a single resource
  * `2` - Simulate a deadlock (locks A then B and B then A)
  * `3` - Use `std::scoped_lock` to avoid deadlock
  * `4` - Use consistent lock ordering to avoid deadlock
* `--iters`: Number of iterations each thread should run.

**Defaults:**
* `method = 1`
* `iters = 1,000,000`
---

## Example Output

### Running safe single resource access (`--method 1 --iters 10`)

```bash
Thread 1 accessing resource A: 100
Thread 2 accessing resource A: 110
Execution time: 1 ms
```

### Running deadlock (`--method 2`)

```bash
Thread 1: Locking A...
Thread 1: Got A, now trying B...
Thread 2: Locking B...
Thread 2: Got B, now trying A...
...
```

### Running scoped lock (`--method 3 --iters 10`)

```bash
Thread 1 safely got both resources
Thread 2 safely got both resources
Execution time: 2 ms
```

### Running ordered lock (`--method 4 --iters 10`)

```bash
Thread 1 got locks in order
Thread 2 got locks in order
Execution time: 2 ms
```

---

## How Does It Work?

The program demonstrates thread synchronization using mutexes and shows the consequences of incorrect locking order:

### Method 1: Safe Single Resource Access

Each thread safely accesses only `resource_a` using `std::lock_guard` on `mutex_a`. Since no resource contention exists, there's no risk of deadlock.

### Method 2: Simulated Deadlock

Two threads acquire locks in reverse order:

* Thread 1: locks `mutex_a`, then waits for `mutex_b`.
* Thread 2: locks `mutex_b`, then waits for `mutex_a`.

This causes a **classic deadlock** since each thread waits indefinitely for the other.

### Method 3: Scoped Lock (`std::scoped_lock`)

Introduced in C++17, `std::scoped_lock` can lock multiple mutexes *atomically in a deadlock-free manner*, ensuring a consistent locking order internally.

```cpp
std::scoped_lock lock(mutex_a, mutex_b);
```

This method avoids deadlock without requiring manual lock ordering.

### Method 4: Ordered Locking with `std::lock_guard`

By always locking `mutex_a` before `mutex_b`, all threads follow a consistent locking strategy:

```cpp
std::lock_guard<std::mutex> lock_a(mutex_a);
std::lock_guard<std::mutex> lock_b(mutex_b);
```

This prevents circular wait conditions, thus avoiding deadlock.
