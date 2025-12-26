// -------------------------------------------------------------------------------------
// Leftist Heap priority queue
// -------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------
// A classic children-links-only implementation: no iteration, no decrease-key support,
// only push or multi-push, tip, pop
// -------------------------------------------------------------------------------------
// base class implementation
// -------------------------------------------------------------------------------------

#include "lhqueue2.hpp"
#include <climits>

LeftistHeapEasyT::BaseNodeT*
LeftistHeapEasyT::_singleton(BaseNodeT* node)
{
    if (nullptr != node) {
        node->_m_lptr = node->_m_rptr = nullptr;
        node->_m_dist = 1;
    }
    return node;
}

/// @brief merge two heaps. O(log(N)) actual
/// @param h1   1st heap
/// @param h2   2nd heap
/// @return     root of combined heap
LeftistHeapEasyT::BaseNodeT*
LeftistHeapEasyT::_merge(
    BaseNodeT *h1,
    BaseNodeT *h2) const
{
    if (nullptr == h1) {
        std::swap(h1, h2);
    }
    if (nullptr != h2) {
        if (_pred(*h2, *h1)) {
            std::swap(h1, h2);
        }
        h1->_m_rptr = _merge(h1->_m_rptr, h2);
        if ((nullptr == h1->_m_lptr) || (h1->_m_rptr->_m_dist > h1->_m_lptr->_m_dist)) {
            std::swap(h1->_m_rptr, h1->_m_lptr);
        }
        h1->_m_dist = (h1->_m_rptr ? h1->_m_rptr->_m_dist : 0) + 1;
    }
    return h1;
}

/// @brief push a node into the heap
/// @param node node to insert
void
LeftistHeapEasyT::_push(
    BaseNodeT* node)
{
    _m_root = _merge(_m_root, _singleton(node));
}

/// @brief batch-building a heap from a list of nodes in O(N)
/// @param head head of a list chained via @c _m_rptr
void
LeftistHeapEasyT::_push_list(
    BaseNodeT* head)
{
    static constexpr unsigned limit{ sizeof(void*) * CHAR_BIT };
    BaseNodeT* hedge[limit];
    unsigned   hsize{ 0 }, hidx;
    BaseNodeT* node;

    // Phase I: construct the hedge, bottom-up
    while (nullptr != (node = head)) {  // more work to do?
        head = node->_m_rptr;

        _singleton(node);
        for (hidx = 0; (hidx < hsize) && (nullptr != hedge[hidx]); ++hidx) {
            node = _merge(hedge[hidx], node);
            hedge[hidx] = nullptr;
        }
        if (hidx < hsize) {
            hedge[hidx] = node;
        } else if (hsize < limit) {
            hedge[hsize++] = node;
        } else {
            hedge[limit - 1] = node;
        }
    }

    // Phase II: combine all nodes in hedge
    for (hidx = 0; hidx < hsize; ++hidx)
        if (nullptr != hedge[hidx])
            node = _merge(hedge[hidx], node);

    // Phase III: merge the created heap with the existing heap!
    _m_root = _merge(_m_root, node);
}

/// @brief pop the tip/root node from the heap and build a new heap from its children
/// @return pointer to former root or @c nullptr if empty
LeftistHeapEasyT::BaseNodeT*
LeftistHeapEasyT::_pop()
{
    BaseNodeT *retv { _m_root };
    if (nullptr != retv) {
        _m_root = _merge(retv->_m_lptr, retv->_m_rptr);
    }
    return _singleton(retv);
}

/// @brief shred a tree to single nodes
/// @param pref root of tree to shred
/// @return     pointer to next node or @c nullptr if no more nodes
///
/// This a "tree funnel": For every node at the root position, replace the root by a tree
/// where the right child of the root is grafted to the end of the right spine of the
/// left subtree.  This guarantees serialization of the tree in O(1) amortized per node
/// and in O(N) strict for the whole tree. (And since this is a leftist heap, we reach
/// the end of the right spine _soon_ !)
///
/// While every shredding step creates a new binary tree, that tree has no structural bounds.
LeftistHeapEasyT::BaseNodeT*
LeftistHeapEasyT::_shred_pop(
    BaseNodeT * &pref)
{
    BaseNodeT *retv { pref }, *scan;
    if (nullptr != retv) {
        if (nullptr == retv->_m_rptr) {
            pref = retv->_m_lptr;
        } else if (nullptr == retv->_m_lptr) {
            pref = retv->_m_rptr;
        } else {
            scan = retv->_m_lptr;
            while (nullptr != scan->_m_rptr) {
                scan = scan->_m_rptr;
            }
            scan->_m_rptr = retv->_m_rptr;
            pref = retv->_m_lptr;
        }
    }
    return _singleton(retv);
}
// --*-- that's all folks --*--
