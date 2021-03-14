//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#include "../include/DataFrame.h"
// Other files
#include <fstream>
#include <sstream>
#include "../include/TypeConversion.h"
#include "../include/templates/Column.h"

#include <iostream>
#include <utility>


DataFrame::DataFrame() : index_map_(std::make_shared<std::vector<std::size_t>>()) {}

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
        return index_map_->size();
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
        data_.emplace_back(name, Column(DType::None, index_map_, NumRows()));
        return data_.back().second;
    }
    else {
        return it->second;
    }
}

DataFrame DataFrame::operator[](const Indicator& indicator) const {
    if (indicator.size() != NumRows()) {
        return DataFrame();
    }
    auto df = Ref();

    auto index_map = std::make_shared<std::vector<std::size_t>>();
    for (std::size_t i = 0; i < indicator.size(); ++i) {
        bool ind = indicator[i];
        if (ind) {
            index_map->push_back((*index_map_)[i]);
        }
    }
    df.UpdateIndexMap(index_map);
    return df;
}

DataFrame DataFrame::Ref() const {
    return *this;
}

DataFrame DataFrame::Clone() const {
    auto df = DataFrame();
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
    std::vector<StorageType::const_iterator> iters;
    for (const auto& col : data_) {
        auto& name = col.first;
        auto it = df.GetColumn(name);
        if (!col.second.SameTypeAs(it->second)) {
            std::cout << "Types do not match for column " << col.first << ". DTypes are " \
                << col.second.GetDType() << " and " << it->second.GetDType() << "\n";
            return;
        }
        if (it == df.data_.end()) {
            std::cout << "Did not find column " << col.first << " in DF.\n";
            return;
        }
        iters.push_back(it);
    }

    // Record what the next index in index_map_ needs to be.
    std::size_t next_index = NumRowsFull();

    // If we make it here, all columns matched.
    std::size_t index = 0;
    for (auto& col : data_) {
        auto& column = col.second;
        if (!column.Append(iters[index]->second)) {
            throw std::exception(); // \TODO: Handle failure?
        }
        ++index;
    }

    // Update the index map.
    for (std::size_t i = 0; i < df.NumRows(); ++i) {
        index_map_->push_back(next_index + i);
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
    auto index_map = std::make_shared<std::vector<std::size_t>>();
    for (const auto& name : colNames) {
        internal.emplace_back(name, Column(DType::None, index_map));
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
        index_map->push_back(numRows);
        ++numRows;
    }

    return DataFrame(std::move(internal), std::move(index_map));
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

DataFrame::StorageType::iterator DataFrame::GetColumn(const std::string& name) {
    return std::find_if(data_.begin(), data_.end(),
                        [&](const auto& pr) { return pr.first == name; });
}

DataFrame::StorageType::const_iterator DataFrame::GetColumn(const std::string& name) const {
    return std::find_if(data_.cbegin(), data_.cend(),
                        [&](const auto& pr) { return pr.first == name; });
}

void DataFrame::UpdateIndexMap(IMapType index_map) {
    index_map_ = std::move(index_map);
    for (auto& pr : data_) {
        pr.second.index_map_ = index_map_;
    }
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