//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#include "DataFrame.h"
// Other files
#include <fstream>
#include <sstream>
#include "TypeConversion.h"
#include "Column.h"


bool DataFrame::HasColumn(const std::string& name) {
    return GetColumn(name) != data_.end();
}

//! \brief Returns a vector of all the column names.
std::vector<std::string> DataFrame::Columns() const {
    std::vector<std::string> output;
    std::for_each(data_.begin(), data_.end(), [&](auto pr) { output.push_back(pr.first); });
    return output;
}

DataFrame::Column& DataFrame::operator[](const std::string& name) {
    auto it = GetColumn(name);
    if (it == data_.end()) {
        data_.emplace_back(name, Column(DType::None, NumRows()));
        return data_.back().second;
    }
    else {
        return it->second;
    }
}

std::size_t DataFrame::NumRows() const {
    if (Empty()) {
        return 0;
    }
    else {
        return data_.front().second.Size();
    }
}

std::size_t DataFrame::NumCols() const {
    return data_.size();
}

bool DataFrame::Empty() const {
    return data_.empty();
}

void DataFrame::Append(const DataFrame& df) {

}

// ========================================
//  Reading and writing.
// ========================================

DataFrame DataFrame::from_stream(std::istream& instream) {
    // First, look for columns.
    int numUnnamedColumns = 0;

    std::vector<std::string> colNames;
    std::string data;
    {
        getline(instream, data);
        std::istringstream stream(data);
        while (getline(stream, data, ',')) {
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
    while (getline(instream, data)) {

        std::istringstream stream(data);

        auto it = internal.begin();
        auto dt = dtype_record.begin();
        while (getline( stream, data, ',' )) {
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

DataFrame DataFrame::read_csv(const std::string& filename) {
    std::ifstream fin(filename);
    if (fin.fail()) {
        return DataFrame();
    }
    auto df = from_stream(fin);
    fin.close();
    return df;
}

bool DataFrame::to_stream(std::ostream& out) {
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

bool DataFrame::to_csv(const std::string& filename) {
    std::ofstream fout(filename);
    if (fout.fail()) {
        return false;
    }
    bool status = to_stream(fout);
    fout.close();
    return status;
}

// ========================================
//  Private helper functions.
// ========================================

DataFrame::StorageType::iterator DataFrame::GetColumn(const std::string& name) {
    return std::find_if(data_.begin(), data_.end(),
                        [&](auto pr) { return pr.first == name; });
}