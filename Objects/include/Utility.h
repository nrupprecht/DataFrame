#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <type_traits>
#include <utility>

namespace util {
    // ========================================
    //  Typetrait for if a type is streamable.
    // ========================================

    // Thanks, https://stackoverflow.com/questions/22758291/how-can-i-detect-if-a-type-can-be-streamed-to-an-stdostream

    namespace detail {
        template<typename S, typename T>
        static auto test(int) -> decltype(std::declval<S &>() << std::declval<T>(), std::true_type());

        template<typename, typename>
        static auto test(...) -> std::false_type;
    }

    template<typename S, typename T>
    struct is_streamable {
        static const bool value = decltype(detail::test<S, T>(0))::value;
    };

    template<typename T>
    struct is_ostreamable : public is_streamable<std::ostream, T> {};

    // ========================================
    //  Concatenate strings into one string.
    // ========================================

    template<typename ...Args>
    inline std::string str_cat(const std::string &head, const Args &... strs) {
        return head + "," + str_cat(strs...);
    }

    template<>
    inline std::string str_cat(const std::string &head) {
        return head;
    }

    // ========================================
    //  Reinterpret cast.
    // ========================================

    //! \brief Reinterpret cast a shared pointer. For some reason, my compiler doesn't
    //! recognize std::reinterpret_pointer_cast, which is obviously what should generally be used.
    //! Apparently, clang is missing some features: https://github.com/conda-forge/libcxx-feedstock/issues/44.
    template<class T, class U>
    inline std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U> &r) noexcept {
        auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type *>(r.get());
        return std::shared_ptr<T>(r, p);
    }

    // ========================================
    //  Remove cv-ref
    // ========================================

    //! \brief Remove const, volatile, and references from a type. C++20 has this as
    //! std::remove_cvref and std::remove_cvref_t.
    template<typename T>
    struct remove_cvref : public std::remove_cv<typename std::remove_reference<T>::type> {
        //using type = typename ::type;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    // ========================================
    //  Logic
    // ========================================

    template<typename ...Args>
    inline bool And(bool head, Args... args) {
        return head && And(args...);
    }

    template<>
    inline bool And(bool head) {
        return head;
    }

    template<typename ...Args>
    inline bool Or(bool head, Args... args) {
        return head || And(args...);
    }

    template<>
    inline bool Or(bool head) {
        return head;
    }

    // ========================================
    //  Template convenience.
    // ========================================

    template<typename ...Args>
    inline void Null(Args...) {};

}
#endif // __UTILITY_H__