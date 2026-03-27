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

// Compile: g++ -std=c++17 -I../include test_dsa.cpp -o test_dsa && ./test_dsa
#include <cassert>
#include <iostream>

#include <resq/dsa/bloom.hpp>
#include <resq/dsa/count_min.hpp>
#include <resq/dsa/graph.hpp>
#include <resq/dsa/heap.hpp>
#include <resq/dsa/trie.hpp>

struct Item {
    int id;
    double dist;
};

void test_heap() {
    resq::dsa::BoundedHeap<Item> h(3, [](const Item& x) { return x.dist; });
    h.insert({1, 10});
    h.insert({2, 2});
    h.insert({3, 7});
    h.insert({4, 1});   // evicts id=1
    h.insert({5, 50});  // rejected
    assert(h.size() == 3);
    auto sorted = h.to_sorted();
    assert(sorted[0].id == 4 && sorted[1].id == 2 && sorted[2].id == 3);
    std::cout << "heap: PASS\n";
}

void test_graph() {
    resq::dsa::Graph<std::string> g;
    g.add_edge("A", "B", 1);
    g.add_edge("A", "C", 4);
    g.add_edge("B", "C", 2);
    g.add_edge("C", "D", 1);
    auto bfs = g.bfs("A");
    std::vector<std::string> expected_bfs{"A", "B", "C", "D"};
    assert(bfs == expected_bfs);
    auto r = g.dijkstra("A", "D");
    assert(r.has_value() && r->cost == 4.0);
    std::vector<std::string> expected_path{"A", "B", "C", "D"};
    assert(r->path == expected_path);
    assert(!g.dijkstra("A", "Z").has_value());
    std::cout << "graph: PASS\n";
}

void test_trie() {
    resq::dsa::Trie t;
    t.insert("drone");
    t.insert("droning");
    assert(t.search("drone") && !t.search("dron") && t.search("droning"));
    auto rk = resq::dsa::rabin_karp("ababab", "ab");
    std::vector<std::size_t> expected_rk{0, 2, 4};
    assert(rk == expected_rk);
    std::cout << "trie+rk: PASS\n";
}

void test_bloom() {
    resq::dsa::BloomFilter bf(500, 0.01);
    for (int i = 0; i < 100; ++i) bf.add("item-" + std::to_string(i));
    bool ok = true;
    for (int i = 0; i < 100; ++i) ok &= bf.has("item-" + std::to_string(i));
    assert(ok && !bf.has("ghost-item"));
    std::cout << "bloom: PASS\n";
}

void test_count_min() {
    resq::dsa::CountMinSketch cms(0.01, 0.01);
    cms.increment("a", 5);
    cms.increment("a", 3);
    cms.increment("b", 1);
    assert(cms.estimate("a") >= 8 && cms.estimate("b") >= 1);
    assert(cms.estimate("ghost") == 0);
    std::cout << "count_min: PASS\n";
}

int main() {
    test_heap();
    test_graph();
    test_trie();
    test_bloom();
    test_count_min();
    std::cout << "\nAll C++ DSA tests passed.\n";
    return 0;
}
