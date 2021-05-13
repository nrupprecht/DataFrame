//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __BOX_TEMPLATES_H__
#define __BOX_TEMPLATES_H__

// This file is INCLUDED in Column.h

struct DataFrame::Column::Box {
    explicit Box(std::shared_ptr<Wrapper>&& wrapper) : wrapper_(std::move(wrapper)) {}

    template<typename T>
    static std::shared_ptr<Box> MakeBox(std::size_t size) {
        return std::make_shared<Box>(std::make_shared<ConcreteWrapper<T>>(size));
    }

    Box Clone() {
        return Box(wrapper_->Clone());
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
                    if (IsNaN<float>::check(ptr->data_[i])) {
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
                    if (IsNaN<double>::check(ptr->data_[i])) {
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
                    if (IsNaN<double>::check(ptr->data_[i])) {
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
                    if (IsNaN<float>::check(ptr->data_[i])) {
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
                    if (IsNaN<bool>::check(ptr->data_[i])) {
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

    //! \brief The wrapper the box contains. This wraps the actual data.
    std::shared_ptr<Wrapper> wrapper_;
};

#endif // __BOX_TEMPLATES_H__
