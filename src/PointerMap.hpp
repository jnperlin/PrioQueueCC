// -------------------------------------------------------------------------------------------
// Fast hash set for pointer values
// -------------------------------------------------------------------------------------------
// This file is part of "PrioQueueCC" by J.Perlinger.
//
// PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
//    visit https://creativecommons.org/publicdomain/zero/1.0/
//
// -------------------------------------------------------------------------------------------
// With forward-only tree (Pairing / Leftist Heap with 2 pointers) detecting cross-linking
// is tricky.  The best we can do is checking whether a node has been seen before or not, and
// the pointer map is designed exactly for that purpose.
// -------------------------------------------------------------------------------------------
#ifndef POINTERMAP_9687E0DD_D406_474B_9534_94B7C1D81D33
#define POINTERMAP_9687E0DD_D406_474B_9534_94B7C1D81D33

#include <cstdint>
#include <utility>
#include <vector>

class PointerMapT {
public:
    PointerMapT(std::size_t N);

    bool insert(const void* p);
    bool lookup(const void* p) const;

    static std::uint32_t hash_ptr(const void* ptr);

    std::size_t capacity() const { return _tinfo->tlen; }
    std::size_t limit() const { return _tinfo->tcap; }
    std::size_t used() const { return _used; }

protected:
    struct HMapInfoT {
        std::uint32_t tcap; // capacit limit
        std::uint32_t tlen; // table size / multiplier
        std::uint32_t bias; // bias (actually debias correction helper)
    };

    static const HMapInfoT _mapInfo[];
    static const size_t    _mapSize;

    std::pair<std::size_t, std::size_t> _stepinfo(const void* p) const;
    void _rehash();

    std::vector<const void*> _table;
    HMapInfoT const         *_tinfo;
    std::size_t              _used{ 0 };
};

#endif // POINTERMAP_9687E0DD_D406_474B_9534_94B7C1D81D33
