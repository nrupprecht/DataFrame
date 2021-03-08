#ifndef __DTYPES_H__
#define __DTYPES_H__

#include <string>
#include <ostream>
#include <cmath>

#include "Utility.h"
#include "TypeDescription.h"


// ========================================
//  Define the DTypes.
// ========================================

//! \brief The possible DTypes. None represents a column of size zero. Empty is a column of non-zero size with only
//! NaN values. Integer, Float, Double, Bool, and String represent the obvious data types. Other is any other type of
//! data.
enum class DType { None, Other, Empty, Integer, Float, Double, Bool, String };

// ========================================
//  Tag types.
// ========================================

//! \brief A tag type to allow ConcreteWrappers of DType None.
struct NoneDType {
    bool operator==(const NoneDType&) const { return true; }
    bool operator<(const NoneDType&) const { return true; }
    bool operator<=(const NoneDType&) const { return true; }
};

//! \brief A tag type to allow ConcreteWrappers of DType Empty.
struct EmptyDType {
    bool operator==(const EmptyDType&) const { return true; }
    bool operator<(const EmptyDType&) const { return true; }
    bool operator<=(const EmptyDType&) const { return true; }
};

// ========================================
//  Define conversions from types to dtypes.
// ========================================

template<typename T>
struct DTypeOf {
    static const DType dtype = DType::Other;
};

template <>
struct DTypeOf<NoneDType> {
    static const DType dtype = DType::None;
};

template <>
struct DTypeOf<EmptyDType> {
    static const DType dtype = DType::Empty;
};

template <>
struct DTypeOf<int> {
    static const DType dtype = DType::Integer;
};

template <>
struct DTypeOf<float> {
    static const DType dtype = DType::Float;
};

template <>
struct DTypeOf<double> {
    static const DType dtype = DType::Double;
};

template <>
struct DTypeOf<bool> {
    static const DType dtype = DType::Bool;
};

template <>
struct DTypeOf<std::string> {
    static const DType dtype = DType::String;
};

// ========================================
//  Convert DTypes to types
// ========================================

template<DType dtype>
struct TypeOfDType { // Other. Should be unused.
    using type = int;
};

template<>
struct TypeOfDType<DType::None> {
    using type = NoneDType;
};

template<>
struct TypeOfDType<DType::Empty> {
    using type = EmptyDType;
};

template<>
struct TypeOfDType<DType::Integer> {
    using type = int;
};

template<>
struct TypeOfDType<DType::Double> {
    using type = double;
};

template<>
struct TypeOfDType<DType::Float> {
    using type = float;
};

template<>
struct TypeOfDType<DType::String> {
    using type = std::string;
};

// ========================================
//  Define storage types.
// ========================================

namespace {
    template<typename T>
    struct StorageTypeHelper {
        using type = T;
    };

    template<>
    struct StorageTypeHelper<bool> {
        using type = char;
    };
}

//! \brief Defines the storage type associated with a concrete column type. This headache is necessary
//! because someone thought it would be smart for std::vector<bool> to be a template specialization that
//! doesn't include a reference at() or reference operator[] that returns bools. There is the
//! std::vector<bool>::reference type, but to keep things simple, we just use vectors of chars to store bools.
template <typename T>
struct StorageType : public StorageTypeHelper<util::remove_cvref_t<T>> {};

template <typename T>
using StorageType_t = typename StorageType<T>::type;


// ========================================
//  Check for NaNs.
// ========================================

template<typename container_type, typename value_type>
struct IsNaN {
    static bool check(const value_type& v) {
        return false; // Not NaN.
    }
};

template<typename container_type>
struct IsNaN<container_type, NoneDType> {
    static bool check(const NoneDType& v) {
        return true;
    }
};

template<typename container_type>
struct IsNaN<container_type, EmptyDType> {
    static bool check(const EmptyDType& v) {
        return true;
    }
};

template<typename container_type>
struct IsNaN<container_type, double> {
    static bool check(const double& v) {
        return isnan(v);
    }
};

template<typename container_type>
struct IsNaN<container_type, float> {
    static bool check(const float& v) {
        return isnan(v);
    }
};

template<typename container_type>
struct IsNaN<container_type, std::string> {
    static bool check(const std::string& v) {
        return v.empty();
    }
};

template<typename container_type>
struct IsNaN<container_type, char> {
    static bool check(const char& v) {
        return v == -1;
    }
};


// ========================================
//  Some basic formatting.
// ========================================

namespace {
    template<bool value, typename value_type>
    struct StreamCheck {
        static void ToStream(const value_type &v, std::ostream &out) {
            out << v;
        }
    };

    template<typename value_type>
    struct StreamCheck<false, value_type> {
        static void ToStream(const value_type &v, std::ostream &out) {}
    };
}

template<typename container_type, typename value_type>
struct Format {
    static void ToStream(const value_type& v, std::ostream& out) {
        StreamCheck<util::is_ostreamable<value_type>::value, value_type>::ToStream(v, out);
    }
};

template<typename container_type>
struct Format<container_type, NoneDType> {
    static void ToStream(const NoneDType& v, std::ostream& out) {}
};

template<typename container_type>
struct Format<container_type, EmptyDType> {
    static void ToStream(const EmptyDType& v, std::ostream& out) {}
};

template<>
struct Format<bool, StorageType_t<bool>> {
    static void ToStream(const StorageType_t<bool>& v, std::ostream& out) {
        if (v == 0) {
            out << "False";
        }
        else if (v == 1) {
            out << "True";
        }
        else { // NaN
            out << "";
        }
    }
};

#endif // __DTYPES_H__