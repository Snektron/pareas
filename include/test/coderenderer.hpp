#ifndef _PAREAS_TEST_CODERENDERER_HPP
#define _PAREAS_TEST_CODERENDERER_HPP

#include <iosfwd>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class RenderState;

struct SymbolInfo {
    std::string name;
    size_t category;
};

bool operator==(const SymbolInfo&, const SymbolInfo&);
bool operator!=(const SymbolInfo&, const SymbolInfo&);

namespace std {
    template<>
    struct hash<SymbolInfo> {
        size_t operator()(const SymbolInfo&) const;
    };
}

class CodeRenderer {
    private:
        const RenderState* root_node;
        std::mt19937 rng;
        size_t indent = 0;
        size_t depth;
        size_t max_depth = 0;

        std::unordered_set<SymbolInfo> commit_table;
        std::vector<std::unordered_set<SymbolInfo>> generated_ids;
    public:
        CodeRenderer(const RenderState*, size_t);

        void render(std::ostream&);

        auto& get_rng() {
            return this->rng;
        }

        bool addId(const std::string&, size_t);
        void enterScope();
        void exitScope();
        void commitIds();
        void setIndent(size_t);
        size_t getIndent() const;
        size_t getMaxDepth() const;
        void setMaxDepth(size_t);
        size_t getDepth() const;
        void setDepth(size_t);
        std::string getRandomID(size_t, bool&);
};

#endif
