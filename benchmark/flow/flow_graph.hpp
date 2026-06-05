#pragma once

#include <flow/builder.hpp>
#include <flow/flow.hpp>
#include <flow/graph_builder.hpp>

namespace flow_bench {
// the number of nodes in the graph used for calculating benchmark codegen
constexpr inline int graph_node_count = 16;

template <typename Action> constexpr auto make_graph(Action act) {
    auto const a = flow::action<"a">(act);
    auto const b = flow::action<"b">(act);
    auto const c = flow::action<"c">(act);
    auto const d = flow::action<"d">(act);
    auto const e = flow::action<"e">(act);
    auto const f = flow::action<"f">(act);
    auto const g = flow::action<"g">(act);
    auto const h = flow::action<"h">(act);
    auto const i = flow::action<"i">(act);
    auto const j = flow::action<"j">(act);
    auto const k = flow::action<"k">(act);
    auto const l = flow::action<"l">(act);
    auto const m = flow::action<"m">(act);
    auto const n = flow::action<"n">(act);
    auto const o = flow::action<"o">(act);
    auto const p = flow::action<"p">(act);

    return flow::builder<>{}
        .add(*a >> *b >> *c)
        .add(b >> *d >> *e)
        .add(c >> *f)
        .add((d && e) >> *g)
        .add(f >> *h >> *i)
        .add(g >> *j)
        .add((i && j) >> *k >> *l)
        .add(l >> *m >> *n >> *o >> *p)
        .add(e >> i)
        .add(f >> k);
}
} // namespace flow_bench
