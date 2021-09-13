
#include <catch2/catch.hpp>

#include "chunky_mem/version.h"
#include "chunky_mem/index_set_linked_list_array.h"
#include "chunky_mem/index_set_linked_list_vector.h"

using namespace chunky_mem;

template <typename LinkedListType, int MaxCapacity>
struct Require {
    using idx_type = typename LinkedListType::idx_type;
    //using idx_type = idx_type;
    //void ListEmpty(LinkedListType list, bool testGrowing);

    //void RequireListNonEmpty(LinkedListType list, bool testGrowing);

    void RequireContainsNone(LinkedListType list)
    {
        SECTION("RequireContainsNone")
        {

            using idx_type = idx_type;
            idx_type result = 0;
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                REQUIRE(list.contains(i) == false);
            }
        }
    }
    void RequireContainsSomething(LinkedListType list)
    {
        SECTION("RequireContainsSomething")
        {

            using idx_type = idx_type;
            idx_type result = 0;
            bool containsSomething = false;
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                containsSomething |= list.contains(i);
                if (containsSomething) break;
            }
            REQUIRE(containsSomething == true);
        }
    }

    void RequireFindsAndContains(LinkedListType list, typename idx_type idx)
    {
        SECTION("RequireFindsAndContains " + std::to_string(idx))
        {

            using idx_type = idx_type;
            idx_type result = 0;

            REQUIRE(list.contains(idx) == true);

            SECTION("find")
            {
                REQUIRE(list.find(idx, result) == true);
                //REQUIRE(result == idx); // it will give count
            }
            SECTION("find_forward")
            {
                REQUIRE(list.find_forward(idx, result) == true);
                //REQUIRE(result == idx);
            }
            SECTION("find_backward")
            {
                REQUIRE(list.find_backward(idx, result) == true);
                //REQUIRE(result == idx);
            }

            for (idx_type k = 0; k < list.capacity(); ++k)
            {
                if (!list.contains(k)) continue;
                std::string sk = std::to_string(k);
                SECTION("find_forward(from="+sk+") || find_backward(from="+sk+")")
                {
                    REQUIRE((list.find_forward(k, idx, result) || list.find_backward(k, idx, result)));
                    //REQUIRE(result == idx);
                }
            }
        }
    }

    void RequireFindsNotAndContainsNot(LinkedListType list, typename idx_type idx)
    {
        SECTION("RequireFindsNotAndContainsNot " + std::to_string(idx))
        {

            using idx_type = idx_type;
            idx_type result = 0;

            REQUIRE(list.contains(idx) == false);

            SECTION("find")
            {
                REQUIRE(list.find(idx, result) == false);
                //REQUIRE(result == idx); // it will give count
            }
            SECTION("find_forward")
            {
                REQUIRE(list.find_forward(idx, result) == false);
                //REQUIRE(result == idx);
            }
            SECTION("find_backward")
            {
                REQUIRE(list.find_backward(idx, result) == false);
                //REQUIRE(result == idx);
            }

            for (idx_type k = 0; k < list.capacity(); ++k)
            {
                std::string sk = std::to_string(k);
                SECTION("find_forward(from=" + sk + ") || find_backward(from=" + sk + ")")
                {
                    REQUIRE(false == (list.find_forward(k, idx, result) || list.find_backward(k, idx, result)));
                    //REQUIRE(result == idx);
                }
            }
        }
    }

    void RequireFindsContained(LinkedListType list)
    {
        SECTION("RequireFindsContained")
        {
            using idx_type = idx_type;
            idx_type result = 0;

            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                if (list.contains(i))
                {
                    RequireFindsAndContains(list, i);
                }
                REQUIRE(list.contains(i) == true);
            }
        }
    }

    void RequireFindsAndContainsOnlyIterated(LinkedListType list)
    {
        RequireFindsAndContainsOnlyIteratedForward(list);
        RequireFindsAndContainsOnlyIteratedBackward(list);
    }
    void RequireFindsAndContainsOnlyIteratedForward(LinkedListType list)
    {
        SECTION("RequireFindsAndContainsOnlyIteratedForward")
        {
            using idx_type = idx_type;
            if (list.size() == 0) return;
            idx_type result = 0;
            idx_type current = 0;
            idx_type last = 0;
            std::vector<bool> iterated;
            iterated.resize(list.capacity());
            for (int i = 0; i < list.capacity(); ++i) iterated[i] = false;
            //std::fill_n(iterated.begin(), iterated.end(), false);
            REQUIRE(list.front(current) == true);
            iterated[current] = true;
            int count = 1;
            while (list.next(current, current))
            {
                iterated[current] = true;
                last = current;
                ++count;
                REQUIRE(count <= list.capacity());
            } 
            REQUIRE(count == list.size());
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                if (iterated[i])
                {
                    RequireFindsAndContains(list, i);
                }
                else
                {
                    RequireFindsNotAndContainsNot(list, i);
                }
            }

        }
    }

    void RequireFindsAndContainsOnlyIteratedBackward(LinkedListType list)
    {
        SECTION("RequireFindsAndContainsOnlyIteratedBackward")
        {
            using idx_type = idx_type;
            if (list.size() == 0) return;
            idx_type result = 0;
            idx_type current = 0;
            idx_type last = 0;
            std::vector<bool> iterated;
            iterated.resize(list.capacity());
            for (int i = 0; i < list.capacity(); ++i) iterated[i] = false;
            //std::fill_n(iterated.begin(), iterated.end(), false);
            REQUIRE(list.back(current) == true);
            iterated[current] = true;
            int count = 1;
            while (list.prev(current, current))
            {
                iterated[current] = true;
                last = current;
                ++count;
                REQUIRE(count <= list.capacity());
            } 
            REQUIRE(count == list.size());
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                if (iterated[i])
                {
                    RequireFindsAndContains(list, i);
                }
                else
                {
                    RequireFindsNotAndContainsNot(list, i);
                }
            }

        }
    }

    void RequireClear(LinkedListType list)
    {
        SECTION("RequireClear list.clear()")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            list.clear();
            RequireListEmpty(list, false, false, false);
        }
    }

    void RequireRemoveFails(LinkedListType list)
    {
        SECTION("RequireRemoveFails")
        {
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                RequireRemoveFailsFor(list, i);
            }
        }
    }

    void RequireRemoveFailsFor(LinkedListType list, idx_type idx)
    {
        SECTION("RequireRemoveFailsFor " + std::to_string(idx))
        {
            using idx_type = idx_type;
            idx_type result = 0;
            idx_type size_before = list.size();
            REQUIRE(list.remove(idx) == false);
            REQUIRE(list.size() == size_before);
        }
    }

    void RequireRemoveSucceedsFor(LinkedListType list, idx_type idx)
    {
        SECTION("RequireRemoveSucceedsFor " + std::to_string(idx))
        {
            using idx_type = idx_type;
            idx_type result = 0;
            idx_type size_before = list.size();
            REQUIRE(list.remove(idx) == true);
            REQUIRE(list.size() == size_before-1);
            REQUIRE(list.contains(idx) == false);
        }
    }

    void RequireRemoveSucceedsForContained(LinkedListType list)
    {
        SECTION("RequireRemoveSucceedsForContained")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            for (idx_type idx = 0; idx < list.capacity(); ++idx)
            {
                SECTION(std::to_string(idx))
                {
                    if (list.contains(idx))
                        RequireRemoveSucceedsFor(list, idx);
                    else
                        RequireRemoveFailsFor(list, idx);
                }
            }
        }
    }

    void RequireFindFails(LinkedListType list)
    {
        SECTION("RequireFindFails")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                REQUIRE(list.find(i, result) == false);
                REQUIRE(list.find_forward(i, result) == false);
                REQUIRE(list.find_backward(i, result) == false);
                for (idx_type k = 0; k < list.capacity(); ++k)
                {
                    REQUIRE(list.find_forward(k, i, result) == false);
                    REQUIRE(list.find_backward(k, i, result) == false);
                }
            }
        }
    }

    void RequirePopFails(LinkedListType list)
    {
        SECTION("RequirePopFails")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            SECTION("pop_front")
            {
                REQUIRE(list.pop_front(result) == false);
            }
            SECTION("pop_back")
            {
                REQUIRE(list.pop_back(result) == false);
            }
        }
    }

    void RequirePopSucceeds(LinkedListType list)
    {
        SECTION("RequirePopSucceeds")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            auto size_before = list.size();
            idx_type front = 0; 
            idx_type back = 0; 
            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);
            SECTION("pop_front")
            {
                REQUIRE(list.pop_front(result) == true);
                REQUIRE(result == front);
                if (size_before > 1)
                {
                    REQUIRE(list.front(result) == true);
                    REQUIRE(result != front);
                    REQUIRE(list.back(result) == true);
                    REQUIRE(result == back);
                }
                else
                {
                    REQUIRE(list.back(result) == false);
                }

                REQUIRE(list.size() == size_before - 1);
                if (list.size() == 0)
                {
                    RequireListEmpty(list, false, false, false);
                }
                else
                {
                    RequireListNonEmpty(list, true, true, false);
                }
            }
            SECTION("pop_back")
            {
                REQUIRE(list.pop_back(result) == true);
                REQUIRE(result == back);
                if (size_before > 1)
                {
                    REQUIRE(list.front(result) == true);
                    REQUIRE(result == front);
                    REQUIRE(list.back(result) == true);
                    REQUIRE(result != back);
                }
                else
                {
                    REQUIRE(list.front(result) == false);
                }
                REQUIRE(list.size() == size_before - 1);
                if (list.size() == 0)
                {
                    RequireListEmpty(list, false, false, false);
                }
                else
                {
                    RequireListNonEmpty(list, true, true, false);
                }
            }

        }
    }

    void RequirePushFails(LinkedListType list)
    {
        SECTION("RequirePushFails")
        {
            using idx_type = idx_type;
            idx_type result;
            SECTION("push_front")
            {
                REQUIRE(list.push_front(result) == false);
            }
            SECTION("push_back")
            {
                REQUIRE(list.push_back(result) == false);
            }
        }
    }


    void RequireInsertFails(LinkedListType list)
    {
        SECTION("RequireInsertFails")
        {

            using idx_type = idx_type;
            idx_type result = 0;
            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                for (idx_type k = 0; k < list.capacity(); ++k)
                {
                    SECTION("insert")
                    {
                        REQUIRE(list.insert(i, k) == false);
                    }
                    SECTION("insert_forward")
                    {
                        REQUIRE(list.insert_forward(i, k) == false);
                    }
                    SECTION("insert_backward")
                    {
                        REQUIRE(list.insert_backward(i, k) == false);
                    }
                }
            }
        }
    }

    void RequireAddedFirstItem(LinkedListType list, typename idx_type idx)
    {
        SECTION("RequireAddedFirstItem " + std::to_string(idx))
        {
            using idx_type = idx_type;
            idx_type result = 0;

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == 1);
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == idx);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == idx);
            RequireFindsAndContains(list, idx);
            RequireListNonEmpty(list, true, true, true);
        }
    }

    void RequireAddFirstItem(LinkedListType list)
    {
        SECTION("RequireAddFirstItem")
        {
            using idx_type = idx_type;
            //RequireListEmpty(list, false, false, false);

            for (idx_type i = 0; i < list.capacity(); ++i)
            {
                SECTION("push_front idx=" + std::to_string(i))
                {
                    REQUIRE(list.push_front(i) == true);
                    RequireAddedFirstItem(list, i);
                }
                SECTION("push_back idx=" + std::to_string(i))
                {
                    REQUIRE(list.push_front(i) == true);
                    RequireAddedFirstItem(list, i);
                }
                for (idx_type k = 0; k < list.capacity(); ++k)
                {

                    SECTION("fails: insert behind=" + std::to_string(k) + " idx=" + std::to_string(i))
                    {
                        REQUIRE(list.insert(k, i) == false);
                    }
                    SECTION("fails: insert_forward behind=" + std::to_string(k) + " idx=" + std::to_string(i))
                    {
                        REQUIRE(list.insert_forward(k, i) == false);
                    }
                    SECTION("fails: insert_backward before=" + std::to_string(k) + " idx=" + std::to_string(i))
                    {
                        REQUIRE(list.insert_backward(k, i) == false);
                    }
                }
            }
        }
    }

    void RequirePrev(LinkedListType list)
    {
        SECTION("RequirePrev")
        {
            idx_type result = 0;
            idx_type front = 0;
            for (int i = 0; i < list.capacity(); ++i)
            {
                if (!list.contains(i))
                {
                    REQUIRE(list.prev(i, result) == false);
                }
                else
                {
                    // list contains something, therefore front must work
                    REQUIRE(list.front(front) == true);
                    if (front == i)
                    {
                        result = -1;
                        REQUIRE(list.prev(i, result) == false); // no prev after result
                        REQUIRE(result == i); // result is set despite false return value
                    }
                    else
                    {
                        REQUIRE(list.prev(i, result) == true);
                        REQUIRE(result != i); 
                    }
                }
            }
        }
    }
    void RequireNext(LinkedListType list)
    {
        SECTION("RequireNext")
        {
            idx_type result = 0;
            idx_type back = 0;
            for (int i = 0; i < list.capacity(); ++i)
            {
                if (!list.contains(i))
                {
                    REQUIRE(list.next(i, result) == false);
                }
                else
                {
                    // list contains something, therefore back must work
                    REQUIRE(list.back(back) == true);
                    if (back == i)
                    {
                        result = -1;
                        REQUIRE(list.next(i, result) == false); // no next after result
                        REQUIRE(result == i); // result is set despite false return value
                    }
                    else
                    {
                        REQUIRE(list.next(i, result) == true);
                        REQUIRE(result != i);
                    }
                }
            }
        }
    }

    void RequirePrevNextAre(
        LinkedListType list,
        idx_type k,
        idx_type prev_k,
        idx_type next_k
    )
    {
        // SECTION("RequirePrevNextAre")
        // {
        using idx_type = idx_type;
        idx_type result = 0;
        list.prev(k, result);
        REQUIRE(result == prev_k);
        list.next(k, result);
        REQUIRE(result == next_k);
        // }
    }

    void RequirePrevNextAre(
        LinkedListType list,
        idx_type k,
        idx_type i,
        idx_type prev_k,
        idx_type next_k,
        idx_type prev_i,
        idx_type next_i
    )
    {
        RequirePrevNextAre(list, k, prev_k, next_k);
        RequirePrevNextAre(list, i, prev_i, next_i);
    }
    void RequireAddItemSucceeds(LinkedListType list)
    {
        SECTION("RequireAddItemSucceeds")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            //RequireListNonEmpty(list, false, false, false);
            auto size_before = list.size();
            idx_type front = 0;
            idx_type back = 0;
            idx_type front_next = 0;
            idx_type back_prev = 0;
            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);
            list.next(front, front_next);
            list.prev(back, back_prev);
            for (idx_type idx = 0; idx < list.capacity(); ++idx)
            {
                if ((!list.fixCapacity() || (list.size() < list.capacity())) && (!list.contains(idx)))
                {
                    RequirePushFrontItemSucceeds(list, idx);
                    RequirePushBackItemSucceeds(list, idx);
                }
                else
                {
                    RequirePushFrontItemFails(list, idx);
                    RequirePushBackItemFails(list, idx);
                }

                for (idx_type insert_idx = 0; insert_idx < list.capacity(); ++insert_idx)
                {
                    if (list.contains(insert_idx) && !list.contains(idx))
                    {
                        RequireInsertItemSucceeds(list, insert_idx, idx, size_before, front, back);
                    }
                    else
                    {
                        RequireInsertItemFails(list, insert_idx, idx, size_before, front, back);
                    }
                }
            }
        }
    }

    void RequirePushFrontItemSucceeds(LinkedListType list, idx_type idx)
    {
        if (list.size() == 0)
            RequirePushFrontItemSucceedsOnEmptyList(list, idx);
        else
            RequirePushFrontItemSucceedsOnNonEmptyList(list, idx);
    }

    void RequirePushFrontItemSucceedsOnNonEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushFrontItemSucceedsOnNonEmptyList idx=" + std::to_string(idx))
        {
            REQUIRE(!list.contains(idx));
            idx_type front = 0;
            idx_type back = 0;
            idx_type front_next = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before > 0);

            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);
            list.next(front, front_next);

            REQUIRE(list.push_front(idx) == true);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before + 1);
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == idx);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == back);

            RequirePrevNextAre(
                list,
                idx, front,
                idx, front, idx, front_next
            );

            RequireFindsAndContains(list, idx);
            RequireListNonEmpty(list, true, true, true);
        }
    }

    void RequirePushFrontItemSucceedsOnEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushFrontItemSucceedsOnEmptyList idx=" + std::to_string(idx))
        {
            REQUIRE(!list.contains(idx));
            idx_type front = 0;
            idx_type back = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before == 0);
            REQUIRE(list.front(front) == false);
            REQUIRE(list.back(back) == false);

            REQUIRE(list.push_front(idx) == true);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before + 1);
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == idx);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == idx);

            RequirePrevNextAre(
                list,
                idx, 
                idx, idx
            );

            RequireFindsAndContains(list, idx);
            RequireListNonEmpty(list, true, true, true);
        }
    }

    void RequirePushFrontItemFails(LinkedListType list, idx_type idx)
    {
        if (list.size() == 0)
            RequirePushFrontItemFailsOnEmptyList(list, idx);
        else
            RequirePushFrontItemFailsOnNonEmptyList(list, idx);
    }
    void RequirePushFrontItemFailsOnNonEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushFrontItemFailsOnNonEmptyList idx=" + std::to_string(idx))
        {
            idx_type front = 0;
            idx_type back = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before > 0);
            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);

            REQUIRE(list.push_front(idx) == false);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before);
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == front);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == back);
        }
    }
    
    void RequirePushFrontItemFailsOnEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushFrontItemFailsOnEmptyList idx=" + std::to_string(idx))
        {
            idx_type front = 0;
            idx_type back = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before == 0);
            REQUIRE(list.front(front) == false);
            REQUIRE(list.back(back) == false);

            REQUIRE(list.push_front(idx) == false);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before);
            REQUIRE(list.empty() == true);
            REQUIRE(list.front(result) == false);
            REQUIRE(list.back(result) == false);
        }
    }
        
    void RequirePushBackItemSucceeds(LinkedListType list, idx_type idx)
    {
        if (list.size() == 0)
            RequirePushBackItemSucceedsOnEmptyList(list, idx);
        else
            RequirePushBackItemSucceedsOnNonEmptyList(list, idx);
    }

    void RequirePushBackItemSucceedsOnNonEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushBackItemSucceedsOnNonEmptyList idx=" + std::to_string(idx))
        {
            REQUIRE(!list.contains(idx));
            idx_type front = 0;
            idx_type back = 0;
            idx_type back_prev = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before > 0);

            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);
            list.prev(back, back_prev);

            REQUIRE(list.push_back(idx) == true);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before + 1);
            REQUIRE(list.empty() == false);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == idx);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == front);

            RequirePrevNextAre(
                list,
                back, idx,
                back_prev, idx, back, idx
            );

            RequireFindsAndContains(list, idx);
            RequireListNonEmpty(list, true, true, true);            
        }
    }

    void RequirePushBackItemSucceedsOnEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushBackItemSucceedsOnEmptyList idx=" + std::to_string(idx))
        {
            idx_type front = 0;
            idx_type back = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before == 0);
            REQUIRE(list.front(front) == false);
            REQUIRE(list.back(back) == false);

            REQUIRE(list.push_back(idx) == true);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before + 1);
            REQUIRE(list.empty() == false);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == idx);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == idx);

            RequirePrevNextAre(
                list,
                idx, 
                idx, idx
            );

            RequireFindsAndContains(list, idx);
            RequireListNonEmpty(list, true, true, true);                   
        }
    }

    void RequirePushBackItemFails(LinkedListType list, idx_type idx)
    {
        if (list.size() == 0)
            RequirePushBackItemFailsOnEmptyList(list, idx);
        else
            RequirePushBackItemFailsOnNonEmptyList(list, idx);
    }
    void RequirePushBackItemFailsOnNonEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushBackItemFailsOnNonEmptyList idx=" + std::to_string(idx))
        {
            idx_type front = 0;
            idx_type back = 0;
            idx_type result = 0;
            auto size_before = list.size();
            REQUIRE(size_before > 0);
            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);

            REQUIRE(list.push_back(idx) == false);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before);
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(result == front);
            REQUIRE(list.back(result) == true);
            REQUIRE(result == back);
        }
    }
    
    void RequirePushBackItemFailsOnEmptyList(LinkedListType list, idx_type idx)
    {
        SECTION("RequirePushBackItemFailsOnEmptyList idx=" + std::to_string(idx))
        {
            idx_type front = 0;
            idx_type back = 0;
            auto size_before = list.size();
            REQUIRE(size_before == 0);
            REQUIRE(list.front(front) == false);
            REQUIRE(list.back(back) == false);

            REQUIRE(list.push_back(idx) == false);

            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.size() == size_before);
            REQUIRE(list.empty() == true);
            REQUIRE(list.front(front) == false);
            REQUIRE(list.back(back) == false);
        }
    }
    
    void RequireInsertItemSucceeds(LinkedListType list, idx_type insert_idx, idx_type idx, int size_before, int front, int back)
    {
        SECTION("RequireInsertItemSucceeds insert_idx=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
        {
            REQUIRE(list.contains(insert_idx));
            REQUIRE(!list.contains(idx));

            idx_type result = 0; 
            idx_type prev = 0; 
            idx_type next = 0;
            list.prev(insert_idx, prev);
            list.next(insert_idx, next);

            SECTION("insert behind=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert(insert_idx, idx) == true);
                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before + 1);
                REQUIRE(list.empty() == false);
                REQUIRE(list.front(result) == true);
                REQUIRE(result == front);
                REQUIRE(list.back(result) == true);
                if (insert_idx == back)
                    REQUIRE(result == idx);
                else
                    REQUIRE(result == back);


                RequirePrevNextAre(
                    list,
                    insert_idx, idx,
                    prev, idx, insert_idx, ((insert_idx == back) ? idx : next)
                );

                RequireFindsAndContains(list, idx);
                RequireListNonEmpty(list, true, true, true);
            }
            SECTION("insert_forward behind=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert_forward(insert_idx, idx) == true);
                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before + 1);
                REQUIRE(list.empty() == false);
                REQUIRE(list.front(result) == true);
                REQUIRE(result == front);
                REQUIRE(list.back(result) == true);
                if (insert_idx == back)
                    REQUIRE(result == idx);
                else
                    REQUIRE(result == back);

                RequirePrevNextAre(
                    list,
                    insert_idx, idx,
                    prev, idx, insert_idx, ((insert_idx == back) ? idx : next)
                );

                RequireFindsAndContains(list, idx);
                RequireListNonEmpty(list, true, true, true);
            }
            SECTION("insert_backward before=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert_backward(insert_idx, idx) == true);
                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before + 1);
                REQUIRE(list.empty() == false);
                REQUIRE(list.back(result) == true);
                REQUIRE(result == back);
                REQUIRE(list.front(result) == true);
                if (insert_idx == front)
                    REQUIRE(result == idx);
                else
                    REQUIRE(result == front);

                RequirePrevNextAre(
                    list,
                    idx, insert_idx,
                    ((insert_idx == front) ? idx : prev), insert_idx, idx, next
                );

                RequireFindsAndContains(list, idx);
                RequireListNonEmpty(list, true, true, true);
            }
        }
    }
    void RequireInsertItemFails(LinkedListType list, idx_type insert_idx, idx_type idx, int size_before, int front, int back)
    {
        SECTION("RequireInsertItemFails insert_idx=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
        {
            idx_type result = 0;
            SECTION("insert behind=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert(insert_idx, idx) == false);

                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before);
                REQUIRE(list.empty() == false);
                REQUIRE(list.front(result) == true);
                REQUIRE(result == front);
                REQUIRE(list.back(result) == true);
                REQUIRE(result == back);
            }
            SECTION("insert_forward behind=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert_forward(insert_idx, idx) == false);

                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before);
                REQUIRE(list.empty() == false);
                REQUIRE(list.front(result) == true);
                REQUIRE(result == front);
                REQUIRE(list.back(result) == true);
                REQUIRE(result == back);

            }
            SECTION("insert_backward before=" + std::to_string(insert_idx) + " idx=" + std::to_string(idx))
            {
                REQUIRE(list.insert_backward(insert_idx, idx) == false);

                // REQUIRE(list.capacity() == list.capacity());
                REQUIRE(list.size() == size_before);
                REQUIRE(list.empty() == false);
                REQUIRE(list.front(result) == true);
                REQUIRE(result == front);
                REQUIRE(list.back(result) == true);
                REQUIRE(result == back);
            }
        }
    }

    void RequireAddItemFails(LinkedListType list)
    {
        SECTION("RequireAddItemFails")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            //RequireListNonEmpty(list, false, false, false);
            auto size_before = list.size();
            idx_type front = 0;
            idx_type back = 0;
            REQUIRE(list.front(front) == true);
            REQUIRE(list.back(back) == true);
            for (idx_type idx = 0; idx < list.capacity(); ++idx)
            {
                RequirePushFrontItemFails(list, idx);
                RequirePushBackItemFails(list, idx);
                for (idx_type k = 0; k < list.capacity(); ++k)
                {
                    RequireInsertItemFails(list, k, idx, size_before, front, back);
                }
            }
        }
    }

    void RequireListEmpty(LinkedListType list, bool testRemove, bool testGrowing, bool testClear)
    {
        static int counter = 0;
        //SECTION("RequireListEmpty #" + std::to_string(counter++))
        SECTION("RequireListEmpty")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            REQUIRE(list.size() == 0);
            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.empty() == true);
            REQUIRE(list.front(result) == false);
            REQUIRE(list.back(result) == false);
            RequireContainsNone(list);
            RequireFindFails(list);
            RequirePrev(list);
            RequireNext(list);
            //static bool testMutations = true;
            //if (testMutations)
            //    testMutations = false;
            //else
            //    return;
            if (testRemove)
            {
                RequirePopFails(list);
                RequireRemoveFails(list);
            }
            if (testGrowing)
            {
                if (list.capacity() == 0)
                {
                    RequirePushFails(list);
                    RequireInsertFails(list);
                }
                RequireAddFirstItem(list);
            }
            if (testClear)
            {
                RequireClear(list);
            }
        }
        counter++;
    }

    void RequireListNonEmpty(LinkedListType list, bool testRemove, bool testClear, bool testGrowing)
    {
        SECTION("RequireListNonEmpty")
        {
            using idx_type = idx_type;
            idx_type result = 0;
            REQUIRE(list.size() > 0);
            // REQUIRE(list.capacity() == list.capacity());
            REQUIRE(list.empty() == false);
            REQUIRE(list.front(result) == true);
            REQUIRE(list.back(result) == true);
            RequireContainsSomething(list);
            // RequireFindsContained(list);
            RequireFindsAndContainsOnlyIterated(list);
            RequirePrev(list);
            RequireNext(list);
            if (testRemove)
            {
                RequirePopSucceeds(list);
                RequireRemoveSucceedsForContained(list);
            }
            if (testClear)
            {
                RequireClear(list);
            }
            if (testGrowing && (list.capacity() <= MaxCapacity))
            {
                // RequireInsertFails(list);
                if (!list.fixCapacity() || (list.size() < list.capacity()))
                    RequireAddItemSucceeds(list);
                else
                    RequireAddItemFails(list);
            }
        }
    }
};
TEST_CASE("gui_application::memory_pool::IndexSetLinkedListArray", "[gui_application][memory_pool][IndexSetLinkedListArray]")
{
   SECTION("capacity = 1")
   {
       constexpr auto Capacity = 1;
       using ListType = IndexSetLinkedListArray<Capacity>;
       using idx_type = ListType::idx_type;
       ListType list;
       idx_type idx = 0;
       
       Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
       // RequireListEmpty<Capacity>(list, true);
   }
   SECTION("capacity = 2")
   {
       constexpr auto Capacity = 2;
       using ListType = IndexSetLinkedListArray<Capacity>;
       using idx_type = ListType::idx_type;
       ListType list;

       Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
   }
   SECTION("capacity = 3")
   {
       constexpr auto Capacity = 3;
       using ListType = IndexSetLinkedListArray<Capacity>;
       using idx_type = ListType::idx_type;
       ListType list;

       Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
   }
}
TEST_CASE("gui_application::memory_pool::IndexSetLinkedListVector", "[gui_application][memory_pool][IndexSetLinkedListVector]")
{
    SECTION("capacity = 1")
    {
        constexpr auto Capacity = 1;
        using ListType = IndexSetLinkedListVector<>;
        using idx_type = ListType::idx_type;
        ListType list(Capacity);
        idx_type idx = 0;
        
        Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
    }
    SECTION("capacity = 2")
    {
        constexpr auto Capacity = 2;
        using ListType = IndexSetLinkedListVector<>;
        using idx_type = ListType::idx_type;
        ListType list(Capacity);

        Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
    }
    SECTION("capacity = 3")
    {
        constexpr auto Capacity = 3;
        using ListType = IndexSetLinkedListVector<>;
        using idx_type = ListType::idx_type;
        ListType list(Capacity);

        Require<ListType,Capacity>().RequireListEmpty(list, true, true, true);
    }
}
