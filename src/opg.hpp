#ifndef _PAREAS_OPG_H
#define _PAREAS_OPG_H

#include <unordered_set>
#include <deque>
#include <stdexcept>

#include <cassert>

#include "grammar.hpp"
#include "graph.hpp"

enum class PrecedenceOrder {
    LESS,
    EQUAL,
    GREATER,
};

template <typename Grammar>
struct OperatorPrecedenceGrammar {
    using Terminal = typename Grammar::Terminal;
    using NonTerminal = typename Grammar::NonTerminal;

    struct TerminalPairHasher {
        size_t operator()(std::pair<Terminal, Terminal> pair) const {
            auto l = std::hash<Terminal>{}(pair.first);
            auto r = std::hash<Terminal>{}(pair.second);

            // ye olde boost hash_combine
            return l ^ (r + 0x9e3779b9 + (l << 6) + (l >> 2));
        }
    };

    using TerminalSet = std::unordered_set<Terminal>;
    using PrecedenceMatrix = std::unordered_map<std::pair<Terminal, Terminal>, PrecedenceOrder, TerminalPairHasher>;
    using PrecedenceFunction = std::unordered_map<Terminal, size_t>;

    Grammar* grammar;

    OperatorPrecedenceGrammar(Grammar* grammar): grammar(grammar) {}

    auto first(NonTerminal start) -> TerminalSet {
        return this->first_or_last(start, true);
    }

    auto last(NonTerminal start) -> TerminalSet {
        return this->first_or_last(start, false);
    }

    auto first_or_last(NonTerminal start, bool first) -> TerminalSet;

    auto build_precedence_matrix() -> PrecedenceMatrix;

    auto build_precedence_functions(const PrecedenceMatrix& pm) -> std::pair<PrecedenceFunction, PrecedenceFunction>;
};

template <typename Grammar>
auto OperatorPrecedenceGrammar<Grammar>::first_or_last(NonTerminal start, bool first) -> TerminalSet {
    auto queue = std::deque<NonTerminal>();
    auto seen_nts = std::unordered_set<NonTerminal>();
    auto terminal_set = TerminalSet();

    auto enqueue = [&queue, &seen_nts](NonTerminal nt) {
        auto inserted = seen_nts.insert(nt).second;
        if (inserted) {
            queue.push_back(nt);
        }
    };

    enqueue(start);
    while (!queue.empty()) {
        auto [start, end] = this->grammar->productions.equal_range(queue.front());
        assert(start != end);

        queue.pop_front();

        for (auto it = start; it != end; ++it) {
            auto prod = it->second;
            assert(prod.size() >= 1);

            auto sym = first ? prod.front() : prod.back();
            if (auto* nt = std::get_if<NonTerminal>(&sym)) {
                enqueue(*nt);

                if (prod.size() >= 2) {
                    size_t index = first ? 1 : prod.size() - 2;
                    // If this throws, the grammar is not OPG
                    terminal_set.insert(std::get<Terminal>(prod[index]));
                }
            } else {
                // If this throws, the grammar is not OPG
                terminal_set.insert(std::get<Terminal>(sym));
            }
        }
    }

    return terminal_set;
}

template <typename Grammar>
auto OperatorPrecedenceGrammar<Grammar>::build_precedence_matrix() -> PrecedenceMatrix {
    auto pm = PrecedenceMatrix();

    auto insert = [&pm](Terminal lhs, Terminal rhs, PrecedenceOrder order) {
        auto inserted = pm.insert({{lhs, rhs}, order}).second;
        if (!inserted) {
            throw std::runtime_error("Invalid grammar");
        }
    };

    // Insert relations with delimiter
    for (const auto t : this->first(this->grammar->start)) {
        insert(this->grammar->delim, t, PrecedenceOrder::LESS);
    }

    for (const auto t : this->last(this->grammar->start)) {
        insert(t, this->grammar->delim, PrecedenceOrder::GREATER);
    }

    // Insert equality relations between pairs of terminals (optionally
    // with a single nonterminal between them)
    auto terminals = std::vector<Terminal>();
    for (const auto& [_, prod] : this->grammar->productions) {
        terminals.clear();
        for (auto& sym : prod) {
            if (auto* t = std::get_if<Terminal>(&sym)) {
                terminals.push_back(*t);
            }
        }

        for (size_t i = 1; i < terminals.size(); ++i) {
            insert(terminals[i - 1], terminals[i], PrecedenceOrder::EQUAL);
        }
    }

    // Insert inequality relations
    for (const auto& [_, prod] : this->grammar->productions) {
        for (size_t i = 1; i < prod.size(); ++i) {
            auto* r = std::get_if<Terminal>(&prod[i]);
            auto* nt = std::get_if<NonTerminal>(&prod[i - 1]);
            if (!r || !nt) {
                continue;
            }

            for (auto l : this->last(*nt)) {
                insert(l, *r, PrecedenceOrder::GREATER);
            }
        }

        for (size_t i = 0; i < prod.size() - 1; ++i) {
            auto* l = std::get_if<Terminal>(&prod[i]);
            auto* nt = std::get_if<NonTerminal>(&prod[i + 1]);
            if (!l || !nt) {
                continue;
            }

            for (auto r : this->first(*nt)) {
                insert(*l, r, PrecedenceOrder::LESS);
            }
        }
    }

    return pm;
}

template <typename Grammar>
auto OperatorPrecedenceGrammar<Grammar>::build_precedence_functions(
    const PrecedenceMatrix& pm
) -> std::pair<PrecedenceFunction, PrecedenceFunction> {
    // Build initial terminal graph
    auto symbol_graph = Graph();
    auto terminals = std::unordered_map<Terminal, std::pair<Graph::VertexId, Graph::VertexId>>();
    for (const auto& [terms, _] : pm) {
        for (auto t : {terms.first, terms.second}) {
            auto it = terminals.find(t);
            if (it != terminals.end()) {
                continue;
            }

            auto fa = symbol_graph.add_vertex();
            auto ga = symbol_graph.add_vertex();
            terminals.insert(it, {t, {fa, ga}});
        }
    }

    // Unite components with equal precedence order
    auto ufds = Ufds(&symbol_graph);
    for (const auto& [terms, order] : pm) {
        if (order != PrecedenceOrder::EQUAL) {
            continue;
        }

        auto fa = terminals[terms.first].first;
        auto gb = terminals[terms.second].second;
        ufds.unite(fa, gb);
    }

    // Build the graph of components
    // First, do some preprocessing to get a mapping of component -> vertex
    auto component_graph = Graph();
    auto component_to_vertex = std::unordered_map<Ufds::ComponentId, Graph::VertexId>();
    for (const auto& [term, vertices] : terminals) {
        auto [fa, gb] = vertices;
        for (auto v : {fa, gb}) {
            auto c = ufds.find(v);
            auto it = component_to_vertex.find(c);
            if (it != component_to_vertex.end()) {
                continue;
            }

            auto cv = component_graph.add_vertex();
            component_to_vertex.insert(it, {c, cv});
        }
    }

    // Build the actual graph now
    for (const auto& [terms, order] : pm) {
        auto fa = terminals[terms.first].first;
        auto gb = terminals[terms.second].second;
        switch (order) {
            case PrecedenceOrder::LESS:
                // fa < gb, add an edge from gb -> fa
                component_graph.add_edge(
                    component_to_vertex[ufds.find(gb)],
                    component_to_vertex[ufds.find(fa)]
                );
                break;
            case PrecedenceOrder::GREATER:
                // fa > gb, add an edge from fa -> gb
                component_graph.add_edge(
                    component_to_vertex[ufds.find(fa)],
                    component_to_vertex[ufds.find(gb)]
                );
                break;
            case PrecedenceOrder::EQUAL:
                continue;
        }
    }

    // Do a topological sort to find the precedence
    auto maybe_order = component_graph.topological_ordering();
    if (!maybe_order.has_value()) {
        throw std::runtime_error("Invalid grammar");
    }

    // Invert order so that we have a mapping from vertex to precedence
    auto& order = maybe_order.value();
    auto vertex_to_precedence = std::vector<size_t>(component_graph.vertices.size());
    for (size_t i = 0; i < order.size(); ++i) {
        vertex_to_precedence[order[i]] = order.size() - i;
    }

    // Build the actual precedence functions
    auto f = PrecedenceFunction();
    auto g = PrecedenceFunction();
    for (const auto& [term, vertices] : terminals) {
        auto [fa, gb] = vertices;

        f[term] = vertex_to_precedence[component_to_vertex[ufds.find(fa)]];
        g[term] = vertex_to_precedence[component_to_vertex[ufds.find(gb)]];
    }

    return {f, g};
}

#endif
