// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Implementation of the core functions of a PairingHeap, meant for use in derived classes
// to handle the topological issues in one location without template code bloat.
// -------------------------------------------------------------------------------------------

#include <cassert>
#include "phqueue3.hpp"

/// @file Pairing Heap with 3-way nodes

// -------------------------------------------------------------------------------------------
// connecting nodes / primitives on node level

/// @brief make b the immediate successor of a
/// @param a    left sibling node
/// @param b    right sibling node
/// @return     a if not @c NULL, else b
PairingHeapT::BaseNodeT*
PairingHeapT::_cons(
    BaseNodeT* a,
    BaseNodeT* b)
{
    if (a) a->_m_next = b;
    if (b) b->_m_prev = a;
    return a ? a : b;
}

/// @brief make b the immediate child of a
/// @param a    parent node
/// @param b    child node
/// @return     a if not @c NULL, else b
PairingHeapT::BaseNodeT*
PairingHeapT::_dunk(
    BaseNodeT* a,
    BaseNodeT* b)
{
    if (a) a->_m_down = b;
    if (b) b->_m_prev = a;
    return a ? a : b;
}

PairingHeapT::BaseNodeT *
PairingHeapT::_yield()
{
    BaseNodeT *temp{nullptr};
    std::swap(temp, _m_root._m_down);
    if (temp)
        temp->_m_prev = nullptr;
    return temp;
}

// -------------------------------------------------------------------------------------------
// cutting nodes or whole subtrees from the heap

/// @brief cut a node from the tree
/// @param node node to cut from the tree
/// @return @node as singleton heap
///
/// This replaces @c node by the heap created from its children. If there are none, the replacement
/// has to be the next sibling of the node, of course.  This retains most of the order already
/// achieved in the heap.
PairingHeapT::BaseNodeT*
PairingHeapT::_ncut(
    BaseNodeT * const node)
{
    assert(node && node->_m_prev);    // automagically breaks on sentinel!
    BaseNodeT *repl{ _build(node->_m_down) };
    BaseNodeT * const pred{ node->_m_prev };
    if (node == pred ->_m_next) {
        _cons(pred, _cons(repl, node->_m_next));
    } else {
        _dunk(pred, _cons(repl, node->_m_next));
    }
    node->_m_prev = node->_m_next = node->_m_down = nullptr;
    return node;
}

/// @brief cut a subtree from the heap
/// @param node subtree root
/// @return @c node, but cleanly cut (next/prev are @c nullptr)
PairingHeapT::BaseNodeT*
PairingHeapT::_tcut(
    BaseNodeT* const node)
{
    assert(node && node->_m_prev);    // automagically breaks on sentinel!
    BaseNodeT * const pred{ node->_m_prev };
    if (node == pred->_m_next) {
        _cons(pred, node->_m_next);
    } else {
        _dunk(pred, node->_m_next);
    }

    node->_m_prev = node->_m_next = nullptr;
    return node;
}

// -------------------------------------------------------------------------------------------
// core functions -- this makes a binary tree a Pairing Heap

/// @brief merge two heaps given by the root nodes
/// @param h1   1st heap / left side
/// @param h2   2nd heap / right side
/// @return root of merged heap
PairingHeapT::BaseNodeT*
PairingHeapT::_merge(
    BaseNodeT *h1,
    BaseNodeT *h2) const
{
    BaseNodeT * retv;

    if (nullptr == h1) {
        retv = h2;
    } else if (nullptr == h2) {
        retv = h1;
    } else if ( ! _pred(*h2, *h1)) {
        retv = _dunk(h1, _cons(h2, h1->_m_down));
    } else {
        retv = _dunk(h2, _cons(h1, h2->_m_down));
    }
    if (nullptr != retv) {
        retv->_m_prev = retv->_m_next = nullptr;
    }
    return retv;
}

/// @brief build a heap from a sibling list
/// @param node start of list
/// @return     root of created heap
///
/// This is the "magic" function of the Pairing Heap.  As we have the sibling list as, well,
/// a list, merging pairs of nodes, storing them in a list, and finally merging all these little
/// heaps into one is a moderate effort in pointer swivelling.
PairingHeapT::BaseNodeT*
PairingHeapT::_build(
    BaseNodeT *node) const
{
    BaseNodeT *q{ nullptr }, *a, *b;
    while ((a = node) && (b = a->_m_next)) {
        node = b->_m_next;
        q = _cons(_merge(a, b), q);
    }

    // since we did some sloppy chopping, we have to make sure that 'node' does not keep
    // a dangling pointer to the left/parent side. (This happens if node was a singleton!)
    if ((a = q)) {
        do {
            q = a->_m_next;
            node = _merge(a, node);
        } while ((a = q));
    } else if (node) {
        node->_m_prev = nullptr;
    }

    return node;
}

/// @brief push a node into the heap
/// @param node node to insert
/// @return @c node
PairingHeapT::BaseNodeT*
PairingHeapT::_push(
    BaseNodeT* node)
{
    _dunk(&_m_root, _merge(_m_root._m_down, node));
    return node;
}

/// @brief pop the root element
/// @return the old root or @c NULL on empty heap
PairingHeapT::BaseNodeT*
PairingHeapT::_pop()
{
    BaseNodeT *retv { _m_root._m_down };
    if (nullptr != retv) {
        _dunk(&_m_root, _build(retv->_m_down));
        retv->_m_down = retv->_m_next = nullptr;
    }
    return retv;
}

/// @brief handle a decrease in the node's priority
/// @param node node to reposition in heap
/// @return     @c node
///
/// This an actual O(1) operation, as cutting a subtree from any position is O(1), and so
/// is the following merge of the subtree with the remaining heap.  As decreasing the the
/// node's weight does @e not invalidate the subtree rooted at @c node, we can prune and
/// graft the whole subtree here.  ( @c _reinsert() is more complicated, as we cannot
/// assume the heap invariant between the node and its children is preserved.)
PairingHeapT::BaseNodeT*
PairingHeapT::_decrease(
    BaseNodeT* node)
{
    assert(node && node->_m_prev);
    if (node != _m_root._m_down) {
        _dunk(&_m_root, _merge(_m_root._m_down, _tcut(node)));
    }
    return node;
}

/// @brief re-insert a node after an arbitrary priority change
/// @param node node to reposition in heap
/// @return     @c node
///
/// This cuts the node from the heap, effectively making it a singleton heap, and then
/// merges it again with the heap.
PairingHeapT::BaseNodeT*
PairingHeapT::_reinsert(
    BaseNodeT* node)
{
    assert(node && node->_m_prev);
    _dunk(&_m_root, _merge(_m_root._m_down, _ncut(node)));
    return node;
}

// -----------------------------------------------------------------------------------------------
// iterative serialization (destructive node enumeration)

/// @brief serialization funnel, serializes the tree to a sequence of nodes
/// @param pref reference to pointer holding the tip of a tree to squeeze
/// @return     next node in sequence or @c NULL if pref is also @c NULL
///
/// @note Once you started shredding a tree, the pointer holding the collection of nodes points
///       to something that is no longer a tree.  You @e must continue shredding until no more
///       nodes remain!
PairingHeapT::BaseNodeT*
PairingHeapT::_shred_pop(
    BaseNodeT * &pref)
{
    // The trick for _efficient_ destructive enumeration of nodes in a tree is quickly finding
    // a link where to store the children of a node that's ripped from the structure.
    // For a 2-way node, it could be rotation to one side until only one child remains, but
    // that's expensive.  Creating a tree of remaining nodes by grafting the right child to the
    // end of the right spine of the left child is a way to keep the total effort at O(N).
    //
    // That all works with 3-way nodes, too, but here we can (ab)use the parent/prev link to make
    // the effort O(1) actual per step!  Of course, this warps the structure into something that
    // violates _all_ structural assumptions of a 3-way tree, so once you started ripping, you
    // have to do it to the end!

    BaseNodeT *retv{ pref };
    if (nullptr != retv) {
        pref = retv->_m_prev;
        for (BaseNodeT *hold : { retv->_m_down, retv->_m_next }) {
            if (nullptr != hold) {
                hold->_m_prev = pref;
                pref = hold;
            }
        }
    }
    return retv;
}

// -------------------------------------------------------------------------------------------
// iteration support

/// @brief helper to find the first right-to-left postorder child(!) of a node
/// @param node
/// @return first iteration child or @c node if there are no children
///
/// Since this abseiling along the right spine or the child if a node has no sibling... the
/// name is self-describing.
static PairingHeapT::BaseNodeT*
_abseil(const PairingHeapT::BaseNodeT* node)
{
    const PairingHeapT::BaseNodeT *next{ node ? node->_m_down : node};
    while (next) {
        node = next;
        next = node->_m_next ? node->_m_next : node->_m_down;
    }
    return const_cast<PairingHeapT::BaseNodeT *>(node);
}

/// @brief iterate forward -- next step for right-to-left post-order iteration
/// @param node current position
/// @return     next position or @c node for END
PairingHeapT::BaseNodeT*
PairingHeapT::_iter_succ(
    const BaseNodeT* node)
{
    const BaseNodeT *next;
    if (node && (next = node->_m_prev)) {
        node = ((node == next->_m_next) ? _abseil(next) : next);
    }
    return const_cast<BaseNodeT*>(node);
}

/// @brief iterate backward -- next step for left-to-right pre-order iteration
/// @param node current position
/// @return     next position
/// @throws @c std::out_of_range if predecessor does not exits
PairingHeapT::BaseNodeT*
PairingHeapT::_iter_pred(
    const BaseNodeT* node)
{
    if (node) {
        if (node->_m_down) {
            node = node->_m_down;
        } else if (node->_m_next) {
            node = node->_m_next;
        } else {
            const BaseNodeT* prev{ node->_m_prev };
            while (prev && (node == prev->_m_next || !prev->_m_next)) {
                prev = (node = prev)->_m_prev;
            }
            node = prev ? prev->_m_next : prev;
        }
    }
    if (!node) {
        throw std::out_of_range("--begin() decrement is undefined");
    }
    return const_cast<BaseNodeT*>(node);
}

/// @brief get begin position for left-to-right post-order iteration
/// @return the start node or @c NULL if the heap is empty
PairingHeapT::BaseNodeT*
PairingHeapT::_iter_head() const
{
    return const_cast<BaseNodeT *>(_abseil(&_m_root));
}

/// @brief get the last node for left-to-right post-order iteration
/// @return last node in sequence or @c NULL if heap is empty
/// @throws @c std::out_of_range if heap is empty
PairingHeapT::BaseNodeT*
PairingHeapT::_iter_tail() const
{
    const BaseNodeT* node{ _m_root._m_down };
    if (!node) {
        throw std::out_of_range("empty heap has no last node");
    }
    return const_cast<BaseNodeT*>(node);
}

/// @brief check node pointers for logic equality: all pointers to sentinels are equal.
/// @param p1   1st iterator pointer to check
/// @param p2   2nd iterator pointer to check
/// @return     @c true if both pointers consider to reach equal objects
bool
PairingHeapT::_iter_same(const BaseNodeT* p1, const BaseNodeT* p2)
{
    return (p1 == p2) || (!p1->_m_prev && !p2->_m_prev);
}
// --*-- that's all folks --*--
