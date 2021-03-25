#include "test/coderenderer.hpp"
#include "test/renderstate.hpp"

CodeRenderer::CodeRenderer(const RenderState* root_node, size_t seed) : root_node(root_node), rng(seed) {}

void CodeRenderer::render(std::ostream& os) {
    this->root_node->render(*this, os);
}

bool CodeRenderer::addId(const std::string& name) {
    if(this->commit_table.count(name) > 0)
        return false;
    std::unordered_set<std::string>& current_scope = this->generated_ids.top();
    if(current_scope.count(name))
        return false;
    this->commit_table.insert(name);
    return true;
}

void CodeRenderer::commitIds() {
    this->generated_ids.top() = this->commit_table;
    this->commit_table.clear();
}

void CodeRenderer::enterScope() {
    this->commit_table.clear();
    this->generated_ids.push({});
}

void CodeRenderer::exitScope() {
    this->commit_table.clear();
    this->generated_ids.pop();
}

void CodeRenderer::setIndent(size_t x) {
    this->indent = x;
}

size_t CodeRenderer::getIndent() const {
    return this->indent;
}