#ifndef __DTYPES_H__
#define __DTYPES_H__

#include <string>
#include <ostream>
#include <cmath>

#include "Utility.h"

namespace dataframe {

    // ========================================
    //  Define the DTypes.
    // ========================================

    //! \brief The possible DTypes. None represents a column of size zero. Empty is a column of non-zero size with only
    //! NaN values. Integer, Float, Double, Bool, and String represent the obvious data types. Other is any other type of
    //! data.
    enum class DType {
        None, Other, Empty, Integer, Float, Double, Bool, String
    };

    // ========================================
    //  Tag types.
    // ========================================

    //! \brief A tag type to allow ConcreteWrappers of DType None.
    struct NoneDType {
        bool operator==(const NoneDType &) const { return true; }

        bool operator<(const NoneDType &) const { return true; }

        bool operator<=(const NoneDType &) const { return true; }
    };

    //! \brief A tag type to allow ConcreteWrappers of DType Empty.
    struct EmptyDType {
        bool operator==(const EmptyDType &) const { return true; }

        bool operator<(const EmptyDType &) const { return true; }

        bool operator<=(const EmptyDType &) const { return true; }
    };

    // ========================================
    //  Stream operator.
    // ========================================

    inline std::ostream &operator<<(std::ostream &out, DType dtype) {
        switch (dtype) {
            case DType::None:
                out << "DType::None";
                break;
            case DType::Other:
                out << "DType::Other";
                break;
            case DType::Empty:
                out << "DType::Empty";
                break;
            case DType::Integer:
                out << "DType::Integer";
                break;
            case DType::Float:
                out << "DType::Float";
                break;
            case DType::Double:
                out << "DType::Double";
                break;
            case DType::Bool:
                out << "DType::Bool";
                break;
            case DType::String:
                out << "DType::String";
                break;
        }
        return out;
    }

    // ========================================
    //  Define conversions from types to dtypes.
    // ========================================

    template<typename T>
    struct DTypeOf {
        static const DType dtype = DType::Other;
    };

    template<>
    struct DTypeOf<NoneDType> {
        static const DType dtype = DType::None;
    };

    template<>
    struct DTypeOf<EmptyDType> {
        static const DType dtype = DType::Empty;
    };

    template<>
    struct DTypeOf<int> {
        static const DType dtype = DType::Integer;
    };

    template<>
    struct DTypeOf<float> {
        static const DType dtype = DType::Float;
    };

    template<>
    struct DTypeOf<double> {
        static const DType dtype = DType::Double;
    };

    template<>
    struct DTypeOf<bool> {
        static const DType dtype = DType::Bool;
    };

    template<>
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

    namespace detail {
        template<typename T>
        struct ConvenienceTypeHelper {
            using type = T;
        };

        template<std::size_t N>
        struct ConvenienceTypeHelper<char[N]> {
            using type = std::string;
        };
    }


    //! \brief Any sort of char [N], we want to store as a std::string, and we want
    //! the type of the concrete wrapper to be std::string as well.
    template<typename T>
    struct ConvenienceType : public detail::ConvenienceTypeHelper<util::remove_cvref_t<T>> {};

    //! \brief Define the convenience type.
    template<typename T>
    using ConvenienceType_t = typename ConvenienceType<T>::type;

    // ========================================
    //  Checking for NaNs.
    // ========================================

    template<typename value_type>
    struct IsNaN {
        static bool check(const value_type &v) {
            return false; // Not NaN.
        }
    };

    template<>
    struct IsNaN<NoneDType> {
        static bool check(const NoneDType &v) {
            return true;
        }
    };

    template<>
    struct IsNaN<EmptyDType> {
        static bool check(const EmptyDType &v) {
            return true;
        }
    };

    template<>
    struct IsNaN<double> {
        static bool check(const double &v) {
            return isnan(v);
        }
    };

    template<>
    struct IsNaN<float> {
        static bool check(const float &v) {
            return isnan(v);
        }
    };

    template<>
    struct IsNaN<std::string> {
        static bool check(const std::string &v) {
            return v.empty();
        }
    };

    template<>
    struct IsNaN<bool> {
        static bool check(const bool &v) {
            return !(v == 0 || v == 1);
        }
    };


    // ========================================
    //  Some basic formatting.
    // ========================================

    namespace detail {
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

    template<typename value_type>
    struct Format {
        static void ToStream(const value_type &v, std::ostream &out) {
            detail::StreamCheck<util::is_ostreamable<value_type>::value, value_type>::ToStream(v, out);
        }
    };

    template<>
    struct Format<NoneDType> {
        static void ToStream(const NoneDType &v, std::ostream &out) {}
    };

    template<>
    struct Format<EmptyDType> {
        static void ToStream(const EmptyDType &v, std::ostream &out) {}
    };

    template<>
    struct Format<bool> {
        static void ToStream(const bool &v, std::ostream &out) {
            if (v == 0) {
                out << "FALSE";
            } else if (v == 1) {
                out << "TRUE";
            } else { // NaN
                out << "";
            }
        }
    };

}
#endif // __DTYPES_H__