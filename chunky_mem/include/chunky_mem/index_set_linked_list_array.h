#pragma once

#include <algorithm>
#include <array>
#include <type_traits>

#include "chunky_mem/version.h"
#include "chunky_mem/linked_list.h"

namespace chunky_mem {

    template<int capacity_t_, typename idx_t = int, typename size_t = idx_t>
    class IndexSetLinkedListArray : protected LinkedList<idx_t, size_t>
    {
    public:
        using idx_type = idx_t;
        using size_type = size_t;

        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");
        
        using linked_list_type = LinkedList<idx_type, size_type>;
        using Item = typename linked_list_type::Item;
        using Info = typename linked_list_type::Info;
        static constexpr int capacity_t = capacity_t_;

    protected:
        std::array<Item, capacity_t> m_items;
    public:

        IndexSetLinkedListArray()
        {
            items = m_items.data();
            info.capacity = capacity_t;
            clear();
        }

        inline bool fixCapacity() const { return true; }

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
        using LinkedList::insert;
        using LinkedList::insert_forward;
        using LinkedList::insert_backward;
        using LinkedList::construct;
        using LinkedList::push;
        using LinkedList::pop;
    };

} // namespace chunky_mem
