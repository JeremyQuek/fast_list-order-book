#include <stdio.h>
#include <vector>
#include <list>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <list>

struct OrderBook {
    static const int NULL_ID = -1;

    void* high_water_mark;
    void* buffer_start;
    void* buffer_end;

    struct Order {
        int value;
        int next_order;
        int prev_order;
        int side;
    };

    struct Book {
        size_t count;
        size_t max_order_count;
        int first_order;
        int last_order;
    };

    std::vector<int> freeList;
    bool* orderMap;
    Book book;

    OrderBook();
    OrderBook(size_t max_order_size);
    ~OrderBook();

    Order* resolveAddress(int id);
    int getOrderId(Order* order);
    void invalidateOrder(Order* order);
    bool addOrder(int value);
    bool consumeOrder();
    bool removeOrder(int order_id);
};
