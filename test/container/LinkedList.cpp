#include <catch2/catch_test_macros.hpp>

#include <container/LinkedList.hpp>

namespace {
    struct IntNode {
        int value;
        IntNode * next;
        
        explicit constexpr IntNode(
            int newValue
        ) 
            : value{newValue}
            , next{nullptr}
        {}
    };

    TEST_CASE("PushBackPopFront", "[linked_list]") {
        LinkedList<IntNode> list{};
        IntNode n{5};

        list.pushBack(&n);
        auto poppedNode = list.popFront();

        REQUIRE(poppedNode->value == 5);
    }


    TEST_CASE("IsEmpty", "[linked_list]") {
        LinkedList<IntNode> list{};
        IntNode n{5};

        REQUIRE(list.isEmpty());

        list.pushBack(&n);
        REQUIRE_FALSE(list.isEmpty());

        list.popFront();
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2", "[linked_list]") {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2_sequentially", "[linked_list]") {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.isEmpty());

        list.pushBack(&n2);
        REQUIRE(list.popFront()->value == 2);
        REQUIRE(list.isEmpty());
    }


    TEST_CASE("PushBack2PopFront2_multipleLists", "[linked_list]") {
        LinkedList<IntNode> listA{};
        LinkedList<IntNode> listB{};
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




    TEST_CASE("RemoveNext", "[linked_list]") {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.removeNext(&n1);

        REQUIRE(list.popFront()->value == 1);
        REQUIRE(list.isEmpty());
    }
}
