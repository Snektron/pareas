#include "graph.hpp"
#include <utility>

auto Graph::topological_ordering() -> std::optional<std::vector<VertexId>> {
    auto permanent_mark = std::vector<bool>(this->vertices.size(), false);
    auto temporary_mark = std::vector<bool>(this->vertices.size(), false);
    auto order = std::vector<VertexId>();
    order.reserve(this->vertices.size());

    auto dfs = [&](VertexId v, const auto& dfs) {
        if (permanent_mark[v]) {
            return false;
        } else if (temporary_mark[v]) {
            // Not a DAG
            return true;
        }

        temporary_mark[v] = true;
        for (const auto n : this->edges(v)) {
            if (dfs(n, dfs)) {
                return true;
            }
        }

        temporary_mark[v] = false;
        permanent_mark[v] = true;
        order.push_back(v);
        return false;
    };

    for (VertexId v = 0; v < this->vertices.size(); ++v) {
        if (!permanent_mark[v]) {
            if (dfs(v, dfs)) {
                return std::nullopt;
            }
        }
    }

    std::reverse(order.begin(), order.end());
    return order;
}

auto Ufds::find(Graph::VertexId v) -> ComponentId {
    this->grow_if_required(v);
    if (this->parents[v] < 0) {
        return v;
    } else {
        return this->parents[v] = this->find(this->parents[v]);
    }
}

auto Ufds::unite(Graph::VertexId a, Graph::VertexId b) -> void {
    auto ac = this->find(a);
    auto bc = this->find(b);
    if (ac == bc) {
        return;
    }

    if (this->parents[ac] > this->parents[bc]) {
        std::swap(ac, bc);
    }

    this->parents[ac] += this->parents[bc];
    this->parents[bc] = ac;
    --this->components;
}

auto Ufds::component_size(Graph::VertexId v) -> size_t {
    return -this->parents[this->find(v)];
}

void Ufds::grow_if_required(Graph::VertexId v) {
    if (this->parents.size() <= v) {
        this->components += v + 1 - this->parents.size();
        this->parents.resize(v + 1, -1);
    }
}
