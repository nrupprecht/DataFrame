//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef __COLUMN_H__
#define __COLUMN_H__

#include <memory>
#include <ostream>
#include <limits>
#include <utility>
#include <set>

#include "../DataFrame.h"
#include "../TypeConversion.h"
#include "../Comparison.h"

#include "../dfvector.h"


//! \brief A column is like a type-erased vector, though with a number of important differences.
//!
//! A column (or a concrete column, or "Concrete") does not have a push_back or equivalent function.
//! Since each column in a data frame needs to have the same number of entries, data can only be added via a dataframe.
//! Elements of a Column cannot be accessed directly (right now) since the underlying type of the column is opaque.
//! Instead, a Concrete should be gotten from the column. Elements of the Concrete can be accessed and modified.
class DataFrame::Column {
public:
    // ========================================
    //  Logical operators.
    // ========================================

    friend Indicator operator<(const Column& col, double rhs);
    friend Indicator operator<(const Column& col, int rhs);
    friend Indicator operator<(double rhs, const Column& col);
    friend Indicator operator<(int rhs, const Column& col);

    friend Indicator operator<=(const Column& col, double rhs);
    friend Indicator operator<=(const Column& col, int rhs);
    friend Indicator operator<=(double rhs, const Column& col);
    friend Indicator operator<=(int rhs, const Column& col);

    friend Indicator operator>(const Column& col, double rhs);
    friend Indicator operator>(const Column& col, int rhs);
    friend Indicator operator>(double rhs, const Column& col);
    friend Indicator operator>(int rhs, const Column& col);

    friend Indicator operator>=(const Column& col, double rhs);
    friend Indicator operator>=(const Column& col, int rhs);
    friend Indicator operator>=(double rhs, const Column& col);
    friend Indicator operator>=(int rhs, const Column& col);

    friend Indicator operator==(const Column& col, double rhs);
    friend Indicator operator==(const Column& col, int rhs);
    friend Indicator operator==(double rhs, const Column& col);
    friend Indicator operator==(int rhs, const Column& col);
    friend Indicator operator==(const std::string& rhs, const Column& col);
    friend Indicator operator==(const Column& col, const std::string& rhs);

    friend Indicator operator<(const Column& colA, const Column& colB);
    friend Indicator operator>(const Column& colA, const Column& colB);

    bool operator==(const Column& rhs);

    // ========================================
    //  Assignment.
    // ========================================

    template<typename T>
    Column& operator=(const std::vector<T>& rhs);

    template<typename T>
    Column& operator=(const std::list<T>& rhs);

    Column& operator=(const Column& rhs);

    // ========================================
    //  Other functions.
    // ========================================

    //! \brief Check if another column is a reference to the same underlying data.
    bool IsRefOf(const Column& rhs) const;

    //! \brief Check if another column has the same underlying type as this column.
    bool SameTypeAs(const Column& rhs) const;

    //! \brief Check if the column has a specific underlying type.
    template<typename T>
    bool IsType() const;

    //! \brief Check if a concrete column is a reference to the same underlying data.
    template<typename T>
    bool IsRefOf(const Concrete<T>& rhs) const;

    //! \brief Set all entries of the column to value_.
    template<typename T>
    void Set(const T& value);

    //! \brief Attempt to get a Concrete<T> for the column. If the actual type of the Column is T,
    //! then this operation returns the Concrete<T> for this column. Otherwise, an empty Concrete<T>
    //! is returned.
    template<typename T>
    DataFrame::Concrete<T> GetConcrete() const;

    //! \brief Get unique values held in the column, assuming the concrete type is T. Returns an
    //! empty set if the underlying type is not T.
    //!
    //! This is the same as calling GetConcrete<T>().Unique().
    template<typename T>
    std::set<T> Unique() const;

    // ========================================
    //  Accessors.
    // ========================================

    //! \brief Check if the column is a subset (selection) of some column. If so, returns false.
    bool IsFullColumn() const;

    //! \brief Returns the size of the column.
    std::size_t Size() const;

    //! \brief Returns whether the column is empty.
    bool Empty() const;

    //! \brief Get the dtype of the column.
    DType GetDType() const;

    //! \brief Write the index-th element of the column to an ostream.
    void ToStream(std::size_t index, std::ostream& out) const;

    // ========================================
    //  Friend classes.
    // ========================================

    friend class DataFrame;

    template<typename T>
    friend class DataFrame::Concrete;

private:
    // ========================================
    //  Private classes.
    // ========================================

    struct Box;
    struct Wrapper;
    template<typename> struct ConcreteWrapper;

    // ========================================
    //  Private constructors.
    // ========================================
    //! \brief Private constructor for a column of a specific dtype and size.
    Column(DType dtype, IMapType index_map, std::size_t size = 0);

    // ========================================
    //  Private helper functions.
    // ========================================

    //! \brief Append another column onto this one. This function is private so only
    //! a DataFrame can use it.
    bool Append(const Column& col);

    // ========================================
    //  Private member data.
    // ========================================

    //! \brief The box for this column.
    std::shared_ptr<Box> box_;

    //! \brief The index map.
    IMapType index_map_;
};


#include "Wrapper.h"
#include "../ConcreteWrapper.h"
#include "Box.h"


template<typename T>
Indicator operator<(const DataFrame::Column& col, const T& rhs) {
    std::size_t sz = col.Size();
    auto c_col = col.GetConcrete<T>();
    if (c_col.Size() == col.Size()) {
        Indicator output;
        output.reserve(sz);
        for (std::size_t i = 0; i < c_col.Size(); ++i) {
            output.push_back(c_col[i] < rhs);
        }
        return output;
    }
    return {};
}



template<typename T>
void DataFrame::Column::Set(const T& value) {
    using S = ConvenienceType_t<T>;
    auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<S>>(box_->wrapper_);
    if (ptr) {
        ptr->SetAll(value, index_map_);
    } else {
        box_->wrapper_->is_orphan = true; // Orphan the old wrapper.
        box_->wrapper_ = std::make_shared<ConcreteWrapper<S>>(box_->wrapper_->Size(), value);
    }
}

template<typename T>
DataFrame::Column& DataFrame::Column::operator=(const std::vector<T>& rhs) {
    bool setting_none_col = (Size() == 0 && GetDType() == DType::None);

    // Make sure sizes match, or the column is empty and has zero size.
    if (rhs.size() != Size() && !setting_none_col) {
        throw std::exception();
    }

    auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    if (!ptr) { // Need to change types.
        box_->wrapper_->is_orphan = true; // Orphan the old wrapper.
        box_->wrapper_ = std::shared_ptr<ConcreteWrapper<T>>(new ConcreteWrapper<T>(box_->wrapper_->Size()));
        ptr = util::reinterpret_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    }
    // Copy data
    ptr->data_.resize(rhs.size());
    for (std::size_t i = 0; i < rhs.size(); ++i) {
        ptr->data_[i] = rhs[i];
        if (setting_none_col) {
            index_map_->push_back(index_map_->size());
        }
    }

    return *this;
}

template<typename T>
DataFrame::Column& DataFrame::Column::operator=(const std::list<T>& rhs) {
    // Make sure sizes match, or the column is empty.
    if (rhs.size() != Size() && GetDType() != DType::None) {
        throw std::exception();
    }

    auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    if (!ptr) { // Need to change types.
        box_->wrapper_->is_orphan = true; // Orphan the old wrapper.
        box_->wrapper_ = std::make_shared<ConcreteWrapper<T>>(box_->wrapper_->Size());
        ptr = util::reinterpret_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    }
    // Copy data
    auto it = rhs.begin();
    for (std::size_t i = 0; i < rhs.size(); ++i, ++it) {
        ptr->data_[i] = *it;
    }

    return *this;
}

template<typename T>
bool DataFrame::Column::IsRefOf(const DataFrame::Concrete<T>& rhs) const{
    return box_->wrapper_ == rhs.wrapper_;
}

template<typename T>
bool DataFrame::Column::IsType() const {
    return std::dynamic_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_) != nullptr;
}

template<typename T>
DataFrame::Concrete<T> DataFrame::Column::GetConcrete() const {
    auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    if (ptr) {
        return DataFrame::Concrete<T>(ptr, index_map_);
    } else { // Return empty column.
        return DataFrame::Concrete<T>();
    }
}

template<typename T>
std::set<T> DataFrame::Column::Unique() const {
    return GetConcrete<T>().Unique();
}


#endif // __COLUMN_H__
