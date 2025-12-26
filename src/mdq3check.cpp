// -------------------------------------------------------------------------------------------
// Minimum-Leaf-Distance-balanced (MinDist) Heap with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Tree validation code
// -------------------------------------------------------------------------------------------
#include "mdqueue3.hpp"
#include <queue>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <limits>

#define ASSERT(x) do { if (!(x)) throw std::logic_error( #x ); } while(false)


/// @brief validate a 2-way Leftist heap
/// @param nodes    upper bound of nodes to expect
///
/// This does all checks that are possible on a Leftist Heap with forward-only pointers:
///  - the heap invariant between a node and its children is maintained
///  - any reachable node can be reached only in one way
/// Everything else would need structural assistance a simple 2-way tree cannot provide.
void
MinDistHeapT::validate_tree() const
{
    // Full traversal of a distance-balanced heap can be tricky, as the nesting level can be
    // VERY deep.  We use a trick here that sounds a bit like a chicken-egg problem,
    // but is actually not:
    //
    //   >> We use PRIORITY QUEUE to enumerate nodes of a PRIORITY QUEUE! <<
    //
    // What seems a bit crazy at first, is NOT: The queue used in traversal is ordered by
    // LEAF DISTANCE, and we always proceed with the node having the smallest leaf distance.
    //
    // While this queue can still develop a substantial length, it should keep us close to
    // lowest possible queue length.
    //
    // So... why not always push the heavy left side to a simple stack and continue through
    // the lighter right one?  Wouldn't that be the obvious thing to do?
    //
    // "For every complicated problem, there's a simple, obvious, and utterly wrong solution."
    // A short path on the right spine does not prevent a very heavy left child below.
    // Using a distance-ordered priority queue ensures that we continue with the shortest
    // currently available path, always.

    // local comparator to sort a queue by leaf distance
    struct DistComp {
        bool operator()(const BaseNodeT *p1, const BaseNodeT *p2) const {
          return p1->_m_dist > p2->_m_dist;
        }
    };

    using PointerQueT = std::
      priority_queue<BaseNodeT const*, std::vector<BaseNodeT const*>, DistComp>;

    PointerQueT que;

    // Step I: testing the root node. That one is simple:
    ASSERT(nullptr == _m_root._m_pptr);
    ASSERT(nullptr == _m_root._m_rptr);
    if (nullptr != _m_root._m_lptr) {
        ASSERT(&_m_root == _m_root._m_lptr->_m_pptr);
        que.push(_m_root._m_lptr);
    }

    // Step II: Validate the next node from the queue.
    // Validation has two aspects: no child must go before the parent, and a child
    // must not be in the node set yet.
    // Then push any existing children to the queue and continue.
    while (!que.empty()) {
        short wlc{ 0 }, wrc{ 0 };
        BaseNodeT const *node{ que.top() };
        que.pop();

        if (nullptr != node->_m_lptr) {
            ASSERT(node == node->_m_lptr->_m_pptr);
            ASSERT(!_pred(*node->_m_lptr, *node)); // heap invariant
            que.push(node->_m_lptr);
            wlc = node->_m_lptr->_m_dist;
        }

        if (nullptr != node->_m_rptr) {
            ASSERT(node == node->_m_rptr->_m_pptr);
            ASSERT(!_pred(*node->_m_rptr, *node));   // heap invariant
            que.push(node->_m_rptr);
            wrc = node->_m_rptr->_m_dist;
        }
        ASSERT(node->_m_dist == (std::min(wlc, wrc) + 1));
    }
}
// --*-- that's all folks --*--
