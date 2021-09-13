#pragma once

#include <algorithm>
#include <vector>

#include "chunky_mem/version.h"
#include "chunky_mem/linked_list.h"

namespace chunky_mem {

    template<typename idx_t = int, typename size_t = idx_t>
    class IndexSetLinkedListVector : protected LinkedList<idx_t, size_t>
    {
    public:
        using idx_type = idx_t;
        using size_type = size_t;
        using linked_list_type = LinkedList<idx_type, size_type>;
        using Item = typename linked_list_type::Item;
        using Info = typename linked_list_type::Info;

    protected:
        std::vector<Item> m_items;
    public:

        IndexSetLinkedListVector(const IndexSetLinkedListVector& other)
            : m_items(other.m_items)
        {
            info = other.info;
            items = m_items.data();
        }
        IndexSetLinkedListVector(size_type capacity=8)
        {
            ensureSize(capacity);
            clear();
        }

        inline bool fixCapacity() const { return false; }

        using LinkedList::empty;
        using LinkedList::size;
        using LinkedList::capacity;
        using LinkedList::contains;
        using LinkedList::front;
        using LinkedList::back;
        using LinkedList::prev;
        using LinkedList::next;
        using LinkedList::find;
        using LinkedList::find_forward;
        using LinkedList::find_backward;
        using LinkedList::clear;
        using LinkedList::push_front;
        using LinkedList::push_back;
        using LinkedList::pop_front;
        using LinkedList::pop_back;
        using LinkedList::remove;
        // using LinkedList::insert;
        // using LinkedList::insert_forward;
        // using LinkedList::insert_backward;
        // using LinkedList::construct;
        using LinkedList::push;
        using LinkedList::pop;

        inline bool push_front(const idx_type& idx)
        {
            return push(Item::PREV, idx);
        }

        inline bool push_back(const idx_type& idx)
        {
            return push(Item::NEXT, idx);
        }

        inline bool insert(
            const idx_type& behind_item,
            const idx_type& idx
        )
        {
            return insert_forward(behind_item, idx);
        }

        inline bool insert_forward(
            const idx_type& behind_item,
            const idx_type& idx
        )
        {
            return insert(
                behind_item,
                Item::NEXT,
                idx
            );
        }

        inline bool insert_backward(
            const idx_type& before_item,
            const idx_type& idx
        )
        {
            return insert(
                before_item,
                Item::PREV,
                idx
            );
        }

        inline bool insert(
            const idx_type& insert_idx,
            idx_type direction,
            const idx_type& idx
        )
        {
            ensureSize((insert_idx > idx ? insert_idx : idx) + 1);
            return LinkedList::insert(insert_idx, direction, idx);
        }

        inline bool push(
            idx_type head_direction,
            const idx_type& idx
        )
        {
            ensureSize(idx + 1);
            return LinkedList::push(head_direction, idx);
        }

        inline bool construct(
            const idx_type& idx 
        )
        {
            ensureSize(idx+1);
            return LinkedList::construct(idx);
        }

        inline bool resize(size_type newSize)
        {
            // only allow growing
            if (newSize < m_items.size()) return false;
            else
            {
                ensureSize(newSize);
                return true;
            }
        }

    protected: 
        void ensureSize(size_type size)
        {
            if (size > m_items.size())
            {
                // avoid quadratic runtime of naively resizing to only the required size
                // add constant for case of size==0, otherwise it grows at least a constant
                // factor of the required size, ensuring amortized runtime 
                m_items.resize(size + 16 + size / 2);
                items = m_items.data();
                info.capacity = m_items.size();
            }
        }
    };
    
    template<int capacity_t_, typename idx_t = int, typename size_t = idx_t>
    class IndexSetLinkedListVectorPreSized : public IndexSetLinkedListVector<idx_t, size_t>
    {
    public:
        static constexpr int capacity_t = capacity_t_;
        IndexSetLinkedListVectorPreSized()
            : IndexSetLinkedListVector(capacity_t)
        {}
    };
} // namespace chunky_mem
