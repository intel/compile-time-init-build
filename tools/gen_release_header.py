#!/usr/bin/env python3


import re
from pathlib import Path
import sys
import os


version = os.popen("git describe --tags").read().strip()
visited_includes = set()

def process(base_filepath):
    if base_filepath not in visited_includes:
        visited_includes.add(base_filepath)
        with open(base_filepath) as core:
            for line in core.readlines():
                m = re.match(r"\s*#include\s*\"([a-zA-Z0-9_/. ]+)\"", line)

                if m:
                    sub_filepath = Path(m.group(1))
                    process(base_filepath.parent / sub_filepath)
                else:
                    line = re.sub("\.\.~~VERSION~~\.\.", version, line)
                    print(line, end="")



process(Path(sys.argv[1]))