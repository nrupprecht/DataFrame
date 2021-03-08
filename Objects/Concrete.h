//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef DATAFRAME_CONCRETE_H
#define DATAFRAME_CONCRETE_H

#include "Column.h"

template<typename T>
class DataFrame::Concrete {
public:
    // ========================================
    //  Typedefs.
    // ========================================
    using value_type = StorageType_t<T>;
    using container_type = T;

    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;


    // ========================================
    //  Constructors.
    // ========================================

    Concrete() : wrapper_(std::make_shared<WrapperType>()) {}

    // ========================================
    //  Accessors.
    // ========================================

    std::size_t Size() const { return wrapper_->Size(); }
    bool Empty() const { return wrapper_->Empty(); }
    DType GetDType() const { return wrapper_->GetDType(); }
    bool IsOrphan() const { return wrapper_->is_orphan; }

    T& operator[](std::size_t index) {
        return wrapper_->data_[index];
    }

    const T& operator[](std::size_t index) const {
        return wrapper_->data_[index];
    }

    // ========================================
    //  Iterators.
    // ========================================

    iterator begin() {
        return wrapper_->data_.begin();
    }

    const_iterator begin() const {
        return wrapper_->data_.begin();
    }

    iterator end() {
        return wrapper_->data_.end();
    }

    const_iterator end() const {
        return wrapper_->data_.end();
    }


    // ========================================
    //  Friend classes.
    // ========================================

    friend class DataFrame::Column;
private:
    typedef DataFrame::Column::ConcreteWrapper<T> WrapperType;

    explicit Concrete(std::shared_ptr<WrapperType> wrapper) : wrapper_(std::move(wrapper)) {}

    std::shared_ptr<WrapperType> wrapper_;
};


#endif //DATAFRAME_CONCRETE_H
