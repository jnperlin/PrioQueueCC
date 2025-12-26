// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Tree validation code
// -------------------------------------------------------------------------------------------
#include "phqueue3.hpp"
#include <stdexcept>

#define ASSERT(x) do { if (!(x)) throw std::logic_error( #x ); } while(false)

void
PairingHeapT::validate_tree() const
{
    BaseNodeT const                *node;
    std::vector<BaseNodeT const *> stack;

    if (nullptr != (node = _m_root._m_down)) {
        // root node must not have a successor or a predecessor!
        ASSERT((&_m_root == node->_m_prev) && (nullptr == node->_m_next));
        stack.push_back(node);
    }

    while ( ! stack.empty()) {
        BaseNodeT const *node{ stack.back()  };
        BaseNodeT const *chld{ node->_m_down };

        if (nullptr == (stack.back() = node->_m_next)) {
            stack.pop_back();
        }
        if (nullptr != chld) {
            ASSERT(node == chld->_m_prev);
            stack.push_back(chld);

            do {
                // check heap invariant: no child goes before the parent according to the order.
                ASSERT( ! _pred(*chld, *node));

                // check sibling link: If there is one, it must link back to this node.
                ASSERT((nullptr == chld->_m_next) || (chld == chld->_m_next->_m_prev));
            } while (nullptr != (chld = chld->_m_next));
        }
    }
}
// --*-- that's all folks --*--
