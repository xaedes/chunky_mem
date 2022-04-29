#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <list>
#include <array>
#include <cassert>
#include <functional>
#include <type_traits>
#include <stdint.h>

#include "chunky_mem/version.h"
#include "chunky_mem/chunk.h"
#include "chunky_mem/chunk_static_slots.h"
#include "chunky_mem/index_set_linked_list_vector.h"

namespace chunky_mem {

    template<
        typename idx_t = int,
        typename size_t = size_t,
        typename chunk_t = Chunk<idx_t, size_t>,
        typename chunk_container_t = std::vector<chunk_t*>,
        typename chunk_factory_t = typename chunk_t::Factory,
        typename free_chunks_linked_list_t = IndexSetLinkedListVector<idx_t, size_t>
    >
    class MemoryPool
    {
    public:
        //static constexpr int element_size = element_size_t;

        // using pointer_type = element_type*;
        //static constexpr int chunk_size = chunk_size_t;
        //static constexpr int max_chunks = max_chunks_t;
        //static constexpr int alignment = alignment_t;

        using idx_type = idx_t;
        using size_type = size_t;

        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");
        
        using chunk_type = chunk_t;
        using chunk_container_type = chunk_container_t;
        using chunk_factory_type = chunk_factory_t;
        using free_chunks_linked_list_type = free_chunks_linked_list_t;
        // using chunk_type = Chunk<idx_type, size_type>;
         //using chunk_type = ChunkStaticSlots<element_size, chunk_size, alignment, idx_type, size_type>;

    protected:
        using DestructorCallable = std::function<void(void*)>;


        template <typename element_type>
        struct protected_constructor_helper : public element_type
        {
            // access protected constructors
            template<typename... Args>
            protected_constructor_helper(Args&&... args)
                : element_type{ std::forward< Args >(args)... }
            {}
        };
        chunk_factory_type m_chunkFactory;
        chunk_container_type m_chunks;
        idx_type m_idxInsert = 0;
        size_type m_countAllocated = 0;
        size_type m_countFree = 0;
        // size_type m_chunkSize = 0;
        // size_type m_maxChunks = 0;
        // size_type m_alignment = 0;
        free_chunks_linked_list_type m_chunksWithFree;
        DestructorCallable m_destructor;

    public:
        template <typename... Args>
        MemoryPool(Args&&... args)
            : m_chunkFactory(std::forward< Args >(args)...)
        {
            // m_chunksWithFree.resize(m_maxChunks);
        }
        ~MemoryPool()
        {
            for (int chunkIdx = 0; chunkIdx < m_chunks.size(); ++chunkIdx)
            {
                auto& chunk = getChunk(chunkIdx);
                for (int idx = 0; idx < chunk.chunkSize(); ++idx)
                {
                    if (chunk.isAllocated(idx))
                    {
                        // destroy(chunkIdx, idx);
                    }
                }
                ::operator delete(&chunk);
            }
        }


        template<typename element_type, typename... Args>
        element_type* create(Args&&... args)
        {
            element_type* ptr = nullptr;
            if (allocate(ptr))
            {
                ptr = new(ptr) protected_constructor_helper<element_type>(std::forward< Args >(args)...);
                // assert((void*)(chunk.at(idx)) == (void*)ptr);
            }
            else
            {
                throw std::runtime_error("could not allocate memory");
            }
            return ptr;
        }
        
        template<typename element_type>
        bool destroy(element_type* ptr)
        {
            idx_type chunkIdx = 0, idx = 0;
            if (find(ptr, chunkIdx, idx))
            {
                //std::cout << "chunkIdx=" << chunkIdx << " idx=" << idx << std::endl;
                return destroy<element_type>(ptr, chunkIdx, idx);
            }
            //throw std::runtime_error("destroy(ptr): ptr not found");
            return false;
        }

        template<typename element_type>
        void destruct(element_type* ptr)
        {
            ptr->~element_type();
            return true;
        }

        template<typename element_type>
        bool allocate(element_type*& out_ptr)
        {
            idx_type chunkIdx = 0, idx = 0;
            if (allocate(chunkIdx, idx) == false)
            {
                return false;
                throw std::runtime_error("could not allocate memory");
            }
            auto& chunk = getChunk(chunkIdx);
            out_ptr = chunk.at<element_type>(idx);
            return true;
        }

        bool deallocate(void* ptr)
        {
            idx_type chunkIdx = 0, idx = 0;
            if (find(ptr, chunkIdx, idx))
            {
                //std::cout << "chunkIdx=" << chunkIdx << " idx=" << idx << std::endl;
                return deallocate(chunkIdx, idx);
            }
            throw std::runtime_error("deallocate(ptr): ptr not found");
            return false;
        }

        template<typename element_type = void>
        bool find(const element_type* ptr, idx_type& chunkIdx, idx_type& idx) const
        {
            for (size_t i = 0; i < m_chunks.size(); i++)
            {
                chunkIdx = static_cast<idx_type>(i);
                chunk_type& chunk = getChunk(chunkIdx);
                if (chunk.contains(ptr))
                {
                    idx = chunk.indexOf(ptr);
                    assert((void*)(chunk.at(idx)) == (void*)ptr);
                    return true;
                }
            }
            return false;
        }

        template<typename element_type = void>
        element_type* getItem(idx_type chunkIdx, idx_type idx) const
        {
            auto& chunk = getChunk(chunkIdx);
            void* ptr = static_cast<void*>(chunk.at(idx));
            return static_cast<element_type*>(ptr);
        }

        size_t countChunks() const { return m_chunks.size(); }

        inline chunk_type& getChunk(idx_type chunkIdx) const
        {
            return *m_chunks[chunkIdx];
        }

        template<typename element_type>
        bool isInChunk(idx_type chunkIdx, const element_type* ptr) const
        {
            return getChunk(chunkIdx).contains(ptr);
        }

        inline bool contains(const void* ptr) const
        {
            for (size_t i = 0; i < m_chunks.size(); i++)
                if (chunk.contains(ptr))
                    return true;
            return false;
        }

    protected:
        template<typename element_type>
        bool destroy(element_type* ptr, idx_type chunkIdx, idx_type idx)
        {
            auto& chunk = getChunk(chunkIdx);
            assert(chunk.isAllocated(idx));
            // if (chunk.isFree(idx))
            // {
            //     throw std::runtime_error("element already freed");
            //     return false;
            // }
            ptr->~element_type();
            return deallocate(chunkIdx, idx);
        }

        bool deallocate(idx_type chunkIdx, idx_type idx)
        {
            auto& chunk = getChunk(chunkIdx);
            m_chunksWithFree.push_back(chunkIdx);
            assert(chunk.isAllocated(idx));
            if (chunk.recycle(idx))
            {
                ++m_countFree;
                --m_countAllocated;
                return true;
            }
            else
            {
                throw std::runtime_error("could not recycle element");
                return false;
            }
        }

        bool allocate(idx_type& chunkIdx, idx_type& idx)
        {
            if ((m_chunks.size() == 0) || (m_countFree == 0))
            {
                addChunk();
            }
            while(m_chunksWithFree.front(chunkIdx) && (getChunk(chunkIdx).countFree() == 0))
            {
                m_chunksWithFree.pop_front(chunkIdx);
            }
            auto& chunk = getChunk(chunkIdx);
            if (
                m_chunksWithFree.front(chunkIdx)
                && (chunk.countFree() > 0)
                && chunk.allocate(idx)
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
            return getChunk(m_idxInsert).isInsertAlive();
        }

        void addChunk() 
        {

            // m_chunks.push_back(this->createChunk());
            m_chunks.push_back(m_chunkFactory.createNew());
            //m_chunks.push_back(new chunk_type(element_size));
            //m_countAllocated += m_chunks.back()->countAllocated();
            m_countFree += m_chunks.back()->countFree();
            m_chunksWithFree.push_back(static_cast<idx_type>(m_chunks.size()-1));
        }
    };

} // namespace chunky_mem
