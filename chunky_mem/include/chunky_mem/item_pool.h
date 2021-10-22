#pragma once

#include <vector>
#include "chunky_mem/index_set_linked_list_vector.h"

namespace chunky_mem
{

    template <
        typename item_t,
        typename container_t = std::vector<item_t>,
        typename idx_t = int,
        typename linked_list_t = IndexSetLinkedListVector<idx_t, size_t>
    >
    struct ItemPool
    {
        using item_type        = item_t;
        using container_type   = container_t;
        using idx_type         = idx_t;
        using linked_list_type = linked_list_t;

        idx_type insert();
        idx_type insert(const item_type& item);
        
        // template <typename... Args>
        // idx_type emplace(Args&&... args);

        inline       item_type& operator[] (idx_type idx)       { return m_slots[idx]; }
        inline const item_type& operator[] (idx_type idx) const { return m_slots[idx]; }

        void erase(idx_type idx);
        void clear();

        bool empty()      const { return size() == 0; }
        size_t size()     const { return m_occupiedSlots.size(); }
        size_t capacity() const { return m_slots.size(); }

        const linked_list_type& freeSlots()     const { return m_freeSlots; }
        const linked_list_type& occupiedSlots() const { return m_occupiedSlots; }

        bool contains(idx_type idx) const { return occupiedSlots().contains(idx); }

    protected:
        idx_type slot_push_back();
        idx_type slot_push_back(const item_type& item);

        // template <typename... Args> 
        // idx_type slot_emplace_back(Args&&... args);

        container_type   m_slots;
        linked_list_type m_freeSlots;
        linked_list_type m_occupiedSlots;
    };

    template <class A, class B, class idx_type, class D>
    idx_type ItemPool<A,B,idx_type,D>::insert()
    {
        idx_type slot;
        if (m_slots.empty() || (!m_freeSlots.pop_front(slot)))
        {
            slot = slot_push_back();
        }
        assert(!m_occupiedSlots.contains(slot));
        m_occupiedSlots.push_back(slot);
        return slot;
    }

    template <class A, class B, class idx_type, class D>
    idx_type ItemPool<A,B,idx_type,D>::insert(const item_type& item)
    {
        idx_type slot;
        if (m_slots.empty() || (!m_freeSlots.pop_front(slot)))
        {
            slot = slot_push_back(item);
        }
        else
        {
            m_slots[slot] = item;
        }
        assert(!m_occupiedSlots.contains(slot));
        m_occupiedSlots.push_back(slot);
        return slot;
    }

    // template <class A, class B, class idx_type, class D>
    // template <typename... Args> 
    // idx_type ItemPool<A,B,idx_type,D>::emplace(Args&&... args)
    // {
    //     idx_type slot;
    //     if (m_slots.empty() || (!m_freeSlots.pop_front(slot)))
    //     {
    //         slot = slot_emplace_back(std::forward<Args>(args)...);
    //     }
    //     else 
    //     {
    //         // placement new into free slot
    //         new(&(m_slots[slot])) item_type(std::forward<Args>(args)...);
    //     }
    //     assert(!m_occupiedSlots.contains(slot));
    //     m_occupiedSlots.push_back(slot);
    //     return slot;
    // }

    template <class A, class B, class idx_type, class D>
    idx_type ItemPool<A,B,idx_type,D>::slot_push_back()
    {
        m_slots.emplace_back();
        // m_slots.push_back(item_type());
        return m_slots.size()-1;
    }    

    template <class A, class B, class idx_type, class D>
    idx_type ItemPool<A,B,idx_type,D>::slot_push_back(const item_type& item)
    {
        m_slots.push_back(item);
        return m_slots.size()-1;
    }    

    // template <class A, class B, class idx_type, class D>
    // template <typename... Args> 
    // idx_type ItemPool<A,B,idx_type,D>::slot_emplace_back(Args&&... args)
    // {
    //     m_slots.emplace_back(std::forward<Args>(args)...);
    //     return m_slots.size()-1;
    // }    

    template <class A, class B, class C, class D>
    void ItemPool<A,B,C,D>::erase(idx_type idx)
    {
        m_freeSlots.push_back(idx);
        m_occupiedSlots.remove(idx);
    }    

    template <class A, class B, class C, class D>
    void ItemPool<A,B,C,D>::clear()
    {
        m_freeSlots.clear();
        m_occupiedSlots.clear();
        for(int i=0; i<m_slots.size(); ++i)
        {
            m_freeSlots.push_back(i);
        }
    }    
}
