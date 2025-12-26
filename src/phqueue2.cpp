// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with forward-only pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Base class implementation
// -------------------------------------------------------------------------------------------

#include "phqueue2.hpp"

// two simple helpers to attach nodes in horizontal or vertical order:

inline PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_cons(PairingNodeT* a, PairingNodeT* b)
{
  return a ? ((a->_m_next = b), a) : b;
}

inline PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_dunk(PairingNodeT* a, PairingNodeT* b)
{
  return a ? ((a->_m_down = b), a) : b;
}

/// @brief merge two heaps. O(1) actual -- the magic of Pairing Heaps!
/// @param h1   1st heap
/// @param h2   2nd heap
/// @return     root of combined heap
PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_merge(
    PairingNodeT *h1,
    PairingNodeT *h2) const
{
    PairingNodeT * retv;

    // merging a NULL heap with another heap obviously yields the other heap. With both heaps
    // present, we have have to decide which one becomes a child of the other heap. h1 gets
    // precedence unless that would violate the order constraint.
    if (nullptr == h1) {
        retv = h2;
    } else if (nullptr == h2) {
        retv = h1;
    } else if (!_pred(*h2, *h1)) {
        retv = _dunk(h1, _cons(h2, h1->_m_down));
    } else {
        retv = _dunk(h2, _cons(h1, h2->_m_down));
    }
    if (nullptr != retv) {
        retv->_m_next = nullptr;
    }
    return retv;
}

/// @brief build a heap from a sibling list of sub-heaps
/// @param h    head if sibling list
/// @return     root of combined heap
///
/// This is the core function of the Pairing Heap algorithm: merge pairs of nodes from
/// left to right, and then combine all these heaps into one from right to left.  We use
/// an internal @e stack of sub-heaps, so the reversal comes with no cost.
PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_build(
    PairingNodeT *h) const
{
    PairingNodeT *q{ nullptr }, *a, *b;
    // Combine pairs of sub-heaps. Might leave a single heap in original list, but that's ok
    // as this is the target of the merges anyway.
    while ((a = h) && (b = a->_m_next)) {
        h = b->_m_next;
        q = _cons(_merge(a, b), q);
    }

    // Merge all the heaps from step above into a single heap.
    while ((a = q)) {
        q = q->_m_next;
        h = _merge(a, h);
    }
    // And that's it. Really.
    return h;
}

/// @brief push a node into the heap
/// @param node node to insert
void
PairingHeapEasyT::_push(
    PairingNodeT* node)
{
    _m_root = _merge(_m_root, node);
}

/// @brief pop the tip/root node from the heap and build a new heap from its children
/// @return pointer to former root or @c nullptr if empty
PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_pop()
{
    PairingNodeT *retv { _m_root };
    if (nullptr != retv) {
        _m_root = _build(retv->_m_down);
        retv->_m_down = retv->_m_next = nullptr;
    }
    return retv;
}

/// @brief shred a tree to single nodes
/// @param pref root of tree to shred
/// @return     pointer to next node or @c nullptr if no more nodes
///
/// This a "tree funnel": For every node at the root position, replace the root by a tree
/// where the right child of the root is grafted to the end of the right spine of the
/// left subtree.  This guarantees serialization of the tree in O(1) amortized per node
/// and in O(N) strict for the whole tree.
///
/// While every shredding step creates a new binary tree, that tree has no structural bounds.
PairingHeapEasyT::PairingNodeT*
PairingHeapEasyT::_shred_pop(
    PairingNodeT * &pref)
{
    PairingNodeT *retv { pref }, *scan;
    if (nullptr != retv) {
        if (nullptr == retv->_m_next) {
            pref = retv->_m_down;
        } else if (nullptr == retv->_m_down) {
            pref = retv->_m_next;
        } else {
            scan = retv->_m_down;
            while (nullptr != scan->_m_next) {
                scan = scan->_m_next;
            }
            scan->_m_next = retv->_m_next;
            pref = retv->_m_down;
        }
        retv->_m_down = retv->_m_next = nullptr;
    }
    return retv;
}
// --*-- that's all folks --*--
