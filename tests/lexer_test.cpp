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

    std::vector expected = {PermitAction, OutDir,       Inet4,  Proto, LBrack, TcpTransport,
                            Comma,        UdpTransport, RBrack, From,  String};

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestAllTokens)
{
    std::string input =
        R"(permit block out ::1 in inet inet6 proto {tcp, udp} from "baby" port 53 1.1.1.1 all)";
    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {PermitAction, BlockAction,  OutDir,      Ipv6Address, InDir,
                            Inet4,        Inet6,        Proto,       LBrack,      TcpTransport,
                            Comma,        UdpTransport, RBrack,      From,        String,
                            Port,         Number,       Ipv4Address, All};

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestIgnoresWhiteSpace)
{
    std::string input = "\t\n  block \t    out\n    proto  {  tcp ,\n\t  udp  }  ";

    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {BlockAction,  OutDir, Proto,        LBrack,
                            TcpTransport, Comma,  UdpTransport, RBrack};

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestNumbers)
{
    std::string input = "53";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = {Number, "53"};

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestString)
{
    std::string input = R"("the air can tear dead snails from the elephants lung")";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    // Strips out the outer ""
    Token expected = {String, "the air can tear dead snails from the elephants lung"};

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestIp4Address)
{
    std::string input = "1.1.1.1";

    Lexer lexer{input};
    Token actual = lexer.nextToken();
    Token expected = {Ipv4Address, "1.1.1.1"};

    ASSERT_EQ(actual, expected);
}

TEST(LexerTests, TestIp4AddressWithSubnet)
{
    std::string inRangePrefix = "1.1.1.1/16";
    std::string maxPrefix = "1.1.1.1/32";
    std::string minPrefix = "1.1.1.1/1";
    std::string prefixExceeded = "1.1.1.1/33";
    std::string zeroPrefix = "1.1.1.1/0";

    // Valid subnets
    ASSERT_EQ((Lexer{inRangePrefix}.nextToken()), (Token{Ipv4Address, inRangePrefix}));
    ASSERT_EQ((Lexer{maxPrefix}.nextToken()), (Token{Ipv4Address, maxPrefix}));
    ASSERT_EQ((Lexer{minPrefix}.nextToken()), (Token{Ipv4Address, minPrefix}));

    // Invalid subnets
    ASSERT_THROW((Lexer{prefixExceeded}.nextToken()), wfpk::ParseError);
    ASSERT_THROW((Lexer{zeroPrefix}.nextToken()), wfpk::ParseError);
}

TEST(LexerTests, TestIp4AddressesNoSpaceContext)
{
    std::string input = "from {1.1.1.1,2.2.2.2}";

    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {From, LBrack, Ipv4Address, Comma, Ipv4Address, RBrack};

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

TEST(LexerTests, TestIp6Address)
{
    std::vector<std::string> addresses = {// Full
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
                                          "::1"};

    for(const auto &address : addresses)
    {
        Token actual = Lexer{address}.nextToken();
        Token expected = {Ipv6Address, address};
        ASSERT_EQ(actual, expected);
    }
}

TEST(LexerTests, TestIp6AddressWithSubnet)
{
    std::string inRangePrefix = "2001::123/64";
    std::string maxPrefix = "2001::123/128";
    std::string minPrefix = "2001::123/1";
    std::string prefixExceeded = "2001::123/129";
    std::string zeroPrefix = "2001::123/0";

    // Valid subnets
    ASSERT_EQ((Lexer{inRangePrefix}.nextToken()), (Token{Ipv6Address, inRangePrefix}));
    ASSERT_EQ((Lexer{maxPrefix}.nextToken()), (Token{Ipv6Address, maxPrefix}));
    ASSERT_EQ((Lexer{minPrefix}.nextToken()), (Token{Ipv6Address, minPrefix}));

    // Invalid subnets
    ASSERT_THROW((Lexer{prefixExceeded}.nextToken()), wfpk::ParseError);
    ASSERT_THROW((Lexer{zeroPrefix}.nextToken()), wfpk::ParseError);
}

TEST(LexerTests, TestIp6AddressesNoSpaceContext)
{
    std::string input = "from {::1,5fca:1234::2}";

    Lexer lexer{input};

    auto actual = lexer.allTokens() | views::transform(&Token::type);

    std::vector expected = {From, LBrack, Ipv6Address, Comma, Ipv6Address, RBrack};

    ASSERT_TRUE(std::ranges::equal(actual, expected));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
