#include "test/treeproperties.hpp"
#include "test/astnode.hpp"

#include <deque>
#include <utility>
#include <unordered_map>

TreeProperties::TreeProperties(ASTNode* node) : root(node) {}

size_t TreeProperties::getNodeCount() const {
    std::deque<ASTNode*> queue;

    queue.push_back(this->root);

    size_t count = 0;
    while(queue.size() > 0) {
        ASTNode* next = queue.front();
        queue.pop_front();

        ++count;
        for(ASTNode* child : next->children) {
            queue.push_back(child);
        }
    }
    return count;
}

size_t TreeProperties::getDepth() const {
    std::deque<std::pair<ASTNode*, size_t>> depth_search;

    depth_search.push_back({this->root, 1});

    size_t max_depth = 0;
    while(depth_search.size() > 0) {
        std::pair<ASTNode*, size_t> next = depth_search.front();
        depth_search.pop_front();

        if(next.second > max_depth) {
            max_depth = next.second;
        }
        for(ASTNode* child : next.first->children) {
            depth_search.push_back({child, next.second + 1});
        }
    }

    return max_depth;
}

size_t TreeProperties::getWidth() const {
    std::deque<std::pair<ASTNode*, size_t>> depth_search;

    depth_search.push_back({this->root, 1});

    size_t max_width = 0;
    size_t current_depth = 0;
    size_t current_width = 0;
    while(depth_search.size() > 0) {
        std::pair<ASTNode*, size_t> next = depth_search.front();
        depth_search.pop_front();

        if(next.second != current_depth) {
            current_depth = next.second;
            if(current_width > max_width)
                max_width = current_width;
            current_width = 1;
        }
        else
            ++current_width;
        for(ASTNode* child : next.first->children) {
            depth_search.push_back({child, next.second + 1});
        }
    }
    if(current_width > max_width) {
        max_width = current_width;
    }

    return max_width;
}

size_t TreeProperties::getFunctions() const {
    std::deque<ASTNode*> search;

    search.push_back(this->root);
    size_t func_count = 0;
    while(search.size() > 0) {
        ASTNode* next = search.front();
        search.pop_front();

        if(next->node_type == NodeType::FUNC_DECL) {
            ++func_count;
        }
        else {
            for(ASTNode* child : next->children) {
                search.push_back(child);
            }
        }
    }

    return func_count;
}

size_t TreeProperties::getMaxFuncLen() const {
    std::deque<std::pair<ASTNode*, size_t>> depth_search;

    depth_search.push_back({this->root, 0});

    std::unordered_map<size_t, size_t> func_counts;
    while(depth_search.size() > 0) {
        std::pair<ASTNode*, size_t> next = depth_search.front();
        depth_search.pop_front();

        size_t id = next.second;
        if(next.first->node_type == NodeType::FUNC_DECL) {
            id = func_counts.size() + 1;
            func_counts[id] = 1;
        }
        if(id != 0)
            ++func_counts[id];
        for(ASTNode* child : next.first->children) {
            depth_search.push_back({child, id});
        }
    }

    size_t max_nodes = 0;
    for(auto& it : func_counts) {
        if(it.second > max_nodes) {
            max_nodes = it.second;
        }
    }

    return max_nodes;
}