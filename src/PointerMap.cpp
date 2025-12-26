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
#include "PointerMap.hpp"
#include <stdexcept>
#include <limits>
#include <algorithm>

// The table is constructed according to a few rules:
//
// First, the probing steps we calculate are in the range [1,128], so the table size must be
// co-prime to to all primes <= 128.  (This ensures that all probing steps *WILL* visit the
// whole table!)
//
// Second, the table size should grow roughly exponentially, but not too fast:  We take
// approximately GOLDEN**N as starting point and use the closest number that satisfies the
// co-primality constraint.
//
// (Using the golden ratio, (1 + sqrt(5)) / 2 )

const PointerMapT::HMapInfoT PointerMapT::_mapInfo[] = {
    /* 11 */ {        132,        199,         46 },
    /* 12 */ {        211,        317,        232 },
    /* 13 */ {        347,        521,        117 },
    /* 14 */ {        559,        839,        446 },
    /* 15 */ {        911,       1367,        932 },
    /* 16 */ {       1471,       2207,       1841 },
    /* 17 */ {       2380,       3571,        611 },
    /* 18 */ {       3852,       5779,       2938 },
    /* 19 */ {       6232,       9349,       8649 },
    /* 20 */ {      10087,      15131,       2684 },
    /* 21 */ {      16315,      24473,       4742 },
    /* 22 */ {      26400,      39601,       1240 },
    /* 23 */ {      42719,      64079,       8242 },
    /* 24 */ {      69120,     103681,      85552 },
    /* 25 */ {     111839,     167759,       1378 },
    /* 26 */ {     180960,     271441,     227794 },
    /* 27 */ {     292804,     439207,     401250 },
    /* 28 */ {     473760,     710641,     563733 },
    /* 29 */ {     766568,    1149853,     266341 },
// beyond this point, the table size become more academical than practical:
    /* 30 */ {    1240327,    1860491,     954068 },
    /* 31 */ {    2006899,    3010349,    2209622 },
    /* 32 */ {    3247231,    4870847,    3751089 },
    /* 33 */ {    5254131,    7881197,    7596128 },
    /* 34 */ {    8501360,   12752041,   10281520 },
    /* 35 */ {   13755491,   20633237,    3254000 },
    /* 36 */ {   22256852,   33385279,   21651584 },
    /* 37 */ {   36012347,   54018521,   27504137 },
    /* 38 */ {   58269200,   87403801,   12181047 },
    /* 39 */ {   94281552,  141422329,   52297426 },
    /* 40 */ {  152550748,  228826123,  176097082 },
    /* 41 */ {  246832300,  370248451,  222234335 },
  {0, 0, 0 } // sentinel, not counted!
};
const size_t PointerMapT::_mapSize{ sizeof(_mapInfo) / sizeof(*_mapInfo) - 1 };

PointerMapT::PointerMapT(size_t n)
  : _table()
  , _tinfo(std::lower_bound(_mapInfo, _mapInfo + _mapSize, n,
        [](const HMapInfoT& i, std::uint32_t s){ return i.tcap < s;}) )
{
    if (_tinfo == _mapInfo + _mapSize) {
        throw std::range_error("table size");
    }
    _table.resize(_tinfo->tlen, nullptr);
}

/// @brief bit-twiddler based on the Jenkins OAT finaliser
/// @param ptr  pointer to mix up
/// @return     hash value for pointer itself
std::uint32_t
PointerMapT::hash_ptr(const void* ptr)
{
    static unsigned constexpr pshift {
        (std::numeric_limits<std::uintptr_t>::digits > 32) ?  32 : 0 };

    std::uintptr_t key = reinterpret_cast<std::uintptr_t>(ptr);
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    // We have to reduce to 32bit hash values on 64bit machines.  This can be
    // done with truncation or folding -- we opt for folding here.
    if (pshift) {
        key ^= (key >> pshift);
    }
    return static_cast<std::uint32_t>(key);
}

bool
PointerMapT::insert(const void* p)
{
    // TODO: guard against overstuffing...
    auto [hash, step]{ _stepinfo(p) };
    while (nullptr != _table[hash] && p != _table[hash])
        if ((hash += step) >= _table.size())
            hash -= _table.size();
    bool retv = (nullptr == _table[hash]);
    if (retv) {
        _table[hash] = p;
        if (++_used > _tinfo->tcap)
            _rehash();
    }
    return retv;
}

bool
PointerMapT::lookup(const void* p) const
{
    auto [hash, step]{ _stepinfo(p) };
    while (nullptr != _table[hash] && p != _table[hash])
        if ((hash += step) >= _table.size())
            hash -= _table.size();
    return nullptr != _table[hash];
}

void
PointerMapT::_rehash()
{
    ++_tinfo;
    if (0 == _tinfo->tlen) {
        --_tinfo;
        throw std::overflow_error("cannot rehash");
    }
    std::vector<void const*> holder(_tinfo->tlen, nullptr);
    std::swap(_table, holder);
    _used = 0;
    for (auto p : holder)
        (!p) || insert(p);
}

std::pair<size_t, size_t>
PointerMapT::_stepinfo(const void* p) const
{
    std::uint32_t phash{ hash_ptr(p) };
    std::uint32_t step{ (phash & 127) + 1 };
    std::uint32_t hash{ (std::uint32_t)((std::uint64_t(phash) * _tinfo->tlen + _tinfo->bias) >> 32) };
    if (hash >= _table.size()) {
        throw std::logic_error("hash index mapping");
    }
    return { hash, step };
}
// --*-- that's all folks --*--
