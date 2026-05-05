#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace obz {

template <typename T, std::size_t Capacity>
class mpsc_ring_queue {
public:
    static_assert(Capacity > 0, "Capacity must be > 0");

    mpsc_ring_queue() {
        for (std::size_t index = 0; index < Capacity; ++index) {
            buffer_[index].sequence.store(index, std::memory_order_relaxed);
        }
    }

    mpsc_ring_queue(const mpsc_ring_queue&) = delete;
    mpsc_ring_queue& operator=(const mpsc_ring_queue&) = delete;

    mpsc_ring_queue(mpsc_ring_queue&&) = delete;
    mpsc_ring_queue& operator=(mpsc_ring_queue&&) = delete;

    ~mpsc_ring_queue() {
        clear();
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        static_assert(std::is_nothrow_constructible_v<T, Args...>,
                      "mpsc_ring_queue::try_emplace requires nothrow construction");

        auto write = write_index_.value.load(std::memory_order_relaxed);
        cell* current = nullptr;

        while (true) {
            current = &buffer_[write % Capacity];

            const auto sequence = current->sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::ptrdiff_t>(sequence) -
                              static_cast<std::ptrdiff_t>(write);

            if (diff == 0) {
                if (write_index_.value.compare_exchange_weak(write,
                                                             write + 1,
                                                             std::memory_order_relaxed,
                                                             std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return false; // full
            } else {
                write = write_index_.value.load(std::memory_order_relaxed);
            }
        }

        std::construct_at(current->slot(), std::forward<Args>(args)...);
        current->sequence.store(write + 1, std::memory_order_release);

        return true;
    }

    bool try_push(const T& value) {
        return try_emplace(value);
    }

    bool try_push(T&& value) {
        return try_emplace(std::move(value));
    }

    bool try_pop(T& value) {
        static_assert(std::is_nothrow_move_assignable_v<T>,
                      "mpsc_ring_queue::try_pop requires nothrow move assignment");

        const auto read = read_index_.value.load(std::memory_order_relaxed);
        cell& current = buffer_[read % Capacity];

        const auto sequence = current.sequence.load(std::memory_order_acquire);
        const auto diff = static_cast<std::ptrdiff_t>(sequence) -
                          static_cast<std::ptrdiff_t>(read + 1);

        if (diff != 0) {
            return false; // empty, or the next claimed slot has not been published yet
        }

        T* object = current.slot();

        value = std::move(*object);
        std::destroy_at(object);

        read_index_.value.store(read + 1, std::memory_order_relaxed);
        current.sequence.store(read + Capacity, std::memory_order_release);

        return true;
    }

    bool empty() const {
        const auto read = read_index_.value.load(std::memory_order_acquire);
        const cell& current = buffer_[read % Capacity];

        return current.sequence.load(std::memory_order_acquire) != read + 1;
    }

    bool full() const {
        const auto write = write_index_.value.load(std::memory_order_acquire);
        const cell& current = buffer_[write % Capacity];
        const auto sequence = current.sequence.load(std::memory_order_acquire);

        return static_cast<std::ptrdiff_t>(sequence) -
               static_cast<std::ptrdiff_t>(write) < 0;
    }

    constexpr std::size_t capacity() const {
        return Capacity;
    }

    std::size_t size() const {
        const auto write = write_index_.value.load(std::memory_order_acquire);
        const auto read  = read_index_.value.load(std::memory_order_acquire);

        return write - read;
    }

    void clear() {
        auto read = read_index_.value.load(std::memory_order_relaxed);

        while (true) {
            cell& current = buffer_[read % Capacity];

            const auto sequence = current.sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::ptrdiff_t>(sequence) -
                              static_cast<std::ptrdiff_t>(read + 1);

            if (diff != 0) {
                break;
            }

            std::destroy_at(current.slot());
            read_index_.value.store(read + 1, std::memory_order_relaxed);
            current.sequence.store(read + Capacity, std::memory_order_release);

            ++read;
        }
    }

private:
    using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

    struct cell {
        T* slot() {
            return std::launder(reinterpret_cast<T*>(&storage));
        }

        const T* slot() const {
            return std::launder(reinterpret_cast<const T*>(&storage));
        }

        std::atomic<std::size_t> sequence{0};
        storage_type storage;
    };

    std::array<cell, Capacity> buffer_{};

    struct alignas(std::hardware_destructive_interference_size) cache_aligned_atomic_size_t {
        std::atomic<std::size_t> value{0};
    };

    cache_aligned_atomic_size_t read_index_;
    cache_aligned_atomic_size_t write_index_;
};

} // namespace obz
