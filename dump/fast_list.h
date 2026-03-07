#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <list>

struct fast_list {
    static const int NULL_ID = -1;

    void* high_water_mark;
    void* buffer_start;
    void* buffer_end;

    struct ListNode {
        int value;
        int next_node;
        int prev_node;
        int side;
    };

    struct List {
        size_t count;
        size_t max_node_count;
        int first_node;
        int last_node;
    };

    std::vector<int> freeList;
    bool* nodeMap;
    List list;

    fast_list();
    fast_list(size_t max_node_size);
    ~fast_list();

    ListNode* resolveAddress(int id);
    int getNodeId(ListNode* node);
    void invalidateNode(ListNode* node);
    bool addNode(int value);
    bool addNodeFront(int value);
    bool consumeNode();
    bool consumeNodeBack();
    bool removeNode(int node_id);
};
