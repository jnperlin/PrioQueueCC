// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with parent pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// Supports decrease-key and change-key efficiently
// -------------------------------------------------------------------------------------------
#ifndef PHQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33
#define PHQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33

#include <stdexcept>
#include <functional>

// -------------------------------------------------------------------------------------------
// definition of the core functions of a PairingHeap, meant for use in derived classes
// to handle the topological issues in one location without template code bloat.
// -------------------------------------------------------------------------------------------

class PairingHeapT
{
public:
    struct BaseNodeT {
        BaseNodeT *_m_prev { nullptr };
        BaseNodeT *_m_next { nullptr };
        BaseNodeT *_m_down { nullptr };
    };

    // we keep the order predicate a pure virtual function, type specific overload
    // in template specializations built upon this base class
    virtual bool  _pred(const BaseNodeT &n1, const BaseNodeT &n2) const = 0;

    BaseNodeT* _push(BaseNodeT* node);
    BaseNodeT* _pop();
    BaseNodeT* _merge(BaseNodeT *h1, BaseNodeT *h2) const; // merge (absorb) h2 into h1
    BaseNodeT* _build(BaseNodeT *h) const;                 // pairing phase -- make heap from list
    BaseNodeT* _ncut(BaseNodeT* h);                        // cut the node 'h' from heap
    BaseNodeT* _tcut(BaseNodeT* h);                        // cut branch (subtree) rooted at h from heap

    BaseNodeT* _yield();                                   // cut the whole tree from the sentinel
    BaseNodeT* _decrease(BaseNodeT* h);                    // re-insert for strictly decreasing key of 'h'
    BaseNodeT* _reinsert(BaseNodeT* h);                    // adjust for arbitrary key change of 'h'

    static BaseNodeT* _cons(BaseNodeT* a, BaseNodeT* b);   // connect b as successor ('next') of a
    static BaseNodeT* _dunk(BaseNodeT* a, BaseNodeT* b);   // connect b as child ('down') of a
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
// template class for a typed PairingHeap, derived from the basic heap class.  Supports iteration
// and the value type, the compare predicate and the allocator can be specified.
//
// Since moving nodes assumes the same allocator and since merge requires the same order criterion
// both are specified as classes and cannot be substituted by lambdas.
// -----------------------------------------------------------------------------------------------

template<
    typename _Type,
    typename _Comp = std::less<_Type>,
    typename Alloc = std::allocator<_Type> >
class PairingHeap : protected PairingHeapT
{
    // --- allocator guard ---
    static_assert(std::allocator_traits<Alloc>::is_always_equal::value,
         "LeftistHeap requires an allocator with is_always_equal == true");

    // --- comparator guard ---
    static_assert(std::is_empty<_Comp>::value,
        "LeftistHeap merge, move, or assignment require a stateless comparator");

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
        friend PairingHeap;

        BaseNodeT *_m_ipos{ nullptr };

        iterator(BaseNodeT* ipos) : _m_ipos{ ipos } { /*NOP*/ }
    };

    iterator begin() { return { _iter_head() }; }
    iterator end()   { return { &_m_root     }; }

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
        friend PairingHeap;

        BaseNodeT const *_m_ipos{ nullptr };

        const_iterator(BaseNodeT const *ipos): _m_ipos{ ipos } { /*NOP*/ }
    };

    const_iterator begin() const { return { _iter_head() }; }
    const_iterator end()   const { return { &_m_root     }; }

    PairingHeap() { /*NOP*/ }

    PairingHeap(PairingHeap&& rhs) {
        _dunk(&_m_root, rhs._yield());
    }
    PairingHeap(const PairingHeap & rhs) = delete;

    ~PairingHeap() {
        _clear(_yield());
    }

    PairingHeap& operator=(PairingHeap&& rhs) {
        if (this != &rhs) {
            _clear(_yield());
            _dunk(&_m_root, rhs._yield());
        }
        return *this;
    }
    PairingHeap& operator=(const PairingHeap &) = delete;

    PairingHeap& merge(PairingHeap& rhs) {
        if (this != &rhs)
            _dunk(&_m_root, _merge(_yield(), rhs._yield()));
        return *this;
    }

    void clear() {
        _clear(_yield());
    }

    iterator push(const _Type &  rhs) {  return { _push(_create_node(rhs           ))}; }
    iterator push(      _Type && rhs) {  return { _push(_create_node(std::move(rhs)))}; }

    template<typename... Args>
    iterator emplace(Args&&... args) { return { _push(_create_node(std::forward<Args>(args)...)) }; }

    _Type &front() const {
        if (nullptr == _m_root._m_down) {
            throw std::invalid_argument("empty");
        }
        return static_cast<_XNode*>(_m_root._m_down)->_m_value;
    }

    void pop() {
        _destroy_node(_pop());
    }

    bool empty() const {
        return nullptr == _m_root._m_down;
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

    using PairingHeapT::validate_tree;
};

#endif // PHQUEUE3_9687E0DD_D406_474B_9534_94B7C1D81D33