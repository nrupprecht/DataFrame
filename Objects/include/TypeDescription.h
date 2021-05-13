//
// Created by Nathaniel Rupprecht on 3/6/21.
//

#ifndef __TYPE_DESCRIPTIONS_H__
#define __TYPE_DESCRIPTIONS_H__

#include <string>
#include <vector>
#include <list>
#include <map>

#include "Utility.h"

namespace dataframe {
namespace types {
    // ========================================
    //  Type identification
    // ========================================

    template<typename T>
    struct TypeDetails {
        const std::string description = "Unknown";
    };

    // ========================================
    //  Basic types.
    // ========================================

    template<>
    struct TypeDetails<float> {
        const std::string description = "float";
    };

    template<>
    struct TypeDetails<double> {
        const std::string description = "double";
    };

    template<>
    struct TypeDetails<int> {
        const std::string description = "int";
    };

    template<>
    struct TypeDetails<std::string> {
        const std::string description = "std::string";
    };

    template<>
    struct TypeDetails<bool> {
        const std::string description = "bool";
    };

    // ========================================
    //  Pointers, references, cv
    // ========================================

    template<typename T>
    struct TypeDetails<T *> {
        const std::string description = TypeDetails<T>().description + "*";
    };

    template<typename T>
    struct TypeDetails<T &> {
        const std::string description = TypeDetails<T>().description + "&";
    };

    template<typename T>
    struct TypeDetails<const T> {
        const std::string description = TypeDetails<T>().description + " const";
    };

    template<typename T>
    struct TypeDetails<volatile T> {
        const std::string description = TypeDetails<T>().description + " volatile";
    };

    template<typename T>
    struct TypeDetails<const volatile T> {
        const std::string description = TypeDetails<T>().description + " const volatile";
    };

    // ========================================
    //  Function pointers
    // ========================================

    template<typename R, typename ...Args>
    struct TypeDetails<R (*)(Args...)> {
        const std::string description = TypeDetails<R>().description
                                        + "(*)(" + util::str_cat(TypeDetails<Args>().description...) + ")";
    };

    // ========================================
    //  Containers
    // ========================================

    template<typename T>
    struct TypeDetails<std::vector<T>> {
        const std::string description = "std::vector<" + TypeDetails<T>().description + ">";
    };

    template<typename T>
    struct TypeDetails<std::list<T>> {
        const std::string description = "std::list<" + TypeDetails<T>().description + ">";
    };

    template<typename S, typename T>
    struct TypeDetails<std::map<S, T>> {
        const std::string description = "std::map<"
                                        + TypeDetails<S>().description + "," + TypeDetails<T>().description + ">";
    };

    template<typename S, typename T>
    struct TypeDetails<std::pair<S, T>> {
        const std::string description = "std::pair<"
                                        + TypeDetails<S>().description + "," + TypeDetails<T>().description + ">";
    };

}
}
#endif // __TYPE_DESCRIPTIONS_H__
