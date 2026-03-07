#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string.h>

#include "fast_list.h"

fast_list::fast_list() {
    list.count = 0;
    list.first_node = NULL_ID;
    list.last_node = NULL_ID;
    list.max_node_count = 10000000;

    buffer_start = aligned_alloc(64, sizeof(ListNode) *list.max_node_count);
    memset(buffer_start, 0, sizeof(ListNode) *list.max_node_count);
    buffer_end = (char*)buffer_start + (sizeof(ListNode) *list.max_node_count);
    high_water_mark = buffer_start;

    nodeMap = (bool*)aligned_alloc(64,list.max_node_count * sizeof(bool));
    memset(nodeMap, 0,list.max_node_count * sizeof(bool));
}

fast_list::fast_list(size_t max_node_size) {
    list.count = 0;
    list.first_node = NULL_ID;
    list.last_node = NULL_ID;
    list.max_node_count = max_node_size;

    buffer_start = aligned_alloc(64, sizeof(ListNode) *list.max_node_count);
    memset(buffer_start, 0, sizeof(ListNode) *list.max_node_count);
    buffer_end = (char*)buffer_start + (sizeof(ListNode) *list.max_node_count);
    high_water_mark = buffer_start;

    nodeMap = (bool*)aligned_alloc(64,list.max_node_count * sizeof(bool));
    memset(nodeMap, 0,list.max_node_count * sizeof(bool));
}


fast_list::~fast_list() {
    free(buffer_start);
    free(nodeMap);
}

fast_list::ListNode* fast_list::resolveAddress(int id) {
    if (id == NULL_ID) return nullptr;
    return ((ListNode*)buffer_start) + id;
}

int fast_list::getNodeId(fast_list::ListNode* node) {
    return (int)(node - (ListNode*)buffer_start);
}

void fast_list::invalidateNode(fast_list::ListNode* node) {
    node->next_node = NULL_ID;
    node->prev_node = NULL_ID;
}

bool fast_list::addNodeFront(int value) {
    int new_id;

    if (!freeList.empty()) {
        new_id = freeList.back();
        freeList.pop_back();
    } else {
        if (high_water_mark >= buffer_end) return false;
        new_id = getNodeId((ListNode*)high_water_mark);
        high_water_mark = (char*)high_water_mark + sizeof(ListNode);
    }

    fast_list::ListNode* new_node = resolveAddress(new_id);
    new_node->value = value;
    new_node->next_node = NULL_ID;
    new_node->prev_node = NULL_ID;

    if (list.count == 0) {
        list.first_node = new_id;
        list.last_node = new_id;
    } else {
        ListNode* first_node = resolveAddress(list.first_node);
        first_node->prev_node = new_id;

        new_node->next_node = list.first_node;
        list.first_node = new_id;
    }

    nodeMap[new_id] = true;
    list.count++;
    return true;
}

bool fast_list::consumeNode() {
    if (list.first_node == NULL_ID) return false;
    int first_node = list.first_node;
    fast_list::ListNode* first = resolveAddress(first_node);

    if (list.count == 1) {
        list.first_node = NULL_ID;
        list.last_node = NULL_ID;
    } else {
        list.first_node = first->next_node;
        resolveAddress(list.first_node)->prev_node = NULL_ID;
    }

    nodeMap[first_node] = false;
    invalidateNode(first);
    freeList.push_back(first_node);
    list.count--;
    return true;
}


bool fast_list::addNode(int value) {
    int new_id;
    if (!freeList.empty()) {
        new_id = freeList.back();
        freeList.pop_back();
    } else {
        if (high_water_mark >= buffer_end) return false;
        new_id = getNodeId((ListNode*)high_water_mark);
        high_water_mark = (char*)high_water_mark + sizeof(ListNode);
    }

    fast_list::ListNode* new_node = resolveAddress(new_id);
    new_node->value = value;
    new_node->next_node = NULL_ID;
    new_node->prev_node = NULL_ID;

    if (list.count == 0) {
        list.first_node = new_id;
        list.last_node = new_id;
    } else {
        ListNode* prev_node = resolveAddress(list.last_node);
        prev_node->next_node = new_id;
        new_node->prev_node = list.last_node;
        list.last_node = new_id;
    }

    nodeMap[new_id] = true;
    list.count++;
    return true;
}


bool fast_list::consumeNodeBack() {
    if (list.first_node == NULL_ID) return false;

    int last_node = list.last_node;
    fast_list::ListNode* last = resolveAddress(last_node);

    if (list.count == 1) {
        list.first_node = NULL_ID;
        list.last_node = NULL_ID;
    } else {
        list.last_node = last->prev_node;
        resolveAddress(list.last_node)->next_node = NULL_ID;
    }

    nodeMap[last_node] = false;
    invalidateNode(last);
    freeList.push_back(last_node);
    list.count--;
    return true;
}

bool fast_list::removeNode(int node_id) {
    if (node_id < 0 || node_id >=list.max_node_count || !nodeMap[node_id]) return false;

    fast_list::ListNode* target = resolveAddress(node_id);
    int next_node = target->next_node;
    int prev_node = target->prev_node;

    if (list.count == 1) {
        list.first_node = NULL_ID;
        list.last_node = NULL_ID;
    } else if (node_id == list.first_node) {
        list.first_node = next_node;
        resolveAddress(next_node)->prev_node = NULL_ID;
    } else if (node_id == list.last_node) {
        list.last_node = prev_node;
        resolveAddress(prev_node)->next_node = NULL_ID;
    } else {
        resolveAddress(prev_node)->next_node = next_node;
        resolveAddress(next_node)->prev_node = prev_node;
    }

    nodeMap[node_id] = false;
    invalidateNode(target);
    freeList.push_back(node_id);
    list.count--;
    return true;
}
