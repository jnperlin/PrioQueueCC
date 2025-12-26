# -------------------------------------------------------------------------------------------
# pointer map hastable size generator
# -------------------------------------------------------------------------------------------
# #is file is part of "PrioQueueCC" by J.Perlinger.
#
# PrioQueueCC by J.Perlinger is marked CC0 1.0. To view a copy of this mark,
#    visit https://creativecommons.org/publicdomain/zero/1.0/
#
# -------------------------------------------------------------------------------------------
# A rough generator tor the table sizes used in the hash map for binary tree link pointer
# validation.
# -------------------------------------------------------------------------------------------

import math

GOLDEN = (1 + math.sqrt(5.0)) / 2.0

def coprime(x, seq):
    for q in seq:
        if q * q > x:
            break
        if x % q == 0:
            return False
    return True

primes = [3, 5, 7, 11, 13, 17]
for x in  range(19, 129, 2):
    if coprime(x, primes):
        primes.append(x)

print(primes)
triples = []
for p in range(11, 42):
    s = int(math.floor(math.pow(GOLDEN, p))) | 1
    d = -2
    while not coprime(s, primes):
        s += d
        d = -d + (2 if d < 0 else -2)
    t = (1 << 32) % s
    l = int(math.floor(2*s/3))
    triples.append((p, l, s, t))

print("""
struct HMapInfoT {
    std::uint32_t tcap; // capaciyt limit
    std::uint32_t tlen; // table size / multiplier
    std::uint32_t bias; // bias (actually de-bias correction helper)
};
static const HMapInfoT mapInfo[] = {
""")
for (p, l, s, t) in triples:
    print(f"    /* {p:2} */ {{ {l:10}, {s:10}, {t:10} }},")
print(f"}};\nstatic const size_t mapSize{{ {len(triples)} }};")
# --*-- that's all folks --*--
