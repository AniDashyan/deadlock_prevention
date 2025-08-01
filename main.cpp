#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <utility>

#include "kaizen.h"

// Two shared resources
int resource_a = 100;
int resource_b = 200;

// Two mutexes to protect the resources
std::mutex mutex_a;
std::mutex mutex_b;

// Safe access to single resource
void safe_single_access(int id, int iters) {
    for (int i = 0; i < iters; i++) {
        std::lock_guard<std::mutex> lock(mutex_a);
        
        if (i == 0) {  // Only print first iteration to reduce output
            std::cout << "Thread " << id << " accessing resource A: " << resource_a << std::endl;
        }
        resource_a += 1;
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

// Deadlock, Thread 1 locks A then B
void deadlock_thread1() {
    std::cout << "Thread 1: Locking A..." << std::endl;
    mutex_a.lock();
    std::cout << "Thread 1: Got A, now trying B..." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex_b.lock();  // waits forever
    std::cout << "Thread 1: Got both!" << std::endl;
    
    resource_a += 10;
    resource_b += 10;
    
    mutex_b.unlock();
    mutex_a.unlock();
}

// Deadlock, Thread 2 locks B then A
void deadlock_thread2() {
    std::cout << "Thread 2: Locking B..." << std::endl;
    mutex_b.lock();
    std::cout << "Thread 2: Got B, now trying A..." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    mutex_a.lock();  // waits forever
    std::cout << "Thread 2: Got both!" << std::endl;
    
    resource_a += 20;
    resource_b += 20;
    
    mutex_a.unlock();
    mutex_b.unlock();
}

// Fix deadlock using scoped_lock
void scoped_lock_access(int id, int iters) {
    for (int i = 0; i < iters; i++) {
        std::scoped_lock lock(mutex_a, mutex_b);
        
        if (i == 0) {  // Only print first iteration
            std::cout << "Thread " << id << " safely got both resources" << std::endl;
        }
        
        // Modify both resources
        resource_a += 1;
        resource_b += 1;
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

// Fix deadlock using consistent lock ordering
void ordered_lock_access(int id, int iterations) {
    for (int i = 0; i < iterations; i++) {
        // Always lock A first, then B
        std::lock_guard<std::mutex> lock_a(mutex_a);
        std::lock_guard<std::mutex> lock_b(mutex_b);
        
        if (i == 0) {
            std::cout << "Thread " << id << " got locks in order" << std::endl;
        }
        
        resource_a += 1;
        resource_b += 1;
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

// Print usage information
void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << "--method [method] --iters [iters]" << std::endl;
    std::cout << "Methods:" << std::endl;
    std::cout << "  1 - Single resource access (safe)" << std::endl;
    std::cout << "  2 - Deadlock demo (simulated)" << std::endl;
    std::cout << "  3 - Scoped lock method" << std::endl;
    std::cout << "  4 - Ordered locks method" << std::endl;
    std::cout << "Default: method=1 iters=1000" << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " 3" << std::endl;
    std::cout << "  " << program_name << " 4 10000" << std::endl;
}

std::pair<int, int> parse_args(int argc, char* argv[]) {
    int method = 1;  // Default method
    int num_threads = 1'000'000;  // Default iterations
    
    zen::cmd_args args(argv, argc);
    if (!args.is_present("--method") || !args.is_present("--iters")) {
        zen::log(zen::color::yellow("No --method or --iters provided. Using default values: "));
        return {method, num_threads}; 
    }
    else {
        method = std::stoi(args.get_options("--method")[0]);
        num_threads = std::stoi(args.get_options("--iters")[0]);
    }
    return {method, num_threads};
}


int main(int argc, char* argv[]) {
    
    print_usage(argv[0]);
    auto [method, iters] = parse_args(argc, argv); 

    auto start = std::chrono::steady_clock::now();
    std::thread t1, t2;
    switch (method) {
        case 1:
            t1 = std::thread(safe_single_access, 1, iters);
            t2 = std::thread(safe_single_access, 2, iters);
            break;
        case 2:
            t1 = std::thread(deadlock_thread1);
            t2 = std::thread(deadlock_thread2);
            std::cout << "Deadlock occurred, threads did not complete." << std::endl;
            break;
        case 3:
            t1 = std::thread(scoped_lock_access, 1, iters);
            t2 = std::thread(scoped_lock_access, 2, iters);
            break;
        case 4:
            t1 = std::thread(ordered_lock_access, 1, iters);
            t2 = std::thread(ordered_lock_access, 2, iters);
            break;
        default:
            std::cout << "Invalid method: " << method << std::endl;
            std::cout << "Valid methods: 1, 2, 3, 4" << std::endl;
            print_usage(argv[0]);
            return 1;
        }

    t1.join();
    t2.join();

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Execution time: " << duration << " ms" << std::endl;
    return 0;
}