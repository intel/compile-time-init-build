#!/usr/bin/env python3

import argparse
import itertools
import json
import re
import sys
import xml.etree.ElementTree as et


# https://stackoverflow.com/questions/174890/how-to-output-cdata-using-elementtree
def _escape_cdata(text):
    try:
        if "&" in text:
            text = text.replace("&", "&amp;")
        return text
    except TypeError:
        raise TypeError("cannot serialize %r (type %s)" % (text, type(text).__name__))


et._escape_cdata = _escape_cdata


def find_arg_split_pos(s, start):
    angle_count = 0
    for i in range(start, len(s)):
        if s[i] == ">":
            angle_count -= 1
        elif s[i] == "<":
            angle_count += 1
        elif s[i] == ",":
            if angle_count == 0:
                return i
    return len(s)


def split_args(s):
    args = []
    start = 0
    while start < len(s):
        pos = find_arg_split_pos(s, start)
        args.append(s[start:pos].strip())
        start = pos + 1
    return args


string_re = re.compile(
    "message<\(logging::level\)(\d+), sc::undefined<sc::args<(.*)>, char, (.*)>\s*>"
)


def extract_message(line_num, catalog_m):
    levels = ["MAX", "FATAL", "ERROR", "WARN", "INFO", "USER1", "USER2", "TRACE"]

    try:
        catalog_type = catalog_m.group(1)
        string_m = string_re.match(catalog_m.group(2))

        log_level = string_m.group(1)
        arg_tuple = string_m.group(2)
        string_tuple = string_m.group(3).replace("(char)", "")
        string_value = "".join(
            (chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple))
        )
        args = split_args(arg_tuple)

        return (
            (catalog_type, arg_tuple),
            dict(
                level=levels[int(log_level)],
                msg=string_value,
                type="flow" if string_value.startswith("flow.") else "msg",
                arg_types=args,
                arg_count=len(args),
            ),
        )

    except Exception:
        raise Exception(
            f"Couldn't extract catalog info at line {line_num} ({catalog_m.group(0)})"
        )


def read_input(filenames):
    catalog_re = re.compile("^.+?(unsigned int catalog<(.+?)>\(\))$")

    def read_file(filename):
        with open(filename, "r") as f:
            matching_lines = filter(
                lambda p: p[1] is not None,
                (
                    (num + 1, catalog_re.match(line.strip()))
                    for num, line in enumerate(f)
                ),
            )
            return [extract_message(*m) for m in matching_lines]

    messages = itertools.chain.from_iterable(read_file(f) for f in filenames)
    unique_messages = {i[0][0]: i for i in messages}.values()
    return {item[0]: {**item[1], "id": i} for i, item in enumerate(unique_messages)}


def make_cpp_defn(types, msg):
    catalog_type, arg_tuple = types
    return f"""/*
    "{msg["msg"]}"
    {arg_tuple}
 */
template<> {catalog_type} {{
    return {msg["id"]};
}}"""


def write_cpp(messages, extra_headers, filename):
    with open(filename, "w") as f:
        f.write("\n".join(f'#include "{h}"' for h in extra_headers))
        f.write("\n#include <log/catalog/catalog.hpp>\n\n")
        cpp_defns = (make_cpp_defn(k, v) for k, v in messages.items())
        f.write("\n".join(cpp_defns))


def write_json(messages, extra_inputs, filename):
    str_catalog = dict(messages=list(messages.values()))
    for extra in extra_inputs:
        with open(extra, "r") as f:
            str_catalog.update(json.load(f))
    with open(filename, "w") as f:
        json.dump(str_catalog, f, indent=4)


def write_xml(messages, filename, client_name, version, guid_id, guid_mask):
    syst_collateral = et.Element(
        "syst:Collateral",
        attrib={
            "xmlns:syst": "http://www.mipi.org/1.0/sys-t",
            "xmlns:xsi": "http://www.w3.org/2001/XMLSchema-instance",
            "xsi:schemaLocation": "http://www.mipi.org/1.0/sys-t https://www.mipi.org/schema/sys-t/sys-t_1-0.xsd",
        },
    )

    syst_client = et.SubElement(
        syst_collateral, "syst:Client", attrib={"Name": client_name}
    )

    syst_fwversion = et.SubElement(
        syst_collateral, "syst:FwVersion", attrib={"FW_Version": version}
    )

    syst_guids = et.SubElement(syst_client, "syst:Guids")
    syst_guid = et.SubElement(
        syst_guids,
        "syst:Guid",
        attrib={"ID": f"{{{guid_id}}}", "Mask": f"{{{guid_mask}}}"},
    )

    syst_short_msg = et.SubElement(syst_client, "syst:Short32")
    syst_catalog_msg = et.SubElement(syst_client, "syst:Catalog32")

    for msg in messages.values():
        syst_format = et.SubElement(
            syst_short_msg if msg["arg_count"] == 0 else syst_catalog_msg,
            "syst:Format",
            attrib={"ID": "0x%08X" % msg["id"], "Mask": "0x0FFFFFFF"},
        )
        fmt_string = re.sub(r"{}", r"%d", msg["msg"])
        if msg["arg_count"] != 0:
            fmt_string = re.sub(r"{:(.*?)}", r"%\1", fmt_string)
        syst_format.text = f"<![CDATA[{fmt_string}]]>"

    et.indent(syst_collateral, space="    ")
    xml_string = et.tostring(syst_collateral, encoding="utf8", method="xml")
    with open(filename, "wb") as xf:
        xf.write(xml_string)


def parse_cmdline():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        type=str,
        nargs="+",
        required=True,
        help=(
            "Input filename(s): file(s) of undefined symbols produced by running"
            "`nm -uC <archive>`."
        ),
    )
    parser.add_argument(
        "--cpp_output", type=str, help="Output filename for generated C++ code."
    )
    parser.add_argument(
        "--json_output", type=str, help="Output filename for generated JSON."
    )
    parser.add_argument(
        "--xml_output", type=str, help="Output filename for generated XML."
    )
    parser.add_argument(
        "--json_input",
        type=str,
        nargs="*",
        default=[],
        help="Extra JSON inputs to copy into the output.",
    )
    parser.add_argument(
        "--cpp_headers",
        type=str,
        nargs="*",
        default=[],
        help="Extra C++ headers to include in the C++ output.",
    )
    parser.add_argument(
        "--client_name",
        type=str,
        default="CIB Framework FW",
        help="Client name in the generated XML.",
    )
    parser.add_argument(
        "--version",
        type=str,
        default="VERSION",
        help="Version in the generated XML.",
    )
    parser.add_argument(
        "--guid_id",
        type=str,
        default="00000000-0017-0001-0000-000000000000",
        help="GUID ID in the generated XML.",
    )
    parser.add_argument(
        "--guid_mask",
        type=str,
        default="00000000-FFFF-FFFF-8000-000000000000",
        help="GUID mask in the generated XML.",
    )
    return parser.parse_args()


def main():
    args = parse_cmdline()
    try:
        messages = read_input(args.input)
    except Exception as e:
        raise Exception(f"{str(e)} from file {args.input}")

    if args.cpp_output:
        write_cpp(messages, args.cpp_headers, args.cpp_output)

    if args.json_output:
        write_json(messages, args.json_input, args.json_output)

    if args.xml_output:
        write_xml(
            messages,
            args.xml_output,
            client_name=args.client_name,
            version=args.version,
            guid_id=args.guid_id,
            guid_mask=args.guid_mask,
        )


if __name__ == "__main__":
    main()
