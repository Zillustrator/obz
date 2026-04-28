#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace obz {

template <typename T>
class bounded_blocking_queue {
public:
    explicit bounded_blocking_queue(std::size_t capacity)
        : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("bounded_blocking_queue capacity must be greater than zero");
        }
    }

    bounded_blocking_queue(const bounded_blocking_queue&) = delete;
    bounded_blocking_queue& operator=(const bounded_blocking_queue&) = delete;

    bounded_blocking_queue(bounded_blocking_queue&&) = delete;
    bounded_blocking_queue& operator=(bounded_blocking_queue&&) = delete;

    ~bounded_blocking_queue() {
        close();
    }

    void push(const T& value) {
        emplace(value);
    }

    void push(T&& value) {
        emplace(std::move(value));
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        {
            std::unique_lock lock(mutex_);

            not_full_.wait(lock, [this] {
                return closed_ || queue_.size() < capacity_;
            });

            if (closed_) {
                throw std::runtime_error("cannot push to closed bounded_blocking_queue");
            }

            queue_.emplace_back(std::forward<Args>(args)...);
        }

        not_empty_.notify_one();
    }

    bool try_push(const T& value) {
        return try_emplace(value);
    }

    bool try_push(T&& value) {
        return try_emplace(std::move(value));
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        {
            std::lock_guard lock(mutex_);

            if (closed_ || queue_.size() >= capacity_) {
                return false;
            }

            queue_.emplace_back(std::forward<Args>(args)...);
        }

        not_empty_.notify_one();
        return true;
    }

    bool wait_and_pop(T& value) {
        {
            std::unique_lock lock(mutex_);

            not_empty_.wait(lock, [this] {
                return closed_ || !queue_.empty();
            });

            if (queue_.empty()) {
                return false;
            }

            value = std::move(queue_.front());
            queue_.pop_front();
        }

        not_full_.notify_one();
        return true;
    }

    bool try_pop(T& value) {
        {
            std::lock_guard lock(mutex_);

            if (queue_.empty()) {
                return false;
            }

            value = std::move(queue_.front());
            queue_.pop_front();
        }

        not_full_.notify_one();
        return true;
    }

    void close() {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }

        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool closed() const {
        std::lock_guard lock(mutex_);
        return closed_;
    }

    bool empty() const {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    bool full() const {
        std::lock_guard lock(mutex_);
        return queue_.size() >= capacity_;
    }

    std::size_t size() const {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    std::size_t capacity() const {
        return capacity_;
    }

private:
    const std::size_t capacity_;

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<T> queue_;
    bool closed_ = false;
};

} // namespace obz