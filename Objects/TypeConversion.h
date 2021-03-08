//
// Created by Nathaniel Rupprecht on 2/28/21.
//

#ifndef __TYPE_CONVERSION_H__
#define __TYPE_CONVERSION_H__

#include "DTypes.h"

inline bool ValidBool(const std::string& data) {
    return data == "True" || data == "False" || data.empty();
}

inline char ToBool(const std::string& data) {
    if (data.empty()) {
        return -1; // NaN
    }
    return data == "True" ? 1 : 0;
}

inline bool ValidInteger(const std::string& data) {
    return (data.size() == 1 && isdigit(data[0]))
           || (((1 < data.size() && (data[0] == '+' || data[0] == '-')) || isdigit(data[0]))
               && std::find_if(data.begin() + 1, data.end(), [](char c) { return !isdigit(c);}) == data.end());
}

inline int ToInteger(const std::string& data) {
    return static_cast<int>(strtol(data.c_str(), nullptr, 10));
}

inline bool ValidDouble(const std::string& data) {
    if (data.empty()) {
        return true; // NaN is a valid double.
    }
    bool postSign = false, postDecimal = false, postNumber = false;
    for (auto c : data) {
        if (isdigit(c)) {
            postSign = true;
        }
        else if (c == '.') {
            if (postDecimal || postNumber) {
                return false;
            }
            postDecimal = true;
        }
        else if (c == 'e' || c == 'E') {
            if (postNumber) {
                return false;
            }
            postNumber = true;
            postSign = false; // The exponent can have a sign.
        }
        else if (c == '+' || c == '-') {
            if (postSign) {
                return false;
            }
            postSign = true;
        }
        else {
            return false;
        }
    }
    return true;
}

inline double ToDouble(const std::string& data) {
    return atof(data.c_str());
}

inline DType CheckDType(const std::string& data) {
    // This is a NaN value.
    if (data.empty()) {
        return DType::Empty;
    }

    // Check if data is a bool
    if (ValidBool(data)) {
        return DType::Bool;
    }
    // Check if data is an integer.
    if (ValidInteger(data)) {
        return DType::Integer;
    }
    // Check if data is a double. Integers are valid doubles too.
    if (ValidDouble(data)) {
        return DType::Double;
    }

    // Otherwise, data must be a string.
    return DType::String;
}

inline bool RecheckDType(const std::string& data, DType dtype) {
    switch (dtype) {
        case DType::Double:
        case DType::Float:
            return ValidDouble(data);
        case DType::String:
            // Anything is a valid string.
            return true;
        case DType::Bool:
            return ValidBool(data);
        case DType::Integer:
            return ValidInteger(data);
        default:
        case DType::Other:
            // Only a valid Other if it can't be anything else.
            return CheckDType(data) == DType::Other;
    }
}

// ============================================
//  Type conversion from string
// ============================================

template<typename T>
inline T ToType(const std::string& data) {
    return T();
}

template<>
inline NoneDType ToType<NoneDType>(const std::string& data) {
    return {};
}

template<>
inline EmptyDType ToType<EmptyDType>(const std::string& data) {
    return {};
}

template<>
inline double ToType<double>(const std::string& data) {
    return ToDouble(data);
}

template<>
inline float ToType<float>(const std::string& data) {
    return static_cast<float>(ToDouble(data));
}

template<>
inline int ToType<int>(const std::string& data) {
    return ToInteger(data);
}

template<>
inline char ToType<char>(const std::string& data) {
    return ToBool(data);
}

template<>
inline std::string ToType<std::string>(const std::string& data) {
    return data;
}

// ============================================
//  Type conversion from types
// ============================================

template<typename S, typename T>
inline S cast(const T& value) {
    return static_cast<S>(value);
}

template<typename T>
inline std::string cast(const T& value) {
    if (IsNaN<T, T>::check(value)) {
        return "";
    }
    return std::to_string(value);
}

template<>
inline std::string cast(const bool& value) {
    if (IsNaN<bool, bool>::check(value)) {
        return "";
    }
    return value ? "True" : "False";
}

// ============================================
//  Check DType type conversions.
// ============================================

inline bool CanConvert(DType initialT, DType finalT) {
    if (initialT == DType::Other || finalT == DType::Other) {
        return false;
    }
    if (initialT == finalT) {
        return true;
    }

    switch (initialT) {
        case DType::None:
            return true;
        case DType::Empty:
            return finalT != DType::None && finalT != DType::Integer;
        case DType::Bool:
            return finalT != DType::Other;
        case DType::Integer:
            return finalT != DType::None && finalT != DType::Empty && finalT != DType::Bool;
        case DType::Float:
        case DType::Double:
            return finalT == DType::Float || finalT == DType::String;
        default:
        case DType::String:
            return false;
    }
}

#endif // __TYPE_CONVERSION_H__
