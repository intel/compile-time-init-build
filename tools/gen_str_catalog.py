#!/usr/bin/env python3

import argparse
import itertools
import json
import re
import xml.etree.ElementTree as et
from functools import partial


def find_arg_split_pos(s: str, start: int) -> int:
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


def split_args(s: str) -> list[str]:
    args = []
    start = 0
    while start < len(s):
        pos = find_arg_split_pos(s, start)
        args.append(s[start:pos].strip())
        start = pos + 1
    return args


string_re = re.compile(
    r"sc::message<sc::undefined<sc::args<(.*)>, char, (.*)>\s*>"
)


def extract_string_id(line_m):
    catalog_type = line_m.group(1)
    string_m = string_re.match(line_m.group(3))
    arg_tuple = string_m.group(1)
    string_tuple = string_m.group(2).replace("(char)", "")
    string_value = "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)))
    args = split_args(arg_tuple)

    return (
        (catalog_type, arg_tuple),
        dict(
            msg=string_value,
            type="flow" if string_value.startswith("flow.") else "msg",
            arg_types=args,
            arg_count=len(args),
        ),
    )


module_re = re.compile(r"sc::module_string<sc::undefined<void, char, (.*)>\s?>")


def module_string(module: str) -> str:
    string_tuple = module.replace("(char)", "")
    return "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)))


def msg_string(msg: dict) -> str:
    return msg["msg"]


def extract_module_id(line_m):
    return module_re.match(line_m.group(3)).group(1)


def extract(line_num: int, line_m):
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


def stable_msg_key(msg: dict):
    return hash(msg["msg"]) ^ hash("".join(msg["arg_types"]))


def stable_module_key(module: str):
    return hash(module_string(module))


def typo_error(s: str, stable: str, i: int) -> str:
    raise Exception(f"Error: typo detected: \"{s}\" is similar to \"{stable}\"")


def typo_warn(s: str, stable: str, i: int) -> str:
    print(f"Warning: typo detected: \"{s}\" is similar to \"{stable}\"")
    return s


def typo_fix(s: str, stable: str, i: int) -> str:
    print(f"Warning: typo detected: \"{s}\" is similar to \"{stable}\". Fixing to ID {i}.")
    return stable


def typo_fix_quiet(s: str, stable: str, i: int) -> str:
    return stable


typo_behavior = {
    "error": typo_error,
    "warn": typo_warn,
    "fix": typo_fix,
    "fix_quiet": typo_fix_quiet
}


def handle_typo(stable_ids: dict, s: str, d: int, fn, gen) -> str:
    if d != 0:
        from Levenshtein import distance
        for (i, value) in stable_ids.values():
            if distance(s, value) <= d:
                if fn(s, value, i) == value:
                    return i
    return next(gen)


def read_input(filenames: list[str], stable_ids, typo_distance: int, typo_detect: str):
    line_re = re.compile(r"^.*(unsigned int (catalog|module)<(.+?)>\(\))$")

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

    def get_id(stable_ids, key_fn, string_fn, gen, obj):
        key = key_fn(obj)
        if key in stable_ids:
            return stable_ids[key][0]
        else:
            return handle_typo(stable_ids, string_fn(obj), typo_distance, typo_behavior[typo_detect], gen)

    stable_msg_ids, stable_module_ids = stable_ids

    old_msg_ids = set(stable_msg_ids.values())
    msg_id_gen = itertools.filterfalse(old_msg_ids.__contains__, itertools.count(0))
    get_msg_id = partial(get_id, stable_msg_ids, stable_msg_key, msg_string, msg_id_gen)

    old_module_ids = set(stable_module_ids.values())
    module_id_gen = itertools.filterfalse(
        old_module_ids.__contains__, itertools.count(0)
    )
    get_module_id = partial(get_id, stable_module_ids, stable_module_key, module_string, module_id_gen)

    unique_strings = {i[0][0]: i for i in strings}.values()
    return (
        {m: {"string": module_string(m), "id": get_module_id(m)} for m in sorted(set(modules))},
        {item[0]: {**item[1], "id": get_msg_id(item[1])} for item in unique_strings},
    )


def make_cpp_catalog_defn(types, msg) -> str:
    catalog_type, arg_tuple = types
    return f"""/*
    "{msg["msg"]}"
    {arg_tuple}
 */
template<> {catalog_type} {{
    return {msg["id"]};
}}"""


def make_cpp_module_defn(m: str, text: str, n: int) -> str:
    return f"""/*
    "{text}"
 */
template<> unsigned int module<sc::module_string<sc::undefined<void, char, {m}>>>() {{
    return {n};
}}"""


def write_cpp(messages, modules, extra_headers: list[str], filename: str):
    with open(filename, "w") as f:
        f.write("\n".join(f'#include "{h}"' for h in extra_headers))
        f.write("\n#include <log/catalog/catalog.hpp>\n\n")
        cpp_catalog_defns = (make_cpp_catalog_defn(k, v) for k, v in messages.items())
        f.write("\n".join(cpp_catalog_defns))
        f.write("\n\n")
        cpp_module_defns = (
            make_cpp_module_defn(k, m["string"], m["id"]) for k, m in modules.items()
        )
        f.write("\n".join(cpp_module_defns))


def extract_enums(filename: str):
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


def write_json(messages, modules, enums, extra_inputs: list[str], filename: str, stable_ids):
    d = dict(messages=list(messages.values()), modules=list(modules.values()))
    for msg in stable_ids.get("messages"):
        if not msg in d["messages"]:
            d["messages"].append(msg)
    for mod in stable_ids.get("modules"):
        if not mod in d["modules"]:
            d["modules"].append(mod)

    str_catalog = dict(**d, enums=dict())
    for extra in extra_inputs:
        with open(extra, "r") as f:
            str_catalog.update(json.load(f))

    es = dict()
    for (k, (_, v)) in enums.items():
        es.update({k: {value: name for (name, value) in v.items()}})
    for (k, v) in es.items():
        if k in str_catalog["enums"]:
            str_catalog["enums"][k].update(v) # type: ignore
        else:
            str_catalog["enums"].update({k: v}) # type: ignore

    with open(filename, "w") as f:
        json.dump(str_catalog, f, indent=4)


def read_stable(stable_filenames: list[str]):
    stable_catalog: dict[str, list] = dict(messages=[], modules=[])
    for filename in stable_filenames:
        with open(filename, "r") as f:
            stable_catalog.update(json.load(f))
    return stable_catalog


def serialize_guids(client_node: et.Element, guid_id: str, guid_mask: str):
    syst_guids = et.SubElement(client_node, "syst:Guids")
    et.SubElement(
        syst_guids,
        "syst:Guid",
        attrib={"ID": f"{{{guid_id}}}", "Mask": f"{{{guid_mask}}}"},
    )


def serialize_enums(client_node: et.Element, enums):
    syst_enums = et.SubElement(client_node, "syst:EnumDefinition")
    for enum_name, (i, values) in enums.items():
        syst_enum = et.SubElement(
            syst_enums, "syst:Enum", attrib={"Name": enum_name, "ID": f"{i}"}
        )
        for name, value in values.items():
            et.SubElement(
                syst_enum, "syst:EnumEntry", attrib={"Value": f"{value}", "Name": name}
            )


def serialize_modules(client_node: et.Element, modules):
    syst_modules = et.SubElement(client_node, "syst:Modules")
    for m in modules.values():
        syst_module = et.SubElement(
            syst_modules, "syst:Module", attrib={"ID": f"{m['id']}"}
        )
        syst_module.text = f"<![CDATA[{m['string']}]]>"


def arg_printf_spec(arg: str):
    printf_dict = {"encode_32": "%d", "encode_u32": "%u",
                   "encode_64": "%lld", "encode_u64": "%llu"}
    return printf_dict.get(arg, "%d")


def serialize_messages(short_node: et.Element, catalog_node: et.Element, messages):
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

        fmt_string = msg["msg"]
        for arg in msg["arg_types"]:
            m = re.search(r"{:([^}]+)}", fmt_string)
            if m:
                fmt_string = f"{fmt_string[:m.start()]}%{m.group(1)}{fmt_string[m.end():]}"
            else:
                fmt_string = re.sub(r"{}", arg_printf_spec(arg), fmt_string, count = 1)
        syst_format.text = f"<![CDATA[{fmt_string}]]>"


def write_xml(
    messages,
    modules,
    enums,
    filename: str,
    client_name: str,
    version: str,
    guid_id: str,
    guid_mask: str,
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


def check_module_limit(modules, max):
    for m in modules.values():
        if m["id"] > max:
            raise Exception(
                f"Module ({m}) assigned value exceeds the module ID max ({max})."
            )


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
    parser.add_argument(
        "--forget_old_ids",
        action="store_true",
        help="When on, stable IDs from a previous run are forgotten. By default, those strings are remembered in the output so that they will not be reused in future.",
    )
    parser.add_argument(
        "--stable_typo_distance",
        type=int,
        default=0,
        help="The Levenshtein distance used to detect typos in comparison to stable strings.",
    )
    parser.add_argument(
        "--typo_detect",
        type=str,
        choices=["error", "warn", "fix", "fix_quiet"],
        default="error",
        help="What to do when detecting a typo against stable strings.",
    )
    parser.add_argument(
        "--module_id_max",
        type=int,
        default=127,
        help="The maximum value of a module ID.",
    )
    return parser.parse_args()


# https://stackoverflow.com/questions/174890/how-to-output-cdata-using-elementtree
def _escape_cdata(text: str) -> str:
    try:
        if "&" in text:
            text = text.replace("&", "&amp;")
        return text
    except TypeError:
        raise TypeError("cannot serialize %r (type %s)" % (text, type(text).__name__))


def main():
    et._escape_cdata = _escape_cdata
    args = parse_cmdline()

    stable_catalog = read_stable(args.stable_json)
    try:
        stable_ids = (
            {stable_msg_key(msg): (msg["id"], msg["msg"]) for msg in stable_catalog["messages"]},
            {hash(m["string"]): (m["id"], m["string"]) for m in stable_catalog["modules"]},
        )
        modules, messages = read_input(args.input, stable_ids, args.stable_typo_distance, args.typo_detect)
    except Exception as e:
        raise Exception(f"{str(e)} from file {args.input}")

    if args.cpp_output is not None:
        write_cpp(messages, modules, args.cpp_headers, args.cpp_output)

    stable_output = dict(messages=[], modules=[])
    if not args.forget_old_ids:
        stable_output = dict(
            messages=stable_catalog["messages"], modules=stable_catalog["modules"]
        )

    check_module_limit(modules, args.module_id_max)

    enums = {}
    if args.xml_output is not None or args.json_output is not None:
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

    if args.json_output is not None:
        write_json(messages, modules, enums, args.json_input, args.json_output, stable_output)

    if args.xml_output is not None:
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
