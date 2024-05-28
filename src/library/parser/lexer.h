#pragma once

#include <utils.h>

namespace wfpk {
// Forward declaration
struct Lexeme;

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
    Tcp,
    Udp,
    IpAddress,
    Ipv4,
    Ipv6,
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
};

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

private:
    // Check if a lexeme was matched, i.e matchTerminal(LBrackText) will return true if
    // "{" appears next in the input stream.
    std::optional<Lexeme> matchTerminal();

    Token string();
    Token ipAddressOrNumber();
    void skipWhitespace();
    char peek() const { return _input[_currentIndex]; }

private:
    std::string _input;
    size_t _currentIndex{0};
};
}
