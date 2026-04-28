#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace obz {

template <typename T>
class blocking_queue {
public:
    blocking_queue() = default;

    blocking_queue(const blocking_queue&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;

    blocking_queue(blocking_queue&&) = delete;
    blocking_queue& operator=(blocking_queue&&) = delete;

    ~blocking_queue() {
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
            std::lock_guard lock(mutex_);

            if (closed_) {
                throw std::runtime_error("cannot push to closed blocking_queue");
            }

            queue_.emplace_back(std::forward<Args>(args)...);
        }

        condition_.notify_one();
    }

    bool wait_and_pop(T& value) {
        std::unique_lock lock(mutex_);

        condition_.wait(lock, [this] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty()) {
            return false;
        }

        value = std::move(queue_.front());
        queue_.pop_front();

        return true;
    }

    bool try_pop(T& value) {
        std::lock_guard lock(mutex_);

        if (queue_.empty()) {
            return false;
        }

        value = std::move(queue_.front());
        queue_.pop_front();

        return true;
    }

    void close() {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }

        condition_.notify_all();
    }

    bool closed() const {
        std::lock_guard lock(mutex_);
        return closed_;
    }

    bool empty() const {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    std::size_t size() const {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<T> queue_;
    bool closed_ = false;
};

} // namespace obz