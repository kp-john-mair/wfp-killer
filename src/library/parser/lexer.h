#pragma once

#include <utils.h>

namespace wfpk {
// Custom error that represents a parsing/lexing failure
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &reason)
        : std::runtime_error(reason) {}
};

enum class TokenType : uint32_t
{
    EndOfInput,
    BlockAction,
    PermitAction,
    LBrack,
    RBrack,
    InDir,
    OutDir,
    Port,
    Proto,
    String,
    Number,
    From,
    To,
    TcpTransport,
    UdpTransport,
    All,

    // These tokens may also represent subnets
    Ipv4Address,
    Ipv6Address,

    Inet4,
    Inet6,
    Comma
};

// Represents a token
struct Token
{
    TokenType type{};
    std::string text;

    // Useful string representation of the token for
    // tracing purposes.
    std::string description() const;

    // comparison
    auto operator<=>(const Token &other) const = default;
};

inline static const Token EndOfInputToken{TokenType::EndOfInput, "EOF"};

// The Lexer is responsible for breaking up a string of text into tokens
class Lexer
{
public:
    explicit Lexer(const std::string &input)
        : _input{input}, _currentIndex{0}
    {
    }

    // Return the next available token from the input
    Token nextToken();
    // Return all tokens at once (primarily useful for tests)
    std::vector<Token> allTokens();

private:
    // Check if a keyword lexeme was matched, i.e maybeKeyword() will return a Token
    // if a keyword appears next in the input stream, otherwise it returns an empty optional.
    auto maybeKeyword() -> std::optional<Token>;
    // Lex a string literal
    Token string();
    // Lex an ipAddress (v4 or v6) together with its subnet.
    // The 'pos' param represents the position of the '/' separating address from subnet.
    // Returns an Ipv4Address or Ipv6Address token - but represents a subnet
    Token ipAddressAndSubnet(const std::string &addressAndSubnet, size_t pos);
    void skipWhitespace();
    char peek() const { return _input[_currentIndex]; }
    std::string identifierString();

private:
    std::string _input;
    size_t _currentIndex{0};
};
}
