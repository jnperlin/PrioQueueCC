// -------------------------------------------------------------------------------------------
// Minimum-Leaf-Distance-balanced (MinDist) Heap with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// This is a Leftist Heap gone symmetric:  While in a Leftist heap the right child of a node
// has no greater leaf distance than the left child, we do not maintain this restriction.
// We just make sure that if a subtree has to be merged as a child of a node, we merge always
// with the 'lighter' side.  This is comes at only moderate additional complexity, but it
// results in much less distortion of iteration:  With a Leftist heap, any node may flip sides
// in its parent along the right spine, on any operation.
// -------------------------------------------------------------------------------------------
#ifndef LDQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33
#define LDQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33

#include <stdexcept>
#include <functional>

// -------------------------------------------------------------------------------------------
// definition of the core functions of a DistanceHeap, meant for use in derived classes
// to handle the topological issues in one location without template code bloat.
// -------------------------------------------------------------------------------------------

class MinDistHeapT
{
public:
    struct BaseNodeT {
        BaseNodeT *_m_lptr{ nullptr };  // left child pointer
        BaseNodeT *_m_rptr{ nullptr };  // right child pointer
        BaseNodeT *_m_pptr{ nullptr };  // parent / uplink
        short      _m_dist{ 1 };        // leaf distance
    };

    BaseNodeT* _singleton(BaseNodeT*);

    // we keep the order predicate a pure virtual function, type specific
    // overload in template specializations built upon this base class
    virtual bool  _pred(const BaseNodeT &n1, const BaseNodeT &n2) const = 0;


    void _push_list(BaseNodeT* head);
    void _merge(BaseNodeT* root, BaseNodeT**link, BaseNodeT *h1, BaseNodeT *h2) const;

    BaseNodeT* _push(BaseNodeT* node);
    BaseNodeT* _pop();
    BaseNodeT* _build(BaseNodeT* head) const;              // make raw heap from list
    BaseNodeT* _ncut(BaseNodeT* h);                        // cut the node 'h' from heap
    BaseNodeT* _tcut(BaseNodeT* h);                        // cut branch (subtree) rooted at h from heap

    BaseNodeT* _yield();                                  // cut the whole tree from the sentinel

    BaseNodeT* _decrease(BaseNodeT* h);                    // re-insert for strictly decreasing key of 'h'
    BaseNodeT* _reinsert(BaseNodeT* h);                    // adjust for arbitrary key change of 'h'
    void       _dist_set(BaseNodeT* h);

    static BaseNodeT* _lgraft(BaseNodeT* a, BaseNodeT* b); // connect b as left child of a
    static BaseNodeT* _rgraft(BaseNodeT* a, BaseNodeT* b); // connect b as right child of a
    static BaseNodeT* _pcons (BaseNodeT* a, BaseNodeT* b); // build chain via parent pointer
    static BaseNodeT* _shred_pop(BaseNodeT*& pref);        // destructive(!) node enumeration

    // -------------------------------------------------------------------------------------------
    // iteration support
    static BaseNodeT* _iter_succ(const BaseNodeT* n);    // L->R->N (left to right postorder) traversal
    static BaseNodeT* _iter_pred(const BaseNodeT* n);    // N->R->L (right to left preorder) traversal

    BaseNodeT* _iter_head() const;                       // 'begin()' node for L->R->N traversal
    BaseNodeT* _iter_tail() const;                       // last node for L->R->N traversal

    static bool _iter_same(const BaseNodeT* p1, const BaseNodeT* np2);

    void validate_tree() const;

    BaseNodeT  _m_root { nullptr };                        // the root holder & end sentinel
};

// -----------------------------------------------------------------------------------------------
// template class for a typed MinDistHeap, derived from the basic heap class.  Supports iteration
// and the value type, the compare predicate and the allocator can be specified.
//
// Since moving nodes assumes the same allocator and since merge requires the same order criterion
// both are specified as classes and cannot be substituted by lambdas.
// -----------------------------------------------------------------------------------------------

template<
    typename _Type,
    typename _Comp = std::less<_Type>,
    typename Alloc = std::allocator<_Type> >
class MinDistHeap : protected MinDistHeapT
{
    // --- allocator guard ---
    static_assert(std::allocator_traits<Alloc>::is_always_equal::value,
         "DistanceHeap requires an allocator with is_always_equal == true");

    // --- comparator guard ---
    static_assert(std::is_empty<_Comp>::value,
        "DistanceHeap merge, move, or assignment require a stateless comparator");

protected:

    struct _XNode : public BaseNodeT {
        _Type _m_value;

        _XNode(const _Type &  rhs) : _m_value{ rhs            } { /*NOP*/ }
        _XNode(      _Type && rhs) : _m_value{ std::move(rhs) } { /*NOP*/ }
    };

    using value_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<_Type>;
    using node_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<_XNode>;
    using node_alloc_traits = std::allocator_traits<node_allocator_type>;

    template<typename... Args>
    BaseNodeT* _create_node(Args&&... args) {
        _XNode* p = node_alloc_traits::allocate(_m_alloc, 1);
        try {
            node_alloc_traits::construct(_m_alloc, p, std::forward<Args>(args)...);
        } catch (...) {
            node_alloc_traits::deallocate(_m_alloc, p, 1);
            throw;
        }
        return p;
    }

    void _destroy_node(BaseNodeT* n) {
        if (n) {
            _XNode* p = static_cast<_XNode*>(n);
            node_alloc_traits::destroy(_m_alloc, p);
            node_alloc_traits::deallocate(_m_alloc, p, 1);
        }
    }

    bool _pred(const BaseNodeT &n1, const BaseNodeT &n2) const override {
        const _Type & rn1{ static_cast<const _XNode&>(n1)._m_value };
        const _Type & rn2{ static_cast<const _XNode&>(n2)._m_value };
        return _Comp()(rn1, rn2);
    }

    void _clear(BaseNodeT *root) {
        while (nullptr != root) {
            _destroy_node(_shred_pop(root));
        }
    }

    node_allocator_type _m_alloc;

  public:

    struct iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = _Type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = _Type*;
        using reference         = _Type&;

        iterator() = default;

        reference operator*()  const { return  static_cast<_XNode*>(_m_ipos)->_m_value; }
        pointer   operator->() const { return &static_cast<_XNode*>(_m_ipos)->_m_value; }

        iterator& operator++()    { _m_ipos = _iter_succ(_m_ipos); return *this; }
        iterator  operator++(int) { return { _iter_succ(_m_ipos) }; }
        iterator& operator--()    { _m_ipos = _iter_pred(_m_ipos); return *this;     }
        iterator  operator--(int) { return { _iter_pred(_m_ipos) }; }

        friend bool operator==(const iterator& i1, const iterator& i2) { return  _iter_same(i1._m_ipos, i2._m_ipos); }
        friend bool operator!=(const iterator& i1, const iterator& i2) { return !_iter_same(i1._m_ipos, i2._m_ipos); }

    protected:
        friend MinDistHeap;

        BaseNodeT *_m_ipos{ nullptr };

        iterator(BaseNodeT* ipos) : _m_ipos{ ipos } { /*NOP*/ }
    };

    iterator begin() { return { _iter_head() }; }
    iterator end()   { return { &_m_root       }; }

    struct const_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = const _Type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const _Type*;
        using reference         = const _Type&;

        const_iterator() = default;
        const_iterator(const iterator &it): _m_ipos{ it._m_ipos } { /*NOP*/ }

        reference operator*()  const { return  static_cast<const _XNode*>(_m_ipos)->_m_value; }
        pointer   operator->() const { return &static_cast<const _XNode*>(_m_ipos)->_m_value; }

        const_iterator& operator++()    { _m_ipos = _iter_succ(_m_ipos); return *this; }
        const_iterator  operator++(int) { return { _iter_succ(_m_ipos) }; }
        const_iterator& operator--()    { _m_ipos = _iter_pred(_m_ipos); return *this; }
        const_iterator  operator--(int) { return { _iter_pred(_m_ipos) }; }

        friend bool operator==(const const_iterator& i1, const const_iterator& i2) { return  _iter_same(i1._m_ipos, i2._m_ipos); }
        friend bool operator!=(const const_iterator& i1, const const_iterator& i2) { return !_iter_same(i1._m_ipos, i2._m_ipos); }

    protected:
        friend MinDistHeap;

        BaseNodeT const *_m_ipos{ nullptr };

        const_iterator(BaseNodeT const *ipos): _m_ipos{ ipos } { /*NOP*/ }
    };

    const_iterator begin() const { return { _iter_head() }; }
    const_iterator end()   const { return { &_m_root       }; }

    MinDistHeap() { /*NOP*/ }

    MinDistHeap(MinDistHeap&& rhs) {
        _lgraft(&_m_root, rhs._yield());
    }
    MinDistHeap(const MinDistHeap & rhs) = delete;

    ~MinDistHeap() {
        _clear(_yield());
    }

    MinDistHeap& operator=(MinDistHeap&& rhs) {
        if (this != &rhs) {
            _clear(_yield());
            _lgraft(&_m_root, rhs._yield());
        }
        return *this;
    }
    MinDistHeap& operator=(const MinDistHeap &) = delete;

    MinDistHeap& merge(MinDistHeap& rhs) {
        if (this != &rhs)
            _merge(&_m_root, &_m_root._m_lptr, _m_root._m_lptr, rhs._yield());
        return *this;
    }

    void clear() {
        _clear(_yield());
    }

    iterator push(const _Type &  rhs) {  return { _push(_create_node(rhs           ))}; }
    iterator push(      _Type && rhs) {  return { _push(_create_node(std::move(rhs)))}; }

    template <typename It>
    void push(It first, It last) {
        using category = typename std::iterator_traits<It>::iterator_category;
        static_assert(std::is_base_of<std::forward_iterator_tag, category>::value, "push() requires a forward iterator");
        BaseNodeT *head{ nullptr }, *node;
        for (; first != last; ++first) {
            head = _pcons(_create_node(*first), head);
        }
        _push_list(head);
    }

    template <typename Range>
    auto push(Range&& r) -> decltype(std::begin(r), std::end(r), void()) {
        using std::begin;
        using std::end;
        push(begin(r), end(r));
    }

    template<typename... Args>
    iterator emplace(Args&&... args) { return { _push(_create_node(std::forward<Args>(args)...)) }; }

    _Type &front() const {
        if (nullptr == _m_root._m_lptr) {
            throw std::invalid_argument("empty");
        }
        return static_cast<_XNode*>(_m_root._m_lptr)->_m_value;
    }

    void pop() {
        _destroy_node(_pop());
    }

    bool empty() const {
        return nullptr == _m_root._m_lptr;
    }

    /// @brief remove the node the iterator references
    /// @param itpos node to remove
    /// @return iterator to successor of @c itpos
    /// @note This invalidates all other iterators to the same position and distorts all other
    ///       active iterators for this heap!
    iterator remove(const iterator &itpos) {
        BaseNodeT*succ{ _iter_succ(itpos._m_ipos) };
        _destroy_node(_ncut(itpos._m_ipos));
        return { succ };
    }

    /// @brief quickly restore heap invariants after key/prio at @c *itpos was reduced
    /// @param itpos    node that should go closer to the root
    /// @return         @c itpos for convenience
    /// @note This will distort all active iterators for this heap!
    iterator decrease(const iterator &itpos) {
        return { _decrease(itpos._m_ipos) };
    }

    /// @brief fully restore heap invariants after key/prio at @c *itpos was changed
    /// @param itpos    node that should be re-evaluated for position in heap
    /// @return         @c itpos for convenience
    /// @note This will distort all active iterators for this heap!
    iterator readjust(const iterator &itpos) {
        return { _reinsert(itpos._m_ipos) };
    }

    using MinDistHeapT::validate_tree;
};

#endif // LDQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33