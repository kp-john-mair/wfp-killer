#include <parser/lexer.h>
#include <gtest/gtest.h>

TEST(LexerTests, TestBasicLexing)
{
    using namespace wfpk;
    using enum TokenType;

    std::string input = R"(permit    out     inet from "baby")";

    Lexer lexer{input};

    std::vector<wfpk::TokenType> actual;
    for(Token token = lexer.nextToken(); token.type != EndOfInput; token = lexer.nextToken())
        actual.push_back(token.type);

    std::vector<wfpk::TokenType> expected = {PermitAction, OutDir, Ipv4, From, String};

    ASSERT_EQ(actual, expected);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;

    return RUN_ALL_TESTS();
}
