//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#include "Column.h"

using Column = DataFrame::Column;

Indicator operator<(const Column& colA, const Column& colB) {
    return colA.box_->wrapper_->cmp_less(colB.box_->wrapper_);
}

bool Column::operator==(const DataFrame::Column& rhs) {
    // If both columns are references to the same data, their data is necessarily equal.
    if (IsRefOf(rhs)) {
        return true;
    } else {
        return box_->wrapper_->CheckEquals(rhs.box_->wrapper_);
    }
}


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
        // Types were not the same. Need to retype.
        box_->wrapper_->is_orphan = true;
        box_->wrapper_ = rhs.box_->wrapper_->Clone();
    }
    return *this;
}

bool Column::IsRefOf(const DataFrame::Column& rhs) const {
    return box_->wrapper_ == rhs.box_->wrapper_;
}

bool Column::SameTypeAs(const DataFrame::Column& rhs) const {
    return box_->wrapper_->IsSameType(rhs.box_->wrapper_);
}

bool Column::IsNull() const {
    return box_->wrapper_ == nullptr;
}

std::size_t Column::Size() const {
    return box_->wrapper_->Size();
}

bool Column::Empty() const {
    return box_->wrapper_->Empty();
}

DType Column::GetDType() const {
    return box_->wrapper_->GetDType();
}

void Column::ToStream(std::size_t index, std::ostream& out) const {
    box_->wrapper_->ToStream(index, out);
}

bool Column::Append(const Column& col) {
    return box_->wrapper_->Append(col.box_->wrapper_);
}