#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <list>
#include <array>
#include <cassert>
#include <stdint.h>
#include <type_traits>

#include "chunky_mem/version.h"
#include "chunky_mem/chunk.h"
#include "chunky_mem/chunk_static_slots.h"
#include "chunky_mem/index_set_linked_list_array.h"

namespace chunky_mem {

    template<
        size_t element_size_t, 
        size_t chunk_size_t = (1024*64-(1024*64)%element_size_t/element_size_t), 
        size_t max_chunks_t = 1024,
        size_t alignment_t = 16,
        typename idx_t = size_t,
        typename size_t = size_t
    >
    class MemoryPoolStatic
    {
    public:
        using idx_type = idx_t;
        using size_type = size_t;

        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");

        static constexpr size_t element_size = element_size_t;
        static constexpr size_t chunk_size = chunk_size_t;
        static constexpr size_t max_chunks = max_chunks_t;
        static constexpr size_t alignment = alignment_t;

        // using chunk_type = Chunk<idx_type, size_type>;
         using chunk_type = ChunkStaticSlots<element_size, chunk_size, alignment, idx_type, size_type>;

    protected:
        template <typename element_type>
        struct protected_constructor_helper : public element_type
        {
            // access protected constructors
            template<typename... Args>
            protected_constructor_helper(Args&&... args)
                : element_type{ std::forward< Args >(args)... }
            {}
        };

        std::vector<chunk_type*> m_chunks;
        idx_type m_idxInsert = 0;
        size_type m_countAllocated = 0;
        size_type m_countFree = 0;
        IndexSetLinkedListArray<max_chunks, idx_type, size_type> m_chunksWithFree;

    public:
        ~MemoryPoolStatic()
        {
            for (idx_type chunkIdx = 0; chunkIdx < m_chunks.size(); ++chunkIdx)
            {
                chunk_type* chunk = getChunk(chunkIdx);
                for (idx_type idx = 0; idx < chunk_size; ++idx)
                {
                    if (chunk->isAllocated(idx))
                    {
                        // destroy(chunkIdx, idx);
                    }
                }
                ::operator delete(chunk);
            }
        }


        template<typename element_type, typename... Args>
        element_type* create(Args&&... args)
        {
            idx_type chunkIdx = 0, idx = 0;
            if (allocate(chunkIdx, idx) == false)
            {
                throw std::runtime_error("could not allocate memory");
            }
            chunk_type* chunk = getChunk(chunkIdx);
            assert(chunk->at(0) + idx == chunk->at(idx));
            element_type* ptr = new(chunk->at(idx)) protected_constructor_helper<element_type>(std::forward< Args >(args)...);
            assert((void*)(chunk->at(idx)) == (void*)ptr);
            return ptr;
        }
        
        template<typename element_type>
        bool destroy(element_type* ptr)
        {
            idx_type chunkIdx = 0, idx = 0;
            if (find(ptr, chunkIdx, idx))
            {
                //std::cout << "chunkIdx=" << chunkIdx << " idx=" << idx << std::endl;
                return destroy<element_type>(chunkIdx, idx);
            }
            throw std::runtime_error("destroy(ptr): ptr not found");
            return false;
        }

        template<typename element_type>
        bool find(const element_type* ptr, idx_type& chunkIdx, idx_type& idx) const
        {
            for (size_t i = 0; i < m_chunks.size(); i++)
            {
                chunkIdx = i;
                chunk_type* chunk = getChunk(chunkIdx);
                if (chunk->contains(ptr))
                {
                    idx = chunk->indexOf(ptr);
                    assert((void*)&(chunk->at(idx)) == (void*)ptr);
                    return true;
                }
            }
            return false;
        }

        template<typename element_type>
        element_type* getItem(idx_type chunkIdx, idx_type idx) const
        {
            auto* chunk = getChunk(chunkIdx);
            void* ptr = static_cast<void*>(chunk->at(idx));
            return static_cast<element_type*>(ptr);
        }

    protected:
        template<typename element_type>
        bool destroy(idx_type chunkIdx, idx_type idx)
        {
            element_type* ptr = getItem<element_type>(chunkIdx, idx);
            auto* chunk = getChunk(chunkIdx);
            if (chunk->isFree(idx))
            {
                throw std::runtime_error("");
                return false;
            }
            ptr->~element_type();
            m_chunksWithFree.push_back(chunkIdx);
            if (chunk->recycle(idx))
            {
                ++m_countFree;
                --m_countAllocated;
                return true;
            }
            else
            {
                throw std::runtime_error("");
                return false;
            }
        }

        inline chunk_type* getChunk(idx_type chunkIdx) const
        {
            return m_chunks[chunkIdx];
        }

        template<typename element_type>
        bool isInChunk(idx_type chunkIdx, const element_type* ptr) const
        {
            return getChunk(chunkIdx)->contains(ptr);
        }

        bool allocate(idx_type& chunkIdx, idx_type& idx)
        {
            if ((m_chunks.size() == 0) || (m_countFree == 0))
            {
                addChunk();
            }
            while(m_chunksWithFree.front(chunkIdx) && (getChunk(chunkIdx)->countFree() == 0))
            {
                m_chunksWithFree.pop_front(chunkIdx);
            }
            if (
                m_chunksWithFree.front(chunkIdx)
                && (getChunk(chunkIdx)->countFree() > 0)
                && getChunk(chunkIdx)->allocate(idx)
            ) {
                --m_countFree;
                ++m_countAllocated;
                return true;
            }
            else
            {
                return false;
            }
        }

        // bool findChunkWithFree(idx_type& chunkIdx)
        // {
        //     if (m_countFree > 0)
        //     {
                
        //     }
        // }

        bool isInsertAlive() const
        {
            return getChunk(m_idxInsert)->isInsertAlive();
        }

        void addChunk() 
        {
            if (m_chunks.size() == max_chunks)
                throw std::runtime_error("too many chunks");

            m_chunks.push_back(new chunk_type(chunk_size, element_size, alignment));
            //m_chunks.push_back(new chunk_type(element_size));
            //m_countAllocated += m_chunks.back()->countAllocated();
            m_countFree += m_chunks.back()->countFree();
            m_chunksWithFree.push_back(m_chunks.size()-1);
        }
    };

} // namespace chunky_mem
