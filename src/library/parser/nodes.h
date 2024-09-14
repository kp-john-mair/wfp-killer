#pragma once

#include <parser/lexer.h>

namespace wfpk
{
// Forward declare our visitor
class WfpExecutor;

// Stores a collection of ipv4 and ipv6 addresses
struct IpAddresses
{
    std::vector<std::string> v4;
    std::vector<std::string> v6;

    bool empty() const
    {
        return v4.empty() && v6.empty();
    }

    auto operator<=>(const IpAddresses &) const = default;
};

class Node
{
    // Make it uninstantiable as it's an abstract class
protected:
    Node() = default;

public:
    virtual ~Node() = default;

    virtual void accept(const WfpExecutor &visitor) = 0;

    void addChild(std::unique_ptr<Node> child)
    {
        _pChildren.push_back(std::move(child));
    }

    auto children() const -> const std::vector<std::unique_ptr<Node>> &
    {
        return _pChildren;
    }

    virtual std::string toString() const
    {
        return "N/A";
    }

private:
    std::vector<std::unique_ptr<Node>> _pChildren;
    Token _token{};
};

class RulesetNode final : public Node, private OStreamTraceable<RulesetNode>
{
public:
    RulesetNode() = default;

    void accept(const WfpExecutor &visitor) override;

    std::string toString() const override
    {
        std::string result;
        for(const auto &child : children())
        {
            result += std::format("{}\n", child->toString());
        }

        return result;
    }
};

struct FilterConditions
{
    enum class IpVersion
    {
        BothInet4Inet6,
        Inet4,
        Inet6
    };
    enum class TransportProtocol
    {
        AllTransports,
        Tcp,
        Udp
    };

    std::vector<uint16_t> sourcePorts;
    std::vector<uint16_t> destPorts;
    std::string sourceApp;
    IpAddresses sourceIps;
    IpAddresses destIps;
    std::string interfaceName;
    IpVersion ipVersion{};
    TransportProtocol transportProtocol{};

    auto operator<=>(const FilterConditions &) const = default;
};

// Represents no conditions be applied
inline const FilterConditions NoFilterConditions = {};

class FilterNode final : public Node, private OStreamTraceable<FilterNode>
{
public:
    enum class Action
    {
        Invalid,
        Block,
        Permit
    };
    enum class Direction
    {
        Invalid,
        Out,
        In
    };

    FilterNode(Action action, Direction direction, FilterConditions conditions)
        : _action{action}
        , _direction{direction}
        , _conditions{conditions}
    {}

    void accept(const WfpExecutor &visitor) override;

    Action action() const
    {
        return _action;
    }
    Direction direction() const
    {
        return _direction;
    }
    const FilterConditions &filterConditions() const
    {
        return _conditions;
    }
    std::string toString() const override;

private:
    Action _action{};
    Direction _direction{};
    FilterConditions _conditions{NoFilterConditions};
};

}
