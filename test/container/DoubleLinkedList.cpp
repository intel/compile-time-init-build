#include <catch2/catch_test_macros.hpp>

#include <container/DoubleLinkedList.hpp>

namespace {
    struct IntNode {
        int value;
        IntNode * prev;
        IntNode * next;
        
        explicit constexpr IntNode(
            int newValue
        ) 
            : value{newValue}
            , prev{nullptr}
            , next{nullptr}
        {}
    };

    TEST_CASE("PushBackPopFront", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n{5};

        list.pushBack(&n);
        auto poppedNode = list.popFront();

        REQUIRE(poppedNode->value == 5);
    }


    TEST_CASE("IsEmpty", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n{5};

        REQUIRE(list.isEmpty());

        list.pushBack(&n);
        REQUIRE_FALSE(list.isEmpty());

        list.popFront();
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2_sequentially", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.isEmpty());

        list.pushBack(&n2);
        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2_multipleLists", "[double_linked_list]") {
        DoubleLinkedList<IntNode> listA{};
        DoubleLinkedList<IntNode> listB{};
        IntNode n1{1};
        IntNode n2{2};

        listA.pushBack(&n1);
        listA.pushBack(&n2);
        listA.popFront();
        listA.popFront();
        REQUIRE(listA.isEmpty());

        listB.pushBack(&n1);
        REQUIRE(listB.popFront()->value == 1);
        REQUIRE(listB.isEmpty());
    }

    TEST_CASE("RemoveMiddleNode", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        list.remove(&n2);

        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.popFront()->value == 3);
        REQUIRE(list.isEmpty());
    }

    TEST_CASE("RemoveOnlyNode", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};

        list.pushBack(&n1);

        list.remove(&n1);

        REQUIRE(list.isEmpty());
    }

    TEST_CASE("RemoveFirstNode", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n1);

        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }

    TEST_CASE("RemoveLastNode", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n2);

        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("Begin", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        REQUIRE(std::begin(list)->value == 1);
    }

    TEST_CASE("IteratorPreIncrement", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        REQUIRE(i->value == 1);


        REQUIRE((++i)->value == 2);
        REQUIRE(i->value == 2);

        REQUIRE((++i)->value == 3);
        REQUIRE(i->value == 3);
    }

    TEST_CASE("IteratorPostIncrement", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        REQUIRE(i->value == 1);

        REQUIRE((i++)->value == 1);
        REQUIRE(i->value == 2);

        REQUIRE((i++)->value == 2);
        REQUIRE(i->value == 3);
    }

    TEST_CASE("IteratorEquality", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};

        list.pushBack(&n1);

        REQUIRE(std::begin(list) == std::begin(list));
    }

    TEST_CASE("IteratorInequality", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        REQUIRE(i == std::begin(list));
        REQUIRE(i != std::end(list));

        i++;
        REQUIRE(i != std::begin(list));
        REQUIRE(i != std::end(list));

        i++;
        REQUIRE(i != std::begin(list));
        REQUIRE(i != std::end(list));

        i++;
        REQUIRE(i != std::begin(list));
        REQUIRE(i == std::end(list));
    }


    TEST_CASE("RemoveAllNodes", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n2);
        list.remove(&n1);

        REQUIRE(list.isEmpty());
    }

    TEST_CASE("RemoveAndAddSameNode", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n2);
        list.pushBack(&n1);

        list.remove(&n2);
        list.pushBack(&n2);
        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }

    TEST_CASE("RemoveEmptyList", "[double_linked_list]") {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.remove(&n1);
        list.remove(&n2);

        REQUIRE(list.isEmpty());
    }
}
