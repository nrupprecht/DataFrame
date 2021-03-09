//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef __DATA_FRAME_OBJECT_H__
#define __DATA_FRAME_OBJECT_H__

#include <list>
#include <vector>

#include "DTypes.h"

class DataFrame {
public:
    class Column;

    template<typename T>
    class Concrete;

    // ========================================
    //  Constructors.
    // ========================================

    //! \brief Creates an empty DataFrame.
    DataFrame() = default;

    // ========================================
    //  Accessors.
    // ========================================

    //! \brief Check whether a DataFrame has a specific column.
    bool HasColumn(const std::string& name);

    //! \brief Returns a vector of all the column names.
    std::vector<std::string> Columns() const;

    //! \brief Access a column of the DataFrame.
    Column& operator[](const std::string& name);

    //! \brief Returns the number of rows in the DataFrame.
    std::size_t NumRows() const;

    //! \brief Returns the number of columns in the DataFrame.
    std::size_t NumCols() const;

    //! \brief Returns whether the DataFrame has no columns.
    bool Empty() const;

    // ========================================
    //  Merging and appending.
    // ========================================

    //! \brief Append another DataFrame onto this DataFrame. The columns of the DataFrames must match.
    void Append(const DataFrame& df);

    template<typename ...Args>
    bool Append(const Args&... args);

    // ========================================
    //  Reading and writing.
    // ========================================

    static DataFrame FromStream(std::istream& in);
    static DataFrame ReadCSV(const std::string& filename);

    bool ToStream(std::ostream& out);
    bool ToCSV(const std::string& filename);

private:
    typedef std::pair<std::string, Column> ColPair;
    typedef std::list<ColPair> StorageType;

    //! \brief Private constructor, constructs a data frame from the list of column pairs.
    explicit DataFrame(StorageType&& data) : data_(std::move(data)) {}

    //! \brief Get an iterator to a column, by name. Returns data_.end() if column does not exist.
    StorageType::iterator GetColumn(const std::string& name);

    StorageType::const_iterator GetColumn(const std::string& name) const;

    template<typename ...Args, std::size_t ...Seq>
    bool HelpAppend(std::index_sequence<Seq...>, const Args& ...args) {
        auto tuples = std::make_tuple(std::next(data_.begin(), Seq)...);
        bool types_match = util::And(std::get<Seq>(tuples)->second.template IsType<ConvenienceType_t<Args>>()...);
        if (!types_match) {
            return false;
        }
        util::Null((std::get<Seq>(tuples)->second.template GetConcrete<ConvenienceType_t<Args>>().push_back(args), true)...);
        return true;
    }

    //! \brief The dataframe's actual data.
    StorageType data_;
};




template<typename ...Args>
bool DataFrame::Append(const Args&... args) {
    if (sizeof...(args) != NumCols()) {
        return false;
    }
    return HelpAppend(std::make_index_sequence<sizeof...(args)>{}, args...);
}

#endif // __DATA_FRAME_OBJECT_H__
