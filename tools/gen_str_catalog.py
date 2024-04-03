#!/usr/bin/env python3

import argparse
import itertools
import json
import re
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
    "sc::message<\(logging::level\)(\d+), sc::undefined<sc::args<(.*)>, char, (.*)>\s*>"
)


def extract_string_id(line_m):
    levels = ["MAX", "FATAL", "ERROR", "WARN", "INFO", "USER1", "USER2", "TRACE"]

    catalog_type = line_m.group(1)
    string_m = string_re.match(line_m.group(3))
    log_level = string_m.group(1)
    arg_tuple = string_m.group(2)
    string_tuple = string_m.group(3).replace("(char)", "")
    string_value = "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)))
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


module_re = re.compile("sc::module_string<sc::undefined<void, char, (.*)>\s?>")


def extract_module_id(line_m):
    return module_re.match(line_m.group(3)).group(1)


def extract(line_num, line_m):
    try:
        return (
            extract_string_id(line_m)
            if line_m.group(2) == "catalog"
            else extract_module_id(line_m)
        )
    except Exception:
        raise Exception(
            f"Couldn't extract catalog info at line {line_num} ({line_m.group(0)})"
        )


def stable_msg_key(msg):
    return hash(msg["level"]) ^ hash(msg["msg"]) ^ hash("".join(msg["arg_types"]))


def read_input(filenames, stable_ids):
    line_re = re.compile("^.+?(unsigned int (catalog|module)<(.+?)>\(\))$")

    def read_file(filename):
        with open(filename, "r") as f:
            matching_lines = filter(
                lambda p: p[1] is not None,
                ((num + 1, line_re.match(line.strip())) for num, line in enumerate(f)),
            )
            return [extract(*m) for m in matching_lines]

    messages = list(itertools.chain.from_iterable(read_file(f) for f in filenames))
    strings = filter(lambda x: not isinstance(x, str), messages)
    modules = filter(lambda x: isinstance(x, str), messages)

    old_ids = set(stable_ids.values())
    id_gen = itertools.filterfalse(old_ids.__contains__, itertools.count(0))

    def get_id(msg):
        key = stable_msg_key(msg)
        if key in stable_ids:
            return stable_ids[key]
        else:
            return next(id_gen)

    unique_strings = {i[0][0]: i for i in strings}.values()
    return (
        set(modules),
        {item[0]: {**item[1], "id": get_id(item[1])} for item in unique_strings},
    )


def make_cpp_catalog_defn(types, msg):
    catalog_type, arg_tuple = types
    return f"""/*
    "{msg["msg"]}"
    {arg_tuple}
 */
template<> {catalog_type} {{
    return {msg["id"]};
}}"""


def module_string(module):
    string_tuple = module.replace("(char)", "")
    return "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)))


def make_cpp_module_defn(n, module):
    return f"""/*
    "{module_string(module)}"
 */
template<> unsigned int module<sc::module_string<sc::undefined<void, char, {module}>>>() {{
    return {n};
}}"""


def write_cpp(messages, modules, extra_headers, filename):
    with open(filename, "w") as f:
        f.write("\n".join(f'#include "{h}"' for h in extra_headers))
        f.write("\n#include <log/catalog/catalog.hpp>\n\n")
        cpp_catalog_defns = (make_cpp_catalog_defn(k, v) for k, v in messages.items())
        f.write("\n".join(cpp_catalog_defns))
        f.write("\n\n")
        cpp_module_defns = (make_cpp_module_defn(n, m) for n, m in enumerate(modules))
        f.write("\n".join(cpp_module_defns))


def extract_enums(filename):
    from clang.cindex import CursorKind, Index

    def walk_up(node):
        if node.semantic_parent.kind == CursorKind.TRANSLATION_UNIT:
            yield node
        else:
            yield from (node, *walk_up(node.semantic_parent))

    def fully_qualified(node):
        return reversed(list(walk_up(node)))

    def fq_name(node):
        return "::".join(n.spelling for n in fully_qualified(node))

    index = Index.create()
    translation_unit = index.parse(filename)

    enums = {}
    for node in translation_unit.cursor.walk_preorder():
        if node.kind == CursorKind.ENUM_DECL:
            enums.update(
                {
                    fq_name(node): {
                        e.spelling: e.enum_value
                        for e in node.walk_preorder()
                        if e.kind == CursorKind.ENUM_CONSTANT_DECL
                    }
                }
            )
    return enums


def write_json(messages, extra_inputs, filename):
    str_catalog = dict(messages=list(messages.values()))
    for extra in extra_inputs:
        with open(extra, "r") as f:
            str_catalog.update(json.load(f))
    with open(filename, "w") as f:
        json.dump(str_catalog, f, indent=4)


def read_stable(stable_filenames):
    stable_catalog = dict()
    for filename in stable_filenames:
        with open(filename, "r") as f:
            stable_catalog.update(json.load(f))
    return {stable_msg_key(msg): msg["id"] for msg in stable_catalog["messages"]}


def serialize_guids(client_node, guid_id, guid_mask):
    syst_guids = et.SubElement(client_node, "syst:Guids")
    et.SubElement(
        syst_guids,
        "syst:Guid",
        attrib={"ID": f"{{{guid_id}}}", "Mask": f"{{{guid_mask}}}"},
    )


def serialize_enums(client_node, enums):
    syst_enums = et.SubElement(client_node, "syst:EnumDefinition")
    for enum_name, (i, values) in enums.items():
        syst_enum = et.SubElement(
            syst_enums, "syst:Enum", attrib={"Name": enum_name, "ID": f"{i}"}
        )
        for name, value in values.items():
            et.SubElement(
                syst_enum, "syst:EnumEntry", attrib={"Value": f"{value}", "Name": name}
            )


def serialize_modules(client_node, modules):
    syst_modules = et.SubElement(client_node, "syst:Modules")
    for n, m in enumerate(modules):
        syst_module = et.SubElement(syst_modules, "syst:Module", attrib={"ID": f"{n}"})
        syst_module.text = f"<![CDATA[{module_string(m)}]]>"


def serialize_messages(short_node, catalog_node, messages):
    for msg in messages.values():
        syst_format = et.SubElement(
            short_node if msg["arg_count"] == 0 else catalog_node,
            "syst:Format",
            attrib={"ID": "0x%08X" % msg["id"], "Mask": "0x0FFFFFFF"},
        )
        if "enum_lookup" in msg:
            syst_format.set(
                "EnumLookup", f'{msg["enum_lookup"][0]}:{msg["enum_lookup"][1]}'
            )

        fmt_string = re.sub(r"{}", r"%d", msg["msg"])
        if msg["arg_count"] != 0:
            fmt_string = re.sub(r"{:(.*?)}", r"%\1", fmt_string)
        syst_format.text = f"<![CDATA[{fmt_string}]]>"


def write_xml(
    messages, modules, enums, filename, client_name, version, guid_id, guid_mask
):
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
    et.SubElement(syst_collateral, "syst:FwVersion", attrib={"FW_Version": version})

    serialize_guids(syst_client, guid_id, guid_mask)
    serialize_enums(syst_client, enums)
    serialize_modules(syst_client, modules)

    syst_short_msg = et.SubElement(syst_client, "syst:Short32")
    syst_catalog_msg = et.SubElement(syst_client, "syst:Catalog32")
    serialize_messages(syst_short_msg, syst_catalog_msg, messages)

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
    parser.add_argument(
        "--stable_json",
        type=str,
        nargs="*",
        default=[],
        help="Input filename(s) for previously generated JSON; this is used to fix stable IDs.",
    )
    return parser.parse_args()


def main():
    args = parse_cmdline()

    stable_ids = read_stable(args.stable_json)
    try:
        modules, messages = read_input(args.input, stable_ids)
    except Exception as e:
        raise Exception(f"{str(e)} from file {args.input}")

    if args.cpp_output is not None:
        write_cpp(messages, modules, args.cpp_headers, args.cpp_output)

    if args.json_output is not None:
        write_json(messages, args.json_input, args.json_output)

    if args.xml_output is not None:
        enums = {}
        if args.cpp_output is not None:
            try:
                enums = {
                    name: (i, value)
                    for i, (name, value) in enumerate(
                        extract_enums(args.cpp_output).items()
                    )
                }
                for k, v in messages.items():
                    for i, arg_type in enumerate(v["arg_types"]):
                        if arg_type in enums:
                            v.update({"enum_lookup": (i + 1, enums[arg_type][0])})
            except Exception as e:
                print(
                    f"Couldn't extract enum info ({str(e)}), enum lookup will not be available"
                )

        else:
            print("XML output without C++ output: enum lookup will not be available")

        write_xml(
            messages,
            modules,
            enums,
            args.xml_output,
            client_name=args.client_name,
            version=args.version,
            guid_id=args.guid_id,
            guid_mask=args.guid_mask,
        )


if __name__ == "__main__":
    main()
