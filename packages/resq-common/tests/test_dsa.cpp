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

#include <gtest/gtest.h>

#include <resq/dsa/bloom.hpp>
#include <resq/dsa/count_min.hpp>
#include <resq/dsa/graph.hpp>
#include <resq/dsa/heap.hpp>
#include <resq/dsa/trie.hpp>

struct Item {
    int id;
    double dist;
};

TEST(BoundedHeap, InsertAndEvict) {
    resq::dsa::BoundedHeap<Item> h(3, [](const Item& x) { return x.dist; });
    h.insert({1, 10});
    h.insert({2, 2});
    h.insert({3, 7});
    h.insert({4, 1});   // evicts id=1
    h.insert({5, 50});  // rejected
    EXPECT_EQ(h.size(), 3);
    auto sorted = h.to_sorted();
    EXPECT_EQ(sorted[0].id, 4);
    EXPECT_EQ(sorted[1].id, 2);
    EXPECT_EQ(sorted[2].id, 3);
}

TEST(Graph, BFS) {
    resq::dsa::Graph<std::string> g;
    g.add_edge("A", "B", 1);
    g.add_edge("A", "C", 4);
    g.add_edge("B", "C", 2);
    g.add_edge("C", "D", 1);
    auto bfs = g.bfs("A");
    std::vector<std::string> expected{"A", "B", "C", "D"};
    EXPECT_EQ(bfs, expected);
}

TEST(Graph, Dijkstra) {
    resq::dsa::Graph<std::string> g;
    g.add_edge("A", "B", 1);
    g.add_edge("A", "C", 4);
    g.add_edge("B", "C", 2);
    g.add_edge("C", "D", 1);
    auto r = g.dijkstra("A", "D");
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->cost, 4.0);
    std::vector<std::string> expected_path{"A", "B", "C", "D"};
    EXPECT_EQ(r->path, expected_path);
}

TEST(Graph, DijkstraUnreachable) {
    resq::dsa::Graph<std::string> g;
    g.add_edge("A", "B", 1);
    EXPECT_FALSE(g.dijkstra("A", "Z").has_value());
}

TEST(Trie, SearchInserted) {
    resq::dsa::Trie t;
    t.insert("drone");
    t.insert("droning");
    EXPECT_TRUE(t.search("drone"));
    EXPECT_FALSE(t.search("dron"));
    EXPECT_TRUE(t.search("droning"));
}

TEST(Trie, RabinKarp) {
    auto rk = resq::dsa::rabin_karp("ababab", "ab");
    std::vector<std::size_t> expected{0, 2, 4};
    EXPECT_EQ(rk, expected);
}

TEST(BloomFilter, AddAndCheck) {
    resq::dsa::BloomFilter bf(500, 0.01);
    for (int i = 0; i < 100; ++i) bf.add("item-" + std::to_string(i));
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(bf.has("item-" + std::to_string(i))) << "Missing item-" << i;
    }
    EXPECT_FALSE(bf.has("ghost-item"));
}

TEST(CountMinSketch, IncrementAndEstimate) {
    resq::dsa::CountMinSketch cms(0.01, 0.01);
    cms.increment("a", 5);
    cms.increment("a", 3);
    cms.increment("b", 1);
    EXPECT_GE(cms.estimate("a"), 8);
    EXPECT_GE(cms.estimate("b"), 1);
    EXPECT_EQ(cms.estimate("ghost"), 0);
}
