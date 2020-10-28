#ifndef _PAREAS_GRAPH_H
#define _PAREAS_GRAPH_H

#include <vector>
#include <optional>
#include <cstdint>
#include <cassert>

struct Graph {
    using VertexId = size_t;

    std::vector<std::vector<VertexId>> vertices;

    auto add_vertex() -> VertexId {
        VertexId v = this->vertices.size();
        this->vertices.emplace_back();
        return v;
    }

    auto add_edge(VertexId a, VertexId b) -> void {
        this->vertices[a].push_back(b);
    }

    auto edges(VertexId v) -> std::vector<VertexId>& {
        return this->vertices[v];
    }

    auto edges(VertexId v) const -> const std::vector<VertexId>& {
        return this->vertices[v];
    }

    auto topological_ordering() -> std::optional<std::vector<VertexId>>;
};

struct Ufds {
    using ComponentId = size_t;

    const Graph* graph;
    std::vector<int64_t> parents;
    size_t components;

    Ufds(const Graph* graph):
        graph(graph), parents(graph->vertices.size(), -1),
        components(graph->vertices.size()) {}

    auto find(Graph::VertexId v) -> ComponentId;
    auto unite(Graph::VertexId a, Graph::VertexId b) -> void;
    auto component_size(Graph::VertexId v) -> size_t;

private:
    void grow_if_required(Graph::VertexId v);
};

#endif
