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
class spsc_ring_queue {
public:
    static_assert(Capacity > 0, "Capacity must be > 0");

    spsc_ring_queue() = default;

    spsc_ring_queue(const spsc_ring_queue&) = delete;
    spsc_ring_queue& operator=(const spsc_ring_queue&) = delete;

    spsc_ring_queue(spsc_ring_queue&&) = delete;
    spsc_ring_queue& operator=(spsc_ring_queue&&) = delete;

    ~spsc_ring_queue() {
        clear();
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        const auto write = write_index_.value.load(std::memory_order_relaxed);
        const auto read  = read_index_.value.load(std::memory_order_acquire);

        if (write - read == Capacity) {
            return false; // full
        }

        std::construct_at(slot(write), std::forward<Args>(args)...);

        write_index_.value.store(write + 1, std::memory_order_release);
        return true;
    }

    bool try_push(const T& value) {
        return try_emplace(value);
    }

    bool try_push(T&& value) {
        return try_emplace(std::move(value));
    }

    bool try_pop(T& value) {
        const auto read  = read_index_.value.load(std::memory_order_relaxed);
        const auto write = write_index_.value.load(std::memory_order_acquire);

        if (read == write) {
            return false; // empty
        }

        T* current = slot(read);

        value = std::move(*current);
        std::destroy_at(current);

        read_index_.value.store(read + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return read_index_.value.load(std::memory_order_acquire) ==
               write_index_.value.load(std::memory_order_acquire);
    }

    bool full() const {
        return (write_index_.value.load(std::memory_order_acquire) -
                read_index_.value.load(std::memory_order_acquire)) == Capacity;
    }

    constexpr std::size_t capacity() const {
        return Capacity;
    }

    std::size_t size() const {
        return write_index_.value.load(std::memory_order_acquire) -
               read_index_.value.load(std::memory_order_acquire);
    }

    void clear() {
        while (true) {
            const auto read  = read_index_.value.load(std::memory_order_relaxed);
            const auto write = write_index_.value.load(std::memory_order_acquire);

            if (read == write) break;

            std::destroy_at(slot(read));
            read_index_.value.store(read + 1, std::memory_order_release);
        }
    }

private:
    using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

    T* slot(std::size_t index) {
        return std::launder(reinterpret_cast<T*>(&buffer_[index % Capacity]));
    }

    const T* slot(std::size_t index) const {
        return std::launder(reinterpret_cast<const T*>(&buffer_[index % Capacity]));
    }

    std::array<storage_type, Capacity> buffer_{};

    struct alignas(std::hardware_destructive_interference_size) cache_aligned_atomic_size_t {
        std::atomic<std::size_t> value{0};
    };

    cache_aligned_atomic_size_t read_index_;
    cache_aligned_atomic_size_t write_index_;
};

} // namespace obz
