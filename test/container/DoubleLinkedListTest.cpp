#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>

#include <container/DoubleLinkedList.hpp>

namespace {
    using ::testing::_;

    class DoubleLinkedListTest : public ::testing::Test {
    protected:
        MockLogger logger;

        void SetUp() override {
            mockLoggerPtr = &logger;
            EXPECT_CALL(logger, fatal(_)).Times(0);
            EXPECT_CALL(logger, fatal(_, _, _)).Times(0);
        }
    };

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

    TEST_F(DoubleLinkedListTest, PushBackPopFront) {
        DoubleLinkedList<IntNode> list{};
        IntNode n{5};

        list.pushBack(&n);
        auto poppedNode = list.popFront();

        EXPECT_EQ(poppedNode->value, 5);
    }


    TEST_F(DoubleLinkedListTest, IsEmpty) {
        DoubleLinkedList<IntNode> list{};
        IntNode n{5};

        EXPECT_TRUE(list.isEmpty());

        list.pushBack(&n);
        EXPECT_FALSE(list.isEmpty());

        list.popFront();
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(DoubleLinkedListTest, PushBack2PopFront2) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(DoubleLinkedListTest, PushBack2PopFront2_sequentially) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_TRUE(list.isEmpty());

        list.pushBack(&n2);
        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(DoubleLinkedListTest, PushBack2PopFront2_multipleLists) {
        DoubleLinkedList<IntNode> listA{};
        DoubleLinkedList<IntNode> listB{};
        IntNode n1{1};
        IntNode n2{2};

        listA.pushBack(&n1);
        listA.pushBack(&n2);
        listA.popFront();
        listA.popFront();
        EXPECT_TRUE(listA.isEmpty());

        listB.pushBack(&n1);
        EXPECT_EQ(listB.popFront()->value, 1);
        EXPECT_TRUE(listB.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveMiddleNode) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        list.remove(&n2);

        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_EQ(list.popFront()->value, 3);
        EXPECT_TRUE(list.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveOnlyNode) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};

        list.pushBack(&n1);

        list.remove(&n1);

        EXPECT_TRUE(list.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveFirstNode) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n1);

        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveLastNode) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n2);

        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(DoubleLinkedListTest, Begin) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        EXPECT_EQ(std::begin(list)->value, 1);
    }

    TEST_F(DoubleLinkedListTest, IteratorPreIncrement) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        EXPECT_EQ(i->value, 1);


        EXPECT_EQ((++i)->value, 2);
        EXPECT_EQ(i->value, 2);

        EXPECT_EQ((++i)->value, 3);
        EXPECT_EQ(i->value, 3);
    }

    TEST_F(DoubleLinkedListTest, IteratorPostIncrement) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        EXPECT_EQ(i->value, 1);

        EXPECT_EQ((i++)->value, 1);
        EXPECT_EQ(i->value, 2);

        EXPECT_EQ((i++)->value, 2);
        EXPECT_EQ(i->value, 3);
    }

    TEST_F(DoubleLinkedListTest, IteratorEquality) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};

        list.pushBack(&n1);

        EXPECT_EQ(std::begin(list), std::begin(list));
    }

    TEST_F(DoubleLinkedListTest, IteratorInequality) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};
        IntNode n3{3};

        list.pushBack(&n1);
        list.pushBack(&n2);
        list.pushBack(&n3);

        auto i = std::begin(list);
        EXPECT_EQ(i, std::begin(list));
        EXPECT_NE(i, std::end(list));

        i++;
        EXPECT_NE(i, std::begin(list));
        EXPECT_NE(i, std::end(list));

        i++;
        EXPECT_NE(i, std::begin(list));
        EXPECT_NE(i, std::end(list));

        i++;
        EXPECT_NE(i, std::begin(list));
        EXPECT_EQ(i, std::end(list));
    }


    TEST_F(DoubleLinkedListTest, RemoveAllNodes) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.remove(&n2);
        list.remove(&n1);

        EXPECT_TRUE(list.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveAndAddSameNode) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n2);
        list.pushBack(&n1);

        list.remove(&n2);
        list.pushBack(&n2);
        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }

    TEST_F(DoubleLinkedListTest, RemoveEmptyList) {
        DoubleLinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.remove(&n1);
        list.remove(&n2);

        EXPECT_TRUE(list.isEmpty());
    }
}
