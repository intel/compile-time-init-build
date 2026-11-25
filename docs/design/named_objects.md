# Named objects

A lot of the nouns in the various CIB libraries are named:

- interrupts
- flows and steps
- messages and fields
- etc.

This generally means that a `stdx::ct_string` is given as the first template
argument. One of the problem with excessive unique naming is that it can cause
code and/or symbol bloat.

Code bloat can be tamed by the usual methods, primarily by splitting out data
and functions so that they are not unnecessarily duplicated and only depend on
the required template arguments. Small functions and lambdas also help the
compiler to inline appropriately and potentially to not emit (or allow the
linker to remove) functions.

Symbol bloat is an issue that manifests in at least two ways: slower compilation
times because of compiler-internal wrangling, and potentially hitting a limit on
the symbol table size for a toolchain (and even potentially hitting a limit for
an individual symbol size, although this is not really the fault of named types
per se).

To counter symbol bloat, many types in CIB use the technique of `stdx::shrink`.
This can reduce a "name symbol" length for example from something like:

```
ct_string<2u>{std::array<char, 2u>{char [2]{(char) 65}}}
```

to:

```
{lambda#10}{}
```

which is a significant saving even for very short names (like this example).
And especially so when applied across a codebase with many named types.

This form of shrinking comes with a drawback though: the shrunk type is a
lambda, and is unique to each translation unit. Meaning that the same string
results in different types in different translation units. If it is then used to
parameterize static data through a variable template or similar, that data is
duplicated for each translation unit. Not an ODR violation, but we wanted the
same-named things to reference the same data in the program. So that is not
good. To avoid this we must take care to expand the original `ct_string` before
passing it to a class template containing such data.

Another option might be to implement `stdx::shrink` on `ct_string`s by hashing
them -- although this would preclude expansion. This could reduce each
`ct_string` to a `std::uint64_t` that would be stable across TUs. A good choice
of hashing function might be FNV1a. Hash collisions should be incredibly rare,
but for the sake of paranoia we could perhaps post-scan the object files to
ensure no mishaps.
