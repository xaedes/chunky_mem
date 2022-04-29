#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <type_traits>

#include "chunky_mem/version.h"
#include "chunky_mem/index_set_linked_list_array.h"
#include "chunky_mem/index_set_linked_list_vector.h"

namespace chunky_mem {

    template<
        typename idx_t = size_t,
        typename size_t = size_t
    >
    struct Chunk
    {
        using idx_type = idx_t;
        using size_type = size_t;

        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");

        //static constexpr int chunk_size = chunk_size_t;
        //static constexpr int alignment = alignment_t;

        size_type chunkByteSize() const { return chunkSize() * slotSize(); }
        size_type chunkSize() const { return m_chunkSize; }
        size_type alignment() const { return m_alignment; }
        size_type elementSize() const { return m_elementSize; }
        size_type paddingSize() const { return (alignment() - (elementSize() % alignment())) % alignment(); };
        size_type slotSize() const { return m_slotSize; };
        static size_type SlotSize(size_type elementSize, size_type alignment)
        {
            return elementSize + ((alignment - (elementSize % alignment)) % alignment);
        }

        // use raw bytes array struct instead of value_type to allow default constructor
        // struct value_data_buffer
        // {
        //     uint8_t bytes[element_size];
        //     uint8_t padding[padding_size];
        //     value_data_buffer()
        //     {
        //         std::fill_n(bytes, element_size, 0);
        //         std::fill_n(padding, padding_size, 0);
        //     }
        // };

        struct Factory
        {
            size_type chunkSize = 1;
            size_type elementSize = 16;
            size_type alignment = 16;
            Factory(size_type chunkSize=1, size_type elementSize=16, size_type alignment=16)
                : chunkSize(chunkSize), elementSize(elementSize), alignment(alignment)
            {}
            Chunk* createNew() const
            {
                return new Chunk(chunkSize, elementSize, alignment);
            }
        };

        Chunk(size_type chunkSize_=1, size_type elementSize_=16, size_type alignment_=16) 
            : m_bytes{}
            , m_isFree{}
            , m_chunkSize(chunkSize_)
            , m_elementSize(elementSize_)
            , m_alignment(alignment_)
            , m_slotSize(SlotSize(elementSize_, alignment_))
        { 

            m_bytes.resize(chunkSize()*slotSize());
            m_isFree.resize(chunkSize());
            m_recycled.resize(chunkSize());
            std::fill_n(m_bytes.data(), chunkSize()*slotSize(), 0); 
            std::fill_n(m_isFree.begin(), chunkSize(), true); 
            // std::fill_n(m_isRecycled.data(), chunk_size, false); 
            //std::fill_n(m_recycledLLPrev.data(), chunk_size, -1); 
            //std::fill_n(m_recycledLLPrev.data(), chunk_size, -1); 
        }
        inline size_type size() const { return m_countAllocated; }
        inline size_type countAllocated() const { return m_countAllocated; }
        inline size_type countRecycled() const { return m_recycled.size(); }
        inline size_type countFree() const { return chunkSize() - countAllocated(); }
        inline size_type countFilled() const { return m_idxInsertFillChunk; }

        inline const void* data() const { return m_bytes.data(); }
        inline const void* data() { return m_bytes.data(); }

        template<typename element_type>
        inline const element_type* at(const idx_type& idx) const
        {
            return static_cast<const element_type*>(&m_bytes[idx * m_slotSize]);
        }

        template<typename element_type>
        inline element_type* at(const idx_type& idx)
        {
            //assert(isAllocated(idx) == true);
            return static_cast<element_type*>(static_cast<void*>(&m_bytes[idx * m_slotSize]));
        }

        inline const void* at(const idx_type& idx) const
        {
            return &m_bytes[idx * m_slotSize];
        }

        inline void* at(const idx_type& idx)
        {
            //assert(isAllocated(idx) == true);
            return &m_bytes[idx * m_slotSize];
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
            return ((0 <= dist) && (dist < chunkSize()));
        }

        inline idx_type indexOf(const void* ptr) const
        {
            idx_type idx = static_cast<idx_type>( ((const uint8_t*)ptr - (uint8_t*)data()) );
            idx /= static_cast<idx_type>(slotSize());
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
        //inline size_type countFilled() const { return m_idxInsertFillChunk; }

        inline bool allocateFreeFromFilling(idx_type& idx)
        {
            while (countFilled() < chunkSize())
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
            for (idx_type i = 0; i < chunkSize(); ++i)
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
            m_countAllocated = chunkSize();
            for(idx_type idx = 0; idx < chunkSize(); ++idx)
            {
                if (isFree(idx))
                {
                    --m_countAllocated;
                    m_recycled.push_back(idx);
                }
            }
        }
        std::vector<uint8_t> m_bytes;
        std::vector<bool> m_isFree;
        // std::array<bool, chunk_size> m_isRecycled;
        //IndexSetLinkedListArray<chunk_size, idx_type, size_type> m_recycled;
        IndexSetLinkedListVector<idx_type, size_type> m_recycled;
        // std::array<idx_type, chunk_size> m_recycledLLPrev;  // for free items: points to prev recycled item
        // std::array<idx_type, chunk_size> m_recycledLLNext;  // for free items: points to next recycled item
        inline bool chunkFillingComplete() const { return m_idxInsertFillChunk >= chunkSize(); }
        idx_type m_idxInsertFillChunk = 0;
        // idx_type m_idxRecycledFront = 0;
        // idx_type m_idxRecycledBack = 0;
        size_type m_countAllocated = 0;
        // size_type m_countRecycled = 0;
        const size_type m_slotSize;
        const size_type m_elementSize;
        const size_type m_chunkSize;
        const size_type m_alignment;
    };

} // namespace chunky_mem
