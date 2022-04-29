#pragma once

#include <algorithm>
#include <array>
#include <type_traits>
#include "chunky_mem/version.h"

namespace chunky_mem {

    template<typename idx_t = size_t, typename size_t = idx_t>
    struct LinkedList
    {
        using idx_type = idx_t;
        using size_type = size_t;
        
        static_assert(std::is_integral<idx_type>::value, "idx_type must be integral");
        static_assert(std::is_same<idx_type, bool>::value == false, "idx_type must not be bool");

        static_assert(std::is_integral<size_type>::value, "size_type must be integral");
        static_assert(std::is_same<size_type, bool>::value == false, "size_type must not be bool");


        struct Item
        {
            union 
            {
                struct
                {
                    idx_type prev;
                    idx_type next;
                };
                idx_type links[2];
            };
            static constexpr idx_type PREV = 0;
            static constexpr idx_type NEXT = 1;
            bool contained = false;
            Item() : prev(0), next(0), contained(false) {}
        };

        struct Info
        {
            union
            {
                struct
                {
                    idx_type front;
                    idx_type back;
                };
                idx_type ends[2];
            };
            size_type size = 0;
            size_type capacity = 0;
            Info()
                : front(0)
                , back(0)
                , size(0)
                , capacity(0)
            {}
        };

        // derived classes or user will provide this.
        Item* items;
        Info info;

        inline bool empty() const { return size() == 0; }
        inline size_type size() const { return info.size; }
        inline size_type capacity() const { return info.capacity; }
        inline bool contains(const idx_type& idx) const { return (0 <= idx) && (idx < info.capacity) && items[idx].contained; }

        inline bool front(idx_type& idx) const
        {
            if(empty()) return false;
            idx = info.front;
            return true;
        }

        inline bool back(idx_type& idx) const
        {
            if(empty()) return false;
            idx = info.back;
            return true;
        }

        inline bool prev(const idx_type& idx, idx_type& out_idx) const
        {
            if (!contains(idx)) return false;
            bool contains_more = (idx != items[idx].prev);
            out_idx = items[idx].prev;
            return contains_more;
        }

        inline bool next(const idx_type& idx, idx_type& out_idx) const
        {
            if (!contains(idx)) return false;
            bool contains_more = (idx != items[idx].next);
            out_idx = items[idx].next;
            return contains_more;
        }

        inline bool find(            
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find_forward(needle, found);
        }

        inline bool find_forward(            
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find_forward(info.front, needle, found);
        }

        inline bool find_backward(
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find_backward(info.back, needle, found);
        }

        inline bool find_forward(            
            const idx_type& start,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find(start, Item::NEXT, needle, found);
        }

        inline bool find_backward(
            const idx_type& start,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find(start, Item::PREV, needle, found);
        }

        inline bool push_front(const idx_type& idx)
        {
            return push(Item::PREV, idx);
        }

        inline bool push_back(const idx_type& idx)
        {
            return push(Item::NEXT, idx);
        }

        inline bool pop_front(idx_type& idx)
        {
            return pop(Item::PREV, idx);
        }

        inline bool pop_back(idx_type& idx)
        {
            return pop(Item::NEXT, idx);
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

        inline bool find(            
            const idx_type& start,
            idx_type direction,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            if (!items[start].contained) return false;
            if (!items[needle].contained) return false;
            idx_type current = start;
            idx_type counter = 0;
            while ((current != needle) && (current != items[current].links[direction]))
            {
                current = items[current].links[direction];
                ++counter;
            }
            if (current == needle)
            {
                found = counter;
            }
            return (current == needle);
        }

        inline bool insert(
            const idx_type& insert_idx,
            idx_type direction,
            const idx_type& idx
        )
        {
            if (info.size == info.capacity) return false;
            if (items[idx].contained) return false;
            if (!items[insert_idx].contained) return false;
            idx_type tail = info.ends[direction];
            // if (insert_idx == head) return push(head, next, prev, idx);
            if (insert_idx == tail) return push(direction, idx);

            //assert(items[insert_idx).contained);
            assert(items[items[insert_idx].links[direction]].contained);
            assert(items[items[insert_idx].links[1-direction]].contained);
            
            auto next_item = items[insert_idx].links[direction];

            assert(items[items[next_item].links[direction]].contained);
            assert(items[items[next_item].links[1-direction]].contained);

            //auto next_item = items[insert_idx].links[direction];
            //assert(items[items[insert_idx].links[1-direction]).contained);

            items[insert_idx].links[direction] = idx;
            items[idx].links[1-direction] = insert_idx;
            items[idx].links[direction] = next_item;
            items[next_item].links[1-direction] = idx;

            assert(items[items[next_item].links[direction]].contained);
            items[idx].contained = true;
            ++info.size;
            return true;
        }

        inline bool push(
            idx_type head_direction,
            const idx_type& idx
        )
        {
            if (info.size == 0)
            {
                return construct(idx);
            }
            else if ((info.size < info.capacity) && !items[idx].contained)
            {
                idx_type& head = info.ends[head_direction];
                auto tail_direction = 1-head_direction;
                // auto tail_direction = direction;

                assert(items[head].links[head_direction] == head);

                // auto prev_item = items[head].prev;
                assert(items[items[head].links[tail_direction]].links[head_direction] == head); 
                assert(items[head].links[head_direction] == head); 

                // works also for items[head].links[tail_direction] == head

                // insert idx before old head and tail
                items[head].links[head_direction] = idx;
                items[idx].links[tail_direction] = head;
                items[idx].links[head_direction] = idx;
                head = idx;
                assert(items[items[head].links[tail_direction]].links[head_direction] == head);
                assert(items[head].links[head_direction] == head);
                items[idx].contained = true;
                ++info.size;
                return true;
            }
            //throw std::runtime_error("");
            return false;
        }

        inline bool pop(
            idx_type head_direction,
            idx_type& idx
        )
        {
            if (info.size)
            {
                idx_type& head = info.ends[head_direction];
                idx = head;
                auto tail_direction = 1-head_direction; 
                // auto head_direction = 1-direction;

                auto tail_item = items[head].links[tail_direction];

                assert(items[tail_item].links[head_direction] == head);
                assert(items[head].links[head_direction] == head);

                // remove the old head
                items[tail_item].links[head_direction] = tail_item;
                items[head].links[head_direction] = head;
                items[head].links[tail_direction] = head;
                items[head].contained = false;
                head = tail_item;
                --info.size;
                return true;
            }
            return false;
        }

        inline bool remove(
            const idx_type& idx
        )
        {
            if (!items[idx].contained) return false;
            idx_type tmp = 0;
            if (info.front == idx) return pop(Item::PREV, tmp);
            if (info.back == idx) return pop(Item::NEXT, tmp);
            
            assert(items[idx].contained);
            assert(items[items[idx].prev].contained);
            assert(items[items[idx].next].contained);

            auto prev_item = items[idx].prev;
            auto next_item = items[idx].next;
            assert(items[items[prev_item].prev].contained);
            items[prev_item].next = next_item;
            items[idx].prev = idx;
            items[idx].next = idx;
            items[next_item].prev = prev_item;
            items[idx].contained = false;
            --info.size;
            assert(items[items[next_item].next].contained);
            return true;
        }

        inline void clear()
        {
            info.size = 0;
            std::fill_n(items, info.capacity, Item());
        }
        
        inline bool construct(
            const idx_type& idx 
        )
        {
            if (items[idx].contained) return false;
            if (info.capacity < 1) return false;
            info.front = idx;
            info.back = idx;
            info.size = 1;
            items[idx].prev = idx;
            items[idx].next = idx;
            items[idx].contained = true;
            return true;
        }

    };

} // namespace chunky_mem
