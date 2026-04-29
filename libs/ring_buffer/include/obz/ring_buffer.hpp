#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace obz {

template <typename T>
class ring_buffer {
public:
    explicit ring_buffer(std::size_t capacity)
        : capacity_(capacity),
          buffer_(std::allocator<T>{}.allocate(capacity)) {
        if (capacity_ == 0) {
            throw std::invalid_argument("ring_buffer capacity must be greater than zero");
        }
    }

    ring_buffer(const ring_buffer&) = delete;
    ring_buffer& operator=(const ring_buffer&) = delete;

    ring_buffer(ring_buffer&&) = delete;
    ring_buffer& operator=(ring_buffer&&) = delete;

    ~ring_buffer() {
        clear();
        std::allocator<T>{}.deallocate(buffer_, capacity_);
    }

    bool push_back(const T& value) {
        return emplace_back(value);
    }

    bool push_back(T&& value) {
        return emplace_back(std::move(value));
    }

    template <typename... Args>
    bool emplace_back(Args&&... args) {
        if (full()) {
            return false;
        }

        std::construct_at(slot(tail_), std::forward<Args>(args)...);

        tail_ = next_index(tail_);
        ++size_;

        return true;
    }

    bool pop_front(T& value) {
        if (empty()) {
            return false;
        }

        T* current = slot(head_);

        value = std::move(*current);
        std::destroy_at(current);

        head_ = next_index(head_);
        --size_;

        return true;
    }

    T& front() {
        if (empty()) {
            throw std::runtime_error("cannot access front of empty ring_buffer");
        }

        return *slot(head_);
    }

    const T& front() const {
        if (empty()) {
            throw std::runtime_error("cannot access front of empty ring_buffer");
        }

        return *slot(head_);
    }

    void clear() {
        while (!empty()) {
            std::destroy_at(slot(head_));
            head_ = next_index(head_);
            --size_;
        }

        head_ = 0;
        tail_ = 0;
    }

    bool empty() const {
        return size_ == 0;
    }

    bool full() const {
        return size_ == capacity_;
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return capacity_;
    }

private:
    T* slot(std::size_t index) {
        return buffer_ + index;
    }

    const T* slot(std::size_t index) const {
        return buffer_ + index;
    }

    std::size_t next_index(std::size_t index) const {
        return (index + 1) % capacity_;
    }

    const std::size_t capacity_;
    T* buffer_;

    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
};

} // namespace obz