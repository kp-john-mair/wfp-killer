#pragma once

#include <parser/lexer.h>
#include <utility>

namespace wfpk {

class Node
{
protected:
    // Make it uninstantiable as it's an abstract class
    Node() = default;

public:
    virtual ~Node() = default;

    void addChild(std::unique_ptr<Node> child)
    {
        _pChildren.push_back(std::move(child));
    }

    auto children() const
        -> const std::vector<std::unique_ptr<Node>>&
    {
        return _pChildren;
    }

    virtual std::string toString() const { return "N/A"; }

private:
    std::vector<std::unique_ptr<Node>> _pChildren;
    Token _token{EndOfInputToken};
};

class RulesetNode final : public Node, private OStreamTraceable<RulesetNode>
{
public:
    RulesetNode() = default;

    std::string toString() const override
    {
        std::string result;
        for(const auto &child : children())
            result += std::format("{}\n", child->toString());

        return result;
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
    IpVersion ipVersion{IpVersion::Both};

    auto operator<=>(const FilterConditions&) const = default;
};

// Represents no conditions be applied
inline const FilterConditions NoFilterConditions = {};

class FilterNode final : public Node,  private OStreamTraceable<FilterNode>
{
public:
    enum class Action { Block, Permit };
    using enum Action;

    enum class Layer { AUTH_CONNECT_V4,
                       AUTH_RECV_V4,
                       AUTH_CONNECT_V6,
                       AUTH_RECV_V6 };
    using enum Layer;

    FilterNode(Action action, Layer layer, FilterConditions conditions)
    : _action{action}
    , _layer{layer}
    , _conditions{conditions}
    {
    }

    Action action() const { return _action; }
    Layer layer() const { return _layer; }
    const FilterConditions &filterConditions() const { return _conditions; }
    std::string toString() const override;

private:
    Action _action{};
    Layer _layer{};
    FilterConditions _conditions;
};

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
