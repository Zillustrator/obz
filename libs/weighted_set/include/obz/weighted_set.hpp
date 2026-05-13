#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace obz {

template <
    typename T,
    typename Hash = std::hash<T>,
    typename KeyEqual = std::equal_to<T>>
class weighted_set {
public:
    using value_type = T;
    using weight_type = std::uint64_t;
    using size_type = std::size_t;
    using map_type = std::unordered_map<T, weight_type, Hash, KeyEqual>;
    using const_iterator = typename map_type::const_iterator;

    weighted_set() = default;

    weighted_set(const weighted_set&) = default;
    weighted_set& operator=(const weighted_set&) = default;

    weighted_set(weighted_set&&) = default;
    weighted_set& operator=(weighted_set&&) = default;

    bool insert(T value, weight_type weight) {
        validate_weight(weight);

        if (weights_.contains(value)) {
            return false;
        }

        ensure_can_add(weight);

        weights_.emplace(std::move(value), weight);

        total_weight_ += weight;
        return true;
    }

    bool erase(const T& value) {
        const auto found = weights_.find(value);

        if (found == weights_.end()) {
            return false;
        }

        total_weight_ -= found->second;
        weights_.erase(found);

        return true;
    }

    bool set_weight(const T& value, weight_type weight) {
        validate_weight(weight);

        const auto found = weights_.find(value);

        if (found == weights_.end()) {
            return false;
        }

        const auto previous_weight = found->second;

        ensure_can_replace(previous_weight, weight);

        found->second = weight;
        total_weight_ = total_weight_ - previous_weight + weight;

        return true;
    }

    weight_type weight_of(const T& value) const {
        const auto found = weights_.find(value);

        if (found == weights_.end()) {
            throw std::out_of_range("weighted_set does not contain value");
        }

        return found->second;
    }

    bool contains(const T& value) const {
        return weights_.contains(value);
    }

    template <typename UniformRandomBitGenerator>
    const T& random(UniformRandomBitGenerator& generator) const {
        if (empty()) {
            throw std::runtime_error("cannot choose random value from empty weighted_set");
        }

        std::uniform_int_distribution<weight_type> distribution(weight_type{}, total_weight_ - 1);
        const auto target = distribution(generator);

        weight_type cumulative{};

        for (const auto& [value, weight] : weights_) {
            cumulative += weight;

            if (target < cumulative) {
                return value;
            }
        }

        throw std::logic_error("weighted_set random selection failed");
    }

    void clear() {
        weights_.clear();
        total_weight_ = weight_type{};
    }

    bool empty() const {
        return weights_.empty();
    }

    size_type size() const {
        return weights_.size();
    }

    weight_type total_weight() const {
        return total_weight_;
    }

    const_iterator begin() const {
        return weights_.begin();
    }

    const_iterator end() const {
        return weights_.end();
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

    map_type weights_;
    weight_type total_weight_{};
};

} // namespace obz
