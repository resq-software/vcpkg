/**
 * Copyright 2026 ResQ Software
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
/**
 * @brief Graph data structures and pathfinding algorithms
 *
 * Provides generic graph representation with common pathfinding algorithms.
 * Supports both unweighted and weighted edges with Dijkstra and A* search.
 *
 * @note Uses adjacency list representation for memory efficiency
 * @note Id type must be hashable and equality-comparable
 *
 * @par Example:
 * @code
 * Graph<std::string> g;
 * g.add_edge("A", "B", 1.0);
 * g.add_edge("B", "C", 2.0);
 *
 * auto result = g.dijkstra("A", "C");
 * if (result) {
 *     std::cout << "Path: ";
 *     for (const auto& node : result->path) {
 *         std::cout << node << " ";
 *     }
 * }
 * @endcode
 */

#include <algorithm>
#include <deque>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

namespace resq::dsa {

/**
 * @brief Generic directed weighted graph
 *
 * Implements an adjacency list graph with support for:
 * - Breadth-first search (unweighted shortest path)
 * - Dijkstra's algorithm (weighted shortest path)
 * - A* search (heuristic-guided pathfinding)
 *
 * @tparam Id Vertex identifier type (must be hashable)
 * @tparam IdHash Hash function for Id type (default: std::hash<Id>)
 *
 * @note Edge weights must be non-negative for Dijkstra and A*
 * @note Self-loops and duplicate edges are supported
 */
template <typename Id, typename IdHash = std::hash<Id>>
class Graph {
public:
    /**
     * @brief Add a directed edge to the graph
     *
     * @param from Source vertex
     * @param to Destination vertex
     * @param w Edge weight (default: 1.0)
     *
     * @post Edge from->to exists with weight w
     * @note If edge already exists, adds additional parallel edge
     *
     * @par Example:
     * @code
     * Graph<int> g;
     * g.add_edge(1, 2, 5.0);   // Weighted edge
     * g.add_edge(2, 3);        // Unweighted edge (weight=1)
     * @endcode
     */
    void add_edge(const Id& from, const Id& to, double w = 1.0) { adj_[from].push_back({to, w}); }

    /**
     * @brief Breadth-first search from a start vertex
     *
     * Returns all vertices reachable from start in BFS order.
     * Uses O(V + E) time where V = vertices, E = edges.
     *
     * @param start Starting vertex
     * @return Vector of visited vertices in BFS order
     *
     * @post Returns empty vector if start not in graph
     * @note Does not consider edge weights (treats all as weight 1)
     */
    [[nodiscard]] std::vector<Id> bfs(const Id& start) const {
        std::unordered_map<Id, bool, IdHash> seen;
        std::deque<Id> q;
        std::vector<Id> result;
        seen[start] = true;
        q.push_back(start);
        while (!q.empty()) {
            auto node = q.front();
            q.pop_front();
            result.push_back(node);
            auto it = adj_.find(node);
            if (it == adj_.end()) continue;
            for (auto& [to, _] : it->second)
                if (!seen[to]) {
                    seen[to] = true;
                    q.push_back(to);
                }
        }
        return result;
    }

    /**
     * @brief Result of a shortest path search
     */
    struct PathResult {
        /// Vertices on the path from start to end
        std::vector<Id> path;
        /// Total cost of the path
        double cost;
    };

    /**
     * @brief Dijkstra's algorithm for shortest path
     *
     * Finds the shortest path between two vertices using Dijkstra's
     * algorithm with a binary heap priority queue.
     *
     * @param start Starting vertex
     * @param end Destination vertex
     * @return PathResult if path exists, std::nullopt otherwise
     *
     * @post On success, path contains vertices from start to end
     * @post cost equals sum of edge weights along path
     *
     * @note Time complexity: O((V + E) log V)
     * @note Edge weights must be non-negative
     *
     * @par Example:
     * @code
     * Graph<std::string> g;
     * g.add_edge("home", "store", 5.0);
     * g.add_edge("store", "work", 3.0);
     *
     * auto result = g.dijkstra("home", "work");
     * if (result) {
     *     // result->path = ["home", "store", "work"]
     *     // result->cost = 8.0
     * }
     * @endcode
     */
    [[nodiscard]] std::optional<PathResult> dijkstra(const Id& start, const Id& end) const {
        using P = std::pair<double, Id>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        std::unordered_map<Id, double, IdHash> dist;
        std::unordered_map<Id, Id, IdHash> prev;
        dist[start] = 0;
        pq.push({0, start});
        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (u == end) break;
            if (dist.count(u) && d > dist[u]) continue;
            auto it = adj_.find(u);
            if (it == adj_.end()) continue;
            for (auto& [v, w] : it->second) {
                double alt = d + w;
                if (!dist.count(v) || alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                    pq.push({alt, v});
                }
            }
        }
        if (!dist.count(end)) return std::nullopt;
        std::vector<Id> path;
        Id cur = end;
        while (true) {
            path.push_back(cur);
            auto it = prev.find(cur);
            if (it == prev.end()) break;
            cur = it->second;
        }
        std::reverse(path.begin(), path.end());
        return PathResult{path, dist[end]};
    }

    /**
     * @brief A* pathfinding algorithm
     *
     * Finds shortest path using heuristic-guided search. More efficient
     * than Dijkstra when a good heuristic is available.
     *
     * @param start Starting vertex
     * @param end Destination vertex
     * @param h Heuristic function: estimated cost from node to goal
     *          Must be admissible (never overestimate)
     *
     * @return PathResult if path exists, std::nullopt otherwise
     *
     * @post On success, path contains vertices from start to end
     * @post cost equals g-score (actual cost, not f-score)
     *
     * @note Time complexity: O(E log V) in best case with good heuristic
     * @note Heuristic must be admissible for optimal results
     * @note Edge weights must be non-negative
     *
     * @par Example:
     * @code
     * Graph<int> g;
     * // Add edges representing a grid...
     *
     * // Manhattan distance heuristic for grid
     * auto h = [&](int from, int to) {
     *     return std::abs(from - to);  // Simplified
     * };
     *
     * auto result = g.astar(0, 100, h);
     * @endcode
     */
    [[nodiscard]] std::optional<PathResult> astar(
        const Id& start, const Id& end, std::function<double(const Id&, const Id&)> h) const {
        using P = std::pair<double, Id>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        std::unordered_map<Id, double, IdHash> g;
        std::unordered_map<Id, Id, IdHash> prev;
        g[start] = 0;
        pq.push({h(start, end), start});
        while (!pq.empty()) {
            auto [_, u] = pq.top();
            pq.pop();
            if (u == end) {
                std::vector<Id> path;
                Id cur = end;
                while (true) {
                    path.push_back(cur);
                    auto it = prev.find(cur);
                    if (it == prev.end()) break;
                    cur = it->second;
                }
                std::reverse(path.begin(), path.end());
                return PathResult{path, g[end]};
            }
            double cost = g.count(u) ? g[u] : std::numeric_limits<double>::infinity();
            if (g.count(u) && cost > g[u]) continue;  // stale entry
            auto it = adj_.find(u);
            if (it == adj_.end()) continue;
            for (auto& [v, w] : it->second) {
                double alt = cost + w;
                if (!g.count(v) || alt < g[v]) {
                    g[v] = alt;
                    prev[v] = u;
                    pq.push({alt + h(v, end), v});
                }
            }
        }
        return std::nullopt;
    }

private:
    /// Adjacency list: vertex -> list of (neighbor, weight) pairs
    std::unordered_map<Id, std::vector<std::pair<Id, double>>, IdHash> adj_;
};

}  // namespace resq::dsa
