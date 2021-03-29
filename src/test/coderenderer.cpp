#include "test/coderenderer.hpp"
#include "test/renderstate.hpp"



#include <iostream>

bool operator==(const SymbolInfo& a, const SymbolInfo& b) {
    return a.name == b.name;
}

bool operator!=(const SymbolInfo& a, const SymbolInfo& b) {
    return !(a == b);
}

namespace std {
    size_t hash<SymbolInfo>::operator()(const SymbolInfo& symb) const {
        return std::hash<std::string>()(symb.name);
    }
}

CodeRenderer::CodeRenderer(const RenderState* root_node, size_t seed) : root_node(root_node), rng(seed) {}

void CodeRenderer::render(std::ostream& os) {
    this->root_node->render(*this, os);
}

bool CodeRenderer::addId(const std::string& name, size_t category) {
    SymbolInfo symb_info{name, category};

    if(this->commit_table.count(symb_info) > 0)
        return false;
    std::unordered_set<SymbolInfo>& current_scope = this->generated_ids[this->generated_ids.size()-1];
    if(current_scope.count(symb_info))
        return false;
    this->commit_table.insert(symb_info);
    return true;
}

void CodeRenderer::commitIds() {
    this->generated_ids[this->generated_ids.size()-1] = this->commit_table;
}

void CodeRenderer::enterScope() {
    this->commit_table.clear();
    this->generated_ids.push_back({});
}

void CodeRenderer::exitScope() {
    this->commit_table.clear();
    this->generated_ids.pop_back();
}

void CodeRenderer::setIndent(size_t x) {
    this->indent = x;
}

size_t CodeRenderer::getIndent() const {
    return this->indent;
}

size_t CodeRenderer::getMaxDepth() const {
    return this->max_depth;
}

void CodeRenderer::setMaxDepth(size_t depth) {
    this->max_depth = depth;
}

size_t CodeRenderer::getDepth() const {
    return this->depth;
}

void CodeRenderer::setDepth(size_t depth) {
    this->depth = depth;
}

std::string CodeRenderer::getRandomID(size_t category, bool& valid) {
    std::vector<std::string> candidates;
    for(size_t i = this->generated_ids.size(); i > 0; --i) {
        std::unordered_set<SymbolInfo>& current_scope = this->generated_ids[i-1];

        for(const SymbolInfo& s : current_scope) {
            if(s.category == category || category == 0)
                candidates.push_back(s.name);
        }
    }
    valid = candidates.size() != 0;
    if(!valid)
        return "";

    std::uniform_int_distribution<size_t> distr(0, candidates.size() - 1);
    return candidates[distr(this->get_rng())];
}