#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <core/mock/MockLogger.hpp>

#include <container/LinkedList.hpp>

namespace {
    using ::testing::_;

    class LinkedListTest : public ::testing::Test {
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
        IntNode * next;
        
        explicit constexpr IntNode(
            int newValue
        ) 
            : value{newValue}
            , next{nullptr}
        {}
    };

    TEST_F(LinkedListTest, PushBackPopFront) {
        LinkedList<IntNode> list{};
        IntNode n{5};

        list.pushBack(&n);
        auto poppedNode = list.popFront();

        EXPECT_EQ(poppedNode->value, 5);
    }


    TEST_F(LinkedListTest, IsEmpty) {
        LinkedList<IntNode> list{};
        IntNode n{5};

        EXPECT_TRUE(list.isEmpty());

        list.pushBack(&n);
        EXPECT_FALSE(list.isEmpty());

        list.popFront();
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(LinkedListTest, PushBack2PopFront2) {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(LinkedListTest, PushBack2PopFront2_sequentially) {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_TRUE(list.isEmpty());

        list.pushBack(&n2);
        EXPECT_EQ(list.popFront()->value, 2);
        EXPECT_TRUE(list.isEmpty());
    }


    TEST_F(LinkedListTest, PushBack2PopFront2_multipleLists) {
        LinkedList<IntNode> listA{};
        LinkedList<IntNode> listB{};
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




    TEST_F(LinkedListTest, RemoveNext) {
        LinkedList<IntNode> list{};
        IntNode n1{1};
        IntNode n2{2};

        list.pushBack(&n1);
        list.pushBack(&n2);

        list.removeNext(&n1);

        EXPECT_EQ(list.popFront()->value, 1);
        EXPECT_TRUE(list.isEmpty());
    }
}
