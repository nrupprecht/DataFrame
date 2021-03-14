//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __DF_VECTOR_H__
#define __DF_VECTOR_H__

#include "DTypes.h"

//! \brief A type that is a simple wrapper around another type. This is used to get around the
//! fact that std::vector<bool> is specialized, and lacks a reference access operator or .at function.
template<typename T>
struct holder {
    // ========================================
    //  Constructors
    // ========================================

    holder() = default;
    holder(const holder<T>& rhs) : value_(rhs.value_) {}
    holder(holder<T>&& rhs) noexcept : value_(std::move(rhs.value_)) {}
    explicit holder(const T& value) : value_(value) {}
    explicit holder(T&& value) : value_(std::move(value)) {}

    // ========================================
    //  Logicial operators.
    // ========================================

    bool operator==(const holder& rhs) const { return value_ == rhs.value_; }

    // ========================================
    //  Assignment.
    // ========================================

    holder& operator=(const holder& rhs) {
        value_ = rhs.value_;
        return *this;
    }

    holder& operator=(holder&& rhs) {
        value_ = std::move(rhs.value_);
        return *this;
    }

    // ========================================
    //  Data.
    // ========================================

    T value_;
};

template<typename T>
class dfvector {
public:
    // ========================================
    //  Typedefs
    // ========================================
    using value_type = T;
    using CType = std::vector<holder<value_type>>;
    using self_type = dfvector<value_type>;

    // ========================================
    //  Constructors.
    // ========================================
    dfvector() = default;
    explicit dfvector(std::size_t sz) : data_(sz) {}
    dfvector(std::size_t sz, const T& value) : data_(sz, holder<T>(value)) {};

    // ========================================
    //  Functions to allow dfvector to operate like a std::vector when we need it to.
    // ========================================

    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    T operator[](std::size_t index) const { return data_[index].value_; }
    T& operator[](std::size_t index) { return data_[index].value_; }
    bool operator==(const dfvector<T>& rhs) const { return data_ == rhs.data_; }

    void push_back(const T& value) { data_.push_back(holder<T>(value)); }
    void push_back(T&& value) { data_.push_back(holder<T>(std::move(value))); }
    void append(const self_type& rhs) {
        data_.insert(data_.end(), rhs.data_.begin(), rhs.data_.end());
    }

    void resize(std::size_t sz) { data_.resize(sz); }

private:
    //! \brief Non-specialized dfvector<T>'s are just wrappers for a std::vectors<holder<T>>.
    CType data_;
};

//! \brief Specialization of dfvector. Since NoneDType is just a placeholder, there is no point
//! actually allocating a bunch of memory to hold many useless placeholders. This specialization
//! allows the container to act like a vector, but not actually contain any extra information.
template<>
class dfvector<NoneDType> {
public:
    using value_type = NoneDType;
    using CType = NoneDType;
    using self_type = dfvector<NoneDType>;


    dfvector() = default;
    explicit dfvector(std::size_t sz) : size_(sz) {}
    dfvector(std::size_t sz, const NoneDType&) : size_(sz) {}

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    NoneDType operator[](std::size_t index) const { return x; }
    NoneDType& operator[](std::size_t index) { return x; }
    bool operator==(const dfvector<NoneDType>& rhs) const { return size_ == rhs.size_; }

    void push_back(const NoneDType& value) { ++size_; }
    void push_back(NoneDType&& value) { ++size_; }
    void append(const self_type& rhs) { size_ += rhs.size_; }

    void resize(std::size_t sz) { size_ = sz; }
private:
    //! \brief Having an object of type EmptyDType allows for a reference access operator without
    //! having to return a temporary.
    NoneDType x;

    //! \brief Since this specialization does not have an actual std::vector underlying it, it needs
    //! to separately keep track of its size.
    std::size_t size_;
};

//! \brief Specialization of dfvector. Since EmptyDType is just a placeholder, there is no point
//! actually allocating a bunch of memory to hold many useless placeholders. This specialization
//! allows the container to act like a vector, but not actually contain any extra information.
template<>
class dfvector<EmptyDType> {
public:
    using value_type = EmptyDType;
    using CType = EmptyDType;
    using self_type = dfvector<EmptyDType>;

    dfvector() = default;
    explicit dfvector(std::size_t sz) : size_(sz) {}
    dfvector(std::size_t sz, const EmptyDType&) : size_(sz) {}

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    EmptyDType operator[](std::size_t index) const { return x; }
    EmptyDType& operator[](std::size_t index) { return x; }
    bool operator==(const dfvector<EmptyDType>& rhs) const { return size_ == rhs.size_; }

    void push_back(const EmptyDType& value) { ++size_; }
    void push_back(EmptyDType&& value) { ++size_; }
    void append(const self_type& rhs) { size_ += rhs.size_; }

    void resize(std::size_t sz) { size_ = sz; }
private:
    //! \brief Having an object of type EmptyDType allows for a reference access operator without
    //! having to return a temporary.
    EmptyDType x;

    //! \brief Since this specialization does not have an actual std::vector underlying it, it needs
    //! to separately keep track of its size.
    std::size_t size_;
};

#endif // __DF_VECTOR_H__
