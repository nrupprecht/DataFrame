//
// Created by Nathaniel Rupprecht on 3/13/21.
//

#ifndef __WRAPPER_TEMPLATES_H__
#define __WRAPPER_TEMPLATES_H__

// This file is INCLUDED in Column.h

//! \brief A base class for child classes with concrete storage type.
struct DataFrame::Column::Wrapper {
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

    //! \brief Return a copy of the wrapper, only keeping the data indicated by the index_map.
    virtual std::shared_ptr<Wrapper> Clone(const IMapType& index_map) const = 0;

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

    virtual Indicator lt(double value, const IMapType& index_map) const = 0;
    virtual Indicator gt(double value, const IMapType& index_map) const = 0;
    virtual Indicator le(double value, const IMapType& index_map) const = 0;
    virtual Indicator ge(double value, const IMapType& index_map) const = 0;
    virtual Indicator eq(double value, const IMapType& index_map) const = 0;

    virtual Indicator lt(int value, const IMapType& index_map) const = 0;
    virtual Indicator gt(int value, const IMapType& index_map) const = 0;
    virtual Indicator le(int value, const IMapType& index_map) const = 0;
    virtual Indicator ge(int value, const IMapType& index_map) const = 0;
    virtual Indicator eq(int value, const IMapType& index_map) const = 0;

    virtual Indicator eq(std::string value, const IMapType& index_map) const = 0;

    // ========================================
    //  Casting.
    // ========================================

    virtual std::vector<int> castToInt() = 0;
    virtual std::vector<char> castToChar() = 0;
    virtual std::vector<long> castToLong() = 0;
    virtual std::vector<unsigned int> castToUInt() = 0;
    virtual std::vector<bool> castToBool() = 0;
    virtual std::vector<float> castToFloat() = 0;
    virtual std::vector<double> castToDouble() = 0;
    virtual std::vector<std::string> castToString() = 0;

    // ========================================
    //  Data.
    // ========================================

    //! \brief Records whether the wrapper has been orphaned (disconnected from it's
    //! original box).
    bool is_orphan = false;
};

#endif // __WRAPPER_TEMPLATES_H__
