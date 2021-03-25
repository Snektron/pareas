#ifndef _PAREAS_TEST_CODERENDERER_HPP
#define _PAREAS_TEST_CODERENDERER_HPP

#include <iosfwd>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <stack>

class RenderState;

class CodeRenderer {
    private:
        const RenderState* root_node;
        std::mt19937 rng;
        size_t indent = 0;

        std::unordered_set<std::string> commit_table;
        std::stack<std::unordered_set<std::string>> generated_ids;
    public:
        CodeRenderer(const RenderState*, size_t);

        void render(std::ostream&);

        auto& get_rng() {
            return this->rng;
        }

        bool addId(const std::string&);
        void enterScope();
        void exitScope();
        void commitIds();
        void setIndent(size_t);
        size_t getIndent() const;
};

#endif
