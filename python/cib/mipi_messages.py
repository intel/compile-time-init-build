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
    "encode_enum": lambda reader: bytes(itertools.islice(reader, 4)),
}

encoding_converter = {
    "int": lambda seq: struct.unpack("<i", seq)[0],
    "unsigned int": lambda seq: struct.unpack("<I", seq)[0],
    "long": lambda seq: struct.unpack("<q", seq)[0],
    "float": lambda seq: struct.unpack("<f", seq)[0],
    "double": lambda seq: struct.unpack("<d", seq)[0],
}


def enum_lookup(db, enum_type, value):
    if enum_type in db["enums"] and value in db["enums"][enum_type]:
        return db["enums"][enum_type][value]
    else:
        return f"static_cast<{enum_type}>({value})"


def enum_converter(db, enum_type, underlying_type, seq):
    convert = encoding_converter[underlying_type]
    value = str(convert(seq))
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
        convert = encoding_converter["unsigned int"]
        return convert(read(reader))

    @staticmethod
    def extract_arg(db, reader, arg):
        encode_tag, spec = arg[:-1].split("<")
        read = encoding_reader[encode_tag]

        if encode_tag == "encode_enum":
            cpp_type, underlying = spec.split(",")
            convert = partial(enum_converter, db, cpp_type, underlying.strip())
        else:
            if spec in encoding_converter:
                convert = encoding_converter[spec]
            else:
                convert = partial(enum_converter, db, spec, "unsigned int")

        return convert(read(reader))

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
