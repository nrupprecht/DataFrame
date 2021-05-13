//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __DF_VECTOR_H__
#define __DF_VECTOR_H__

#include "DTypes.h"

namespace dataframe {

//! \brief A type that is a simple wrapper around another type. This is used to get around the
//! fact that std::vector<bool> is specialized, and lacks a reference access operator or .at function.
template<typename T>
struct Holder {
    // ========================================
    //  Constructors
    // ========================================

    Holder() = default;

    Holder(const Holder<T> &rhs) : value_(rhs.value_) {}

    Holder(Holder<T> &&rhs) noexcept: value_(std::move(rhs.value_)) {}

    explicit Holder(const T &value) : value_(value) {}

    explicit Holder(T &&value) : value_(std::move(value)) {}

    // ========================================
    //  Logicial operators.
    // ========================================

    bool operator==(const Holder &rhs) const { return value_ == rhs.value_; }

    // ========================================
    //  Assignment.
    // ========================================

    Holder &operator=(const Holder &rhs) {
        value_ = rhs.value_;
        return *this;
    }

    Holder &operator=(Holder &&rhs) {
        value_ = std::move(rhs.value_);
        return *this;
    }

    // ========================================
    //  Data.
    // ========================================

    //! \brief The actual bid of data contained in the Holder.
    T value_;
};

template<typename T>
class DFVector {
public:
    // ========================================
    //  Typedefs
    // ========================================
    using value_type = T;
    using CType = std::vector<Holder<value_type>>;
    using self_type = DFVector<value_type>;

    // ========================================
    //  Constructors.
    // ========================================
    DFVector() = default;

    explicit DFVector(std::size_t sz) : data_(sz) {}

    DFVector(std::size_t sz, const T &value) : data_(sz, Holder<T>(value)) {};

    // ========================================
    //  Functions to allow DFVector to operate like a std::vector when we need it to.
    // ========================================

    std::size_t size() const { return data_.size(); }

    bool empty() const { return data_.empty(); }

    T operator[](std::size_t index) const { return data_[index].value_; }

    T &operator[](std::size_t index) { return data_[index].value_; }

    bool operator==(const DFVector<T> &rhs) const { return data_ == rhs.data_; }

    void push_back(const T &value) { data_.push_back(Holder<T>(value)); }

    void push_back(T &&value) { data_.push_back(Holder<T>(std::move(value))); }

    void append(const self_type &rhs) {
        data_.insert(data_.end(), rhs.data_.begin(), rhs.data_.end());
    }

    void resize(std::size_t sz) { data_.resize(sz); }

    void Reserve(std::size_t sz) { data_.reserve(sz); }

    //! \brief Non-specialized DFVector<T>'s are just wrappers for a std::vectors<Holder<T>>.
    CType data_;
};

//! \brief Specialization of DFVector. Since NoneDType is just a placeholder, there is no point
//! actually allocating a bunch of memory to hold many useless placeholders. This specialization
//! allows the container to act like a vector, but not actually contain any extra information.
template<>
class DFVector<NoneDType> {
public:
    using value_type = NoneDType;
    using CType = NoneDType;
    using self_type = DFVector<NoneDType>;


    DFVector() = default;

    explicit DFVector(std::size_t sz) : size_(sz) {}

    DFVector(std::size_t sz, const NoneDType &) : size_(sz) {}

    std::size_t size() const { return size_; }

    bool empty() const { return size_ == 0; }

    NoneDType operator[](std::size_t index) const { return x; }

    NoneDType &operator[](std::size_t index) { return x; }

    bool operator==(const DFVector<NoneDType> &rhs) const { return size_ == rhs.size_; }

    void push_back(const NoneDType &value) { ++size_; }

    void push_back(NoneDType &&value) { ++size_; }

    void append(const self_type &rhs) { size_ += rhs.size_; }

    void resize(std::size_t sz) { size_ = sz; }

    void Reserve(std::size_t sz) {}

private:
    //! \brief Having an object of type EmptyDType allows for a reference access operator without
    //! having to return a temporary.
    NoneDType x;

    //! \brief Since this specialization does not have an actual std::vector underlying it, it needs
    //! to separately keep track of its size.
    std::size_t size_;
};

//! \brief Specialization of DFVector. Since EmptyDType is just a placeholder, there is no point
//! actually allocating a bunch of memory to hold many useless placeholders. This specialization
//! allows the container to act like a vector, but not actually contain any extra information.
template<>
class DFVector<EmptyDType> {
public:
    using value_type = EmptyDType;
    using CType = EmptyDType;
    using self_type = DFVector<EmptyDType>;

    DFVector() = default;

    explicit DFVector(std::size_t sz) : size_(sz) {}

    DFVector(std::size_t sz, const EmptyDType &) : size_(sz) {}

    std::size_t size() const { return size_; }

    bool empty() const { return size_ == 0; }

    EmptyDType operator[](std::size_t index) const { return x; }

    EmptyDType &operator[](std::size_t index) { return x; }

    bool operator==(const DFVector<EmptyDType> &rhs) const { return size_ == rhs.size_; }

    void push_back(const EmptyDType &value) { ++size_; }

    void push_back(EmptyDType &&value) { ++size_; }

    void append(const self_type &rhs) { size_ += rhs.size_; }

    void resize(std::size_t sz) { size_ = sz; }

    void Reserve(std::size_t sz) {}

private:
    //! \brief Having an object of type EmptyDType allows for a reference access operator without
    //! having to return a temporary.
    EmptyDType x;

    //! \brief Since this specialization does not have an actual std::vector underlying it, it needs
    //! to separately keep track of its size.
    std::size_t size_;
};

}
#endif // __DF_VECTOR_H__
