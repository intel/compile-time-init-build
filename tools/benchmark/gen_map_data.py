#!/usr/bin/env python3

import argparse
import random


def gen_table(size, t, fn, f):
    # ensure there are 'size' unique entries
    keys = set()
    while len(keys) < size:
        keys.add(fn())

    # build a mapping of keys to values such that the entire set
    # can be visited by repeatedly looking up the result
    keys = list(keys)
    values = keys[1:] + keys[:1]

    indent = " " * 4
    f.write(
        f"constexpr auto exp_{t}_{size} = std::array<std::pair<std::{t}_t, std::{t}_t>, {size}>{{{{\n{indent}"
    )
    f.write(
        f",\n{indent}".join(
            f"{{0x{k:08x}u, 0x{v:08x}u}}" for k, v, in zip(keys, values)
        )
    )
    f.write("\n}};\n")


def parse_cmdline():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--type",
        type=str,
        choices=["uint32", "uint16"],
        required=True,
        help=("Unsigned integral type of the input data."),
    )
    parser.add_argument(
        "--size",
        type=int,
        required=True,
        help=("Number of items in the input data (e.g. 500)."),
    )
    parser.add_argument(
        "--output",
        type=str,
        required=True,
        help="Output filename for generated C++ code.",
    )
    return parser.parse_args()


gen_fns = {
    "uint16": lambda: int(random.expovariate(10) * (1 << 14)) & 0xFFFF,
    "uint32": lambda: int(random.expovariate(10) * (1 << 28)),
}


def main():
    args = parse_cmdline()

    with open(args.output, "w") as f:
        f.write(
            """#pragma once

#include <array>
#include <cstdint>
#include <utility>

"""
        )
        gen_table(
            args.size,
            args.type,
            gen_fns[args.type],
            f,
        )


if __name__ == "__main__":
    main()
