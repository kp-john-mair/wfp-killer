#pragma once

#include <parser/lexer.h>
#include <pair>

namespace wfpk {

class Visitor
{
};

class Node
{
protected:
    // Make it uninstantiable as it's an abstract class
    Node() = default;

public:
    void accept(const Visitor &visitor)
    {
        acceptNode(visitor);
    }

    void setChildren(std::vector<std::unique_ptr<Node>> children)
    {
        _pChildren = std::move(children);
    }

    void addChild(std::unique_ptr<Node> child)
    {
        _pChildren.push_back(child);
    }

private:
    virtual void acceptNode(const Visitor &visitor) = 0;

private:
    std::vector<std::unique_ptr<Node>> _pChildren;
    Token _token{EndOfInputToken};
};

class RulesetNode final : public Node
{
public:
    RulesetNode(std::vector<std::unique_ptr<Node>> children)
    : Node()
    {
        setChildren(children);
    }
};

class FilterNode final : public Node
{
public:
    enum class Action { Block, Permit };
    using enum Action;

    enum class Layer { AUTH_CONNECT_V4,
                       AUTH_RECV_V4,
                       AUTH_CONNECT_V6,
                       AUTH_RECV_V6 };
    using enum Layer;

    FilterNode() : Node{}
    {
    }
};

struct FilterConditions
{
    enum class IpVersion { Both, Inet4, Inet6 };
    using enum IpVersion;

    std::vector<uint16_t> sourcePorts;
    std::vector<uint16_t> destPorts;
    std::string sourceApp;
    std::string sourceIp;
    std::string destIp;
    std::string interfaceName;
    IpVersion ipVersion{};
};

// Represents no conditions be applied
inline const FilterConditions NoFilterConditions = {};

class Parser
{
public:
    explicit Parser(Lexer &lexer)
    : _lexer{lexer}
    {}

public:
    // Parse the token stream
    void parse();
    // Parse the token stream with verbose tracing
    void parseTrace()
    {
        _shouldTrace = true;
        return parse();
    }

private:

    template <typename... TokenTypes>
    requires (std::same_as<TokenTypes, TokenType> && ...)
    auto match(TokenTypes... types) -> std::optional<Token>
    {
        return (match(types) || ...);
    }

    // Token processing
    // Asserts that a given token must come next and increments the cursor on success
    // throws a ParseError on failure
    auto match(TokenType type) -> std::optional<Token>;

    void mustMatch(TokenType type)
    {
        if(!match(type))
            unexpectedTokenError;
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

    auto addressAndPort() -> std::pair<std::string, std::vector<uint16_t>>;
    auto numberList() -> std::vector<uint16_t>;

    void unexpectedTokenError()
    {
        throw ParseError{std::format("Unexpected token, got: {}", peek().description())};
    }

private:
    Lexer &_lexer;
    Token _lookahead{};
    bool _shouldTrace{false};
};

}
