#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(catch_exceptions) = 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}