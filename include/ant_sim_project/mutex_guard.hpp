#pragma once

#include <mutex>
#include <utility>
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

template <typename T, typename Mutex = std::mutex>
class mutex_with_data {
    T t;

    // Allow locking a const mutex_with_data
    mutable Mutex mutex;

  public:
    mutex_with_data() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
    = default;
    explicit mutex_with_data(T t) noexcept(std::is_nothrow_move_constructible_v<T>) : t{std::move(t)} {}
    explicit mutex_with_data(std::in_place_t, auto&&... args) : t(std::forward<decltype(args)>(args)...) {}

    [[nodiscard]] auto lock(this auto&& self) { return mutex_guard{&self.t, self.mutex}; }

    // Returns a reference to the protected data, without locking
    // Use with care
    [[nodiscard]] auto& get_unsafe(this auto&& self) { return self.t; }
};
