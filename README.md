# PrioQueueCC
Min-Distance and Pairing Heaps in C++

```MD
 PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
    visit https://creativecommons.org/publicdomain/zero/1.0/
```

## Overview

This provides source code for an implementation of the following priority queues:

 + Leftist Heap _without_ decrease-key or iteration (2-way nodes)
 + Min-Dist Heap _with_ decrease-key and iteration
 + Pairing Heap _without_ decrease-key or iteration (2-way nodes)
 + Pairing Heap _with_ decrease-key and iteration

> Note: To support decrease-key (or change-key), three pointers per node are required
  in any heap based on a binary tree. For least-N or top-K use cases,
  a simpler 2-pointer heap may suffice.

---
### Leftist Heap

- Textbook implementation.
- Recursive merge, recursion depth limited by **O(log N)**.
- All operations have **actual O(log N) bounds**.
- Batch construction from N items is supported in **O(N) time with constant auxiliary space**.

---

### Min-Dist Heap

- Conceptually a *“Leftist Heap gone symmetric”*.
- Iterative meld operation only moves nodes vertically; it never swaps sides.
- Ensures forward iteration never skips nodes, even when deleting nodes during iteration.
- All operations, including decrease-key and change-key, have **actual O(log N) bounds**.
- Batch construction from N items is supported in **O(N) time with constant auxiliary space**.

---

### Pairing Heap

- Implemented using **child-list head and sibling pointers**, essentially a binary tree rotated 45°.
- Operations use **simple pointer manipulations**, without auxiliary arrays.
- The 3-way node variant supports decrease-key/change-key with textbook cost bounds.
- Iterators provide **O(1) amortized step cost**; traversal of the entire heap is O(N).
- Pairing Heaps already provide O(1) insert, so a batch inserter is purely cosmetic.

---

### General Design

- Uses an untyped base class for pointer logic and template-derived classes for type safety, allocators, and predicates.
- Not a “zero-cost abstraction”, but balances genericity and specialization.
- 3-way node heaps include a **root sentinel**, simplifying core operations and supporting `--container.end()`.
- For meld operations:
  - Allocators must be equal across heaps.
  - The order predicate must be context-free to guarantee correctness.

### Examples

The unit test code should give a very good idea hwo to use these priority queues.  As for a quick teaser,
have a look at this:

```cpp
#include "phqueue3.h"
#include <vector>
#include <iostream>

int main() {
    PairingHeap<int> heap;
    std::vector<int> values{1, 3, 5, 2, 4, 6};

    for (int v : values)
        heap.push(v);

    // Iterate forward
    for (auto it = heap.begin(); it != heap.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // Pop all elements
    while (!heap.empty()) {
        std::cout << heap.front() << " ";
        heap.pop();
    }
    std::cout << "\n";
}
```

---
## Heap Iteration (TL;DR)

Priority queues have very weak structural constraints, and very loose ordering
restrictions.  The only thing iteration can guarantee here is that all nodes will be
visited exactly _once_.  That's it -- the enumeration order is totally arbitrary.
Internally we will do right-to-left post-order traversal.  This is handy for deletion
while iterating, because with some care the structure above the cutting point will
_not_ change, and that means that the iteration successor remains the valid point
to continue iteration.

The iterators are bidirectional: `--it` is possible, even from the end position,
with one caveat: `--Q.end()` will fail for an empty queue, and `--Q.begin()`
will _always_ fail. A `std::out_of_range` exception will be thrown in these cases.


### An excursion on Pairing Heaps

Most textbooks describe Pairing Heaps in terms of sibling lists, where every
node may have a sibling and/or a pointer to sibling list of children.  This is useful
for formalizing the heap procedures and do the cost analysis -- but it totally
obscures the fact that a Pairing Heap is a binary tree!  Typically it is drawn
with the siblings going horizontally, while the children list link is drawn vertically
downwards.

Now take the picture, rotate it by 45° clockwise, and what you see is... a binary tree,
the down/child pointer becoming the left child pointer, and the sibling link becoming
the right child pointer.

While this is _not_ the way Pairing heaps are normally depicted, it explains very much
why iteration over a Pairing Heap with back-links and iteration over a Min-Dist Heap
with parent pointers are structurally _exactly_ the same.

### Iteration Stepping Mechanics

As said before, the 'forward' direction is the node order retrieved by right-to-left
post-order traversal.  That is easy to imagine for the Min-Dist Heap, as it is by
definition a binary tree.  To make it work for the Pairing Heap, too, one has start
thinking firmly that a Pairing Heap is also binary tree by _implementation_.
If you cannot get a firm grasp on that idea, the reverse stepping becomes a nightmare
until you recognize that

 + the reverse of right-to-left post-order is left-to-right pre-order
 + to implement back-stepping actually needs to to revert all moves done in
   a forward iteration step.
 + a Pairing Heap is also a binary tree

The main difference between stepping forward and stepping backward is that in this
implementation `++Q.end()` is well-defined and idempotent, while `--Q.begin()`
will throw `std::out_of_range`.  And for an empty heap, where `Q.begin()` equals
`Q.end()`, `--Q.end()` will also throw.

It should also be noted that reverse iteration is _not_ supported.  As the node order
for forward iteration is arbitrary, so would be the node order for complete reverse
iteration.  So there's not much use for it, and it could not be implemented in a way
that would support deletion under iteration and maintaining full reachability of
all remaining nodes.  It's probably better not to attempt what cannot be done
consistently and without nasty surprises.

### Iterator Invalidation

Deleting a node from a heap obviously _invalidates_ all iterators pointing to this
node -- any use of such an iterator is UB.  Avoiding this would require a form of
garbage collection, either be explicit reference counting or other GC techniques.
Either was deemed too expensive, so the usual caveats known from node-based STL
containers apply here, too.

And as usual in node-based containers, any structural change may _disturb_ all iterators
for the container. (Disturbing means that the completeness of the iteration cannot
be guaranteed any more.)  While for BSTs with in-order traversal more detailed conditions
can be established, this is not possible for priority heaps.  So while iterators do
not get _invalid_ by insertion, and by deletion only if pointing to the node to be deleted,
they become pretty useless for _iteration_ purposes.

They remain valid references, though, and can be used for remove/decrease-key/change-key
or general access operations.
