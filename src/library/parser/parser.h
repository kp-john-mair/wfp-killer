#pragma once

#include <parser/lexer.h>
#include <utility>

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
    virtual ~Node() = default;

    void accept(const Visitor &visitor)
    {
        acceptNode(visitor);
    }

    void addChild(std::unique_ptr<Node> child)
    {
        _pChildren.push_back(std::move(child));
    }

    auto children() -> const std::vector<std::unique_ptr<Node>>&
    {
        return _pChildren;
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
    RulesetNode() = default;

private:
    virtual void acceptNode(const Visitor &visitor) override {}
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

    FilterNode(Action action, Layer layer, FilterConditions conditions)
    : _action{action}
    , _layer{layer}
    , _conditions{conditions}
    {
    }

    Action action() const { return _action; }
    Layer layer() const { return _layer; }
    const FilterConditions &filterConditions() const { return _conditions; }
private:
    virtual void acceptNode(const Visitor &visitor) override {}

private:
    Action _action{};
    Layer _layer{};
    FilterConditions _conditions;
};

inline std::ostream& operator<<(std::ostream &ostream, const FilterNode &node)
{

    ostream << enumName(node.action()) << " " << enumName(node.layer()) << " ";

    auto conditions = node.filterConditions();

    if(conditions == NoFilterConditions)
    {
        ostream << "all ";
        return ostream;
    }

    ostream << enumName(conditions.ipVersion) << " ";

    if(!conditions.sourceIp.empty() || !conditions.sourcePorts.empty())
        ostream << "from ";

    if(!conditions.sourceIp.empty())
        ostream << conditions.sourceIp << " ";

    if(!conditions.sourcePorts.empty())
        ostream << std::format("port {{ {} }}", joinVec(conditions.sourcePorts)) << " ";

    if(!conditions.destIp.empty() || !conditions.destPorts.empty())
        ostream << "to ";

    if(!conditions.destIp.empty())
        ostream << conditions.destIp << " ";

    if(!conditions.destPorts.empty())
        ostream << std::format("port {{ {} }}", joinVec(conditions.destPorts)) << " ";

    return ostream;
}

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
