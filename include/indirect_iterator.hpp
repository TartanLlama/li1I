#pragma once

namespace li1I
{
    template <class I>
    class indirect_iterator {
        I base_;

    public:
        indirect_iterator()=default;
        indirect_iterator(I base) : base_(std::move(base)) {}

        auto operator->() const {
            return &**base_;
        }

        decltype(auto) operator*() const {
            return **base_;
        }

        indirect_iterator& operator++() {
            ++base_;
            return *this;
        }

        friend bool operator==(indirect_iterator const& lhs, indirect_iterator const& rhs) {
            return lhs.base_ == rhs.base_;
        }
        
        friend bool operator!=(indirect_iterator const& lhs, indirect_iterator const& rhs) {
            return lhs.base_ != rhs.base_;
        }
    };
}