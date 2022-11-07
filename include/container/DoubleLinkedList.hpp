#pragma once

#include <iterator>

template <typename NodeType> class DoubleLinkedList {
  public:
    struct iterator {
        using difference_type = void;
        using value_type = NodeType;
        using pointer = NodeType *;
        using reference = NodeType &;
        using iterator_category = std::forward_iterator_tag;

      private:
        NodeType *node;

      public:
        constexpr explicit iterator(NodeType *n) : node(n) {}

        constexpr auto operator*() -> NodeType & { return *node; }

        constexpr auto operator->() -> NodeType * { return node; }

        constexpr auto operator++() -> iterator & {
            node = node->next;
            return *this;
        }

        constexpr auto operator++(int) -> iterator {
            iterator const oldIterator(node);
            node = node->next;
            return oldIterator;
        }

        constexpr auto operator==(iterator const &rhs) const -> bool {
            return node == rhs.node;
        }

        constexpr auto operator!=(iterator const &rhs) const -> bool {
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
    NodeType *head{};
    NodeType *tail{};

  public:
    constexpr auto begin() -> iterator { return iterator(head); }

    constexpr auto end() -> iterator { return iterator(nullptr); }

    auto pushBack(NodeType *const n) -> void {
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

    auto popFront() -> NodeType * {
        NodeType *const poppedNode = head;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        } else {
            head->prev = nullptr;
        }

        return poppedNode;
    }

    [[nodiscard]] auto isEmpty() const -> bool { return head == nullptr; }

    auto remove(NodeType *const removedNode) -> void {
        NodeType *const nextNode = removedNode->next;
        NodeType *const prevNode = removedNode->prev;

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
