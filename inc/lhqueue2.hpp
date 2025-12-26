// -------------------------------------------------------------------------------------------
// Leftist Heap priority queue
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// A classic children-links-only implementation: no iteration, no decrease-key support,
// only push or multi-push, tip, pop
// -------------------------------------------------------------------------------------------
#ifndef LHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33
#define LHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33

#include <memory>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>

class LeftistHeapEasyT
{
public:
    struct BaseNodeT {
        BaseNodeT *_m_lptr{ nullptr };  // left child pointer
        BaseNodeT *_m_rptr{ nullptr };  // right child pointer
        short      _m_dist{ 1 };        // leaf distance
    };

    virtual bool        _pred(const BaseNodeT &n1, const BaseNodeT &n2) const = 0;
    void                _push(BaseNodeT *node);
    void                _push_list(BaseNodeT *node);
    BaseNodeT          *_pop();
    BaseNodeT          *_merge(BaseNodeT *h1, BaseNodeT *h2) const;

    static BaseNodeT   *_shred_pop(BaseNodeT * &pref);
    static BaseNodeT   *_singleton(BaseNodeT *node);
    void                validate_tree(size_t nodes) const;

    static BaseNodeT   *_cons(BaseNodeT *node, BaseNodeT *tail) { return node ? ((node->_m_rptr = tail), node) : tail; }

    BaseNodeT *_m_root{ nullptr };
};

template<typename _Type,
         typename _Comp = std::less<_Type>,
         typename Alloc = std::allocator<_Type> >
class LeftistHeapEasy : protected LeftistHeapEasyT
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

    LeftistHeapEasy()
    { /*NOP*/ }

    LeftistHeapEasy(LeftistHeapEasy&& rhs) {
        std::swap(_m_root, rhs._m_root);
    }
    LeftistHeapEasy(const LeftistHeapEasy & rhs) = delete;

    ~LeftistHeapEasy() {
        clear();
    }

    LeftistHeapEasy& operator=(LeftistHeapEasy&& rhs) {
        BaseNodeT *hold { _m_root };
        _m_root = rhs._m_root;
        rhs._m_root = nullptr;
        _clear(hold);
        return *this;
    }
    LeftistHeapEasy& operator=(const LeftistHeapEasy &) = delete;

    LeftistHeapEasy& merge(LeftistHeapEasy& rhs) {
        BaseNodeT *hold{ nullptr };
        std::swap(hold, rhs._m_root);
        _m_root = _merge(_m_root, hold);
        return *this;
    }

    void clear() {
        BaseNodeT *hold{ nullptr };
        std::swap(hold, _m_root);
        _clear(hold);
    }

    void push(const _Type &  rhs) { _push(_create_node(rhs           )); }
    void push(      _Type && rhs) { _push(_create_node(std::move(rhs))); }

    template <typename It>
    void push(It first, It last) {
        using category = typename std::iterator_traits<It>::iterator_category;
        static_assert(std::is_base_of<std::forward_iterator_tag, category>::value, "push() requires a forward iterator");
        BaseNodeT *head{ nullptr }, *node;
        for (; first != last; ++first) {
            head = _cons(_create_node(*first), head);
        }
        _push_list(head);
    }

    template <typename Range>
    auto push(Range&& r) -> decltype(std::begin(r), std::end(r), void()) {
        using std::begin;
        using std::end;
        push(begin(r), end(r));
    }

    _Type &front() const {
        if (nullptr == _m_root) {
            throw std::invalid_argument("empty");
        }
        return static_cast<_XNode*>(_m_root)->_m_value;
    }

    void pop() {
        BaseNodeT *ptr { _pop() };
        if (nullptr != ptr) {
            _destroy_node(ptr);
        }
    }

    bool empty() const {
        return nullptr == _m_root;
    }

    using LeftistHeapEasyT::validate_tree;
};

#endif // LHQUEUE2_9687E0DD_D406_474B_9534_94B7C1D81D33
