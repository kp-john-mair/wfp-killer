#include <utils.h>
#include <regex>
#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <parser/lexer.h>

namespace wfpk {
struct Lexeme
{
    TokenType tokenType{};
    std::string tokenName;
    std::string lexeme;

    size_t length() const { return lexeme.length(); }
};

namespace {
// Enum class for token types

// Associate the name of token with the token type
const std::unordered_map<TokenType, std::string> tokenMap = {
    {TokenType::BlockAction, "BlockAction"},
    {TokenType::PermitAction, "PermitAction"},
    {TokenType::LBrack, "LBRACK"},
    {TokenType::RBrack, "RBRACK"},
    {TokenType::InDir, "InDir"},
    {TokenType::OutDir, "OutDir"},
    {TokenType::Port, "Port"},
    {TokenType::Proto, "Proto"},
    {TokenType::String, "String"},
    {TokenType::Number, "Number"},
    {TokenType::From, "From"},
    {TokenType::To, "To"},
    {TokenType::Tcp, "Tcp"},
    {TokenType::Udp, "Udp"},
    {TokenType::IpAddress, "IpAddress"},
    {TokenType::Ipv4, "inet"},
    {TokenType::Ipv6, "inet6"},
};

const std::vector<Lexeme> keywords = {
    { .tokenType = TokenType::BlockAction, .tokenName = "BlockAction", .lexeme = "block" },
    { .tokenType = TokenType::PermitAction, .tokenName = "PermitAction", .lexeme = "permit" },
    { .tokenType = TokenType::LBrack, .tokenName = "LBrack", .lexeme = "{" },
    { .tokenType = TokenType::RBrack, .tokenName = "RBrack", .lexeme = "}" },
    { .tokenType = TokenType::Ipv4, .tokenName = "Ipv4", .lexeme = "inet" },
    { .tokenType = TokenType::Ipv6, .tokenName = "Ipv6", .lexeme = "inet6" },
    { .tokenType = TokenType::InDir, .tokenName = "InDir", .lexeme = "in" },
    { .tokenType = TokenType::OutDir, .tokenName = "OutDir", .lexeme = "out" },
    { .tokenType = TokenType::Port, .tokenName = "Port", .lexeme = "port" },
    { .tokenType = TokenType::Proto, .tokenName = "Proto", .lexeme = "proto" },
    { .tokenType = TokenType::From, .tokenName = "From", .lexeme = "from" },
    { .tokenType = TokenType::To, .tokenName = "To", .lexeme = "to" },
    { .tokenType = TokenType::Tcp, .tokenName = "Tcp", .lexeme = "tcp" },
    { .tokenType = TokenType::Udp, .tokenName = "Udp", .lexeme = "udp" },
    { .tokenType = TokenType::Comma, .tokenName = "Comma", .lexeme = "," }
};

static const std::regex ipv4Regex{std::string{R"(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\."
                                               "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\."
                                               "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\."
                                               "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"}};
}

std::string Token::description() const
{
    const std::string name = tokenMap.at(type);
    if(text.empty())
        return name;
    else
        return std::format("{}({})", name, text);
}

// Custom error that represents a parsing/lexing failure
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &reason)
        : std::runtime_error(reason) {}
};

std::optional<Lexeme> Lexer::matchTerminal()
{
    auto itLexeme = std::find_if(keywords.begin(), keywords.end(),
        [&](const auto &terminal)
        {
            return _input.compare(_currentIndex, terminal.length(), terminal.lexeme) == 0;
        });

    if(itLexeme == keywords.end())
        return {};

    return *itLexeme;
}

Token Lexer::string()
{
    ++_currentIndex;
    size_t start = _currentIndex;
    while(_currentIndex < _input.length() && peek() != '"')
        ++_currentIndex;

    std::string content = _input.substr(start, _currentIndex - start);

    // Skip closing "
    ++_currentIndex;

    return {TokenType::String, content};
}

Token Lexer::ipAddressOrNumber()
{
    size_t start = _currentIndex;
    while(_currentIndex < _input.length() && (isdigit(peek()) || peek() == '.'))
            ++_currentIndex;

    std::string content = _input.substr(start, _currentIndex - start);

    if(std::regex_match(content, ipv4Regex))
        return {TokenType::IpAddress, content};
    else if(std::ranges::all_of(content, isdigit))
        return {TokenType::Number, content};

    throw ParseError{std::format("Invalid token: got {} - not a number or an ipv4 address!", content)};
}

void Lexer::skipWhitespace()
{
    // Eat up all whitespace between lexemes
    while(_currentIndex < _input.length() && std::isspace(_input[_currentIndex]))
        ++_currentIndex;
}

Token Lexer::nextToken() {
    // End of input
    if(_currentIndex >= _input.length())
        return {TokenType::EndOfInput, "EOF"};

    skipWhitespace();

    std::optional<Lexeme> keyword = matchTerminal();
    if(keyword)
    {
        _currentIndex += keyword->length();
        return {keyword->tokenType, keyword->lexeme};
    }

    const auto lookahead = peek();

    switch(lookahead)
    {
    case '"':
    {
       return string();
    }
    default:
    {
        if(isdigit(lookahead))
            return ipAddressOrNumber();

        throw ParseError{std::format("Unrecognized symbol: '{}'!", lookahead)};
    }
    }
}
}

