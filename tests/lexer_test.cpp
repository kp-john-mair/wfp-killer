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
        PermitAction, OutDir, Inet4, Proto, LBrack,
        Tcp, Comma, Udp, RBrack, From, String
    };

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestAllTokens)
{
    std::string input = R"(permit block out ::1 in inet inet6 proto {tcp, udp} from "baby" port 53 1.1.1.1)";
    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {
        PermitAction, BlockAction, OutDir, Ipv6Address, InDir, Inet4, Inet6, Proto, LBrack,
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
    // Strips out the outer ""
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
    std::vector<std::string> addresses =
    {
        // Full
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        // Compressed
        "2407::9a62:2100:483:91ec:9221:cad",
        // Link-local
        "fe80::1096:e770:9d55:7fef",
        // Unique-local
        "fd12:3456:789a:1::1",
        // With leading zeroes
        "2001:0db8::0001",
        // Embedded ipv4
        "::ffff:192.168.1.1",
        // Loopback
        "::1"
    };

    for(const auto &address : addresses)
    {
        Token actual = Lexer{address}.nextToken();
        Token expected = { Ipv6Address, address };
        ASSERT_EQ(actual, expected);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
