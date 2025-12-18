#!/usr/bin/env python3

import argparse
import itertools
import json
import re
import xml.etree.ElementTree as et
from functools import partial


def find_arg_split_pos(s: str, start: int) -> int:
    angle_count: int = 0
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
    args: list[str] = []
    start: int = 0
    while start < len(s):
        pos: int = find_arg_split_pos(s, start)
        args.append(s[start:pos].strip())
        start = pos + 1
    return args


class Intervals:
    def __init__(self, text: str):
        self.intervals = []
        for i in text.split(","):
            rng = i.split("-")
            if rng != [""]:
                start = int(rng[0], 0)
                self.intervals.append(
                    (start, int(rng[1], 0) if len(rng) == 2 else start)
                )

    def contains(self, v: int) -> bool:
        return any(map(lambda x: v >= x[0] and v <= x[1], self.intervals))

    def __str__(self):
        rngs = map(
            lambda x: f"{x[0]}-{x[1]}" if x[0] != x[1] else f"{x[0]}",
            self.intervals,
        )
        return ",".join(rngs)

    def __repr__(self):
        return f'Intervals("{self}")'


class Message:
    cpp_prefix: str = "sc::message<sc::undefined"

    def __init__(self, text: str, args: list[str], id: int, id_suffix: str|None = None):
        self.text = text
        self.args = args
        self.id = id
        self.id_suffix = id_suffix if id_suffix is not None else ""
        self.orig_id = id
        self.enum_lookup = None

    @classmethod
    def from_cpp_type(cls, s):
        string_re = re.compile(
            rf"{cls.cpp_prefix}<sc::args<(.*)>, (-?\d+)([a-zA-Z]*), char, (.*)>\s*>"
        )
        m = string_re.match(s)
        string_tuple = m.group(4).replace("(char)", "")
        return cls(
            "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple))),
            split_args(m.group(1)),
            int(m.group(2)),
            m.group(3),
        )

    @classmethod
    def from_json(cls, json: dict):
        return cls(
            json["msg"],
            json["arg_types"],
            json["id"],
        )

    def to_json(self):
        return dict(
            msg=self.text,
            type="flow" if self.text.startswith("flow.") else "msg",
            arg_types=self.args,
            arg_count=len(self.args),
            id=self.id,
        )

    @property
    def type(self):
        return "flow" if self.text.startswith("flow.") else "msg"

    def to_cpp_type(self):
        return rf"{self.cpp_prefix}<sc::args<{', '.join(self.args)}>, {self.orig_id}{self.id_suffix}, char, {', '.join(f'static_cast<char>({ord(c)})' for c in self.text)}>>"

    def key(self):
        return hash(self.text) ^ hash("".join(self.args))

    def __repr__(self):
        return f"Message('{self.text}', {self.args}, {self.id}, {self.id_suffix})"

    def __lt__(self, other):
        return self.text < other.text


class Module:
    cpp_prefix: str = "sc::module_string<sc::undefined"

    def __init__(self, text: str, id: int, id_suffix: str|None = None):
        self.text = text
        self.id = id
        self.id_suffix = id_suffix if id_suffix is not None else ""
        self.orig_id = id

    @classmethod
    def from_cpp_type(cls, s):
        string_re = re.compile(rf"{cls.cpp_prefix}<(.*), (-?\d+)([a-zA-Z]*), char, (.*)>\s*>")
        m = string_re.match(s)
        string_tuple = m.group(4).replace("(char)", "")
        return cls(
            "".join((chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple))),
            int(m.group(2)),
            m.group(3),
        )

    @classmethod
    def from_json(cls, json: dict):
        return cls(
            json["string"],
            json["id"],
        )

    def to_json(self):
        return dict(
            string=self.text,
            id=self.id,
        )

    def to_cpp_type(self):
        return rf"{self.cpp_prefix}<void, {self.orig_id}{self.id_suffix}, char, {', '.join(f'static_cast<char>({ord(c)})' for c in self.text)}>>"

    def key(self):
        return hash(self.text)

    def __repr__(self):
        return f"Module('{self.text}', {self.id}, {self.id_suffix})"

    def __lt__(self, other):
        return self.text < other.text


def extract(line_num: int, line_m: re.Match[str]):
    try:
        return (
            Message.from_cpp_type(line_m.group(2))
            if line_m.group(1) == "catalog"
            else Module.from_cpp_type(line_m.group(2))
        )
    except Exception:
        raise Exception(
            f"Couldn't extract catalog info at line {line_num} ({line_m.group(0)})"
        )


def assign_ids_with(items, id_fn):
    sorted_items = sorted(items)
    for i in sorted_items:
        i.id = id_fn(i)
    return list(sorted_items)


def assign_ids(messages, modules, stable_data, reserved_ids):
    def get_id(stables, gen, obj):
        key = obj.key()
        if key in stables:
            return stables[key].id
        else:
            return next(gen)

    stable_msgs, stable_modules = stable_data
    for msg in filter(lambda m: m.id != -1, messages):
        stable_msgs[msg.key()] = msg
    for module in filter(lambda m: m.id != -1, modules):
        stable_modules[module.key()] = module

    old_msg_ids = set(m.id for m in stable_msgs.values())
    msg_id_gen = itertools.filterfalse(
        lambda x: old_msg_ids.__contains__(x) or reserved_ids.contains(x),
        itertools.count(0),
    )
    get_msg_id = partial(get_id, stable_msgs, msg_id_gen)

    old_module_ids = set(m.id for m in stable_modules.values())
    module_id_gen = itertools.filterfalse(
        old_module_ids.__contains__, itertools.count(0)
    )
    get_module_id = partial(get_id, stable_modules, module_id_gen)

    return (
        assign_ids_with(messages, get_msg_id),
        assign_ids_with(modules, get_module_id),
    )


def read_input(filenames: list[str], stable_data, reserved_ids):
    line_re = re.compile(r"^.*unsigned (?:int|long) (catalog|module)<(.+?)>\(\)$")

    def read_file(filename):
        with open(filename, "r") as f:
            matching_lines = filter(
                lambda p: p[1] is not None,
                ((num + 1, line_re.match(line.strip())) for num, line in enumerate(f)),
            )
            return [extract(*m) for m in matching_lines]

    items = list(itertools.chain.from_iterable(read_file(f) for f in filenames))
    messages = filter(lambda x: isinstance(x, Message), items)
    unique_messages = {m.key(): m for m in messages}.values()
    modules = filter(lambda x: isinstance(x, Module), items)
    unique_modules = {m.key(): m for m in modules}.values()

    return assign_ids(unique_messages, unique_modules, stable_data, reserved_ids)


def make_cpp_scoped_enum_decl(e: str, ut: str) -> str:
    parts = e.split("::")
    enum = parts[-1]
    if "(anonymous namespace)" in parts or "{anonymous}" in parts:
        raise Exception(
            f"Scoped enum {e} is inside an anonymous namespace and cannot be forward declared."
        )
    if len(parts) > 1:
        ns = "::".join(parts[:-1])
        return f"namespace {ns} {{ enum struct {enum} : {ut}; }}"
    return f"enum struct {enum} : {ut};"


def make_cpp_catalog_defn(m: Message) -> str:
    return f"""/*
    "{m.text}"
    {m.args}
 */
template<> auto catalog<{m.to_cpp_type()}>() -> string_id {{
    return {m.id};
}}"""


def make_cpp_module_defn(m: Module) -> str:
    return f"""/*
    "{m.text}"
 */
template<> auto module<{m.to_cpp_type()}>() -> module_id {{
    return {m.id};
}}"""


def write_cpp(messages, modules, scoped_enums, extra_headers: list[str], filename: str):
    with open(filename, "w") as f:
        f.write("\n".join(f'#include "{h}"' for h in extra_headers))
        f.write("\n#include <log_binary/catalog/arguments.hpp>\n")
        f.write("\n#include <log_binary/catalog/catalog.hpp>\n\n")
        scoped_enum_decls = [
            make_cpp_scoped_enum_decl(e, ut) for e, ut in scoped_enums.items()
        ]
        f.write("\n".join(scoped_enum_decls))
        f.write("\n\n")
        cpp_catalog_defns = [make_cpp_catalog_defn(m) for m in messages]
        f.write("\n".join(cpp_catalog_defns))
        f.write("\n\n")
        cpp_module_defns = [make_cpp_module_defn(m) for m in modules]
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

    enums: dict[str, dict] = {}
    for node in translation_unit.cursor.walk_preorder():
        if node.kind == CursorKind.ENUM_DECL:
            new_decl = {
                e.spelling: e.enum_value
                for e in node.walk_preorder()
                if e.kind == CursorKind.ENUM_CONSTANT_DECL
            }
            enums[fq_name(node)] = enums.get(fq_name(node), {}) | new_decl
    return enums


def write_json(
    messages, modules, enums, extra_inputs: list[str], filename: str, stable_data
):
    d = dict(
        messages=[m.to_json() for m in messages], modules=[m.to_json() for m in modules]
    )
    for m in stable_data.get("messages"):
        j = m.to_json()
        if j not in d["messages"]:
            d["messages"].append(j)
    for m in stable_data.get("modules"):
        j = m.to_json()
        if j not in d["modules"]:
            d["modules"].append(j)

    str_catalog = dict(**d, enums=dict())
    for extra in extra_inputs:
        with open(extra, "r") as f:
            str_catalog.update(json.load(f))

    es = dict()
    for k, (_, v) in enums.items():
        es.update({k: {value: name for (name, value) in v.items()}})
    for k, v in es.items():
        if k in str_catalog["enums"]:
            str_catalog["enums"][k].update(v)  # type: ignore
        else:
            str_catalog["enums"].update({k: v})  # type: ignore

    with open(filename, "w") as f:
        json.dump(str_catalog, f, indent=4)


def read_stable(stable_filenames: list[str]):
    stable_catalog: dict[str, list] = dict(messages=[], modules=[])
    for filename in stable_filenames:
        with open(filename, "r") as f:
            stable_catalog.update(json.load(f))
    return (
        {
            Message.from_json(m).key(): Message.from_json(m)
            for m in stable_catalog["messages"]
        },
        {
            Module.from_json(m).key(): Module.from_json(m)
            for m in stable_catalog["modules"]
        },
    )


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


def serialize_modules(client_node: et.Element, modules: list[Module]):
    syst_modules = et.SubElement(client_node, "syst:Modules")
    for m in modules:
        syst_module = et.SubElement(
            syst_modules, "syst:Module", attrib={"ID": f"{m.id}"}
        )
        syst_module.text = f"<![CDATA[{m.text}]]>"


def arg_type_encoding(arg):
    string_re = re.compile(r"encode_(32|u32|64|u64|enum)<(.*)>")
    m = string_re.match(arg)
    if "enum" in m.group(1):
        args_re = re.compile(r"(.*), (.*)")
        args_m = args_re.match(m.group(2))
        return (f"encode_{m.group(1)}", args_m.group(1), args_m.group(2))
    return (f"encode_{m.group(1)}", m.group(2), m.group(2))


def arg_printf_spec(arg: str):
    printf_dict = {
        "encode_32": "%d",
        "encode_u32": "%u",
        "encode_64": "%lld",
        "encode_u64": "%llu",
        "float": "%f",
        "double": "%f",
        "int": "%d",
        "unsigned int": "%u",
        "short": "%d",
        "unsigned short": "%u",
        "signed char": "%d",
        "unsigned char": "%u",
        "char": "%c",
        "long": "%ld",
        "unsigned long": "%lu",
        "long long": "%lld",
        "unsigned long long": "%llu",
    }
    enc, _, ut = arg_type_encoding(arg)
    return printf_dict.get(ut, printf_dict.get(enc, "%d"))


def serialize_messages(
    short_node: et.Element, catalog_node: et.Element, messages: list[Message]
):
    for msg in messages:
        syst_format = et.SubElement(
            short_node if len(msg.args) == 0 else catalog_node,
            "syst:Format",
            attrib={"ID": "0x%08X" % msg.id, "Mask": "0x0FFFFFFF"},
        )
        if msg.enum_lookup:
            syst_format.set("EnumLookup", f"{msg.enum_lookup[0]}:{msg.enum_lookup[1]}")

        fmt_string = msg.text
        for arg in msg.args:
            m = re.search(r"{:([^}]+)}", fmt_string)
            if m:
                fmt_string = (
                    f"{fmt_string[:m.start()]}%{m.group(1)}{fmt_string[m.end():]}"
                )
            else:
                fmt_string = re.sub(r"{}", arg_printf_spec(arg), fmt_string, count=1)
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


def check_module_limit(modules: list[Module], max: int):
    for m in modules:
        if m.id > max:
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
        "--module_id_max",
        type=int,
        default=127,
        help="The maximum value of a module ID.",
    )
    parser.add_argument(
        "--reserved_ids",
        type=lambda x: Intervals(x),
        default="",
        help="A list of (inclusive) ranges of string IDs that should be reserved and not used. e.g. '1-5,10-15,20'",
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


def stable_msg_key(msg: dict):
    return hash(msg["msg"]) ^ hash("".join(msg["arg_types"]))


def main():
    et._escape_cdata = _escape_cdata
    args = parse_cmdline()

    stable_data = read_stable(args.stable_json)
    try:
        messages, modules = read_input(args.input, stable_data, args.reserved_ids)
    except Exception as e:
        raise Exception(f"{str(e)} from file {args.input}")

    scoped_enums = {}
    if args.cpp_output is not None:
        for m in messages:
            for i, arg_type in enumerate(m.args):
                enc, enum, ut = arg_type_encoding(arg_type)
                if "enum" in enc:
                    scoped_enums.update({enum: ut})

        write_cpp(messages, modules, scoped_enums, args.cpp_headers, args.cpp_output)

    stable_output = dict(messages=[], modules=[])
    if not args.forget_old_ids:
        stable_msgs, stable_modules = stable_data
        stable_output = dict(
            messages=list(stable_msgs.values()), modules=list(stable_modules.values())
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
                for m in messages:
                    for i, arg_type in enumerate(m.args):
                        _, enum, _ = arg_type_encoding(arg_type)
                        if enum in enums:
                            m.enum_lookup = (i + 1, enums[enum][0])
            except Exception as e:
                print(
                    f"Couldn't extract enum info ({str(e)}), enum lookup will not be available"
                )
        else:
            print("XML output without C++ output: enum lookup will not be available")

    if args.json_output is not None:
        write_json(
            messages, modules, enums, args.json_input, args.json_output, stable_output
        )

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
