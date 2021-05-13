//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#include "../include/DataFrame.h"
// Other files
#include <fstream>
#include <sstream>
#include "../include/TypeConversion.h"
#include "../include/Column.h"

#include <iostream>
#include <utility>

using namespace dataframe;

bool DataFrame::HasColumn(const std::string& name) {
    return GetColumn(name) != data_.end();
}

//! \brief Returns a vector of all the column names.
std::vector<std::string> DataFrame::Columns() const {
    std::vector<std::string> output;
    std::for_each(data_.begin(), data_.end(),
                  [&](auto pr) { output.push_back(pr.first); });
    return output;
}

std::vector<DType> DataFrame::DTypes() const {
    std::vector<DType> output;
    std::for_each(data_.begin(), data_.end(),
                  [&](auto pr) { output.push_back(pr.second.GetDType()); });
    return output;
}

std::size_t DataFrame::NumRows() const {
    if (Empty()) {
        return 0;
    } else {
        return data_.begin()->second.Size();
    }
}

std::size_t DataFrame::NumRowsFull() const {
    if (Empty()) {
        return 0;
    } else {
        return data_.front().second.Size();
    }
}

std::size_t DataFrame::NumCols() const {
    return data_.size();
}

bool DataFrame::Empty() const {
    return data_.empty();
}

// ========================================
//  Selection
// ========================================

DataFrame::Column& DataFrame::operator[](const std::string& name) {
    auto it = GetColumn(name);
    if (it == data_.end()) {
        data_.emplace_back(name, Column(DType::None, nullptr, NumRows()));
        return data_.back().second;
    }
    else {
        return it->second;
    }
}

DataFrame DataFrame::operator[](const Indicator& indicator) const {
    if (indicator.size() != NumRows()) {
        return DataFrame(); // Return an empty data frame upon failure.
    }

    // Check if the indicator is all true (in which case we can just return a reference to this data frame)
    // or all false (in which case we can return dataframes where all the index maps are non-null, but empty).
    bool all_true = true, all_false = true;
    for (bool ind : indicator) {
        if (ind) {
            all_false = false;
        }
        else {
            all_true = false;
        }
        if (!all_true && !all_false) {
            break;
        }
    }
    if (all_true) {
        return Ref(); // Return a data frame that references all that data in this data frame.
    }
    if (all_false) {
        auto df = Ref();
        for (auto& col_pair : df.data_) {
            col_pair.second.index_map_ = MakeIndexMap();
        }
    }

    // Create a reference copy of this data frame.
    auto df = Ref();

    // A map from old index maps (raw ptrs b/c shared ptrs might be different ) to new index maps.
    std::map<std::vector<std::size_t>*, IMapType> new_index_maps;

    // auto index_map = std::make_shared<std::vector<std::size_t>>();
    for (const auto& col_pair : data_) {
        auto& col = col_pair.second;
        // Check if we already processed the index map that this column uses.
        auto it = new_index_maps.find(col.index_map_.get());
        if (it != new_index_maps.end()) {
            continue;
        }
        // We have not processed this column's index map yet. Do so and save it.
        auto new_index_map = MakeIndexMap();
        if (col.index_map_ == nullptr) {
            for (std::size_t i = 0; i < indicator.size(); ++i) {
                if (indicator[i]) {
                    new_index_map->push_back(i);
                }
            }
        }
        else {
            for (std::size_t i = 0; i < indicator.size(); ++i) {
                if (indicator[i]) {
                    new_index_map->push_back((*col.index_map_)[i]);
                }
            }
        }
        // Add the new index map to the collection of new index maps.
        new_index_maps.emplace(col.index_map_.get(), new_index_map);
    }

    // Update all the index maps in the columns of the new data frame.
    for (auto& col_pair : df.data_) {
        auto& col = col_pair.second;
        auto it = new_index_maps.find(col.index_map_.get());
        col.index_map_ = it->second; // Update the index map to be the new index map.
    }

    return df;
}

DataFrame DataFrame::Ref() const {
    return *this;
}

DataFrame DataFrame::Clone() const {
    auto df = DataFrame();
    for (const auto& col : data_) {
        df[col.first] = col.second.Clone();
    }
    // \TODO: Actual cloning part.
    return df;
}

// ========================================
//  Merging and appending.
// ========================================

void DataFrame::Append(const DataFrame& df) {
    // Check that the dataframes have the same columns, or that the columns
    // of this DataFrame are a subset of those of df.
    if (df.NumCols() < NumCols()) {
        return;
    }
    // If this column is empty, set this dataframe to be a copy (non-reference) of the
    // appended dataframe
    if (Empty()) {
        *this = df.Clone();
        return;
    }
    // Figure out which columns should be appended.
    std::vector<StorageType::const_iterator> iters;
    for (const auto& col : data_) {
        auto& name = col.first;
        auto it = df.GetColumn(name);
        if (!col.second.SameTypeAs(it->second)) {
            std::cout << "Types do not match for column " << col.first << ". DTypes are " \
                << col.second.GetDType() << " and " << it->second.GetDType() << "\n";
            return; // TODO: Error handling.
        }
        if (it == df.data_.end()) {
            std::cout << "Did not find column " << col.first << " in DF.\n";
            return; // TODO: Error handling.
        }
        iters.push_back(it);
    }
    // If we make it here, all columns matched.

    // Update all the columns' index maps.
    AddEntriesToIndexMap(df.NumRows());
    // Add the data from the second dataframe to all the columns.
    std::size_t index = 0;
    for (auto& col : data_) {
        auto& column = col.second;
        if (!column.Append(iters[index]->second)) {
            throw std::exception(); // \TODO: Handle failure?
        }
        ++index;
    }
}

// ========================================
//  Rearranging, renaming, etc.
// ========================================

bool DataFrame::Rename(const std::string& initial, const std::string& final) {
    if (HasColumn(final)) {
        return false;
    }
    auto it = GetColumn(initial);
    if (it != data_.end()) {
        it->first = final;
        return true;
    }
    return false;
}

std::size_t DataFrame::Rename(const std::map<std::string, std::string>& renaming) {
    std::size_t count = 0;
    for (const auto& pr : renaming) {
        if (Rename(pr.first, pr.second)) {
            ++count;
        }
    }
    return count;
}

bool DataFrame::DropColumn(const std::string& name) {
    auto it = GetColumn(name);
    if (it != data_.end()) {
        data_.erase(it);
        return true;
    }
    return false;
}

std::size_t DataFrame::DropEmpty() {
    std::size_t count_drops = 0;
    for (auto it = data_.begin(); it != data_.end();) {
        if (it->second.GetDType() == DType::Empty) {
            auto jt = it;
            ++it;
            data_.erase(jt);
            ++count_drops;
        }
        else {
            ++it;
        }
    }
    return count_drops;
}

std::size_t DataFrame::DropNones() {
    std::size_t count_drops = 0;
    for (auto it = data_.begin(); it != data_.end();) {
        if (it->second.GetDType() == DType::None) {
            auto jt = it;
            ++it;
            data_.erase(jt);
            ++count_drops;
        }
        else {
            ++it;
        }
    }
    return count_drops;
}

// ========================================
//  Reading and writing.
// ========================================

DataFrame DataFrame::FromStream(std::istream& in) {
    // First, look for columns.
    int numUnnamedColumns = 0;

    std::vector<std::string> colNames;
    std::string data;
    {
        getline(in, data);
        std::istringstream stream(data);
        while (getline(stream, data, ',')) {
            data = TrimWhiteSpaceToFit(data);
            if (data.empty()) {
                colNames.push_back("Unnamed:" + std::to_string(numUnnamedColumns));
                ++numUnnamedColumns;
            }
            else {
                colNames.push_back(data);
            }
        }
    }

    // Set up the column storage.
    StorageType internal;
    for (const auto& name : colNames) {
        internal.emplace_back(name, Column(DType::None));
    }
    // Record the assumed dtype of every column. This will be updated as necessary.
    std::vector<DType> dtype_record(colNames.size(), DType::None);

    // Get lines as long as possible.
    int numRows = 0;
    while (getline(in, data)) {
        std::istringstream stream(data);

        auto it = internal.begin();
        auto dt = dtype_record.begin();
        while (getline( stream, data, ',' )) {
            // Trim whitespaces
            data = TrimWhiteSpaceToFit(data);
            // If the row has no type yet, or has been empty so far.
            if (*dt == DType::None || *dt == DType::Empty) {
                auto dtype = CheckDType(data);
                if (dtype != DType::Empty) {
                    // \TODO: Need to check if dtype can support NaNs, if there have been any.
                    if (!it->second.box_->ConvertDType(dtype)) {
                        throw std::exception();
                    }
                    it->second.box_->wrapper_->AddByString(data);
                }
                *dt = dtype;
            }
            else {
                if (!RecheckDType(data, *dt)) {
                    // Change type if possible.
                    auto dtype = CheckDType(data);
                    if (!it->second.box_->ConvertDType(dtype)) {
                        throw std::exception();
                    }
                    *dt = dtype;
                }
                it->second.box_->wrapper_->AddByString(data);
            }
            ++it, ++dt;
        }
        ++numRows;
    }

    return DataFrame(std::move(internal));
}

DataFrame DataFrame::ReadCSV(const std::string& filename) {
    std::ifstream fin(filename);
    if (fin.fail()) {
        return DataFrame();
    }
    auto df = FromStream(fin);
    fin.close();
    return df;
}

bool DataFrame::ToStream(std::ostream& out) {
    std::size_t i = 0;
    // Print column names.
    for (const auto& pr : data_) {
        out << pr.first;
        if (i != NumCols() - 1) {
            out << ",";
        }
        ++i;
    }
    out << "\n";
    // Print data.
    for (std::size_t it = 0; it < NumRows(); ++it) {
        i = 0;
        for (const auto& pr : data_) {
            pr.second.ToStream(it, out);
            if (i != NumCols() - 1) {
                out << ",";
            }
            ++i;
        }
        out << "\n";
    }

    return true;
}

bool DataFrame::ToCSV(const std::string& filename) {
    std::ofstream fout(filename);
    if (fout.fail()) {
        return false;
    }
    bool status = ToStream(fout);
    fout.close();
    return status;
}

// ========================================
//  Private helper functions.
// ========================================

DataFrame::IMapType DataFrame::MakeIndexMap() {
    return std::make_shared<std::vector<std::size_t>>();
}

void DataFrame::AddEntriesToIndexMap(std::size_t num_entries) {
    std::map<std::vector<std::size_t>*, std::size_t> index_maps;
    for (const auto& col_pair : data_) {
        auto& col = col_pair.second;
        if (index_maps.find(col.index_map_.get()) == index_maps.end()) {
            index_maps.emplace(col.index_map_.get(), col.FullSize());
        }
    }
    for (const auto pr : index_maps) {
        if (pr.first) {
            for (std::size_t i = 0; i < num_entries; ++i) {
                pr.first->push_back(pr.second + i);
            }
        }
    }
}

DataFrame::StorageType::iterator DataFrame::GetColumn(const std::string& name) {
    return std::find_if(data_.begin(), data_.end(),
                        [&](const auto& pr) { return pr.first == name; });
}

DataFrame::StorageType::const_iterator DataFrame::GetColumn(const std::string& name) const {
    return std::find_if(data_.cbegin(), data_.cend(),
                        [&](const auto& pr) { return pr.first == name; });
}

std::string DataFrame::TrimWhiteSpaceToFit(const std::string& name) {
    std::string output;
    if (name.empty()) {
        return output;
    }
    std::size_t i = 0;
    // Pass initial whitespaces.
    for (; i < name.size() && isspace(name[i]); ++i);
    // Copy the rest of the string to the output buffer.
    std::copy(&name[i], &name[0] + name.size(), std::back_inserter(output));
    // Remove trailing whitespaces.
    while (!output.empty() && isspace(output.back())) {
        output.pop_back();
    }

    return output;
}