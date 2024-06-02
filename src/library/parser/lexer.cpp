#include <utils.h>
#include <regex>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <optional>
#include <parser/lexer.h>
#include <magic_enum.h>

namespace wfpk {
// Keywords are simple lexemes with static content.
// Numbers (and Strings) are NOT keywords as they could be
// anything, i.e 53 or "hello", etc
struct Keyword
{
    TokenType tokenType{};
    std::string lexeme;

    size_t length() const { return lexeme.length(); }
};

namespace {
const std::vector<Keyword> keywords = {
    { .tokenType = TokenType::BlockAction, .lexeme = "block" },
    { .tokenType = TokenType::PermitAction, .lexeme = "permit" },
    { .tokenType = TokenType::LBrack, .lexeme = "{" },
    { .tokenType = TokenType::RBrack, .lexeme = "}" },
    // Longer lexemes (that share a prefix with smaller lexemes) need to appear first -
    // so 'inet6' before 'inet' and inet before 'in'
    // otherwise the longer lexemes will never be matched.
    { .tokenType = TokenType::Inet6, .lexeme = "inet6" },
    { .tokenType = TokenType::Inet4, .lexeme = "inet" },
    { .tokenType = TokenType::InDir, .lexeme = "in" },
    { .tokenType = TokenType::OutDir, .lexeme = "out" },
    { .tokenType = TokenType::Port, .lexeme = "port" },
    { .tokenType = TokenType::Proto, .lexeme = "proto" },
    { .tokenType = TokenType::From, .lexeme = "from" },
    { .tokenType = TokenType::To, .lexeme = "to" },
    { .tokenType = TokenType::Tcp, .lexeme = "tcp" },
    { .tokenType = TokenType::Udp, .lexeme = "udp" },
    { .tokenType = TokenType::Comma, .lexeme = "," }
};

// Due to ipv4, ipv6 and subnet addresses we allow these additional chars in our identifiers.
const std::unordered_set<char> allowedIdentSymbols = { ':', '.', '/' };
}

std::string Token::description() const
{
    const std::string name{magic_enum::enum_name(type)};
    if(text.empty())
        return name;
    else
        return std::format("{}({})", name, text);
}

std::string Lexer::identifierString()
{
    int start = _currentIndex;

    // Identifiers are alphanumeric + additional allowed symbols used by special identifiers
    // such as ip subnets - so '.' and ':' and '/' are allowed too.
    while(_currentIndex < _input.length() && (std::isalnum(peek()) || allowedIdentSymbols.contains(peek())))
        _currentIndex++;

    return _input.substr(start, _currentIndex - start);
}

 auto Lexer::maybeKeyword() -> std::optional<Token>
{
    auto itKeyword = std::find_if(keywords.begin(), keywords.end(),
        [&](const auto &terminal)
        {
            return _input.compare(_currentIndex, terminal.length(), terminal.lexeme) == 0;
        });

    // A keyword match was found
    if(itKeyword != keywords.end())
    {
        // Increment index by lexeme length
        _currentIndex += itKeyword->length();
        // Create a token for the lexeme and return it
        return Token{itKeyword->tokenType, itKeyword->lexeme};
    }
    else
    {
        return {};
    }
}

Token Lexer::string()
{
    ++_currentIndex; // skip over initial ""
    size_t start = _currentIndex;
    while(_currentIndex < _input.length() && peek() != '"')
        ++_currentIndex;

    std::string content = _input.substr(start, _currentIndex - start);

    // Skip closing "
    ++_currentIndex;

    return {TokenType::String, content};
}

void Lexer::skipWhitespace()
{
    // Eat up all whitespace between lexemes
    while(_currentIndex < _input.length() && std::isspace(peek()))
        ++_currentIndex;
}

std::vector<Token> Lexer::allTokens()
{
    std::vector<Token> tokens;
    for(Token token = nextToken(); token.type != TokenType::EndOfInput; token = nextToken())
        tokens.push_back(token);

    return tokens;
}

Token Lexer::ipAddressAndSubnet(const std::string &addressAndSubnet, size_t pos)
{
    assert(pos < addressAndSubnet.length());

    const std::string address{addressAndSubnet.begin(), addressAndSubnet.begin() + pos};
    const std::string subnet{addressAndSubnet.begin() + pos + 1, addressAndSubnet.end()};

    uint32_t subnetValue = static_cast<uint32_t>(std::atoi(subnet.c_str()));

    if(subnetValue == 0)
        throw ParseError{std::format("Got an invalid 0 prefix for {}", address)};

    if(isIpv6(address) && subnetValue <= 128)
        return Token{TokenType::Ipv6Address, std::format("{}/{}", address, subnet)};
    else if(isIpv4(address) && subnetValue <= 32)
        return Token{TokenType::Ipv4Address, std::format("{}/{}", address, subnet)};

    throw ParseError{std::format("Invalid ip address and subnet: {}/{}", address, subnet)};
}

Token Lexer::nextToken() {
    static const Token EndOfInputToken{TokenType::EndOfInput, "EOF"};

    // End of input
    if(_currentIndex >= _input.length())
        return EndOfInputToken;

    // Eat up spaces, tabs, newlines
    skipWhitespace();

    const auto lookahead = peek();
    switch(lookahead)
    {
    // Special handling of null byte - if there's whitespace at the end of the input
    // then skipWhitespace will go right to the last char, meaning the next char will be
    // the null byte
    case '\0':
        return EndOfInputToken;
    case '"':
       return string();
    default:
    {
        // Look for keywords
        std::optional<Token> pKeyword = maybeKeyword();
        if(pKeyword)
            return *pKeyword;

        // Identifiers are comprised of alphanumeric chars as well as some additional
        // symbols such as '.', '/' and ':' which are used by ipv{4,6} subnets.
        const std::string ident = identifierString();

        // If it contains a '/' it must be a subnet
        auto pos = ident.find('/');
        if(pos != std::string::npos)
             return ipAddressAndSubnet(ident, pos);

        if(isIpv6(ident))
            return {TokenType::Ipv6Address, ident};
        else if(isIpv4(ident))
            return {TokenType::Ipv4Address, ident};
        else if(std::ranges::all_of(ident, isdigit))
            return {TokenType::Number, ident};

        // Anything else - not supported.
        throw ParseError{std::format("Unrecognized identifier: '{}'!", ident)};
    }
    }
}
}

