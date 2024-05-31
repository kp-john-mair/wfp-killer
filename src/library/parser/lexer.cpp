#include <utils.h>
#include <regex>
#include <iostream>
#include <string>
#include <unordered_map>
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
    { .tokenType = TokenType::Ipv6, .lexeme = "inet6" },
    { .tokenType = TokenType::Ipv4, .lexeme = "inet" },
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

   // Our regex matcher
   static const std::regex ipv4Regex(R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$)");
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

    // Identifiers cannot contain spaces, read remaining text until the next space
    // and store it in an indentifier token.
    while(_currentIndex < _input.length() && !std::isspace(peek()))
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

        std::string ident = identifierString();

        if(std::regex_match(ident, ipv4Regex))
            return {TokenType::IpAddress, ident};
        else if(std::ranges::all_of(ident, isdigit))
            return {TokenType::Number, ident};

        // Anything else - not supported.
        throw ParseError{std::format("Unrecognized symbol: '{}'!", lookahead)};
    }
    }
}
}

