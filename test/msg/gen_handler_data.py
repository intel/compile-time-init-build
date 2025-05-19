import random


big_vals = [int(random.expovariate(10) * (1 << 28)) & 0xFFFFFFFF for i in range(0, 100)]
med_vals = [int(random.expovariate(10) * (1 << 14)) & 0xFFFF for i in range(0, 50)]
small_vals = [int(random.expovariate(10) * (1 << 6)) & 0xFF for i in range(0, 25)]

combos = [
    (random.choice(big_vals), random.choice(med_vals), random.choice(small_vals))
    for i in range(0, 256)
]


print(
    """
template<typename T>
struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<T>, 
        cib::extend<T>("""
)
for c in combos:
    print(f"            cb<{c[0]}, {c[1]}, {c[2]}>,")
print(
    """        )
    );
};
"""
)

print(
    """
    auto msgs = std::array{"""
)
for c in combos:
    print(f"        m<{c[0]}, {c[1]}, {c[2]}>,")
print(
    """    };
"""
)
