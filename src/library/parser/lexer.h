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
    InvalidToken,
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

struct SourceLocation
{
    uint32_t line{1};
    uint32_t column{1};

    std::string toString() const
    {
        return std::format("({},{})", line, column);
    }
};

// Represents a token
struct Token
{
    TokenType type{};
    std::string text;
    SourceLocation sourceLocation;

    // comparison - do not consider sourceLocation field
    bool operator==(const Token &other) const
    {
        return type == other.type && text == other.text;
    }

    // Useful string representation of the token for
    // tracing purposes.
    std::string toString() const
    {
        if(!text.empty())
            return std::format("{}('{}') {}", enumName(type), text, sourceLocation.toString());
        else
            return std::format("{} {}", enumName(type), sourceLocation.toString());
    }
};

inline std::ostream& operator<<(std::ostream &ostream, const Token &token)
{
    ostream << token.toString();
    return ostream;
}

// The Lexer is responsible for breaking up a string of text into tokens
class Lexer
{
public:
    explicit Lexer(std::string input)
        : _input{std::move(input)}
        , _currentIndex{0}
        , _sourceLocation{1, 1}
    {}

    Lexer(const Lexer&) = default;
    Lexer(Lexer&&) = default;
    Lexer& operator=(const Lexer&) = default;
    Lexer& operator=(Lexer&&) = default;

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
    // Increment the line if we encounter a newline character
    void updateLineNumber();
    void skipWhitespace();
    void advance(size_t increment = 1);
    char peek() const { return _input[_currentIndex]; }
    std::string identifierString();
    SourceLocation calcSourceLocation(const std::string &lexeme) const;

    Token endOfInputToken() const
    {
        return Token{TokenType::EndOfInput, "", calcSourceLocation("")};
    }

private:
    std::string _input;
    size_t _currentIndex{0};
    SourceLocation _sourceLocation;
};
}
