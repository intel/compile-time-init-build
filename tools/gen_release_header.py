#!/usr/bin/env python3


import re
from pathlib import Path
import sys
import os


version = os.popen("git describe --tags").read().strip()
visited_includes = set()
root = Path(sys.argv[1]).parent.parent

# store content rather than emit directly
content_lines = []
system_headers = set()
insert_system_headers_line = None


def process(base_filepath):
    global version, visited_includes, root
    global content_lines, system_headers, insert_system_headers_line

    if base_filepath not in visited_includes:
        visited_includes.add(base_filepath)
        with open(base_filepath) as core:
            for line in core.readlines():
                m = re.match(r"\s*#include\s*<([a-zA-Z0-9_/. ]+)>", line)

                if m:
                    # capture first include as location for system include list
                    if not insert_system_headers_line:
                        insert_system_headers_line = len(content_lines)

                    sub_filepath = Path(m.group(1))
                    full_path = root / sub_filepath

                    if full_path.exists():
                        # recurse into a cib header
                        process(full_path)
                    else:
                        # otherwise keep the system header for later
                        system_headers.add(line)

                elif line.startswith("#pragma once"):
                    pass

                else:
                    line = re.sub("\.\.~~VERSION~~\.\.", version, line)
                    content_lines.append(line)


process(Path(sys.argv[1]))

# write out the content, when we get to the line number the location to emit
# system headers, write those out before proceeding with the remaining content
print("".join(content_lines[:insert_system_headers_line]))
print("".join(sorted(system_headers)))
print("".join(content_lines[insert_system_headers_line:]))
