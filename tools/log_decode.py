#!/usr/bin/env python3

import argparse
import ctypes
import itertools
import json
import mipi_messages as mipi


def file_reader(path, chunk_size=4096):
    with open(path, "rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            for b in chunk:
                yield b


msg_types = {
    1: mipi.Short32,
    3: mipi.Catalog,
    7: mipi.Short64,
}


def construct_msg(msg_type, first_byte, reader, messages, modules, db):
    if msg_type not in msg_types:
        raise Exception(f"Unknown message type: {msg_type}")
    seq = itertools.chain([first_byte], reader)
    return msg_types[msg_type](seq, messages, modules, db)


def read_logs(filename, db):
    binary_reader = file_reader(filename)
    messages = {msg["id"]: msg for msg in db["messages"]}
    modules = {m["id"]: m["string"] for m in db["modules"]}

    while True:
        try:
            first_byte = next(binary_reader)
            msg_type = first_byte & 0xF
            msg = construct_msg(
                msg_type, first_byte, binary_reader, messages, modules, db
            )
            yield f"{msg}"
        except StopIteration:
            break


def parse_cmdline():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        type=str,
        required=True,
        help=("Input filename: binary file of log output."),
    )
    parser.add_argument(
        "--output", type=str, help="Output filename for human-readable logs."
    )
    parser.add_argument(
        "--json",
        type=str,
        required=True,
        help="Filename for generated JSON collateral.",
    )
    return parser.parse_args()


def main():
    args = parse_cmdline()

    with open(args.json, "r") as f:
        db = json.load(f)
    for log_line in read_logs(args.input, db):
        print(log_line)


if __name__ == "__main__":
    main()
