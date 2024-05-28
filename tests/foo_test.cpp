#include <parser/lexer.h>
#include <gtest/gtest.h>

TEST(FooTests, TestLexer)
{
    using namespace wfpk;

    std::string input = R"(permit    out     inet from "baby")";

    Lexer lexer{input};

    Token token = lexer.nextToken();;

    while(token.type != TokenType::EndOfInput)
    {
        std::cout << token.description() << "\n";
        token = lexer.nextToken();
    }

    ASSERT_EQ(true, true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;

    return RUN_ALL_TESTS();
}
