# Logging

The logging library (libraries) is a case study in decomposition in service of
extreme flexibility without performance penalty.

A client may replace piecemeal any or all of:

- a complete logger
- a backend (destination)
- binary packet building logic

and the log environment can customize any aspect of logging at compile time.
Clients can define their own log levels, flavours, etc. A client frontend can
communicate with a custom backend through environment values that the CIB code
knows nothing about. And in fact CIB itself uses this: there are several
environment queries in `log_binary` that `log` doesn't need to know about.

The flow and message libraries also use this feature to customize how flows and
messages get logged. And the log library doesn't need to know about them.

So there is a distinct separation in the library between flexibility and
convenience. Everything can be customized (flexibility), but not everything need
be customized (convenience).

Flexibility by compile-time substitution is very important for performance. CIB
logging used to have `inline` and `flatten` annotations everywhere. This is not
a good thing for library code to make decisions about. The better approach is to
allow the compiler to decide, but allow the client to replace if necessary for
their particular needs. We want logging to be ubiquitous, which means it's
sensitive to inlining decisions (or outlining decisions). And those decisions
should be in the hands of the client, not the library author.

## binary argument encoding

At the moment, only integral, floating point, and enumeration types are
supported. The MIPI-SyS-T spec also allows structured data: this is not yet
supported. Perhaps when reflection arrives, this will be quite easy.

## environment queries

- level: the log level/severity (probably an enumeration value)
- flavor: intended to allow multiple loggers e.g. secure and insecure (a `type_identity` value to specialize the logging config)
- module: the module name (a compile-time string)
- module ID: the module id (an integral type: for when an ID -- not just a name -- needs to be fixed e.g. for compatibility with existing binaries)
- unit: intended to differentiate multiple binaries at runtime (an integral type)
- string ID: the string id (an integral type: for when an ID -- not just a name -- needs to be fixed e.g. for compatibility with existing binaries)
- builder: for changing the log packet builder
- writer: for changing the log destination writer
- tag: an optional metadata key-value pair that will go into the JSON collateral to be used for decode. Currently just the one tag, but no reason it could not be a list.

