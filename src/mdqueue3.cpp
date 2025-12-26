// -------------------------------------------------------------------------------------------
// Minimum-Leaf-Distance-balanced (MinDist) Heap with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Implementation of the core functions of a Leaf Distance balanced heap, meant for use in
// derived classes to handle the topological issues in one location.
// -------------------------------------------------------------------------------------------

#include <cassert>
#include "mdqueue3.hpp"

/// @file Leaf Distance balanced Heap with 3-way nodes

MinDistHeapT::BaseNodeT*
MinDistHeapT::_singleton(BaseNodeT* const node)
{
    if (node) {
        node->_m_lptr = node->_m_rptr = node->_m_pptr = nullptr;
        node->_m_dist = 1;
    }
    return node;
}

MinDistHeapT::BaseNodeT *MinDistHeapT::_yield()
{
    // cut the whole tree from the sentinel
    BaseNodeT *temp{nullptr};
    std::swap(temp, _m_root._m_lptr);
    if (temp)
        temp->_m_pptr = nullptr;
    return temp;
}

// -------------------------------------------------------------------------------------------
// connecting nodes / primitives on node level

/// @brief make b the left child of a
/// @param a    parent node
/// @param b    new left child
/// @return     a if not @c NULL, else b
MinDistHeapT::BaseNodeT*
MinDistHeapT::_lgraft(
    BaseNodeT* a,
    BaseNodeT* b)
{
    if (a) a->_m_lptr = b;
    if (b) b->_m_pptr = a;
    return a ? a : b;
}

/// @brief make b the right child of a
/// @param a    parent node
/// @param b    new right child
/// @return     a if not @c NULL, else b
MinDistHeapT::BaseNodeT*
MinDistHeapT::_rgraft(
    BaseNodeT* a,
    BaseNodeT* b)
{
    if (a) a->_m_rptr = b;
    if (b) b->_m_pptr = a;
    return a ? a : b;
}

/// @brief unidirectional CONS operation via parent pointer
/// @param a    new head of the list
/// @param b    tail to attach to node
/// @return     a if not @c NULL, else b
MinDistHeapT::BaseNodeT*
MinDistHeapT::_pcons(
    BaseNodeT* a,
    BaseNodeT* b)
{
    if (a) a->_m_pptr = b;
    return a ? a : b;
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
MinDistHeapT::BaseNodeT*
MinDistHeapT::_ncut(
    BaseNodeT * const node)
{
    assert(node && node->_m_pptr);    // automagically breaks on sentinel!
    BaseNodeT* root{ node->_m_pptr };
    if (node == root->_m_lptr) {
        _merge(root, &root->_m_lptr, node->_m_lptr, node->_m_rptr);
    } else {
        _merge(root, &root->_m_rptr, node->_m_lptr, node->_m_rptr);
    }
    node->_m_lptr = node->_m_rptr = node->_m_pptr = nullptr;
    return node;
}

/// @brief cut a subtree from the heap
/// @param node subtree root
/// @return @c node, but cleanly cut (next/prev are @c nullptr)
MinDistHeapT::BaseNodeT*
MinDistHeapT::_tcut(
    BaseNodeT* const node)
{
    assert(node && node->_m_pptr);    // automagically breaks on sentinel!
    BaseNodeT* root{ node->_m_pptr };

    // !Note! Why do we call merge with two empty heaps here? Well, it not only sets a NULL leaf,
    // but it also updates the perent leaf distances. A slight form of abuse, but convenient.
    if (node == root->_m_lptr) {
        _merge(root, &root->_m_lptr, nullptr, nullptr);
    } else {
        _merge(root, &root->_m_lptr, nullptr, nullptr);
    }
    node->_m_pptr = nullptr;
    return node;
}

// -------------------------------------------------------------------------------------------
// core functions -- this makes a binary tree a LDB Heap

void
MinDistHeapT::_merge(
    BaseNodeT *root,
    BaseNodeT **link,
    BaseNodeT  *h1,
    BaseNodeT  *h2) const
{
    int steps{ 1 };

    // Phase I: merge trees until at most one is surviving
    while (h1 && h2) {
        ++steps;
        if (_pred(*h2, *h1)) {
            (*link = h2)->_m_pptr = root;
            root = h2;
            if (!root->_m_lptr || (root->_m_rptr && root->_m_rptr->_m_dist > root->_m_lptr->_m_dist))
                link = &root->_m_lptr;
            else
                link = &root->_m_rptr;
            h2 = *link;
        } else {
            (*link = h1)->_m_pptr = root;
            root = h1;
            if (!root->_m_lptr || (root->_m_rptr && root->_m_rptr->_m_dist > root->_m_lptr->_m_dist))
                link = &root->_m_lptr;
            else
                link = &root->_m_rptr;
            h1 = *link;
        }
    }

    // Phase II: connect the survivor.
    // Unless we entered this with two empty heaps, we have exactly one survivor here. Make sure
    // the survivor is has a proper back/parent link.
    if ((*link = (h1 ? h1 : h2)))
        (*link)->_m_pptr = root;

    // Phase III: update the leaf distances.
    // We have to do AT LEAST as many steps as we had merging steps in Phase I.  If we continue
    // after that, we do so until the node weight does not change any more.
    //
    // In a min-dist heap (and the leftist heap as a special form of that), each node stores
    // the minimum null-path length of its children, which is logarithmically bounded by the
    // subtree size. Any local structural modification can only affect this value while it
    // remains below that bound.  Therefore, any upward propagation of distance updates,
    // whether due to increase or decrease, terminates after at most O(log N) steps.

    while (root) {
        int lcw{ root->_m_lptr ? root->_m_lptr->_m_dist : 0 };
        int rcw{ root->_m_rptr ? root->_m_rptr->_m_dist : 0 };
        int nnw{ std::min(lcw, rcw) + 1 };
        if ((--steps < 0) && (nnw == root->_m_dist))
            break;
        root->_m_dist = nnw;
        root = root->_m_pptr;
    }
}

/// @brief build a heap from a sibling list
/// @param node start of list
/// @return     root of created heap
MinDistHeapT::BaseNodeT*
MinDistHeapT::_build(
    BaseNodeT *head) const
{
    BaseNodeT *h1, *h2;
    while ((h1 = head) && (h2 = head->_m_pptr)) {
        BaseNodeT *list{ nullptr };
        do {
            BaseNodeT *hold;
            head = h2->_m_pptr;
            _merge(nullptr, &hold, h1, h2);
            hold->_m_pptr = list;
            list = hold;
        } while ((h1 = head) && (h2 = head->_m_pptr));
        if (head)
            head->_m_pptr = list;
        else
            head = list;
    }
    return head;
}

/// @brief push a node into the heap
/// @param node node to insert
/// @return @c node
MinDistHeapT::BaseNodeT*
MinDistHeapT::_push(
    BaseNodeT* node)
{
    _merge(&_m_root, &_m_root._m_lptr, _m_root._m_lptr, _singleton(node));
    return node;
}

/// @brief push a node list into the heap
/// @param node node to insert
/// @return @c node
void
MinDistHeapT::_push_list(
    BaseNodeT* head)
{
    _merge(&_m_root, &_m_root._m_lptr, _m_root._m_lptr, _build(head));
}


/// @brief pop the root element
/// @return the old root or @c NULL on empty heap
MinDistHeapT::BaseNodeT*
MinDistHeapT::_pop()
{
    BaseNodeT *retv{ _m_root._m_lptr };
    if (nullptr != retv) {
        _merge(&_m_root, &_m_root._m_lptr, retv->_m_lptr, retv->_m_rptr);
        retv->_m_lptr = retv->_m_rptr = retv->_m_pptr = nullptr;
        retv->_m_dist = 0;
    }
    return retv;
}

/// @brief handle a decrease in the node's priority
/// @param node node to reposition in heap
/// @return     @c node
///
/// This an actual O(1) operation, as cutting a subtree from any position is O(1), and so
/// is the following merge of the subtree with the remaining heap.  As decreasing the the
/// node's weight does _not_ invalidate the subtree rooted at @c node, we can prune and
/// graft the whole subtree here.  ( @c _reinsert() is more complicated, as we cannot
/// assume the heap invariant between the node and its children is preserved.)
MinDistHeapT::BaseNodeT*
MinDistHeapT::_decrease(
    BaseNodeT* node)
{
    assert(node && node->_m_pptr);
    if (node != _m_root._m_lptr) {
        _merge(&_m_root, &_m_root._m_lptr, _m_root._m_lptr, _tcut(node));
    }
    return node;
}

/// @brief re-insert a node after an arbitrary priority change
/// @param node node to reposition in heap
/// @return     @c node
///
/// This cuts the node from the heap, effectively making it a singleton heap, and then
/// merges it again with the heap.
MinDistHeapT::BaseNodeT*
MinDistHeapT::_reinsert(
    BaseNodeT* node)
{
    assert(node && node->_m_pptr);
    _merge(&_m_root, &_m_root._m_lptr, _m_root._m_lptr, _tcut(node));
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
MinDistHeapT::BaseNodeT*
MinDistHeapT::_shred_pop(
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
        pref = retv->_m_pptr;
        for (BaseNodeT *hold : { retv->_m_lptr, retv->_m_rptr }) {
            if (nullptr != hold) {
                hold->_m_pptr = pref;
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
/// Since this abseiling along the right spine or the left child if a node has no right
// child... the name is self-describing.
static MinDistHeapT::BaseNodeT*
_abseil(const MinDistHeapT::BaseNodeT* node)
{
    const MinDistHeapT::BaseNodeT *next{ node ? node->_m_lptr : node};
    while (next) {
        node = next;
        next = node->_m_rptr ? node->_m_rptr : node->_m_lptr;
    }
    return const_cast<MinDistHeapT::BaseNodeT *>(node);
}


/// @brief iterate forward -- next step for right-to-left post-order iteration
/// @param node current position
/// @return     next position or @c nullptr for END
MinDistHeapT::BaseNodeT*
MinDistHeapT::_iter_succ(
    const BaseNodeT* node)
{
    const BaseNodeT *next;
    if (node && (next = node->_m_pptr)) {
        node = ((node == next->_m_rptr) ? _abseil(next) : next);
    }
    return const_cast<BaseNodeT *>(node);

}

/// @brief iterate backward -- next step for left-to-right pre-order iteration
/// @param node current position
/// @return     next position
/// @throws @c std::out_of_range if predecessor does not exits
MinDistHeapT::BaseNodeT*
MinDistHeapT::_iter_pred(
    const BaseNodeT* node)
{
    if (node) {
        if (node->_m_lptr) {
            node = node->_m_lptr;
        } else if (node->_m_rptr) {
            node = node->_m_rptr;
        } else {
            const BaseNodeT *prev{node->_m_pptr};
            while (prev && (node == prev->_m_rptr || !prev->_m_rptr)) {
                prev = (node = prev)->_m_pptr;
            }
            node = prev ? prev->_m_rptr : prev;
        }
    }
    if (!node) {
        throw std::out_of_range("--begin() decrement is undefined");
    }
    return const_cast<BaseNodeT *>(node);
}

/// @brief get begin position for left-to-right post-order iteration
/// @return the start node or @c NULL if the heap is empty
MinDistHeapT::BaseNodeT*
MinDistHeapT::_iter_head() const
{
    return const_cast<BaseNodeT *>(_abseil(&_m_root));
}

/// @brief get the last node for left-to-right post-order iteration
/// @return last node in sequence or @c NULL if heap is empty
/// @throws @c std::out_of_range if heap is empty
MinDistHeapT::BaseNodeT*
MinDistHeapT::_iter_tail() const
{
    const BaseNodeT* node{ _m_root._m_lptr };
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
MinDistHeapT::_iter_same(const BaseNodeT* p1, const BaseNodeT* p2)
{
    return (p1 == p2) || (!p1->_m_pptr && !p2->_m_pptr);
}
// --*-- that's all folks --*--
