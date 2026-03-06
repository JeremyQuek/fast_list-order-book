#include <stdio.h>
#include <vector>
#include <cstdlib>   
#include <chrono>
#include <string.h> 
#include <list>
#include <random>

#include "fast_list.h"

using namespace std;
int main(void) {
    using Order = OrderBook::Order;


// =========================================================
    // TEST 1: Add a bunch & Consume a bunch
    // =========================================================
    {
        OrderBook list;
        int passed = 0, failed = 0;
        auto check = [&](bool condition, const char* test_name) {
            if (condition) { printf("PASS: %s\n", test_name); passed++; }
            else           { printf("FAIL: %s\n", test_name); failed++; }
        };

        list.addOrder(10);
        list.addOrder(20);
        list.addOrder(30);
        list.addOrder(40);
        list.addOrder(50);
        check(list.book.count == 5, "T1: Add 5 orders -> count == 5");

        list.consumeOrder();
        list.consumeOrder();
        list.consumeOrder();
        list.consumeOrder();
        list.consumeOrder();
        check(list.book.count == 0,                        "T1: Consume 5 -> count == 0");
        check(list.book.first_order == OrderBook::NULL_ID,    "T1: first_order is NULL_ID");
        check(list.book.last_order == OrderBook::NULL_ID,     "T1: last_order is NULL_ID");
        check(list.freeList.size() == 5,                   "T1: freeList has 5 recycled nodes");

        printf("T1 Results: %d passed, %d failed\n\n", passed, failed);
    }

    // =========================================================
    // TEST 2: Add a bunch & Remove by ID, check no consumption of removed nodes
    // =========================================================
    {
        OrderBook list;
        int passed = 0, failed = 0;
        auto check = [&](bool condition, const char* test_name) {
            if (condition) { printf("PASS: %s\n", test_name); passed++; }
            else           { printf("FAIL: %s\n", test_name); failed++; }
        };

        list.addOrder(100); int id0 = list.book.last_order;
        list.addOrder(200); int id1 = list.book.last_order;
        list.addOrder(300); int id2 = list.book.last_order;
        list.addOrder(400); int id3 = list.book.last_order;
        check(list.book.count == 4, "T2: Add 4 orders -> count == 4");

        list.removeOrder(id0);
        list.removeOrder(id1);
        list.removeOrder(id2);
        list.removeOrder(id3);
        check(list.book.count == 0,                        "T2: Remove 4 -> count == 0");
        check(list.book.first_order == OrderBook::NULL_ID,    "T2: first_order is NULL_ID");
        check(list.book.last_order == OrderBook::NULL_ID,     "T2: last_order is NULL_ID");
        check(list.freeList.size() == 4,                   "T2: freeList has 4 recycled nodes");

        bool result = list.consumeOrder();
        check(!result,              "T2: Consume from emptied list returns false");
        check(list.book.count == 0, "T2: Count still 0 after failed consume");

        printf("T2 Results: %d passed, %d failed\n\n", passed, failed);
    }

    // =========================================================
    // TEST 3: Mixed add, consume, remove + special cases
    // =========================================================
    {
        OrderBook list;
        int passed = 0, failed = 0;
        auto check = [&](bool condition, const char* test_name) {
            if (condition) { printf("PASS: %s\n", test_name); passed++; }
            else           { printf("FAIL: %s\n", test_name); failed++; }
        };

        list.addOrder(1);
        list.addOrder(2);
        list.addOrder(3);
        list.addOrder(4);
        list.addOrder(5);

        // Walk to value=3: first->next->next
        int mid_id = list.resolveAddress(list.book.first_order)->next_order;
        mid_id     = list.resolveAddress(mid_id)->next_order;

        list.consumeOrder();
        check(list.resolveAddress(list.book.first_order)->value == 2, "T3: After consume, front is value=2");

        list.removeOrder(mid_id);
        check(list.book.count == 3, "T3: After consume+remove, count == 3");

        list.consumeOrder();
        list.consumeOrder();
        list.consumeOrder();
        check(list.book.count == 0,                        "T3: Consume remaining -> count == 0");
        check(list.book.first_order == OrderBook::NULL_ID,    "T3: first_order is NULL_ID at end");
        check(list.book.last_order == OrderBook::NULL_ID,     "T3: last_order is NULL_ID at end");
        check(list.freeList.size() == 5,                   "T3: All 5 nodes returned to freeList");

        // Special case 1: consume from empty
        bool r1 = list.consumeOrder();
        check(!r1, "SC1: Consume from empty list returns false");

        // Special case 2: double remove
        list.addOrder(999);
        int id = list.book.last_order;
        bool r2 = list.removeOrder(id);
        bool r3 = list.removeOrder(id);
        check(r2,                   "SC2: First remove returns true");
        check(!r3,                  "SC2: Second remove returns false");
        check(list.book.count == 0, "SC2: Count still 0 after double remove");

        printf("T3 Results: %d passed, %d failed\n\n", passed, failed);
    }

    // =========================================================
    // TEST 4: Value & Order Consistency vs Reference Vector
    // =========================================================
    {
        OrderBook list;
        vector<int> reference;
        int passed = 0, failed = 0;

        auto check = [&](bool condition, const char* test_name) {
            if (condition) { printf("PASS: %s\n", test_name); passed++; }
            else           { printf("FAIL: %s\n", test_name); failed++; }
        };

        auto sync_check = [&]() {
            if (list.book.count != reference.size()) return false;
            int cur_id = list.book.first_order;
            for (int val : reference) {
                if (cur_id == OrderBook::NULL_ID) return false;
                if (list.resolveAddress(cur_id)->value != val) return false;
                cur_id = list.resolveAddress(cur_id)->next_order;
            }
            return cur_id == OrderBook::NULL_ID;
        };

        for (int i : {0, 10, 20, 30, 40, 50, 60, 70, 80}) {
            list.addOrder(i);
            reference.push_back(i);
        }
        check(sync_check(), "T4: Initial sequence matches reference");

        list.removeOrder(2);
        reference.erase(reference.begin() + 2);
        check(sync_check(), "T4: Order correct after removing middle (20)");

        list.removeOrder(0);
        reference.erase(reference.begin());
        check(sync_check(), "T4: Order correct after removing front (0)");

        list.consumeOrder();
        reference.erase(reference.begin());
        check(sync_check(), "T4: Order correct after consume");

        list.addOrder(90);
        reference.push_back(90);
        check(sync_check(), "T4: Order correct after add to recycled slot");

        printf("Final state: ");
        int cur_id = list.book.first_order;
        while (cur_id != OrderBook::NULL_ID) {
            OrderBook::Order* cur = list.resolveAddress(cur_id);
            printf("%d ", cur->value);
            cur_id = cur->next_order;
        }
        printf("\n");

        printf("T4 Results: %d passed, %d failed\n\n", passed, failed);
    }


    // =========================================================
    // BENCHMARK: std::list comparison
    // =========================================================
    {
        const int N = 1000000;

        struct Order {
            int value;
            int next_order;
            int prev_order;
        };

        list<Order> lst;

        auto time_ms = [](auto fn) {
            auto start = chrono::high_resolution_clock::now();
            fn();
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration<double, milli>(end - start).count();
        };

        // 1. Add N
        double t_add = time_ms([&]() {
            for (int i = 0; i < N; i++) lst.push_back({i, -1, -1});
        });

        // 2. Remove every other
        double t_remove = time_ms([&]() {
            auto it = lst.begin();
            while (it != lst.end()) {
                it = lst.erase(it);
                if (it != lst.end()) ++it;
            }
        });

        // 3. Consume front
        double t_consume = time_ms([&]() {
            while (!lst.empty()) lst.pop_front();
        });

        // 4. Mixed
        double t_mixed = time_ms([&]() {
            for (int i = 0; i < N; i++) lst.push_back({i, -1, -1});
            for (int i = 0; i < N; i++) {
                if      (i % 3 == 0) { if (!lst.empty()) lst.pop_front(); }
                else if (i % 3 == 1) { if (!lst.empty()) lst.pop_back(); }
                else                 lst.push_back({i, -1, -1});
            }
        });

        printf("\n--- std::list baseline ---\n");
        printf("Add    %7d orders: %8.2f ms  (%6.0f ns/op)\n", N,   t_add,     t_add     * 1e6 / N);
        printf("Remove %7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_remove,  t_remove  * 1e6 / (N/2));
        printf("Consume%7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_consume, t_consume * 1e6 / (N/2));
        printf("Mixed  %7d ops:    %8.2f ms  (%6.0f ns/op)\n", N,   t_mixed,   t_mixed   * 1e6 / N);
        printf("\n");
        printf("Add:     %6.1fM ops/sec\n", (N     / (t_add     / 1000.0)) / 1e6);
        printf("Remove:  %6.1fM ops/sec\n", ((N/2) / (t_remove  / 1000.0)) / 1e6);
        printf("Consume: %6.1fM ops/sec\n", ((N/2) / (t_consume / 1000.0)) / 1e6);
        printf("Mixed:   %6.1fM ops/sec\n", (N     / (t_mixed   / 1000.0)) / 1e6);
        printf("\n");
    }


    // =========================================================
    // BENCHMARK: std::list randomized
    // =========================================================
    {
        const int N = 1000000;

        struct Order {
            int value;
            int next_order;
            int prev_order;
        };

        list<Order> lst;

        auto time_ms = [](auto fn) {
            auto start = chrono::high_resolution_clock::now();
            fn();
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration<double, milli>(end - start).count();
        };

        // 1. Add N
        double t_add = time_ms([&]() {
            for (int i = 0; i < N; i++) lst.push_back({i, -1, -1});
        });

        // 2. Remove every other
        double t_remove = time_ms([&]() {
            auto it = lst.begin();
            while (it != lst.end()) {
                it = lst.erase(it);
                if (it != lst.end()) ++it;
            }
        });

        // 3. Consume front
        double t_consume = time_ms([&]() {
            while (!lst.empty()) lst.pop_front();
        });

        // 4. Randomized mixed
        double t_mixed = time_ms([&]() {
            std::mt19937 rng(42);
            std::uniform_int_distribution<int> op_dist(0, 2);
            std::uniform_int_distribution<int> val_dist(0, N);

            for (int i = 0; i < N; ++i) {
                int op = op_dist(rng);
                if (op == 0) {
                    lst.push_back({val_dist(rng), -1, -1});
                } else if (op == 1) {
                    if (!lst.empty()) lst.erase(lst.begin());
                } else {
                    if (!lst.empty()) lst.pop_front();
                }
            }
        });

        printf("\n--- std::list randomized ---\n");
        printf("Add    %7d orders: %8.2f ms  (%6.0f ns/op)\n", N,   t_add,     t_add     * 1e6 / N);
        printf("Remove %7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_remove,  t_remove  * 1e6 / (N/2));
        printf("Consume%7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_consume, t_consume * 1e6 / (N/2));
        printf("Mixed  %7d ops:    %8.2f ms  (%6.0f ns/op)\n", N,   t_mixed,   t_mixed   * 1e6 / N);
        printf("\n");
        printf("Add:     %6.1fM ops/sec\n", (N     / (t_add     / 1000.0)) / 1e6);
        printf("Remove:  %6.1fM ops/sec\n", ((N/2) / (t_remove  / 1000.0)) / 1e6);
        printf("Consume: %6.1fM ops/sec\n", ((N/2) / (t_consume / 1000.0)) / 1e6);
        printf("Mixed:   %6.1fM ops/sec\n", (N     / (t_mixed   / 1000.0)) / 1e6);
        printf("\n");
    }


    // =========================================================
    // BENCHMARK: OrderBook::Fast_list baseline
    // =========================================================
    {
        printf("\n--- OrderBook::Fast_list baseline ---\n");
        OrderBook list;
        const int N = 1000000;

        auto time_ms = [](auto fn) {
            auto start = chrono::high_resolution_clock::now();
            fn();
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration<double, milli>(end - start).count();
        };

        // 1. Add N orders
        double t_add = time_ms([&]() {
            for (int i = 0; i < N; i++) list.addOrder(i);
        });

        // 2. Remove every other order
        double t_remove = time_ms([&]() {
            for (int i = 0; i < N; i += 2) list.removeOrder(i);
        });

        // 3. Consume remaining N/2
        double t_consume = time_ms([&]() {
            while (list.book.count > 0) list.consumeOrder();
        });

        // 4. Mixed: add N, then interleave add/consume/remove
        double t_mixed = time_ms([&]() {
            for (int i = 0; i < N; i++) list.addOrder(i);
            for (int i = 0; i < N; i++) {
                if      (i % 3 == 0) list.consumeOrder();
                else if (i % 3 == 1) list.removeOrder(list.book.last_order);
                else                 list.addOrder(i);
            }
        });

        printf("Add    %7d orders: %8.2f ms  (%6.0f ns/op)\n", N,   t_add,     t_add     * 1e6 / N);
        printf("Remove %7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_remove,  t_remove  * 1e6 / (N/2));
        printf("Consume%7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_consume, t_consume * 1e6 / (N/2));
        printf("Mixed  %7d ops:    %8.2f ms  (%6.0f ns/op)\n", N,   t_mixed,   t_mixed   * 1e6 / N);
        printf("\n");
        printf("Add:     %6.1fM ops/sec\n", (N     / (t_add     / 1000.0)) / 1e6);
        printf("Remove:  %6.1fM ops/sec\n", ((N/2) / (t_remove  / 1000.0)) / 1e6);
        printf("Consume: %6.1fM ops/sec\n", ((N/2) / (t_consume / 1000.0)) / 1e6);
        printf("Mixed:   %6.1fM ops/sec\n", (N     / (t_mixed   / 1000.0)) / 1e6);
        printf("\n");
    }

    printf("sizeof(Order) = %zu bytes\n", sizeof(OrderBook::Order));


    // =========================================================
    // BENCHMARK: OrderBook::Fast_list randomized
    // =========================================================
    {
        printf("\n--- OrderBook::Fast_list randomized ---\n");
        OrderBook list;
        const int N = 1000000;

        auto time_ms = [](auto fn) {
            auto start = chrono::high_resolution_clock::now();
            fn();
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration<double, milli>(end - start).count();
        };

        // 1. Add N orders
        double t_add = time_ms([&]() {
            for (int i = 0; i < N; i++) list.addOrder(i);
        });

        // 2. Remove every other order
        double t_remove = time_ms([&]() {
            for (int i = 0; i < N; i += 2) list.removeOrder(i);
        });

        // 3. Consume remaining N/2
        double t_consume = time_ms([&]() {
            while (list.book.count > 0) list.consumeOrder();
        });

        // 4. Randomized mixed
        double t_mixed = time_ms([&]() {
            std::mt19937 rng(42);
            std::uniform_int_distribution<int> op_dist(0, 2);
            std::uniform_int_distribution<int> val_dist(0, N);

            for (int i = 0; i < N; ++i) {
                int op = op_dist(rng);
                if (op == 0) {
                    list.addOrder(val_dist(rng));
                } else if (op == 1) {
                    list.removeOrder(val_dist(rng));
                } else {
                    list.consumeOrder();
                }
            }
        });

        printf("Add    %7d orders: %8.2f ms  (%6.0f ns/op)\n", N,   t_add,     t_add     * 1e6 / N);
        printf("Remove %7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_remove,  t_remove  * 1e6 / (N/2));
        printf("Consume%7d orders: %8.2f ms  (%6.0f ns/op)\n", N/2, t_consume, t_consume * 1e6 / (N/2));
        printf("Mixed  %7d ops:    %8.2f ms  (%6.0f ns/op)\n", N,   t_mixed,   t_mixed   * 1e6 / N);
        printf("\n");
        printf("Add:     %6.1fM ops/sec\n", (N     / (t_add     / 1000.0)) / 1e6);
        printf("Remove:  %6.1fM ops/sec\n", ((N/2) / (t_remove  / 1000.0)) / 1e6);
        printf("Consume: %6.1fM ops/sec\n", ((N/2) / (t_consume / 1000.0)) / 1e6);
        printf("Mixed:   %6.1fM ops/sec\n", (N     / (t_mixed   / 1000.0)) / 1e6);
        printf("\n");
    }
}