// -------------------------------------------------------------------------------------------
// priority queue unit tests
// -------------------------------------------------------------------------------------------
// this file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
#include "inc/lhqueue2.hpp"
#include "inc/mdqueue3.hpp"

#include <gtest/gtest.h>

TEST(MinDist2, InsertAndPopOrder) {
    LeftistHeapEasy<int> pq;

    pq.push(5);
    pq.push(1);
    pq.push(3);

    EXPECT_EQ(pq.front(), 1);
    pq.pop();
    EXPECT_EQ(pq.front(), 3);
    pq.pop();
    EXPECT_EQ(pq.front(), 5);
}

TEST(MinDist2, MergePreservesOrder) {
    LeftistHeapEasy<int> a, b;

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

TEST(MinDist2, BatchBuild) {
    LeftistHeapEasy<int> a, b;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    for (int i : v) a.push(i);
    b.push(v);

    while (!a.empty() && !b.empty()) {
        int x = a.front();
        EXPECT_EQ(a.front(), b.front());
        a.pop(); b.pop();
    }
    EXPECT_TRUE(a.empty());
    EXPECT_TRUE(b.empty());
}

TEST(MinDist3, InsertAndPopOrder) {
    MinDistHeap<int> pq;

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

TEST(MinDist3, MergePreservesOrder) {
    MinDistHeap<int> a, b;

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

TEST(MinDist3, BatchBuild) {
    MinDistHeap<int> a, b;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    for (int i : v) a.push(i);
    b.push(v);

    while (!a.empty() && !b.empty()) {
        int x = a.front();
        EXPECT_EQ(a.front(), b.front());
        a.pop();
        b.pop();
    }
    EXPECT_TRUE(a.empty());
    EXPECT_TRUE(b.empty());
}

TEST(MinDist3, IterReach) {
    MinDistHeap<int> a;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    a.push(v);
    int cnt = 0;
    for (auto it{a.begin()}; it != a.end(); ++it) {
        ASSERT_LT(cnt, v.size());
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size());
}

TEST(MinDist3, IterDelete) {
    MinDistHeap<int> a;
    std::vector<int> v{1, 3, 5, 2, 4, 6};
    for (auto i : v)
        a.push(i);
    //a.push(v);
    for (auto it{a.begin()}; it != a.end(); /*NOP*/) {
        if (*it & 1) {
            it = a.remove(it);
        } else {
            ++it;
        }
    }

    int cnt = 0;
    for (auto it{a.begin()}; it != a.end(); ++it) {
        ASSERT_LT(cnt, v.size());
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size() / 2);

    int old{0};
    cnt = 0;
    while (!a.empty()) {
        ASSERT_EQ(old + 2, a.front());
        old = a.front();
        a.pop();
        ++cnt;
    }
    ASSERT_EQ(cnt, v.size() / 2);
}

TEST(MinDist3, IterBack) {
    MinDistHeap<int> a;
    std::vector<int> v{1, 3, 5, 2, 4, 6};

    a.push(v);

    auto it{a.end()};
    auto last{a.begin()};
    unsigned cnt{0};
    while (it != last) {
        --it;
        ++cnt;
    }
    ASSERT_EQ(v.size(), cnt);
    ASSERT_THROW(--it, std::out_of_range);
}

TEST(MinDist3, IterBackAfterErase) {
    MinDistHeap<int> a;
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
    auto it{a.end()};
    auto last{a.begin()};
    unsigned cnt = 0;
    while (it != last) {
        --it;
        ASSERT_EQ(0, (*it & 1));
        ++cnt;
    }
    ASSERT_EQ(50, cnt);
}

// --*-- that's all folks --*--
