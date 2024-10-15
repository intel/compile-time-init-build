from hypothesis import strategies as st, given, settings, event, assume
import subprocess

def unpack(l):
    return ", ".join([str(i) for i in l])

uint16s = st.integers(min_value = 0, max_value = (1 << 16) - 1)


def lookup_make(lookup_type, key_type, value_type, default, entries):
    entries = [f"lookup::entry<{key_type}, {value_type}>{{{k}, {v}}}" for k, v in entries]
    return f"""
        {lookup_type}::make(CX_VALUE(lookup::input<{key_type}, {value_type}, {len(entries)}>{{
            {default},
            std::array<lookup::entry<{key_type}, {value_type}>, {len(entries)}>{{ {unpack(entries)} }}
        }}))
    """

@st.composite
def lookup_inputs(draw, keys=uint16s, values=uint16s, default=uint16s, min_size=0, max_size=10):
    entries = draw(st.lists(st.tuples(keys, values), min_size=min_size, max_size=max_size, unique_by=lambda x: x[0]))
    default = draw(default)
    return (default, entries)

pseudo_pext_lookups = st.sampled_from([
    "lookup::pseudo_pext_lookup<>",
    "lookup::pseudo_pext_lookup<true, 1>",
    "lookup::pseudo_pext_lookup<true, 2>",
    "lookup::pseudo_pext_lookup<true, 3>",
    "lookup::pseudo_pext_lookup<true, 4>"
])

@settings(deadline=50000)
@given(
    pseudo_pext_lookups, 
    lookup_inputs(min_size=2, max_size=12), 
    st.lists(uint16s, min_size=10, max_size=1000, unique=True)
)
def test_lookup(compile, t, l, extras):
    default, entries = l

    lookup_model = {k: v for k, v in entries}
    lookup = lookup_make(t, "std::uint16_t", "std::uint16_t", default, entries)

    check_keys = set(lookup_model.keys()).union(set(extras))

    static_asserts = "\n".join(
        [f"static_assert(lookup[{k}] == {lookup_model.get(k, default)});" for k in check_keys] 
    )

    runtime_checks = " &&\n".join(
        [f"(lookup[{k}] == {lookup_model.get(k, default)})" for k in check_keys] 
    )

    out = compile(f"""
        #include <lookup/entry.hpp>
        #include <lookup/input.hpp>
        #include <lookup/lookup.hpp>

        #include <stdx/utility.hpp>

        #include <array>
        #include <cstdint>

        int main() {{
            [[maybe_unused]] constexpr auto lookup = {lookup};
            {static_asserts}

            bool const pass = {runtime_checks};

            return pass ? 0 : 1;
        }}
    """)

    result = subprocess.run([out], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert result.returncode == 0