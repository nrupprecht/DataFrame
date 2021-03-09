//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef __COLUMN_H__
#define __COLUMN_H__

#include <memory>
#include <ostream>
#include <limits>

#include "DataFrame.h"
#include "TypeConversion.h"


//! \brief Indicators (i.e. vectors of bools) can be returned from column comparison and used to select
//! new views of DataFrames.
using Indicator = std::vector<bool>;


//! \brief A column is like a type-erased vector, though with a number of important differences.
//!
//! A column (or a concrete column, or "Concrete") does not have a push_back or equivalent function.
//! Since each column in a data frame needs to have the same number of entries, data can only be added via a dataframe.
//! Elements of a Column cannot be accessed directly (right now) since the underlying type of the column is opaque.
//! Instead, a Concrete should be gotten from the column. Elements of the Concrete can be accessed and modified.
class DataFrame::Column {
public:
    // ========================================
    //  Operators.
    // ========================================

    template<typename T>
    friend Indicator operator<(const Column& col, const T& rhs);

    template<typename T>
    friend Indicator operator<(const T& rhs, const Column& col);

    friend Indicator operator<(const Column& colA, const Column& colB);

    bool operator==(const Column& rhs);

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

    //! \brief Set all entries of the column to value.
    template<typename T>
    void Set(const T& value);

    //! \brief Attempt to get a Concrete<T> for the column. If the actual type of the Column is T,
    //! then this operation returns the Concrete<T> for this column. Otherwise, an empty Concrete<T>
    //! is returned.
    template<typename T>
    DataFrame::Concrete<T> GetConcrete() const;

    // ========================================
    //  Accessors.
    // ========================================

    //! \brief Check if the wrapper is null. It should not be.
    bool IsNull() const;

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
    //! \brief Private constructor for a column of a specific dtype and size.
    Column(DType dtype, std::size_t size = 0) {
        switch (dtype) {
            case DType::None:
                box_ = Box::MakeBox<NoneDType>(size);
                break;
            case DType::Empty:
                box_ = Box::MakeBox<EmptyDType>(size);
                break;
            case DType::Integer:
                box_ = Box::MakeBox<int>(size);
                break;
            case DType::Float:
                box_ = Box::MakeBox<float>(size);
                break;
            case DType::Double:
                box_ = Box::MakeBox<double>(size);
                break;
            case DType::Bool:
                box_ = Box::MakeBox<bool>(size);
                break;
            case DType::String:
                box_ = Box::MakeBox<std::string>(size);
                break;
            default:
                throw std::exception();
        }
    }

    //! \brief Append another column onto this one. This function is private so only
    //! a DataFrame can use it.
    bool Append(const Column& col);

    //! \brief A base class for child classes with concrete storage type.
    struct Wrapper {
        //! \brief Returns the size of the data held by the wrapper.
        virtual std::size_t Size() const = 0;
        //! \brief Returns whether the wrapper holds no data.
        virtual bool Empty() const = 0;
        //! \brief Get the DType of the wrapper.
        virtual DType GetDType() const = 0;

        //! \brief Attempt to copy the data of another wrapper. Return false if the
        //! other wrapper was of a different type.
        virtual bool Copy(const std::shared_ptr<Wrapper>& ptr) = 0;

        //! \brief Return a copy of the wrapper.
        virtual std::shared_ptr<Wrapper> Clone() const = 0;

        //! \brief Write the index-th element of the wrapper to an ostream.
        virtual void ToStream(std::size_t index, std::ostream& out) const = 0;

        //! \brief Check if another wrapper has the same underlying type as this wrapper.
        virtual bool IsSameType(const std::shared_ptr<Wrapper>& wrapper) const = 0;

        //! \brief Check if another wrapper is equal to this one.
        virtual bool CheckEquals(const std::shared_ptr<Wrapper>& wrapper) const = 0;

        //! \brief Add data to the wrapper via its string representation.
        virtual void AddByString(const std::string& value) = 0;

        //! \brief Append the contents of another
        virtual bool Append(const std::shared_ptr<Wrapper>& wrapper) = 0;

        // ========================================
        //  Comparisons.
        // ========================================

        virtual Indicator cmp_less(const std::shared_ptr<Wrapper>& wrapper) const = 0;

        // ========================================
        //  Data.
        // ========================================

        //! \brief Records whether the wrapper has been orphaned (disconnected from it's
        //! original box).
        bool is_orphan = false;
    };

    template<typename T>
    struct ConcreteWrapper : public Wrapper {
        using value_type = StorageType_t<T>;
        using container_type = T;

        ConcreteWrapper() = default;
        explicit ConcreteWrapper(std::size_t size) : data_(size) {}
        ConcreteWrapper(std::size_t size, T&& value)
            : data_(size, static_cast<value_type>(value)) {}

        std::size_t Size() const override {
            return data_.size();
        }
        bool Empty() const override {
            return data_.empty();
        }

        DType GetDType() const override {
            return DTypeOf<T>::dtype;
        }

        bool Copy(const std::shared_ptr<Wrapper>& ptr) override {
            auto c_ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(ptr);
            if (c_ptr) { // Successful cast.
                data_.resize(c_ptr->data_.size());
                for (std::size_t i = 0; i < data_.size(); ++i) {
                    data_[i] = c_ptr->data_[i];
                }
                return true;
            }
            return false; // Different types.
        }

        std::shared_ptr<Wrapper> Clone() const override {
            auto ptr = std::make_shared<ConcreteWrapper<T>>();
            ptr->data_ = data_;
            return ptr;
        }

        void ToStream(std::size_t index, std::ostream& out) const override {
            if (IsNaN<T, value_type>::check(data_[index])) {
                out << "";
            }
            else {
                Format<T, value_type>::ToStream(data_[index], out);
            }
        }

        bool IsSameType(const std::shared_ptr<Wrapper>& wrapper) const override {
            return std::dynamic_pointer_cast<ConcreteWrapper<T>>(wrapper) != nullptr;
        }

        bool CheckEquals(const std::shared_ptr<Wrapper>& wrapper) const override {
            auto c_ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(wrapper);
            if (c_ptr) {
                return data_ == c_ptr->data_;
            }
            return false;
        }

        void AddByString(const std::string& value) override {
            data_.push_back(ToType<value_type>(value));
        }

        bool Append(const std::shared_ptr<Wrapper>& wrapper) override {
            auto c_ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(wrapper);
            if (c_ptr) {
                data_.insert(data_.end(), c_ptr->data_.begin(), c_ptr->data_.end());
                return true;
            }
            return false;
        }

        // ========================================
        //  Comparisons.
        // ========================================

        Indicator cmp_less(const std::shared_ptr<Wrapper>& wrapper) const override {
            if (data_.size() != wrapper->Size()) {
                return {};
            }
            auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(wrapper);
            if (ptr) {
                Indicator output(data_.size());
                for (std::size_t i = 0; i < data_.size(); ++i) {
                    output[i] = data_[i] < ptr->data_[i];
                }
                return output;
            }
            return {};
        }

        // ========================================
        //  Other functions.
        // ========================================

        void SetAll(const T& value) {
            std::for_each(data_.begin(), data_.end(), [&](T& v) { v = static_cast<value_type>(value); });
        }

        // ========================================
        //  Data
        // ========================================

        std::vector<value_type> data_;
    };

    struct Box {
        explicit Box(std::shared_ptr<Wrapper>&& wrapper) : wrapper_(wrapper) {}

        template<typename T>
        static std::shared_ptr<Box> MakeBox(std::size_t size) {
            return std::make_shared<Box>(std::make_shared<ConcreteWrapper<T>>(size));
        }

        void SetWrapper(std::shared_ptr<Wrapper>&& ptr) {
            if (wrapper_) {
                wrapper_->is_orphan = true;
            }
            wrapper_ = std::move(ptr);
        }

        template<typename T>
        bool TryConvert(
                const std::shared_ptr<DataFrame::Column::ConcreteWrapper<T>>& new_wrapper,
                DType old_wrapper_type) {
            return old_wrapper_type == DType::None;
        }

        template<>
        bool TryConvert<double>(
                const std::shared_ptr<DataFrame::Column::ConcreteWrapper<double>>& new_wrapper,
                DType old_wrapper_type)
        {
            std::size_t sz = wrapper_->Size();
            switch (old_wrapper_type) {
                case DType::Integer: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<int>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        new_wrapper->data_[i] = static_cast<double>(ptr->data_[i]);
                    }
                    return true;
                }
                case DType::Float: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<float>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        if (IsNaN<float, float>::check(ptr->data_[i])) {
                            new_wrapper->data_[i] = std::numeric_limits<double>::quiet_NaN();
                        }
                        else {
                            new_wrapper->data_[i] = static_cast<double>(ptr->data_[i]);
                        }
                    }
                    return true;
                }
                case DType::None:
                case DType::Double:
                    return true; // No need to convert.
                default:
                    return false;
            }
        }

        template<>
        bool TryConvert<float>(
                const std::shared_ptr<DataFrame::Column::ConcreteWrapper<float>>& new_wrapper,
                DType old_wrapper_type)
        {
            std::size_t sz = wrapper_->Size();
            switch (old_wrapper_type) {
                case DType::Integer: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<int>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        new_wrapper->data_[i] = static_cast<float>(ptr->data_[i]);
                    }
                    return true;
                }
                case DType::Double: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<float>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        if (IsNaN<double, double>::check(ptr->data_[i])) {
                            new_wrapper->data_[i] = std::numeric_limits<float>::quiet_NaN();
                        }
                        else {
                            new_wrapper->data_[i] = static_cast<float>(ptr->data_[i]);
                        }
                    }
                    return true;
                }
                case DType::None:
                case DType::Float:
                    return true; // No need to convert.
                default:
                    return false;
            }
        }

        template<>
        bool TryConvert<std::string>(
                const std::shared_ptr<DataFrame::Column::ConcreteWrapper<std::string>>& new_wrapper,
                DType old_wrapper_type)
        {
            std::size_t sz = wrapper_->Size();
            switch (old_wrapper_type) {
                case DType::Integer: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<int>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        new_wrapper->data_[i] = std::to_string(ptr->data_[i]);
                    }
                    return true;
                }
                case DType::Double: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<double>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        if (IsNaN<double, double>::check(ptr->data_[i])) {
                            new_wrapper->data_[i] = std::numeric_limits<float>::quiet_NaN();
                        } else {
                            new_wrapper->data_[i] = std::to_string(ptr->data_[i]);
                        }
                    }
                    return true;
                }
                case DType::Float: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<float>> (wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        if (IsNaN<float, float>::check(ptr->data_[i])) {
                            new_wrapper->data_[i] = "";
                        } else {
                            new_wrapper->data_[i] = std::to_string(ptr->data_[i]);
                        }
                    }
                    return true;
                }
                case DType::Bool: {
                    auto ptr = util::reinterpret_pointer_cast<ConcreteWrapper<bool>>(wrapper_);
                    new_wrapper->data_.resize(sz);
                    for (std::size_t i = 0; i < sz; ++i) {
                        if (IsNaN<bool, char>::check(ptr->data_[i])) {
                            new_wrapper->data_[i] = "";
                        } else {
                            new_wrapper->data_[i] = ptr->data_[i] == 0 ? "False" : "True";
                        }
                    }
                    return true;
                }
                case DType::None:
                case DType::String:
                    return true;
                default:
                    return false;
            }
        }

        bool ConvertDType(const DType dtype) {
            auto wrapper_type = wrapper_->GetDType();
            if (wrapper_type == dtype) {
                return true;
            }
            else if (wrapper_type == DType::Other) {
                return false; // Don't know the DType, so can't convert.
            }
            // Create a new wrapper.
            bool status = false;
            std::shared_ptr<Wrapper> new_wrapper_;
            switch (dtype) {
                case DType::None:
                    new_wrapper_ = std::make_shared<ConcreteWrapper<NoneDType>>();
                    status = true;
                    break;
                case DType::Empty:
                    new_wrapper_ = std::make_shared<ConcreteWrapper<EmptyDType>>();
                    status = true;
                    break;
                case DType::Integer: {
                    if (wrapper_type == DType::None) {
                        status = true;
                        new_wrapper_ = std::make_shared<ConcreteWrapper<int>>();
                    } else {
                        return false; // If we are here, wrapper_type != DType::Integer
                    }
                    break;
                }
                case DType::Float: {
                    auto wrapper = std::make_shared<ConcreteWrapper<float>>();
                    status = TryConvert(wrapper, wrapper_type);
                    new_wrapper_ = wrapper;
                    break;
                }
                case DType::Double: {
                    auto wrapper = std::make_shared<ConcreteWrapper<double>>();
                    status = TryConvert(wrapper, wrapper_type);
                    new_wrapper_ = wrapper;
                    break;
                }
                case DType::Bool:
                    status = true;
                    if (wrapper_type == DType::None) {
                        new_wrapper_ = std::make_shared<ConcreteWrapper<bool>>();
                    } else if (wrapper_type == DType::Empty) {
                        new_wrapper_ = std::make_shared<ConcreteWrapper<bool>>(wrapper_->Size(), -1);
                    } else {
                        return false; // If we are here, wrapper_type != DType::Bool
                    }
                    break;
                case DType::String: {
                    auto wrapper = std::make_shared<ConcreteWrapper<std::string>>();
                    status = TryConvert(wrapper, wrapper_type);
                    new_wrapper_ = wrapper;
                    break;
                }
                default:
                    return false; // We should never get here.
            }
            if (status) {
                SetWrapper(std::move(new_wrapper_));
            }
            return status;
        }

        std::shared_ptr<Wrapper> wrapper_;
    };



    std::shared_ptr<Box> box_;
};


template<typename T>
Indicator operator<(const DataFrame::Column& col, const T& rhs) {
    std::size_t sz = col.Size();
    auto c_col = col.GetConcrete<T>();
    if (c_col.Size() == col.Size()) {
        Indicator output;
        output.reserve(sz);
        std::for_each(c_col.begin(), c_col.end(), [&](T& x) { output.push_back(x < rhs); });
        return output;
    }
    return {};
}

template<typename T>
Indicator operator<(const T& rhs, const DataFrame::Column& col) {
    return col < rhs;
}

template<typename T>
void DataFrame::Column::Set(const T& value) {
    auto ptr = std::dynamic_pointer_cast<ConcreteWrapper<T>>(box_->wrapper_);
    if (ptr) {
        ptr->SetAll(value);
    } else {
        box_->wrapper_->is_orphan = true; // Orphan the old wrapper.
        box_->wrapper_ = std::make_shared<ConcreteWrapper<T>>(box_->wrapper_->Size(), value);
    }
}

template<typename T>
DataFrame::Column& DataFrame::Column::operator=(const std::vector<T>& rhs) {
    // Make sure sizes match, or the column is empty and has zero size.
    if (rhs.size() != Size() && !(Size() == 0 && GetDType() == DType::None)) {
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
        return DataFrame::Concrete<T>(ptr);
    } else { // Return empty column.
        return DataFrame::Concrete<T>();
    }
}


#endif // __COLUMN_H__
