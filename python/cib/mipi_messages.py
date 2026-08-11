import itertools
import re
import struct
from ctypes import LittleEndianStructure, c_uint32, c_uint64, sizeof
from functools import partial

encoding_reader = {
    "encode_u32": lambda reader: bytes(itertools.islice(reader, 4)),
    "encode_32": lambda reader: bytes(itertools.islice(reader, 4)),
    "encode_u64": lambda reader: bytes(itertools.islice(reader, 8)),
    "encode_64": lambda reader: bytes(itertools.islice(reader, 8)),
}


def enum_reader(sz, reader):
    return bytes(itertools.islice(reader, sz))


format_table = {
    "char": (1, "c"),
    "signed char": (1, "b"),
    "unsigned char": (1, "B"),
    "bool": (1, "b"),
    "short": (2, "h"),
    "unsigned short": (2, "H"),
    "int": (4, "i"),
    "unsigned int": (4, "I"),
    "long": (4, "l"),
    "unsigned long": (4, "L"),
    "long long": (8, "q"),
    "unsigned long long": (8, "Q"),
    "float": (4, "f"),
    "double": (8, "d"),
}

alt_format_table = {
    "long": (8, "q"),
    "unsigned long": (8, "Q"),
}


def convert(c_type, seq):
    sz, fmt = format_table[c_type]
    # if the production machine has 8-byte longs, adjust
    if (c_type == "long" or c_type == "unsigned long") and len(seq) == 8:
        sz, fmt = alt_format_table[c_type]
    result = struct.unpack(f"<{fmt}", seq[:sz])[0]
    # clang cindex reports "true" as value -1
    if c_type == "bool" and result == 1:
        result = -1
    return result


def enum_lookup(db, enum_type, value):
    if enum_type in db["enums"] and value in db["enums"][enum_type]:
        return db["enums"][enum_type][value]
    else:
        return f"static_cast<{enum_type}>({value})"


def enum_converter(db, enum_type, underlying_type, seq):
    value = str(convert(underlying_type, seq))
    return enum_lookup(db, enum_type, value)


def convert_ct_enums(msg, db):
    enum_re = re.compile(r"\(([a-zA-Z0-9_]+)\)([0-9]+)\b")
    m = re.search(enum_re, msg)
    while m:
        enum_type = m.group(1)
        value = m.group(2)
        s = enum_lookup(db, enum_type, value)
        msg = re.sub(enum_re, s, msg)
        m = re.search(enum_re, msg)
    return msg


class HeaderStruct(LittleEndianStructure):
    _fields_ = [
        ("type", c_uint32, 4),
        ("severity", c_uint32, 3),
        ("r0", c_uint32, 1),
        ("opt_loc", c_uint32, 1),
        ("opt_len", c_uint32, 1),
        ("opt_chk", c_uint32, 1),
        ("opt_ts", c_uint32, 1),
        ("unit", c_uint32, 4),
        ("module", c_uint32, 7),
        ("opt_guid", c_uint32, 1),
        ("subtype", c_uint32, 6),
        ("r1", c_uint32, 1),
        ("r2", c_uint32, 1),
    ]


class Short32Struct(LittleEndianStructure):
    _fields_ = [
        ("type", c_uint32, 4),
        ("id", c_uint32, 28),
    ]


class Short64Struct(LittleEndianStructure):
    _fields_ = [
        ("type", c_uint64, 4),
        ("id", c_uint64, 60),
    ]


def read_struct(struct, reader):
    return struct.from_buffer_copy(bytes(itertools.islice(reader, sizeof(struct))))


class Short32:
    def __init__(self, reader, messages, _, db):
        self.struct = read_struct(Short32Struct, reader)
        assert self.struct.id in messages, (
            f"Message ID {self.struct.id} not found in JSON"
        )
        self.msg_spec = messages[self.struct.id]
        self.msg_spec["msg"] = convert_ct_enums(self.msg_spec["msg"], db)

    def __str__(self):
        return self.msg_spec["msg"]


class Short64:
    def __init__(self, reader, messages, _, db):
        self.struct = read_struct(Short64Struct, reader)
        assert self.struct.id in messages, (
            f"Message ID {self.struct.id} not found in JSON"
        )
        self.msg_spec = messages[self.struct.id]
        self.msg_spec["msg"] = convert_ct_enums(self.msg_spec["msg"], db)

    def __str__(self):
        return self.msg_spec["msg"]


severity = [
    "MAX",
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "USER1",
    "USER2",
    "TRACE",
]


class Catalog:
    @staticmethod
    def read_id(reader):
        read = encoding_reader["encode_u32"]
        return convert("unsigned int", read(reader))

    @staticmethod
    def extract_arg(db, reader, arg):
        encode_tag, spec = arg[:-1].split("<")

        if encode_tag == "encode_enum":
            cpp_type, underlying, sz = spec.split(",")
            conv = partial(enum_converter, db, cpp_type, underlying.strip())
            read = partial(enum_reader, int(sz))
        else:
            read = encoding_reader[encode_tag]
            if spec in format_table:
                conv = partial(convert, spec)
            elif "32" in encode_tag:
                conv = partial(enum_converter, db, spec, "unsigned int")
            else:
                conv = partial(enum_converter, db, spec, "unsigned long long")

        return conv(read(reader))

    def __init__(self, reader, messages, modules, db):
        self.header = read_struct(HeaderStruct, reader)
        assert self.header.subtype == 1, (
            f"Catalog message subtype {self.header.subtype} not supported"
        )

        assert self.header.module in modules, (
            f"Module ID {self.header.module} not found in JSON"
        )
        self.module = modules[self.header.module]

        self.severity = severity[self.header.severity]

        self.id = Catalog.read_id(reader)
        assert self.id in messages, f"Message ID {self.id} not found in JSON"
        self.msg_spec = messages[self.id]
        self.msg_spec["msg"] = convert_ct_enums(self.msg_spec["msg"], db)

        self.args = [
            Catalog.extract_arg(db, reader, arg) for arg in self.msg_spec["arg_types"]
        ]

    def __str__(self):
        return (
            f"{self.severity} [{self.module}] {self.msg_spec['msg'].format(*self.args)}"
        )
