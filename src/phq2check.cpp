// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with forward-only pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Tree validation code
// -------------------------------------------------------------------------------------------
#include "phqueue2.hpp"
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <limits>

#include "PointerMap.hpp"

#define ASSERT(x) do { if (!(x)) throw std::logic_error( #x ); } while(false)


/// @brief validate a 2-way Pairing heap
/// @param nodes    upper bound of nodes to expect
///
/// This does all checks that are possible on a Pairing Heap with forward-only pointers:
///  - the heap invariant between a node and its children is maintained
///  - any reachable node can be reached only in one way
/// Everything else would need structural assistance a simple 2-way tree cannot provide.
void
PairingHeapEasyT::validate_tree(
    size_t nodes) const
{
    // the structure of the tree forming a Pairing heap can be anything below the root node:
    // a horizontal list with no children, or a vertical list with no siblings, and anything
    // in between.  That keeps life interesting and efficient validation a challenge ;)

    PointerMapT                      set(nodes);
    std::vector<PairingNodeT const*> que;

    que.reserve(nodes); // pessimistic approach!

    // Step I: testing the root node. That one is simple:
    if (nullptr != _m_root) {
        ASSERT(nullptr == _m_root->_m_next);    // root must not have a sibling
        ASSERT(set.insert(_m_root));            // and must not yet be in the queue
        que.push_back(_m_root);
    }

    // Step II: validating haas two aspects: no member of the child list must go
    // before the parent, and a child must not be in the node set yet.
    //
    // The trick is managing the queue to avoid excessive growth of the stack.
    // We do this in a somewhat sneaky way here:
    //
    // To validate the the heap invariant, we have test all children against the parent node
    // according to the order predicate, and none of the children must have been visited before.
    // So we have to run down the right spine (the children list) immediately when processing
    // a node.
    //
    // The trick in managing the queue is that we always replace the tip with the sibling
    // of the the tip, unless there is none, in which case we pop and reduce the height
    // of the stack.  And if a node has children, we push the head of the the children
    // list.  This handles both extreme structures very efficient with a max depth of 1.
    while (!que.empty()) {
        PairingNodeT const *node{ que.back() };
        PairingNodeT const *chld{ node->_m_down };
        if (nullptr == (que.back() = node->_m_next)) {
            que.pop_back();
        }
        if (nullptr != chld) {
            que.push_back(chld);
            do {
                ASSERT(set.insert(chld));       // node never seen before
                ASSERT(!_pred(*chld, *node));   // heap invariant
            } while (nullptr != (chld = chld->_m_next));
        }
    }
}
// --*-- that's all folks --*--
