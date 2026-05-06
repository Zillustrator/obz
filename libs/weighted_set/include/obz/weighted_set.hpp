#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace obz {

template <
    typename T,
    typename Hash = std::hash<T>,
    typename KeyEqual = std::equal_to<T>>
class weighted_set {
public:
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "weighted_set requires nothrow move assignment");

    using value_type = T;
    using weight_type = std::uint64_t;
    using size_type = std::size_t;

    struct entry {
        T value;
        weight_type weight;
    };

    using const_iterator = typename std::vector<entry>::const_iterator;

    weighted_set() = default;

    weighted_set(const weighted_set&) = default;
    weighted_set& operator=(const weighted_set&) = default;

    weighted_set(weighted_set&&) = default;
    weighted_set& operator=(weighted_set&&) = default;

    bool insert(T value, weight_type weight) {
        validate_weight(weight);

        if (contains(value)) {
            return false;
        }

        ensure_can_add(weight);

        const auto index = entries_.size();
        entries_.push_back(entry{std::move(value), weight});

        try {
            indices_.emplace(entries_.back().value, index);
        } catch (...) {
            entries_.pop_back();
            throw;
        }

        total_weight_ += weight;
        return true;
    }

    bool erase(const T& value) {
        const auto found = indices_.find(value);

        if (found == indices_.end()) {
            return false;
        }

        const auto index = found->second;
        const auto last_index = entries_.size() - 1;

        total_weight_ -= entries_[index].weight;
        indices_.erase(found);

        if (index != last_index) {
            entries_[index] = std::move(entries_[last_index]);
            indices_[entries_[index].value] = index;
        }

        entries_.pop_back();

        return true;
    }

    bool set_weight(const T& value, weight_type weight) {
        validate_weight(weight);

        const auto found = indices_.find(value);

        if (found == indices_.end()) {
            return false;
        }

        entry& current = entries_[found->second];
        const auto previous_weight = current.weight;

        ensure_can_replace(previous_weight, weight);

        current.weight = weight;
        total_weight_ = total_weight_ - previous_weight + weight;

        return true;
    }

    weight_type weight_of(const T& value) const {
        const auto found = indices_.find(value);

        if (found == indices_.end()) {
            throw std::out_of_range("weighted_set does not contain value");
        }

        return entries_[found->second].weight;
    }

    bool contains(const T& value) const {
        return indices_.contains(value);
    }

    template <typename UniformRandomBitGenerator>
    const T& random(UniformRandomBitGenerator& generator) const {
        if (empty()) {
            throw std::runtime_error("cannot choose random value from empty weighted_set");
        }

        std::uniform_int_distribution<weight_type> distribution(weight_type{1}, total_weight_);
        const auto target = distribution(generator);

        weight_type cumulative{};

        for (const auto& current : entries_) {
            cumulative += current.weight;

            if (target <= cumulative) {
                return current.value;
            }
        }

        throw std::logic_error("weighted_set random selection failed");
    }

    void clear() {
        entries_.clear();
        indices_.clear();
        total_weight_ = weight_type{};
    }

    bool empty() const {
        return entries_.empty();
    }

    size_type size() const {
        return entries_.size();
    }

    weight_type total_weight() const {
        return total_weight_;
    }

    const_iterator begin() const {
        return entries_.begin();
    }

    const_iterator end() const {
        return entries_.end();
    }

private:
    static void validate_weight(weight_type weight) {
        if (weight == weight_type{}) {
            throw std::invalid_argument("weighted_set weight must be positive");
        }
    }

    void ensure_can_add(weight_type weight) const {
        if (weight > std::numeric_limits<weight_type>::max() - total_weight_) {
            throw std::overflow_error("weighted_set total weight overflow");
        }
    }

    void ensure_can_replace(weight_type old_weight, weight_type new_weight) const {
        const auto remaining = total_weight_ - old_weight;

        if (new_weight > std::numeric_limits<weight_type>::max() - remaining) {
            throw std::overflow_error("weighted_set total weight overflow");
        }
    }

    std::vector<entry> entries_;
    std::unordered_map<T, size_type, Hash, KeyEqual> indices_;
    weight_type total_weight_{};
};

} // namespace obz
