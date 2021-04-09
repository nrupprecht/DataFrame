//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#include "../include/templates/Column.h"

using Column = DataFrame::Column;


// ========================================
//  Logical operators.
// ========================================

Indicator operator<(const Column& col, double rhs) {
    return col.box_->wrapper_->lt(rhs, col.index_map_);
}

Indicator operator<(const Column& col, int rhs) {
    return col.box_->wrapper_->lt(rhs, col.index_map_);
}

Indicator operator<(double rhs, const Column& col) {
    return col.box_->wrapper_->gt(rhs, col.index_map_);
}

Indicator operator<(int rhs, const Column& col) {
    return col.box_->wrapper_->gt(rhs, col.index_map_);
}

Indicator operator<=(const Column& col, double rhs) {
    return col.box_->wrapper_->le(rhs, col.index_map_);
}

Indicator operator<=(const Column& col, int rhs) {
    return col.box_->wrapper_->le(rhs, col.index_map_);
}

Indicator operator<=(double rhs, const Column& col) {
    return col.box_->wrapper_->ge(rhs, col.index_map_);
}

Indicator operator<=(int rhs, const Column& col) {
    return col.box_->wrapper_->ge(rhs, col.index_map_);
}

Indicator operator>(const Column& col, double rhs) {
    return rhs < col;
}

Indicator operator>(const Column& col, int rhs) {
    return rhs < col;
}

Indicator operator>(double rhs, const Column& col) {
    return col < rhs;
}

Indicator operator>(int rhs, const Column& col) {
    return col < rhs;
}

Indicator operator>=(const Column& col, double rhs) {
    return rhs <= col;
}

Indicator operator>=(const Column& col, int rhs) {
    return rhs <= col;
}

Indicator operator>=(double rhs, const Column& col) {
    return col <= rhs;
}

Indicator operator>=(int rhs, const Column& col) {
    return col <= rhs;
}

Indicator operator==(const Column& col, double rhs) {
    return col.box_->wrapper_->eq(rhs, col.index_map_);
}

Indicator operator==(const Column& col, int rhs) {
    return col.box_->wrapper_->eq(rhs, col.index_map_);
}

Indicator operator==(double rhs, const Column& col) {
    return col == rhs;
}

Indicator operator==(int rhs, const Column& col) {
    return col == rhs;
}

Indicator operator==(const std::string& rhs, const Column& col) {
    return col.box_->wrapper_->eq(rhs, col.index_map_);
}

Indicator operator==(const Column& col, const std::string& rhs) {
    return col.box_->wrapper_->eq(rhs, col.index_map_);
}

Indicator operator<(const Column& colA, const Column& colB) {
    // \TODO: Work with index map.
    return colA.box_->wrapper_->cmp_less(colB.box_->wrapper_);
}

Indicator operator>(const Column& colA, const Column& colB) {
    return colB < colA;
}

bool Column::operator==(const DataFrame::Column& rhs) {
    // If both columns are references to the same data, their data is necessarily equal.
    if (IsRefOf(rhs)) {
        return true;
    } else {
        return box_->wrapper_->CheckEquals(rhs.box_->wrapper_);
    }
}

// ========================================
//  Assignment.
// ========================================

DataFrame::Column& DataFrame::Column::operator=(const DataFrame::Column& rhs) {
    if (IsRefOf(rhs)) { // Self assignment, or trying to assign to a ref of this column.
        return *this;
    }
    // Make sure sizes match, or the column is empty.
    if (rhs.Size() != Size() && GetDType() != DType::None) {
        throw std::exception();
    }
    // Copy data if the types are the same.
    if (!box_->wrapper_->Copy(rhs.box_->wrapper_)) {
        if (IsFullColumn()) {
            // Types were not the same. Need to retype.
            box_->wrapper_->is_orphan = true;
            box_->wrapper_ = rhs.box_->wrapper_->Clone();
        }
        else {
            // Can't change the type of a column that isn't a full column.
            throw std::exception();
        }
    }
    return *this;
}

bool Column::IsRefOf(const DataFrame::Column& rhs) const {
    return box_->wrapper_ == rhs.box_->wrapper_;
}

bool Column::SameTypeAs(const DataFrame::Column& rhs) const {
    return box_->wrapper_->IsSameType(rhs.box_->wrapper_);
}

bool Column::IsFullColumn() const {
    return Size() == box_->wrapper_->Size();
}

std::size_t Column::Size() const {
    if (index_map_) {
        return index_map_->size();
    } else {
        return box_->wrapper_->Size();
    }
}

bool Column::Empty() const {
    if (index_map_) {
        return index_map_->empty();
    } else {
        return box_->wrapper_->Empty();
    }
}

DType Column::GetDType() const {
    return box_->wrapper_->GetDType();
}

void Column::ToStream(std::size_t index, std::ostream& out) const {
    if (index_map_) {
        box_->wrapper_->ToStream((*index_map_)[index], out);
    } else {
        box_->wrapper_->ToStream(index, out);
    }
}

DataFrame::Column::Column(DType dtype, IMapType index_map, std::size_t size)
: index_map_(std::move(index_map)) {
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

bool Column::Append(const Column& col) {
    // Note: This function does not update the index map, DataFrame will take care of that.
    return box_->wrapper_->Append(col.box_->wrapper_);
}

std::size_t Column::FullSize() const {
    return box_->wrapper_->Size();
}