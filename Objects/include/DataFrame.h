//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef __DATA_FRAME_OBJECT_H__
#define __DATA_FRAME_OBJECT_H__

#include <list>
#include <vector>

#include "DTypes.h"

//! \brief Indicators (i.e. vectors of bools) can be returned from column comparison and used to select
//! new views of DataFrames.
using Indicator = std::vector<bool>;


class DataFrame {
public:
    // ========================================
    //  Member classes.
    // ========================================

    class Column;

    template<typename T>
    class Concrete;

    // ========================================
    //  Constructors.
    // ========================================

    //! \brief Creates an empty DataFrame.
    DataFrame();

    // ========================================
    //  Accessors.
    // ========================================

    //! \brief Check whether a DataFrame has a specific column.
    bool HasColumn(const std::string& name);

    //! \brief Returns a vector of all the column names.
    std::vector<std::string> Columns() const;

    //! \brief Returns a vector of all the dtype of the columns.
    std::vector<DType> DTypes() const;

    //! \brief Returns the number of rows in the DataFrame.
    std::size_t NumRows() const;

    //! \brief Returns the full number of rows in the columns of the DataFrame, not just
    //! those selected by the dataframe.
    std::size_t NumRowsFull() const;

    //! \brief Returns the number of columns in the DataFrame.
    std::size_t NumCols() const;

    //! \brief Returns whether the DataFrame has no columns.
    bool Empty() const;

    // ========================================
    //  Selection
    // ========================================

    //! \brief Access a column of the DataFrame.
    Column& operator[](const std::string& name);

    //! \brief Create a reference DataFrame that is a selected subset of the DataFrame.
    DataFrame operator[](const Indicator& indicator) const;

    //! \brief Explicitly create a reference copy of the DataFrame.
    DataFrame Ref() const;

    //! \brief Create a copy by value_ of the DataFrame.
    DataFrame Clone() const;

    // ========================================
    //  Merging and appending.
    // ========================================

    //! \brief Append another DataFrame onto this DataFrame. The columns of this DataFrames must be a subset of
    //! the columns of the other DataFrame, and the concrete types must match.
    //!
    //! \param df The dataframe that is being appended onto this one.
    void Append(const DataFrame& df);

    //! \brief Append a list of arguments onto this DataFrame. The (convenience) types of the arguments must
    //! match the concrete types of the columns. There must be as many arguments as there are columns in the dataframe.
    //!
    //! \tparam Args The types of the input arguments.
    //! \param args The arguments to append to the DataFrame.
    //! \return True if the append was successful.
    template<typename ...Args>
    bool Append(const Args&... args);

    // ========================================
    //  Column manipulation - Rearranging, renaming, erasing, etc.
    // ========================================

    //! \brief Rename a column. Returns true if the column existed, and there was not already a column
    //! with the final name.
    bool Rename(const std::string& initial, const std::string& final);

    //! \brief Rename columns. Returns the number of columns that were actually renamed.
    std::size_t Rename(const std::map<std::string, std::string>& renaming);

    //! \brief Drops a column from the dataframe. Returns true if a column by that name existed.
    bool DropColumn(const std::string& name);

    //! \brief Drop any empty columns. Returns the number of columns that were dropped.
    std::size_t DropEmpty();

    //! \brief Drop any none columns. Returns the number of columns that were dropped.
    std::size_t DropNones();

    // ========================================
    //  Reading and writing.
    // ========================================

    //! \brief Create a DataFrame from an istream.
    static DataFrame FromStream(std::istream& in);
    //! \brief Read a DataFrame from a csv.
    static DataFrame ReadCSV(const std::string& filename);

    //! \brief Write a representation of the DataFrame to an ostream.
    bool ToStream(std::ostream& out);
    //! \brief Write the DataFrame to a file as a csv.
    bool ToCSV(const std::string& filename);

private:
    // ========================================
    //  Typedefs.
    // ========================================

    using ColPair = std::pair<std::string, Column>;
    using StorageType = std::list<ColPair>;
    using IMapType = std::shared_ptr<std::vector<std::size_t>>;

    // ========================================
    //  Private constructors.
    // ========================================

    //! \brief Private constructor, constructs a data frame from the list of column pairs.
    explicit DataFrame(StorageType&& data, IMapType&& index_map)
        : data_(std::move(data)), index_map_(std::move(index_map)) {}

    // ========================================
    //  Private helper functions.
    // ========================================

    //! \brief Get an iterator to a column, by name. Returns data_.end() if column does not exist.
    StorageType::iterator GetColumn(const std::string& name);

    StorageType::const_iterator GetColumn(const std::string& name) const;

    //! \brief Update the index map. This sets the index_map_ variable, and updates the index maps of
    //! all columns.
    void UpdateIndexMap(IMapType index_map);

    //! \brief Takes a string returns a string that has had the beginning and ending whitespaces
    //! stripped off. Used on column names when reading CSVs.
    static std::string TrimWhiteSpaceToFit(const std::string& name) ;

    template<typename ...Args, std::size_t ...Seq>
    bool HelpAppend(std::index_sequence<Seq...>, const Args& ...args) {
        auto tuples = std::make_tuple(std::next(data_.begin(), Seq)...);
        bool types_match = util::And(std::get<Seq>(tuples)->second.template IsType<ConvenienceType_t<Args>>()...);
        if (!types_match) {
            return false;
        }
        util::Null((std::get<Seq>(tuples)->second.template GetConcrete<ConvenienceType_t<Args>>()
                .push_back(ConvenienceType_t<Args>(args)), true)...);
        // Since a row has been added, we have to update the index map.
        index_map_->push_back(NumRowsFull());
        return true;
    }

    // ========================================
    //  Member data.
    // ========================================

    //! \brief The dataframe's actual data.
    StorageType data_;

    //! \brief The index map for this data frame.
    IMapType index_map_;
};


// ========================================
//  Define template functions.
// ========================================

template<typename ...Args>
bool DataFrame::Append(const Args&... args) {
    if (sizeof...(args) != NumCols()) {
        return false;
    }
    return HelpAppend(std::make_index_sequence<sizeof...(args)>{}, args...);
}

#endif // __DATA_FRAME_OBJECT_H__
