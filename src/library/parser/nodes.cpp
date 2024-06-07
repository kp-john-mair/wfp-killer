#include <parser/nodes.h>

namespace wfpk {
std::string FilterNode::toString() const
{
    std::string output;

    output += enumName(action()) + " " + enumName(layer()) + " ";

    auto conditions = filterConditions();

    if(conditions == NoFilterConditions)
    {
        output += "all ";
        return output;
    }

    output += enumName(conditions.ipVersion) + " ";

    if(!conditions.sourceIp.empty() || !conditions.sourcePorts.empty())
        output += "from ";

    if(!conditions.sourceIp.empty())
        output += conditions.sourceIp + " ";

    if(!conditions.sourcePorts.empty())
        output += std::format("port {{ {} }}", joinVec(conditions.sourcePorts)) + " ";

    if(!conditions.destIp.empty() || !conditions.destPorts.empty())
        output += "to ";

    if(!conditions.destIp.empty())
        output += conditions.destIp + " ";

    if(!conditions.destPorts.empty())
        output += std::format("port {{ {} }}", joinVec(conditions.destPorts)) + " ";

    return output;
}
}
