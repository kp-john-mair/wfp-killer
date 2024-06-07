#pragma once

#include <parser/lexer.h>
#include <parser/nodes.h>
#include <utility> // for std::pair

namespace wfpk {

class Parser
{
public:
    explicit Parser(Lexer &lexer)
    : _lexer{lexer}
    {}

public:
    // Parse the token stream
    std::unique_ptr<RulesetNode> parse();
    // Parse the token stream with verbose tracing
    std::unique_ptr<RulesetNode> parseTrace()
    {
        _shouldTrace = true;
        return parse();
    }

private:
    template <typename... TokenTypes>
    requires (std::same_as<TokenTypes, TokenType> && ...)
    auto match(TokenTypes... types) -> std::optional<Token>
    {
        std::optional<Token> result;
        ((result = match(types)) || ...);
        return result;
    }

    template <typename... TokenTypes>
    requires (std::same_as<TokenTypes, TokenType> && ...)
    auto peek(TokenTypes... types) -> bool
    {
        return (peek(types) || ...);
    }

    template <typename Func_T, typename... TokenTypes>
    auto list(Func_T func, TokenTypes... tokenTypes)
        -> std::vector<std::invoke_result_t<Func_T, Token>>
    {
        using ReturnType = std::invoke_result_t<Func_T, Token>;
        std::vector<ReturnType> listResults;

        mustMatch(TokenType::LBrack);
        while(true)
        {
            if(auto tok = match(tokenTypes...))
            {
                ReturnType result = func(*tok);
                listResults.push_back(result);
                if(peek(TokenType::Comma))
                    consume(); // advance by one token
                else if(match(TokenType::RBrack))
                    return listResults;
                else
                    unexpectedTokenError();
            }
            // This will only trigger in the case of an empty list `{}`
            else if(match(TokenType::RBrack))
            {
                return listResults;
            }
            else
            {
                unexpectedTokenError();
            }
        }

        return listResults;
    }

    // Token processing
    // Asserts that a given token must come next and increments the cursor on success
    // throws a ParseError on failure
    auto match(TokenType type) -> std::optional<Token>;

    void mustMatch(TokenType type)
    {
        if(!match(type))
            unexpectedTokenError();
    }

    // Consumes one token and moves the cursor
    void consume()
    {
        _lookahead = _lexer.nextToken();
    }

    Token peek() const { return _lookahead; }
    bool peek(TokenType type) { return _lookahead.type == type; }

private:
    std::unique_ptr<Node> filter();
    FilterConditions conditions();
    void sourceCondition(FilterConditions *conditions);
    void destCondition(FilterConditions *conditions);

    auto addressAndPorts() -> std::pair<std::string, std::vector<uint16_t>>;
    auto numberList() -> std::vector<uint16_t>;

    void unexpectedTokenError()
    {
        throw ParseError{std::format("Unexpected token {}", _lookahead.description())};
    }

private:
    Lexer &_lexer;
    Token _lookahead{};
    bool _shouldTrace{false};
};

}
