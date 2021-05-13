//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __CONCRETE_WRAPPER_TEMPLATES_H__
#define __CONCRETE_WRAPPER_TEMPLATES_H__

// This file is INCLUDED in Column.h

template<typename T>
struct DataFrame::Column::ConcreteWrapper : public DataFrame::Column::Wrapper {
    using value_type = util::remove_cvref_t<T>;

    ConcreteWrapper() = default;

    explicit ConcreteWrapper(std::size_t size) : data_(size) {}

    ConcreteWrapper(std::size_t size, const T& value)
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

    std::shared_ptr<Wrapper> Clone(const IMapType& index_map) const override {
        auto ptr = std::make_shared<ConcreteWrapper<T>>();
        ptr->data_.Reserve(index_map->size());
        for (std::size_t i = 0; i < index_map->size(); ++i) {
            ptr->data_.push_back(data_[(*index_map)[i]]);
        }
        ptr->data_ = data_;
        return ptr;
    }

    void ToStream(std::size_t index, std::ostream& out) const override {
        if (IsNaN<value_type>::check(data_[index])) {
            out << "";
        }
        else {
            Format<value_type>::ToStream(data_[index], out);
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
            data_.append(c_ptr->data_);
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

    Indicator lt(double value, const IMapType& index_map) const override {
        return DoComparison<double, T>::compare(
                [] (auto d, auto v) { return d < v; }, data_, index_map, value);
    }

    Indicator gt(double value, const IMapType& index_map) const override {
        return DoComparison<double, T>::compare(
                [] (auto d, auto v) { return d > v; }, data_, index_map, value);
    }

    Indicator le(double value, const IMapType& index_map) const override {
        return DoComparison<double, T>::compare(
                [] (auto d, auto v) { return d <= v; }, data_, index_map, value);
    }

    Indicator ge(double value, const IMapType& index_map) const override {
        return DoComparison<double, T>::compare(
                [] (auto d, auto v) { return d >= v; }, data_, index_map, value);
    }

    Indicator eq(double value, const IMapType& index_map) const override {
        return DoComparison<double, T>::compare(
                [] (auto d, auto v) { return d == v; }, data_, index_map, value);
    }

    Indicator lt(int value, const IMapType& index_map) const override {
        return DoComparison<int, T>::compare(
                [] (auto d, auto v) { return d < v; }, data_, index_map, value);
    }

    Indicator gt(int value, const IMapType& index_map) const override {
        return DoComparison<int, T>::compare(
                [] (auto d, auto v) { return d > v; }, data_, index_map, value);
    }

    Indicator le(int value, const IMapType& index_map) const override {
        return DoComparison<int, T>::compare(
                [] (auto d, auto v) { return d <= v; }, data_, index_map, value);
    }

    Indicator ge(int value, const IMapType& index_map) const override {
        return DoComparison<int, T>::compare(
                [] (auto d, auto v) { return d >= v; }, data_, index_map, value);
    }

    Indicator eq(int value, const IMapType& index_map) const override {
        return DoComparison<int, T>::compare(
                [] (auto d, auto v) { return d == v; }, data_, index_map, value);
    }

    Indicator eq(std::string value, const IMapType& index_map) const override {
        return DoComparison<std::string, T>::compare(
                [] (auto d, auto v) { return d == v; }, data_, index_map, value);
    }

    // ========================================
    //  Casting.
    // ========================================

    std::vector<int> castToInt() override {
        return castVector<int>();
    }

    std::vector<char> castToChar() override {
        return castVector<char>();
    }

    std::vector<long> castToLong() override {
        return castVector<long>();
    }

    std::vector<unsigned int> castToUInt() override {
        return castVector<unsigned int>();
    }

    std::vector<bool> castToBool() override {
        return castVector<bool>();
    }

    std::vector<float> castToFloat() override {
        return castVector<float>();
    }

    std::vector<double> castToDouble() override {
        return castVector<double>();
    }

    std::vector<std::string> castToString() override {
        return castVector<std::string>();
    }

    template<typename Type, typename Target, bool can_cast>
    struct Caster {
        static std::vector<Target> castVector(const DFVector<Type>& data) {
            std::vector<Target> output;
            output.reserve(data.size());
            for (const auto& holder : data.data_) {
                output.push_back(static_cast<Target>(holder.value_));
            }
            return output;
        }
    };

    template<typename Type, typename Target>
    struct Caster<Type, Target, false> {
        static std::vector<Target> castVector(const DFVector<Type>& data) {
            return {};
        }
    };

    template<typename Target>
    std::vector<Target> castVector() {
        if (data_.empty()) {
            return {};
        } else {
            return Caster<value_type, Target, is_castable<value_type, Target>::value>::castVector(data_);
        }
    }

    // ========================================
    //  Other functions.
    // ========================================

    void SetAll(const T& value, const IMapType& index_map) {
        if (index_map) {
            for (std::size_t index : *index_map) {
                data_[index] = value;
            }
        }
        else {
            for (auto& entry : data_.data_) {
                entry.value_ = value;
            }
        }
    }

    // ========================================
    //  Data
    // ========================================

    DFVector<value_type> data_;
};

#endif // __CONCRETE_WRAPPER_TEMPLATES_H__
