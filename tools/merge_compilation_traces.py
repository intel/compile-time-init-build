#!/usr/bin/env python3
"""Merge multiple clang ``-ftime-trace`` JSON files into a single trace.

Each ``-ftime-trace`` output is a Chrome Trace Event file describing one
translation unit as a single process (one ``pid``) with several ``tid`` lanes
(the ``Total *`` summary rows). To view several translation units in one trace
we place each input under its own ``pid`` so their timelines appear as separate,
independently-labelled process lanes in chrome://tracing, Perfetto or speedscope.

Usage::

    merge_compilation_traces.py -o compilation_trace.json \
        LABEL1=path/to/first.json LABEL2=path/to/second.json

The ``LABEL=`` prefix is optional; when omitted the file stem is used to name
the lane.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def _load(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        data = json.load(f)
    if "traceEvents" not in data:
        raise ValueError(f"{path}: not a Chrome trace file (no 'traceEvents')")
    return data


def _relabel(data: dict, pid: int, label: str) -> list[dict]:
    """Return this trace's events remapped onto a single, named ``pid`` lane."""
    events: list[dict] = []
    saw_process_name = False
    for event in data["traceEvents"]:
        event = dict(event)
        event["pid"] = pid
        if event.get("ph") == "M" and event.get("name") == "process_name":
            # Name the lane after the translation unit instead of "clang".
            event["args"] = {"name": label}
            saw_process_name = True
        events.append(event)
    if not saw_process_name:
        events.append(
            {
                "cat": "",
                "pid": pid,
                "tid": pid,
                "ts": 0,
                "ph": "M",
                "name": "process_name",
                "args": {"name": label},
            }
        )
    return events


def merge(inputs: list[tuple[str, Path]]) -> dict:
    merged_events: list[dict] = []
    beginning_of_time: int | None = None
    for index, (label, path) in enumerate(inputs, start=1):
        data = _load(path)
        merged_events.extend(_relabel(data, index, label))
        bot = data.get("beginningOfTime")
        if bot is not None:
            beginning_of_time = bot if beginning_of_time is None else min(beginning_of_time, bot)

    result: dict = {"traceEvents": merged_events}
    if beginning_of_time is not None:
        result["beginningOfTime"] = beginning_of_time
    return result


def _parse_input(spec: str) -> tuple[str, Path]:
    label, sep, raw_path = spec.partition("=")
    if not sep:
        raw_path = spec
        label = Path(spec).stem
    path = Path(raw_path)
    if not path.is_file():
        raise FileNotFoundError(f"trace file not found: {path}")
    return label, path


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-o",
        "--output",
        required=True,
        type=Path,
        help="path of the combined trace file to write",
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        metavar="[LABEL=]TRACE.json",
        help="one or more -ftime-trace JSON files, optionally LABEL=prefixed",
    )
    args = parser.parse_args(argv)

    inputs = [_parse_input(spec) for spec in args.inputs]
    merged = merge(inputs)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8") as f:
        json.dump(merged, f)

    print(
        f"merged {len(inputs)} trace(s) -> {args.output} "
        f"({len(merged['traceEvents'])} events)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
