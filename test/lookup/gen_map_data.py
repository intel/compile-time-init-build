import random

print("""
#pragma once
      
#include <array>
#include <cstdint>
#include <utility>          
""")

def gen_table(name, size, t, f):
    keys = set()
    
    # ensure there are 'size' entries and no duplicates
    while len(keys) < size:
        keys.add(f())
    
    # build a mapping of keys to values such that the entire set
    # can be visted by repeatedly looking up the result
    keys = list(keys)
    values = keys[1:] + keys[:1]

    print(f"constexpr auto {name} = std::array<std::pair<{t}, {t}>, {size}>{{{{")
    for k, v in zip(keys, values):
        print(f"    {{0x{k:08x}u, 0x{v:08x}u}},")
    print("}};")

def gen_exp_uint32_table(scale):
    for x in range(1, 10):
        count = x * scale
        gen_table(f"exp_{count}_uint32", count, "uint32_t", lambda: int(random.expovariate(10) * (1 << 28)))

gen_exp_uint32_table(1)
gen_exp_uint32_table(10)
gen_exp_uint32_table(100)
gen_exp_uint32_table(1000)


def gen_exp_uint16_table(scale):
    for x in range(1, 10):
        count = x * scale
        gen_table(f"exp_{count}_uint16", count, "uint16_t", lambda: int(random.expovariate(10) * (1 << 14)) & 0xffff)


gen_exp_uint16_table(1)
gen_exp_uint16_table(10)
gen_exp_uint16_table(100)
gen_exp_uint16_table(1000)
