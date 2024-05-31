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
        Tcp, Comma, Udp, RBrack, From, String, Port, Number, Ipv4Address
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

TEST(LexerTests, TestIp4Address)
{
    std::string input = "1.1.1.1";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = { Ipv4Address, "1.1.1.1" };

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestIp6Address)
{
    std::string full = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    std::string compressed = "2001:db8:85a3::8a2e:370:7334";
    std::string leadingZeroes = "2001:0db8::0001";
    std::string embeddedIpv4 = "::ffff:192.168.1.1";
    std::string loopback = "::1";

    Token actual = Lexer{full}.nextToken();
    Token expected = { Ipv6Address, full };

    ASSERT_EQ(actual, expected);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
