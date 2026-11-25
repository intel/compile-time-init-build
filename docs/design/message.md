# Messages and Fields

## message equivalence

`equivalent` compares fields in two different messages (maybe of different
types). It is not symmetric, i.e. `equivalent(a, b)` might not be the same value
as `equivalent(b, a)` -- because we iterate through the fields of the first
argument. So for instance the second argument might have a superset of the
fields of the first.

If there are fields present in `a` that are not in `b`, currently `equivalent(a,
b)` will fail to compile with a tuple lookup error. A possible other choice
would be for it to return `false` (and that could be decidable at compile time).

## views, ownership, and constraints

The idea of whether or not a message instance is owning or a view type is clear.
The idea that owning can implicitly convert to view is clear. And the idea that
view must be explicitly converted to owning is clear (because this involves a
copy).

What is not clear sometimes is when we write a function template that is
constrained to expect a `viewlike` and we pass it an owning message, and we
perhaps unexpectedly get a compile error. This happens because the template
doesn't do any conversion: it's instantiated with the actual type of the message
(owning) which is not `viewlike`.

Should we make this call possible (presumably by altering `viewlike`, or adding
another concept that includes owning messages)? It might be convenient. But it
also might have performance implications, because view types are usually passed
by value. And it could raise confusion similar to the confusion over "movable"
and "copyable".
