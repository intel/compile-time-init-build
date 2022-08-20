#pragma once

#include <iterator>

template<typename NodeType>
class DoubleLinkedList {
public:
    struct iterator {
        using difference_type = void;
        using value_type = NodeType;
        using pointer = NodeType *;
        using reference = NodeType &;
        using iterator_category = std::forward_iterator_tag;

    private:
        NodeType * node;

    public:
        constexpr iterator(
            NodeType * node
        )
            : node(node)
        {
            // pass
        }

        constexpr NodeType & operator*() {
            return *node;
        }

        constexpr NodeType * operator->() {
            return node;
        }

        constexpr iterator & operator++() {
            node = node->next;
            return *this;
        }

        constexpr iterator operator++(int) {
            iterator const oldIterator(node);
            node = node->next;
            return oldIterator;
        }

        constexpr bool operator==(iterator const & rhs) const {
            return node == rhs.node;
        }

        constexpr bool operator!=(iterator const & rhs) const {
            return !((*this) == rhs);
        }
    };

    using value_type = NodeType;
    using reference = NodeType &;
    using const_reference = const NodeType &;
    using pointer = NodeType *;
    using const_pointer = const NodeType *;
    using const_iterator = const iterator;
    
private:
    NodeType * head;
    NodeType * tail;

public:
    constexpr DoubleLinkedList()
        : head{nullptr}
        , tail{nullptr}
    {}

    constexpr iterator begin() {
        return iterator(head);
    }

    constexpr iterator end() {
        return iterator(nullptr);
    }

    void pushBack(NodeType * const n) {
        if (head == nullptr) {
            head = n;
            n->prev = nullptr;
        } else {
            tail->next = n;
            n->prev = tail;
        }

        n->next = nullptr;
        tail = n;
    }

    NodeType * popFront() {
        NodeType * const poppedNode = head;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        } else {
            head->prev = nullptr;
        }

        return poppedNode;
    }

    [[nodiscard]] bool isEmpty() const {
        return head == nullptr;
    }

    void remove(NodeType * const removedNode) {
        NodeType * const nextNode = removedNode->next;
        NodeType * const prevNode = removedNode->prev;

        if (prevNode == nullptr) {
            head = nextNode;
        } else {
            prevNode->next = nextNode;
        }

        if (nextNode == nullptr) {
            tail = prevNode;
        } else {
            nextNode->prev = prevNode;
        }
    }
};