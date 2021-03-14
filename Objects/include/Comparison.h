//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __COMPARISON_H__
#define __COMPARISON_H__

#include <vector>
#include <type_traits>
#include "dfvector.h"

using Indicator = std::vector<bool>;
using IMap = std::vector<std::size_t>;

// ========================================
//  Type trait for checking whether one type can be static casted to another.
// ========================================

namespace {
    template<typename value_type, typename target_type>
    static auto test(int) -> decltype(static_cast<target_type>(std::declval<value_type>()), std::true_type());

    template<typename, typename>
    static auto test(...) -> std::false_type;
}

template<typename value_type, typename target_type>
struct is_castable {
    static constexpr bool value = decltype(test<value_type, target_type>(0))::value;
};

// ========================================
//  Object to do comparisons between values and dfvectors of potentially different types.
// ========================================

namespace {
    template<typename value_type, typename target_type, bool is_castable>
    struct DoComparisonHelper {
        template<typename Binary>
        static Indicator compare(
                Binary &&op,
                const dfvector<target_type> &data,
                const IMap &index_map,
                const value_type &value) {
            return {};
        }
    };

    template<typename value_type, typename target_type>
    struct DoComparisonHelper<value_type, target_type, true> {
        template<typename Binary>
        static Indicator compare(
                Binary &&op,
                const dfvector<target_type> &data,
                const IMap &index_map,
                const value_type &value) {
            Indicator output;
            output.reserve(index_map.size());
            for (auto i : index_map) {
                output.push_back(op(data[i], static_cast<target_type>(value)));
            }
            return output;
        }
    };
}

template<typename value_type, typename target_type>
struct DoComparison : public DoComparisonHelper<value_type, target_type, is_castable<value_type, target_type>::value> {};

// ========================================
//  Helpful operators for combining or modifying Indicators.
// ========================================

inline Indicator operator&(const Indicator& lhs, const Indicator& rhs) {
    std::size_t sz = std::min(lhs.size(), rhs.size());
    Indicator output;
    output.resize(sz);
    for (std::size_t i = 0; i < sz; ++i) {
        output[i] = lhs[i] && rhs[i];
    }
    return output;
}

inline Indicator operator|(const Indicator& lhs, const Indicator& rhs) {
    std::size_t sz = std::min(lhs.size(), rhs.size());
    Indicator output;
    output.resize(sz);
    for (std::size_t i = 0; i < sz; ++i) {
        output[i] = lhs[i] || rhs[i];
    }
    return output;
}

inline Indicator operator^(const Indicator& lhs, const Indicator& rhs) {
    std::size_t sz = std::min(lhs.size(), rhs.size());
    Indicator output;
    output.resize(sz);
    for (std::size_t i = 0; i < sz; ++i) {
        output[i] = lhs[i] ^ rhs[i];
    }
    return output;
}

inline Indicator operator~(const Indicator& lhs) {
    Indicator output;
    output.resize(lhs.size());
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        output[i] = !lhs[i];
    }
    return output;
}

#endif // __COMPARISON_H__
