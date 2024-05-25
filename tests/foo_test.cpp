#include <wfp_killer.h>
#include <gtest/gtest.h>

TEST(FooTests, TestOptions)
{
    wfpk::WfpKiller::Options options;
    ASSERT_EQ(options.isEmpty(), true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
