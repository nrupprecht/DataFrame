//
// Created by Nathaniel Rupprecht on 2/27/21.
//

#ifndef __DATAFRAME_CONCRETE_H__
#define __DATAFRAME_CONCRETE_H__

#include <set>
#include "Column.h"
#include "DataFrame.h"


namespace dataframe {

    template<typename T>
    class DataFrame::Concrete {
    public:
        // ========================================
        //  Typedefs.
        // ========================================

        using value_type = T;

        // ========================================
        //  Constructors.
        // ========================================

        //! \brief Create an empty concrete column.
        Concrete() : wrapper_(std::make_shared<WrapperType>()) {}

        // ========================================
        //  Accessors.
        // ========================================

        //! \brief Return the size of the concrete column. Because of masking, this may be less than the size
        //! of the data vector.
        std::size_t Size() const {
            if (index_map_) {
                return index_map_->size();
            } else {
                return wrapper_->data_.size();
            }
        }

        //! \brief Return whether the concrete column is empty. Because of masking, this may be true even when the
        //! data vector is not empty.
        bool Empty() const {
            if (index_map_) {
                return index_map_->empty();
            } else {
                return wrapper_->data_.empty();
            }
        }

        //! \brief Get the dtype of the data.
        DType GetDType() const { return wrapper_->GetDType(); }

        //! \brief Whether the concrete column has been orphaned. This will happen if the type of the column that this
        //! concrete column references changes in the DataFrame.
        bool IsOrphan() const { return wrapper_->is_orphan; }

        //! \brief Reference access.
        T& operator[](std::size_t index) {
            if (index_map_) {
                return wrapper_->data_[(*index_map_)[index]];
            } else { // A null index map means we don't need an index map.
                return wrapper_->data_[index];
            }
        }

        //! \brief Constant access.
        const T& operator[](std::size_t index) const {
            if (index_map_) {
                return wrapper_->data_[(*index_map_)[index]];
            } else { // A null index map means we don't need an index map.
                return wrapper_->data_[index];
            }
        }

        //! \brief Get a set of all the unique values in the concrete column.
        std::set<T> Unique() const {
            std::set<T> output;
            for (std::size_t i = 0; i < Size(); ++i) {
                output.insert((*this)[i]);
            }
            return output;
        }

        // ========================================
        //  Friend classes.
        // ========================================

        friend class DataFrame;
        friend class DataFrame::Column;
    private:
        //! \brief The full type of the concrete wrapper.
        using WrapperType = DataFrame::Column::ConcreteWrapper<T>;

        //! \brief Create a concrete column from a concrete wrapper and an index map.
        explicit Concrete(std::shared_ptr<WrapperType> wrapper, IMapType index_map)
            : wrapper_(std::move(wrapper)), index_map_(std::move(index_map)) {}

        //! \brief Private method to add data to the concrete column. Only a DataFrame can add data to columns.
        void push_back(const T& value) {
            wrapper_->data_.push_back(value);
        }

        //! \brief Return the full size of underlying column the concrete points to. If this concrete was selected as a
        //! subsets of a data frame, the size and full size will differ.
        std::size_t FullSize() const {
            return wrapper_->data_.size();
        }

        //! \brief Return the index map and the next index that should be inserted into the index map (if an index map
        //! is needed).
        std::pair<std::vector<std::size_t>*, std::size_t> GetIndexData() const {
            return std::make_pair(index_map_.get(), FullSize());
        }

        //! \brief A pointer to the concrete wrapper that contains the data.
        std::shared_ptr<WrapperType> wrapper_;

        //! \brief A pointer to the index map.
        IMapType index_map_;
    };

}
#endif // __DATAFRAME_CONCRETE_H__
