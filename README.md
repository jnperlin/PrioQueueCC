# PrioQueueCC
Min-Distance and Pairing Heaps in C++


## Overview

This provides source code for an implementation the following priority queues:

 + Leftist Heap _without_ decrease-key or iteration (2-way nodes)
 + Min-Dist Heap _with_ decrease-key and iteration
 + Pairing Heap _without_ decrease-key or iteration (2-way nodes)
 + Pairing Heap _with_ decrease-key and iteration

To support decrease-key (and / or change-key), three pointers per node is the minimum
for a heap based on a binary tree.  OTOH, problems of the least-N or top-K type do
_not_ need decrease-key, and under constrained conditions the savings might be worth
the effort.

The Min-Dist Heap is "Leftist Heap gone symmetric". The meld operation is slightly more
complex than the Leftist Heap implementation, but it does never 'flip sides': Merging
moves nodes only vertically.  This is necessary to make iterators work, or to be more
precise, it is needed to make deleting nodes in the heap via an iterator never skip
nodes in the forward sequence.


## Pairing Heap

This is the implementation of a Pairing heap build upon child-list-head and sibling
pointer.  (If yo will, a typical binary tree, the picture rotated by 45° to the left,
and the children pointers renamed to 'down' and 'next' ...)

That means no arbitrary auxiliary memory requirements, all operations are simple
pointer operations.

In the 3-way node variant _with_ parent / previous pointer, decrease-key and change-key
are available with textbook cost bounds.  Iteration is provided at O(1) amortized costs
per step and O(N) for the heap in total.


## Min-Dist Heap

The 2-way node version is really a Leftist heap, while the 3-way node type with parent
pointers does not swivel the tree to make the right child the one with the shorter
leaf node distance.  The side where to continue a merge is carefully chosen instead,
resulting in a more stable shape of the tree.  With the 2-way Leftist Heap, that's
of no concern, but for an iterable structure permitting delete-key via an iterator
the continuous swivelling is a death stab at iteration completeness.

All operations have a strong cap at O(log N) _actual_, including decrease-key and
change-key.

There is one feature provided for the Min-Dist Heap not present in the Pairing Heap:
Construction of a heap from N items in O(N) time with constant auxiliary space.

(Note: For a pairing heap, the cost for inserting a single node is already O(1) actual.
There's no sense in providing a batch-inserter, apart from API completeness.)


## General Design

In contrast to STL containers, these queues come as an untyped / type-free base class
that implements all the pointer logic and derived template classes that handle the
missing parts like memory allocation via allocators, defining the order predicate,
providing iterators from raw node pointers and the like.

So this is _no_ "zero-cost abstraction", though that zero-cost comes at high costs
anyway, just in dark corners where shiny claims loose a lot of polish anyways.  It is
an implementation that tries to find a good middle-ground between base class genericity
and derivation specialization and type safety.

The 3-way node versions use a full node a root sentinel: Not is this necessary to
support ```--container.end()```, it also makes the core functions easier: Now _all_
nodes in the tree have a parent or predecessor, eliminating one case to handle
otherwise.

To make heaps meldable, the allocators in the two heaps must be all-instance-equal
or deleting the melded heap would become UB. The same holds for the order predicate:
It must be context-free, because this is the only way to guarantee two heaps being
meldable _at all_.


## Heap Iteration

Priority queues have very weak structural constraints, and very loose ordering
restrictions.  The only thing iteration can guarantee here is that all nodes will be
visited exactly _once_.  That's it -- the enumeration order is totally arbitrary.
Internally we will do right-to-left post-order traversal.  This is handy for deletion
while iterating, because with some care the structure above the cutting point will
_not_ change, and that means that the iteration successor remains the valid point
to continue iteration.

The iterators are even bidirectional: `--it` is possible, even from the end position,
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
implementation `++Q.end()` is well-defined and results in a NOP, while `--Q.begin()`
will throw `std::out_of_bounds`.  And for an empty heap, where `Q.begin()` equals
`Q.end()`, `--Q.end()` will also throw.

It should also be noted that reverse iteration is _not_ supported.  As the node order
for forward iteration is arbitrary, so would be the node order for complete reverse
iteration.  So there's not much use for it, and it could not be implemented in a way
that would support deletion under iteration and maintaining full reachability of
all remaining nodes.  So it's probably better not to attempt what cannot be done
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