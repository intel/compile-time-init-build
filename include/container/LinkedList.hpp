#pragma once

template <typename NodeType> class LinkedList {
  private:
    NodeType *head{};
    NodeType *tail{};

  public:
    auto pushBack(NodeType *n) -> void {
        if (head == nullptr) {
            head = n;
        } else {
            tail->next = n;
        }

        n->next = nullptr;
        tail = n;
    }

    auto popFront() -> NodeType * {
        NodeType *poppedNode = head;
        head = head->next;
        return poppedNode;
    }

    [[nodiscard]] auto isEmpty() const -> bool { return head == nullptr; }

    auto removeNext(NodeType *n) -> NodeType * {
        NodeType *poppedNode = n->next;
        n->next = poppedNode->next;
        return poppedNode;
    }
};
