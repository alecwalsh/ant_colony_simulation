#pragma once

#include <mutex>
#include <cassert>

// Wraps a pointer to an object and a unique_lock protecting that object
// Similar to Rust's std::sync::MutexGuard
template <typename T, typename Mutex = std::mutex>
class mutex_guard {
    T* pointer;
    std::unique_lock<Mutex> lock;
public:
    mutex_guard(T* pointer, Mutex& mutex) : pointer{pointer}, lock{mutex} {}

    T* operator->() const noexcept {
        assert(lock.owns_lock());
        return pointer;
    }

    T& operator*() const noexcept {
        assert(lock.owns_lock());
        return *pointer;
    }

    // Unlocks the mutex
    // Users must make sure that no references or pointers to the protected object are used afterwards
    void unlock() noexcept {
        if(lock.owns_lock()) {
            lock.unlock();
        }
    }
};