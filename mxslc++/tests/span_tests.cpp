//
// Created by jaket on 17/11/2025.
//

#include "gtest/gtest.h"
#include "Span.h"

using std::vector;
using mxslc::Span;

TEST(span_tests, span_01)
{
    const vector is{10, 20, 30, 40, 50};

    const Span span{is, 0, 1};
    ASSERT_EQ(span.size(), 1);
    ASSERT_EQ(span[0], 10);
    ASSERT_EQ(span[-1], 10);
    for (const int& i : span)
    {
        ASSERT_EQ(i, 10);
    }
    ASSERT_THROW(span[1], std::out_of_range);
    ASSERT_THROW(span[-2], std::out_of_range);
}

TEST(span_tests, span_01_array)
{
    int is[]{10, 20, 30, 40, 50};

    const Span span{is, 0, 1};
    ASSERT_EQ(span.size(), 1);
    ASSERT_EQ(span[0], 10);
    ASSERT_EQ(span[-1], 10);
    for (const int& i : span)
    {
        ASSERT_EQ(i, 10);
    }
    ASSERT_THROW(span[1], std::out_of_range);
    ASSERT_THROW(span[-2], std::out_of_range);
}

TEST(span_tests, span_02)
{
    const vector is{10, 20, 30, 40, 50};

    const Span span{is, 1, 3};
    ASSERT_EQ(span.size(), 3);
    ASSERT_EQ(span[0], 20);
    ASSERT_EQ(span[1], 30);
    ASSERT_EQ(span[2], 40);
    ASSERT_EQ(span[-1], 40);
    ASSERT_EQ(span[-2], 30);
    ASSERT_EQ(span[-3], 20);

    int j = 1;
    for (const int& i : span)
    {
        ASSERT_EQ(i, is[j++]);
    }
    ASSERT_EQ(j, 4);
    ASSERT_THROW(span[3], std::out_of_range);
    ASSERT_THROW(span[-4], std::out_of_range);
}

TEST(span_tests, span_02_array)
{
    int is[]{10, 20, 30, 40, 50};

    const Span span{is, 1, 3};
    ASSERT_EQ(span.size(), 3);
    ASSERT_EQ(span[0], 20);
    ASSERT_EQ(span[1], 30);
    ASSERT_EQ(span[2], 40);
    ASSERT_EQ(span[-1], 40);
    ASSERT_EQ(span[-2], 30);
    ASSERT_EQ(span[-3], 20);

    int j = 1;
    for (const int& i : span)
    {
        ASSERT_EQ(i, is[j++]);
    }
    ASSERT_EQ(j, 4);
    ASSERT_THROW(span[3], std::out_of_range);
    ASSERT_THROW(span[-4], std::out_of_range);
}

TEST(span_tests, span_03)
{
    const vector is{10, 20, 30, 40, 50};

    Span span{is, 0, 5};
    ASSERT_EQ(span.pop_front(), 10);
    ASSERT_EQ(span.pop_front(), 20);
    ASSERT_EQ(span.pop_back(), 50);
    ASSERT_EQ(span.pop_back(), 40);
    ASSERT_EQ(span.pop_front(), 30);
    ASSERT_TRUE(span.empty());

    ASSERT_THROW(span.pop_front(), std::out_of_range);
    ASSERT_THROW(span.pop_back(), std::out_of_range);
}

TEST(span_tests, span_03_array)
{
    int is[]{10, 20, 30, 40, 50};

    Span span{is, 0, 5};
    ASSERT_EQ(span.pop_front(), 10);
    ASSERT_EQ(span.pop_front(), 20);
    ASSERT_EQ(span.pop_back(), 50);
    ASSERT_EQ(span.pop_back(), 40);
    ASSERT_EQ(span.pop_front(), 30);
    ASSERT_TRUE(span.empty());

    ASSERT_THROW(span.pop_front(), std::out_of_range);
    ASSERT_THROW(span.pop_back(), std::out_of_range);
}

TEST(span_tests, span_04)
{
    const vector is{10, 20, 30, 40, 50};

    ASSERT_THROW(Span(is, 3, 5), std::out_of_range);
    ASSERT_THROW(Span(is, 6, 1), std::out_of_range);
}

TEST(span_tests, span_04_array)
{
    int is[]{10, 20, 30, 40, 50};

    ASSERT_NO_THROW(Span(is, 3, 5));
    ASSERT_NO_THROW(Span(is, 6, 1));
}
