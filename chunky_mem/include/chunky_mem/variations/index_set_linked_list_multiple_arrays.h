#pragma once

#include <algorithm>
#include <array>

namespace chunky_mem {

    template<int capacity_t_, typename idx_t = int, typename size_t = idx_t>
    class IndexSetLinkedList
    {
    protected:
        std::array<idx_type, capacity_t> m_prev;  // for free items: points to previous item
        std::array<idx_type, capacity_t> m_next;  
        std::array<bool, capacity_t> m_contained;  
        idx_type m_front = 0;
        idx_type m_back = 0;
        size_type m_size = 0;

    public:
        using idx_type = idx_t;
        using size_type = size_t;
        static constexpr int capacity_t = capacity_t_;

        IndexSetLinkedList()
        {
            std::fill_n(m_prev.data(), capacity_t, -1);
            std::fill_n(m_next.data(), capacity_t, -1);
            std::fill_n(m_contained.data(), capacity_t, false);
        }

        inline bool empty() const { return size() == 0; }
        inline size_type size() const { return m_size; }
        static inline size_type capacity() { return capacity_t; }
        //inline size_type capacity() const { return capacity_t; }
        inline bool contains(const idx_type& idx) const { return m_contained[idx]; }

        inline bool front(idx_type& idx) const
        {
            if(empty()) return false;
            idx = m_front;
            return true;
        }

        inline bool back(idx_type& idx) const
        {
            if(empty()) return false;
            idx = m_back;
            return true;
        }

        inline bool prev(const idx_type& idx, idx_type& out_idx)
        {
            if (!contains(idx)) return false;
            bool contains_more = (idx != m_prev[idx]);
            out_idx = m_prev[idx];
            return contains_more;
        }

        inline bool next(const idx_type& idx, idx_type& out_idx)
        {
            if (!contains(idx)) return false;
            bool contains_more = (idx != m_next[idx]);
            out_idx = m_next[idx];
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
            return find_forward(m_front, needle, found);
        }

        inline bool find_backward(
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find_backward(m_back, needle, found);
        }

        inline bool find_forward(            
            const idx_type& start,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find(start, m_next.data(), needle, found);
        }

        inline bool find_backward(
            const idx_type& start,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            return find(start, m_prev.data(), needle, found);
        }

        void clear()
        {
            m_size = 0;
            std::fill_n(m_contained.data(), capacity_t, false);
        }

        inline bool push_front(const idx_type& idx)
        {
            return push(m_front, m_prev.data(), m_next.data(), m_contained.data(), idx);
        }

        inline bool push_back(const idx_type& idx)
        {
            return push(m_back, m_next.data(), m_prev.data(), m_contained.data(), idx);
        }

        inline bool pop_front(idx_type& idx)
        {
            return pop(m_front, m_prev.data(), m_next.data(), m_contained.data(), idx);
        }

        inline bool pop_back(idx_type& idx)
        {
            return pop(m_back, m_next.data(), m_prev.data(), m_contained.data(), idx);
        }


        inline bool remove(const idx_type& idx)
        {
            if (!contains(idx)) return false;
            assert(contains(idx));
            assert(contains(m_prev[idx]));
            assert(contains(m_next[idx]));

            auto prev_item = m_prev[idx];
            auto next_item = m_next[idx];
            assert(contains(m_prev[prev_item]));
            m_next[prev_item] = next_item;
            m_prev[idx] = idx;
            m_next[idx] = idx;
            m_prev[next_item] = prev_item;
            assert(contains(m_next[next_item]));
            return true;
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
                m_front,
                m_back,
                m_next.data(),
                m_prev.data(),
                m_contained.data(),
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
                m_back,
                m_front,
                m_prev.data(),
                m_next.data(),
                m_contained.data(),
                idx
            );
        }

    protected:
        inline bool find(            
            const idx_type& head,
            const idx_type* tail_direction,
            const idx_type& needle,
            idx_type& found
        ) const
        {
            if (!contains(head)) return false;
            if (!contains(needle)) return false;
            idx_type current = head;
            idx_type counter = 0;
            while ((current != needle) && (current != tail_direction[current]))
            {
                current = tail_direction[current];
                ++counter;
            }
            if (current == needle)
            {
                found = counter;
            }
            return (current == needle);
        }

        inline bool insert(
            const idx_type& prev_item,
            const idx_type& head,
            idx_type& tail,
            idx_type* next,
            idx_type* prev,
            bool* contains,
            const idx_type& idx
        )
        {
            if (contains[idx]) return false;
            if (!contains[prev_item]) return false;
            // if (prev_item == head) return push(head, next, prev, idx);
            if (prev_item == tail) return push(tail, next, prev, contains, idx);

            //assert(contains[prev_item));
            assert(contains[next[prev_item]]);
            assert(contains[prev[prev_item]]);
            
            auto next_item = next[prev_item];

            assert(contains[next[next_item]]);
            assert(contains[prev[next_item]]);

            //auto next_item = next[prev_item];
            //assert(contains[prev[prev_item]));

            next[prev_item] = idx;
            prev[idx] = prev_item;
            next[idx] = next_item;
            prev[next_item] = idx;

            assert(contains[next[next_item]]);
            contains[idx] = true;
            ++m_size;
            return true;
        }

        inline bool push(
            idx_type& head,
            idx_type* head_direction,
            idx_type* tail_direction,
            bool* contains,
            const idx_type& idx
        )
        {
            if (m_size == 0)
            {
                return construct_from_idx(idx);
            }
            else if ((m_size < capacity_t) && !contains[idx])
            {
                assert(head_direction[head] == head);

                // auto prev_item = m_prev[head];
                assert(head_direction[tail_direction[head]] == head); 
                assert(head_direction[head] == head); 

                // works also for tail_direction[head] == head

                // insert idx before old head and tail
                head_direction[head] = idx;
                tail_direction[idx] = head;
                head_direction[idx] = idx;
                head = idx;
                assert(head_direction[tail_direction[head]] == head);
                assert(head_direction[head] == head);
                contains[idx] = true;
                ++m_size;
                return true;
            }
            return false;
        }

        inline bool pop(
            idx_type& head,
            idx_type* head_direction,
            idx_type* tail_direction,
            bool* contains,
            idx_type& idx
        )
        {
            if (m_size)
            {
                idx = head;

                auto tail_item = tail_direction[head];

                assert(head_direction[tail_item] == head);
                assert(head_direction[head] == head);

                // remove the old head
                head_direction[tail_item] = tail_item;
                head_direction[head] = head;
                tail_direction[head] = head;
                contains[head] = false;
                head = tail_item;
                --m_size;
                return true;
            }
            return false;
        }

        inline bool construct_from_idx(const idx_type& idx)
        {
            if (m_contained[idx]) return false;
            if (capacity() < 1) return false;
            m_front = idx;
            m_back = idx;
            m_prev[idx] = idx;
            m_next[idx] = idx;
            m_contained[idx] = true;
            m_size = 1;
            return true;
        }
    };

} // namespace chunky_mem
