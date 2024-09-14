#include <parser/nodes.h>
#include <visitors/wfp_executor.h>

namespace wfpk
{

void RulesetNode::accept(const WfpExecutor &visitor)
{
    visitor.visit(*this);
}

void FilterNode::accept(const WfpExecutor &visitor)
{
    visitor.visit(*this);
}

std::string FilterNode::toString() const
{
    std::string output;

    output += enumName(action()) + " " + enumName(direction()) + " ";

    auto conditions = filterConditions();

    if(conditions == NoFilterConditions)
    {
        output += "all ";
        return output;
    }

    // Inet4 vs Inet6
    output += enumName(conditions.ipVersion) + " ";
    // Tcp vs Udp
    output += enumName(conditions.transportProtocol) + " ";

    if(!conditions.sourceIps.empty() || !conditions.sourcePorts.empty())
    {
        output += "from ";
    }

    if(!conditions.sourceIps.empty())
    {
        const auto &combinedSourceIps = concatVec(conditions.sourceIps.v4, conditions.sourceIps.v6);
        output += joinVec(combinedSourceIps) + " ";
    }

    if(!conditions.sourcePorts.empty())
    {
        output += std::format("port {{ {} }}", joinVec(conditions.sourcePorts)) + " ";
    }

    if(!conditions.destIps.empty() || !conditions.destPorts.empty())
    {
        output += "to ";
    }

    if(!conditions.destIps.empty())
    {
        const auto &combinedDestIps = concatVec(conditions.destIps.v4, conditions.destIps.v6);
        output += joinVec(combinedDestIps) + " ";
    }

    if(!conditions.destPorts.empty())
    {
        output += std::format("port {{ {} }}", joinVec(conditions.destPorts)) + " ";
    }

    return output;
}
}
