#pragma once


template<typename NodeType>
class LinkedList {
private:
    NodeType * head;
    NodeType * tail;

public:
    constexpr LinkedList()
        : head{nullptr}
        , tail{nullptr}
    {}

    void pushBack(NodeType * n) {
        if (head == nullptr) {
            head = n;
        } else {
            tail->next = n;
        }

        n->next = nullptr;
        tail = n;
    }

    NodeType * popFront() {
        NodeType * poppedNode = head;
        head = head->next;
        return poppedNode;
    }

    [[nodiscard]] bool isEmpty() const {
        return head == nullptr;
    }

    NodeType * removeNext(NodeType * n) {
        NodeType * poppedNode = n->next;
        n->next = poppedNode->next;
        return poppedNode;
    }
};