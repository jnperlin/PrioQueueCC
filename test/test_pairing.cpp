// -------------------------------------------------------------------------------------------
// priority queue unit tests
// -------------------------------------------------------------------------------------------
// this file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
#include "inc/phqueue2.hpp"
#include "inc/phqueue3.hpp"

#include <gtest/gtest.h>

TEST(Pairing2, InsertAndPopOrder) {
    PairingHeapEasy<int> pq;

    pq.push(5);
    pq.push(1);
    pq.push(3);

    EXPECT_EQ(pq.front(), 1);
    pq.pop();
    EXPECT_EQ(pq.front(), 3);
    pq.pop();
    EXPECT_EQ(pq.front(), 5);
}

TEST(Pairing2, MergePreservesOrder) {
    PairingHeapEasy<int> a, b;

    for (int i : {1, 3, 5}) a.push(i);
    for (int i : {2, 4, 6}) b.push(i);

    a.merge(b);

    EXPECT_TRUE(b.empty());

    int prev = -1;
    while (!a.empty()) {
        int x = a.front();
        EXPECT_GE(x, prev);
        prev = x;
        a.pop();
    }
}

TEST(Pairing3, InsertAndPopOrder) {
    PairingHeap<int> pq;

    pq.push(5);
    pq.push(1);
    pq.push(3);

    EXPECT_EQ(pq.front(), 1);
    pq.pop();
    EXPECT_EQ(pq.front(), 3);
    pq.pop();
    EXPECT_EQ(pq.front(), 5);
    pq.pop();
}

TEST(Pairing3, MergePreservesOrder) {
    PairingHeap<int> a, b;

    for (int i : {1, 3, 5}) a.push(i);
    for (int i : {2, 4, 6}) b.push(i);

    a.merge(b);

    EXPECT_TRUE(b.empty());

    int prev = -1;
    while (!a.empty()) {
        int x = a.front();
        EXPECT_GE(x, prev);
        prev = x;
        a.pop();
    }
}

TEST(Pairing3, IterReach) {
    PairingHeap<int> a;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    for (auto i : v)
        a.push(i);
    int cnt = 0;
    for (auto it{a.begin()}; it != a.end(); ++it) {
        ASSERT_LT(cnt, v.size());
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size());
}

TEST(Pairing3, IterDelete) {
    PairingHeap<int> a;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    for (auto i : v)
        a.push(i);
    for (auto it{a.begin()}; it != a.end(); /*NOP*/) {
        if (*it & 1) {
            it = a.remove(it);
        } else {
            ++it;
        }
    }

    int cnt{ 0 };
    for (auto it{a.begin()}; it != a.end(); ++it) {
        ASSERT_LT(cnt, v.size());
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size() / 2);

    int old{ 0 };
    cnt = 0;
    while (!a.empty()) {
        ASSERT_EQ(old + 2, a.front());
        old = a.front();
        a.pop();
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size() / 2);
}

TEST(Pairing3, IterBack) {
    PairingHeap<int> a;
    std::vector<int> v{ 1, 3, 5, 2, 4, 6 };

    for (auto i : v)
        a.push(i);

    auto it{ a.end() };
    auto last{ a.begin() };
    unsigned cnt{0};
    while (it != last) {
        --it;
        ++cnt;
    }
    ASSERT_EQ(v.size(), cnt);
    ASSERT_THROW(--it, std::out_of_range);
}

TEST(Pairing3, IterBackAfterErase) {
    PairingHeap<int> a;
    for (int i = 0; i < 100; ++i)
        a.push(i);

    // erase during forward traversal
    for (auto it = a.begin(); it != a.end(); ) {
        if (*it & 1)
            it = a.remove(it);
        else
            ++it;
    }

    // now reverse-iterate
    auto it{ a.end() };
    auto last{ a.begin() };
    unsigned cnt{ 0 };
    while (it != last) {
        --it;
        ASSERT_EQ(0, (*it & 1));
        ++cnt;
    }
    ASSERT_EQ(50, cnt);
}

// --*-- that's all folks --*--
