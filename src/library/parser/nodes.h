#pragma once

#include <parser/lexer.h>

namespace wfpk {
class Node
{
// Make it uninstantiable as it's an abstract class
protected:
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

class RulesetNode final : public Node,
    private OStreamTraceable<RulesetNode>
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
    FilterConditions _conditions{NoFilterConditions};
};

}
