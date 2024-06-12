#pragma once

#include <parser/lexer.h>
#include <parser/nodes.h>
#include <utility> // for std::pair

namespace wfpk {
class Parser
{
public:
    explicit Parser(Lexer lexer)
    : _lexer{std::move(lexer)}
    {}

    explicit Parser(std::string input)
    : _lexer{std::move(input)}
    {}

public:
    // Parse the token stream
    auto parse() -> std::unique_ptr<RulesetNode>;
    // Parse the token stream with verbose tracing
    auto parseTrace() -> std::unique_ptr<RulesetNode>
    {
        _shouldTrace = true;
        return parse();
    }

private:
    void unexpectedTokenError()
    {
        throw ParseError{std::format("Unexpected token {}", _lookahead.description())};
    }

    // Token processing
    // Asserts that a given token must come next and increments the cursor on success
    // throws a ParseError on failure
    auto match(TokenType type) -> std::optional<Token>;

    template <typename... TokenTypes>
    requires (std::same_as<TokenTypes, TokenType> && ...)
    auto match(TokenTypes... types) -> std::optional<Token>
    {
        std::optional<Token> result;
        ((result = match(types)) || ...);
        return result;
    }

    void mustMatch(TokenType type)
    {
        if(!match(type))
            unexpectedTokenError();
    }

    template <typename... TokenTypes>
    // Restict the function to > 0 params so that it'll fall back to the other peek() overload
    // for no args.
    requires ((std::same_as<TokenTypes, TokenType> && ...)
        && (sizeof...(TokenTypes) > 0))
    auto peek(TokenTypes... types) -> bool
    {
        return (peek(types) || ...);
    }

    // Consumes one token and moves the cursor
    void consume() { _lookahead = _lexer.nextToken(); }

    Token peek() const { return _lookahead; }
    bool peek(TokenType type) { return _lookahead.type == type; }

private:
    std::unique_ptr<Node> filter();
    FilterConditions conditions();
    void sourceCondition(FilterConditions *conditions);
    void destCondition(FilterConditions *conditions);

    auto addressAndPorts() -> std::pair<std::vector<std::string>, std::vector<uint16_t>>;
    auto transportProtocol() -> FilterConditions::TransportProtocol;
    auto numberList() -> std::vector<uint16_t>;
    auto ipList() -> std::vector<std::string>;
    // Does not return a list - only returns one protocol type.
    // But the protocols can be written as a list in the grammar,
    // i.e { tcp, udp } which is a list - maps to the AllTransports enum value.
    auto transportProtocolList() -> FilterConditions::TransportProtocol;


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

private:
    Lexer _lexer;
    Token _lookahead{};
    bool _shouldTrace{false};
};

}
