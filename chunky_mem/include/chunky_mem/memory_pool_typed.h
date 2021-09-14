#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <list>
#include <array>
#include <stdint.h>

#include "chunky_mem/version.h"
#include "chunky_mem/memory_pool.h"

namespace chunky_mem {
    //*/
    template<
        typename element_t, 
        int chunk_size_t=1024*1024/(10*4), 
        int alignment_size_t = 16,
        typename idx_t = int,
        typename size_t = size_t,
        typename chunk_t = Chunk<idx_t, size_t>,
        typename chunk_container_t = std::vector<chunk_t*>,
        typename chunk_factory_t = typename chunk_t::Factory
    >
    class MemoryPoolTyped : public MemoryPool<idx_t, size_t, chunk_t, chunk_container_t, chunk_factory_t>
    {
    public:
        using element_type = element_t;
        using pointer_type = element_type*;

        using idx_type = idx_t;
        using size_type = size_t;

        static constexpr size_type chunk_size = chunk_size_t;
        static constexpr size_type element_size = sizeof(element_type);
        static constexpr size_type alignment_size = alignment_size_t;

        MemoryPoolTyped()
            : MemoryPool(chunk_size, element_size, alignment_size)
        {

        }

        // sizeof(element_t), chunk_size_t, max_chunks_t, alignment_t
        template<typename... Args>
        pointer_type create(Args&&... args)
        {
            return MemoryPool::create<element_type>(std::forward< Args >(args)...);
        }

        // bool destroy(pointer_type ptr)
        // {
        //     return MemoryPool::destroy<element_type>(ptr);
        // }

        // bool find(const pointer_type ptr, idx_type& chunkIdx, idx_type& idx)
        // {
        //     return MemoryPool::find<element_type>(ptr, chunkIdx, idx);
        // }

        // pointer_type getItem(idx_type chunkIdx, idx_type idx)
        // {
        //     return MemoryPool::getItem<element_type>(chunkIdx, idx);
        // }
    };


} // namespace chunky_mem
