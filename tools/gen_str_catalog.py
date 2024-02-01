import argparse
import re
import sys
import json
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


catalog_re = re.compile("^.+?(unsigned int catalog<(.+?)>\(\))\s*$")
string_re = re.compile(
    "message<\(logging::level\)(\d+), sc::undefined<sc::args<(.*)>, char, (.*)>\s*>"
)
string_id = 0
levels = ["MAX", "FATAL", "ERROR", "WARN", "INFO", "USER1", "USER2", "TRACE"]
catalogued_strings = set()


def extract_message(num, line):
    global string_id
    global catalogued_strings

    try:
        catalog_m = catalog_re.match(line)
        if catalog_m is None:
            return None

        string_m = string_re.match(catalog_m.group(2))
        catalog_type = catalog_m.group(1)

        if catalog_type not in catalogued_strings:
            catalogued_strings.add(catalog_type)

            log_level = string_m.group(1)
            arg_tuple = string_m.group(2)
            string_tuple = string_m.group(3).replace("(char)", "")
            string_value = "".join(
                [chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)]
            )
            args = split_args(arg_tuple)
            string_id += 1

            return (
                (catalog_type, arg_tuple),
                dict(
                    level=levels[int(log_level)],
                    msg=string_value,
                    type="flow" if string_value.startswith("flow.") else "msg",
                    id=string_id - 1,
                    arg_types=args,
                    arg_count=len(args),
                ),
            )

    except Exception:
        raise Exception(f"Couldn't extract catalog info at line {num} ({line})")


def read_input(filename):
    with open(filename, "r") as f:
        lines = [line.strip() for line in f]
        return dict(
            filter(
                None, [extract_message(num + 1, line) for num, line in enumerate(lines)]
            )
        )


def make_cpp_defn(types, msg):
    catalog_type, arg_tuple = types
    return f"""/*
    "{msg["msg"]}"
    {arg_tuple}
 */
template<> {catalog_type} {{
    return {msg["id"]};
}}"""


def write_cpp(messages, filename):
    with open(filename, "w") as f:
        f.write("#include <log/catalog/catalog.hpp>\n\n")
        cpp_defns = [make_cpp_defn(k, v) for k, v in messages.items()]
        f.write("\n".join(cpp_defns))


def write_json(messages, filename):
    str_catalog = dict(messages=list(messages.values()))
    with open(filename, "w") as f:
        json.dump(str_catalog, f, indent=4)


def write_xml(messages, filename, client_name, version, guid_id, guid_mask):
    syst_collateral = et.Element("syst:Collateral")
    syst_collateral.set("xmlns:syst", "http://www.mipi.org/1.0/sys-t")
    syst_collateral.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    syst_collateral.set(
        "xsi:schemaLocation",
        "http://www.mipi.org/1.0/sys-t https://www.mipi.org/schema/sys-t/sys-t_1-0.xsd",
    )

    syst_client = et.SubElement(syst_collateral, "syst:Client")
    syst_client.set("Name", client_name)

    syst_fwversion = et.SubElement(syst_collateral, "syst:FwVersion")
    syst_fwversion.set("FW_Version", version)

    syst_guids = et.SubElement(syst_client, "syst:Guids")
    syst_guid = et.SubElement(syst_guids, "syst:Guid")
    syst_guid.set("ID", f"{{{guid_id}}}")
    syst_guid.set("Mask", f"{{{guid_mask}}}")

    syst_short_message = et.SubElement(syst_client, "syst:Short32")
    syst_catalog_message = et.SubElement(syst_client, "syst:Catalog32")

    for msg in messages.values():
        if msg["arg_count"] == 0:
            syst_format = et.SubElement(syst_short_message, "syst:Format")
            syst_format.set("ID", "0x%08X" % msg["id"])
            syst_format.set("Mask", "0x0FFFFFFF")
            printf_string = msg["msg"]
        else:
            syst_format = et.SubElement(syst_catalog_message, "syst:Format")
            syst_format.set("ID", "0x%08X" % msg["id"])
            syst_format.set("Mask", "0xFFFFFFFF")
            printf_string = re.sub(r"{}", r"%d", msg["msg"])
            printf_string = re.sub(r"{:(.*?)}", r"%\1", printf_string)
        syst_format.text = "<![CDATA[" + printf_string + "]]>"

    et.indent(syst_collateral, space="    ")
    xml_string = et.tostring(syst_collateral, encoding="utf8", method="xml")
    with open(filename, "wb") as xf:
        xf.write(xml_string)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        type=str,
        required=True,
        help=(
            "Input filename: a file of undefined symbols produced by running"
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
    args = parser.parse_args()

    try:
        messages = read_input(args.input)
    except Exception as e:
        raise Exception(f"{str(e)} from file {args.input}")

    if args.cpp_output:
        write_cpp(messages, args.cpp_output)
    if args.json_output:
        write_json(messages, args.json_output)
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
