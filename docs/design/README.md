# Design Rationales for Constructs in *cib*

This directory contains design thoughts and rationales that guide *cib* in its
current state and future path.

It is an attempt to capture the so-called "intent capital" of the code,
following the "3 kinds of debt" model outlined in
[From Technical Debt to Cognitive and Intent Debt](https://arxiv.org/abs/2603.22106).

In this model, broadly speaking:

- _Technical_ debt/capital is a measure of how difficult/easy it is to make change: it lives in the structure of the code.
- _Cognitive_ debt/capital is a measure of how poorly/well understood code is by its authors: it lives in the heads of people.
- _Intent_ debt/capital is a measure of how poorly/well the goals of the system are articulated and met: it lives in design artifacts.

Here we try to capture the goals and intent of the code, many times through
examinations of use cases, considerations of constraints, and "wishful thinking"
API design.

Notably, we are not interested in _easy_; we are only interested in
[_simple_](https://www.youtube.com/watch?v=SxdOUGdseq4).
