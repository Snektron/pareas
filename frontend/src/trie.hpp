#ifndef _PAREAS_TRIE_HPP
#define _PAREAS_TRIE_HPP

#include <unordered_map>
#include <memory>
#include <optional>
#include <stdexcept>

template <typename Grammar>
struct ReverseTrie {
    using Symbol = typename Grammar::Symbol;
    using Terminal = typename Grammar::Terminal;
    using NonTerminal = typename Grammar::NonTerminal;
    using Production = typename Grammar::Production;

    struct Node {
        std::unordered_map<Symbol, std::unique_ptr<Node>> children;
        std::optional<NonTerminal> end;
    };

    std::unique_ptr<Node> root;

    explicit ReverseTrie(const Grammar* grammar);

    auto build(const Grammar* grammar) -> ReverseTrie;
    auto insert(const NonTerminal& rhs, const Production& prod) -> void;
};

template <typename Grammar>
ReverseTrie<Grammar>::ReverseTrie(const Grammar* grammar) {
    for (const auto& [rhs, prod] : grammar->productions) {
        this->insert(rhs, prod);
    }
}

template <typename Grammar>
auto ReverseTrie<Grammar>::insert(const NonTerminal& rhs, const Production& prod) -> void {
    if (this->root == nullptr) {
        this->root = std::make_unique<Node>();
    }

    Node* node = this->root.get();
    for (auto it = prod.rbegin(); it != prod.rend(); ++it) {
        auto sym = *it;
        if (node->children.find(sym) == node->children.end()) {
            node->children[sym] = std::make_unique<Node>();
        }

        node = node->children[sym].get();
    }

    if (node->end.has_value()) {
        throw std::runtime_error("Duplicate RHS");
    }

    node->end = rhs;
}

#endif
