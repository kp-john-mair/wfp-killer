#include <parser/lexer.h>
#include <gtest/gtest.h>
#include <ranges>

namespace views = std::ranges::views;
using namespace wfpk;
using enum TokenType;

TEST(LexerTests, TestBasicLexing)
{
    std::string input = R"(permit out inet proto {tcp, udp} from "baby")";
    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {
        PermitAction, OutDir, Ipv4, Proto, LBrack,
        Tcp, Comma, Udp, RBrack, From, String
    };

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestAllTokens)
{
    std::string input = R"(permit block out in inet inet6 proto {tcp, udp} from "baby" port 53 1.1.1.1)";
    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {
        PermitAction, BlockAction, OutDir, InDir, Ipv4, Ipv6, Proto, LBrack,
        Tcp, Comma, Udp, RBrack, From, String, Port, Number, IpAddress
    };

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}


TEST(LexerTests, TestIgnoresWhiteSpace)
{
    std::string input = "\t\n  block \t    out\n    proto  {  tcp ,\n\t  udp  }  ";

    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {
        BlockAction, OutDir, Proto, LBrack,
        Tcp, Comma, Udp, RBrack
    };

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestNumbers)
{
    std::string input = "53";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = { Number, "53" };

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestString)
{
    std::string input = R"("the air can tear dead snails from the elephants lung")";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = { String, "the air can tear dead snails from the elephants lung" };

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestIpAddress)
{
    std::string input = "1.1.1.1";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = { IpAddress, "1.1.1.1" };

    ASSERT_EQ(actual, expected);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
