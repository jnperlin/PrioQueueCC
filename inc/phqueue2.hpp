// -------------------------------------------------------------------------------------------
// Pairing Heap priority queue with forward-only pointers
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// A classic child/next only implementation: no iteration, no decrease-key support,
// only push, tip, pop.  This is sufficient for the least-N or top-K class of problems.
// -------------------------------------------------------------------------------------------
#ifndef PHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33
#define PHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33

#include <functional>
#include <stdexcept>

class PairingHeapEasyT {
public:
    struct PairingNodeT {
        PairingNodeT *_m_next { nullptr };
        PairingNodeT *_m_down { nullptr };
    };

    virtual bool  _pred(const PairingNodeT &n1, const PairingNodeT &n2) const = 0;

    void          _push(PairingNodeT *node);

    PairingNodeT *_pop();
    PairingNodeT *_merge(PairingNodeT *h1, PairingNodeT *h2) const;
    PairingNodeT *_build(PairingNodeT *h) const;

    static PairingNodeT* _cons(PairingNodeT* a, PairingNodeT* b);
    static PairingNodeT* _dunk(PairingNodeT* a, PairingNodeT* b);
    static PairingNodeT *_shred_pop(PairingNodeT * &pref);

    void   validate_tree(size_t nodes) const;

    PairingNodeT *_m_root { nullptr };
};

template<typename _Type,
         typename _Comp = std::less<_Type>,
         typename Alloc = std::allocator<_Type> >
class PairingHeapEasy : protected PairingHeapEasyT
{
    // --- allocator guard ---
    static_assert(std::allocator_traits<Alloc>::is_always_equal::value,
         "LeftistHeap requires an allocator with is_always_equal == true");

    // --- comparator guard ---
    static_assert(std::is_empty<_Comp>::value,
        "LeftistHeap merge, move, or assignment require a stateless comparator");

protected:
    struct _XNode : public PairingNodeT {
        _Type _m_value;

        _XNode(const _Type &  rhs) : _m_value{ rhs            } { /*NOP*/ }
        _XNode(      _Type && rhs) : _m_value{ std::move(rhs) } { /*NOP*/ }
    };

    using value_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<_Type>;
    using node_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<_XNode>;
    using node_alloc_traits = std::allocator_traits<node_allocator_type>;

    template<typename... Args>
    PairingNodeT* _create_node(Args&&... args) {
        _XNode* p = node_alloc_traits::allocate(_m_alloc, 1);
        try {
            node_alloc_traits::construct(_m_alloc, p, std::forward<Args>(args)...);
        } catch (...) {
            node_alloc_traits::deallocate(_m_alloc, p, 1);
            throw;
        }
        return p;
    }

    void _destroy_node(PairingNodeT* n) {
      if (n) {
        _XNode* p = static_cast<_XNode*>(n);
        node_alloc_traits::destroy(_m_alloc, p);
        node_alloc_traits::deallocate(_m_alloc, p, 1);
      }
    }

    bool _pred(const PairingNodeT &n1, const PairingNodeT &n2) const override {
        const _Type & rn1{ static_cast<const _XNode&>(n1)._m_value };
        const _Type & rn2{ static_cast<const _XNode&>(n2)._m_value };
        return _Comp()(rn1, rn2);
    }

    void _clear(PairingNodeT *root) {
        while (nullptr != root) {
            _destroy_node(_shred_pop(root));
        }
    }

    node_allocator_type _m_alloc;

  public:

    PairingHeapEasy()
    { /*NOP*/ }

    PairingHeapEasy(PairingHeapEasy&& rhs) {
        std::swap(_m_root, rhs._m_root);
    }
    PairingHeapEasy(const PairingHeapEasy & rhs) = delete;

    ~PairingHeapEasy() {
        clear();
    }

    PairingHeapEasy& operator=(PairingHeapEasy&& rhs) {
        PairingNodeT *hold { _m_root };
        _m_root = rhs._m_root;
        rhs._m_root = nullptr;
        _clear(hold);
        return *this;
    }
    PairingHeapEasy& operator=(const PairingHeapEasy &) = delete;

    PairingHeapEasy& merge(PairingHeapEasy& rhs) {
        PairingNodeT *hold{ nullptr };
        std::swap(hold, rhs._m_root);
        _m_root = _merge(_m_root, hold);
        return *this;
    }

    void clear() {
        PairingNodeT *hold{ nullptr };
        std::swap(hold, _m_root);
        _clear(hold);
    }

    void push(const _Type &  rhs) {  _push(_create_node(rhs           )); }
    void push(      _Type && rhs) {  _push(_create_node(std::move(rhs))); }

    _Type &front() const
    {
        if (nullptr == _m_root) {
            throw std::invalid_argument("empty");
        }
        return static_cast<_XNode*>(_m_root)->_m_value;
    }

    void pop()
    {
        PairingNodeT *ptr { _pop() };
        if (nullptr != ptr) {
            _destroy_node(ptr);
        }
    }

    bool empty() const {
        return nullptr == _m_root;
    }

    using PairingHeapEasyT::validate_tree;
};

#endif // PHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33
