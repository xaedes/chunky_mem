#pragma once

#include <algorithm>
#include <array>
#include <type_traits>

#include "chunky_mem/version.h"
#include "chunky_mem/index_set_linked_list_array.h"

namespace chunky_mem {

    template<
        size_t element_size_t, 
        size_t chunk_size_t = (1024*64 - ((1024*64) % element_size_t)) / element_size_t, 
        size_t alignment_size_t = 16,
        typename idx_t = int,
        typename size_t = size_t
    >
    struct ChunkStaticSlots
    {
        static constexpr size_t element_size = element_size_t;
        static constexpr size_t chunk_size = chunk_size_t;
        static constexpr size_t alignment_size = alignment_size_t;
        static constexpr size_t padding_size = (alignment - (element_size % alignment_t)) % alignment;
        static constexpr size_t slot_size = element_size + padding_size;       
        using idx_type = idx_t;
        using size_type = size_t;

        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");


        size_type chunkByteSize() const { return chunkSize() * slotSize(); }
        size_type chunkSize() const { return chunk_size; }
        size_type alignment() const { return alignment_size; }
        size_type elementSize() const { return element_size; }
        size_type paddingSize() const { return padding_size; };
        size_type slotSize() const { return elementSize() + paddingSize(); };

        struct Factory
        {
            ChunkStaticSlots* createNew() const
            {
                return new ChunkStaticSlots();
            }
        };

        // use raw bytes array struct instead of value_type to allow default constructor
        struct Slot
        {
            uint8_t bytes[element_size];
            uint8_t padding[padding_size];
            Slot()
            {
                std::fill_n(bytes, element_size, 0);
                std::fill_n(padding, padding_size, 0);
            }
        };

        ChunkStaticSlots()  : m_slots{}, m_isFree{}
        { 
            std::fill_n(m_slots.data(), chunk_size, Slot());
            std::fill_n(m_isFree.data(), chunk_size, true); 
            for(int i=0; i<chunk_size; ++i)
            {
                assert(&(m_slots[i]) == &(m_slots[0]) + i);
            }
            // std::fill_n(m_isRecycled.data(), chunk_size, false); 
            //std::fill_n(m_recycledLLPrev.data(), chunk_size, -1); 
            //std::fill_n(m_recycledLLPrev.data(), chunk_size, -1); 
        }
        inline size_type size() const { return m_countAllocated; }
        inline size_type countAllocated() const { return m_countAllocated; }
        inline size_type countRecycled() const { return m_recycled.size(); }
        inline size_type countFree() const { return chunk_size - countAllocated(); }
        inline size_type countFilled() const { return m_idxInsertFillChunk; }

        inline const void* data() const { return m_slots.data(); }
        inline const void* data() { return m_slots.data(); }

        inline const Slot* at(const idx_type& idx) const
        {
            return &m_slots[idx];
        }

        inline Slot* at(const idx_type& idx)
        {
            //assert(isAllocated(idx) == true);
            return &m_slots[idx];
        }

        inline bool contains(const void* ptr) const
        {
            if (ptr < data()) return false;
            size_type dist = ((const uint8_t*)ptr - (uint8_t*)data());
            if (dist % slotSize() > 0)
            {
                // may point somewhere in chunk, but not at slot begin
                return false;
            }
            dist /= slotSize();
            return ((0 <= dist) && (dist < chunk_size));
        }

        inline idx_type indexOf(const void* ptr) const
        {
            idx_type idx = ((const uint8_t*)ptr - (uint8_t*)data());
            idx /= slotSize();
            return idx;
        }

        inline bool isAllocated(const idx_type& idx) const
        {
            return !m_isFree[idx];
        }

        inline bool isFree(const idx_type& idx) const
        {
            return m_isFree[idx];
        }

        inline bool allocate(idx_type& idx)
        {
            return (
                allocateFreeRecycled(idx) 
                || allocateFreeFromFilling(idx)
                || allocateFreeSearchAll(idx)
            );
        }

        inline bool recycle(const idx_type& idx)
        {
            if (isAllocated(idx) && m_recycled.push_back(idx))
            {
                m_isFree[idx] = true;
                --m_countAllocated;
                return true;
            }
            throw std::runtime_error("");
            return false;
        }

        // inline bool isRecycled(const idx_type& idx) const
        // {
        //     return m_isRecycled[idx];
        // }
    protected:

        inline bool allocateFreeFromFilling(idx_type& idx)
        {
            while (countFilled() < chunk_size)
            {
                // avoid taking recycled elements.
                // when destroying those, they would get recycled twice.
                if (isFree(m_idxInsertFillChunk) && !m_recycled.contains(m_idxInsertFillChunk))
                {
                    idx = m_idxInsertFillChunk;
                    m_isFree[idx] = false;
                    ++m_countAllocated;
                    return true;
                }
                ++m_idxInsertFillChunk;
            }
            return false;
        }

        inline bool allocateFreeRecycled(idx_type& idx) 
        {
            if(m_recycled.pop_front(idx))
            {
                assert(isFree(idx) == true);
                m_isFree[idx] = false;
                ++m_countAllocated;
                return true;
            }
            return false;
        }

        inline bool allocateFreeSearchAll(idx_type& idx)
        {
            for (idx_type i = 0; i < chunk_size; ++i)
            {
                if (isFree(i))
                {
                    idx = i;
                    m_isFree[idx] = false;
                    ++m_countAllocated;
                    return true;
                } 
            }
            return false;
        }

        inline size_type collectAllRecycled()
        {
            m_recycled.clear();
            m_countAllocated = chunk_size;
            for(idx_type& idx = 0; idx < chunk_size; ++idx)
            {
                if (isFree(idx))
                {
                    --m_countAllocated;
                    m_recycled.push_back(idx);
                }
            }
        }
        std::array<Slot, chunk_size> m_slots;
        std::array<bool, chunk_size> m_isFree;
        // std::array<bool, chunk_size> m_isRecycled;
        IndexSetLinkedListArray<chunk_size, idx_type, size_type> m_recycled;
        // std::array<idx_type, chunk_size> m_recycledLLPrev;  // for free items: points to prev recycled item
        // std::array<idx_type, chunk_size> m_recycledLLNext;  // for free items: points to next recycled item
        inline bool chunkFillingComplete() const { return m_idxInsertFillChunk >= chunk_size; }
        idx_type m_idxInsertFillChunk = 0;
        // idx_type m_idxRecycledFront = 0;
        // idx_type m_idxRecycledBack = 0;
        size_type m_countAllocated = 0;
        // size_type m_countRecycled = 0;
    };

} // namespace chunky_mem
