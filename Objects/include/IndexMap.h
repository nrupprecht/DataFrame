//
// Created by Nathaniel Rupprecht on 5/8/21.
//

#ifndef DATAFRAME_INDEXMAP_H
#define DATAFRAME_INDEXMAP_H

#include <vector>
#include <memory>

namespace dataframe {

class IndexMap {
public:
    IndexMap(const std::vector<std::size_t> &indices, bool empty, std::size_t size)
            : impl_(std::make_shared<Impl>(indices, empty, size)) {}

    IndexMap(std::vector<std::size_t> &&indices, bool empty, std::size_t size)
            : impl_(std::shared_ptr<Impl>(new Impl(std::move(indices), empty, size))) {}

    bool IsEmpty() const { return impl_->IsEmpty(); }

    bool IsFull() const { return impl_->IsFull(); }

    std::size_t Size() const { return impl_->Size(); }

    std::size_t operator[](std::size_t index) const { return (*impl_)[index]; }

    std::size_t &operator[](std::size_t index) { return (*impl_)[index]; }

    //! \brief Create a copy by value of this index map.
    IndexMap Clone() const { return IndexMap(impl_->indices_, impl_->empty_, impl_->size_); }

private:
    //! \brief The implementation for the IndexMap. This allows different IndexMaps, copied
    //! by value, to reference the same underlying data.
    struct Impl {
        Impl(const std::vector<std::size_t> &indices, bool empty, std::size_t size)
                : indices_(indices), empty_(empty), size_(size) {}

        Impl(std::vector<std::size_t> &&indices, bool empty, std::size_t size)
                : indices_(std::move(indices)), empty_(empty), size_(size) {}

        bool IsEmpty() const { return empty_; }

        bool IsFull() const { return !empty_ && indices_.empty(); }

        std::size_t Size() const { return size_; }

        std::size_t &operator[](std::size_t index) { return indices_[index]; }

        // =========================================================
        //  Member data.
        // =========================================================

        //! A map between input indices and the index in the actual underlying data vector.
        std::vector<std::size_t> indices_;

        //! \brief Since an empty vector refers to *all* indices being included,
        //! we need a different flag to track if an empty vector actually means
        //! *no* indices should be included.
        bool empty_ = false;

        //! \brief Records the size of the index map. When indices_ is not empty, this will
        //! be the same as indices_.size(). When indices_ is empty and empty_ is false,
        //! this records the
        std::size_t size_ = 0;
    };

    //! \brief The implementation for the IndexMap.
    std::shared_ptr<Impl> impl_;
};

}
#endif //DATAFRAME_INDEXMAP_H
