#include <winsock2.h>
#include <ws2tcpip.h>

#include <iomanip>
#include <algorithm>
#include <format>
#include <filesystem>

#include "wfp_ostream_helpers.h"
#include "wfp_name_mapper.h"
#include "wfp_objects.h"
#include <fwpmu.h>
#include "utils.h"

namespace wfpk {

std::ostream& operator<<(std::ostream& os, const FWPM_NET_EVENT& event)
{
    const FWPM_NET_EVENT_HEADER &header = event.header;

    if(!(header.ipProtocol == IPPROTO_ICMP || header.ipProtocol == IPPROTO_TCP || header.ipProtocol == IPPROTO_UDP))
    {
        os << std::format("Only some protocols are supported! Got unsupported procotol number: {}\n", header.ipProtocol);
        return os;
    }

    std::string eventType = WfpNameMapper::getName(event.type).friendlyName;
    UINT64 filterId{};

    switch(event.type)
    {
    case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP:
        filterId = event.classifyDrop->filterId;
        break;
    case FWPM_NET_EVENT_TYPE_CLASSIFY_ALLOW:
        filterId = event.classifyAllow->filterId;
        break;
    }

    std::string localAddress;
    std::string remoteAddress;
    switch(header.ipVersion)
    {
    case FWP_IP_VERSION_V4:
        localAddress = ipToString(ntohl(header.localAddrV4));
        remoteAddress = ipToString(ntohl(header.remoteAddrV4));
    break;
    case FWP_IP_VERSION_V6:
        localAddress = ipToString(header.localAddrV6.byteArray16);
        remoteAddress = ipToString(header.remoteAddrV6.byteArray16);
    break;
    }

    std::string protocol = WfpNameMapper::getName<WFPK_IPPROTO_TYPE>(static_cast<UINT32>(header.ipProtocol)).friendlyName;
    // Clip it to just the filename, not the full path
    std::string fileName = std::filesystem::path{blobToString(header.appId)}.filename().string();

    os << std::format("[protocol: {}] [FilterId: {}] {} {} {}:{} -> {}:{}", protocol, filterId, eventType, fileName, localAddress, header.localPort, remoteAddress, header.remotePort);

    return os;
}

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER& filter)
{
    // TODO: Treat filter weights in a more generic way - PIA just uses a uint8, but that
    // won't be true of all providers
    os << std::format("[Id: {}] [Weight: {:2}]", filter.filterId, static_cast<int>(filter.weight.uint8));
    os << std::setw(8);
    os << WfpNameMapper::getName<WFPK_ACTION_TYPE>(filter.action.type).friendlyName << " ";
    os << WfpNameMapper::getName(filter.layerKey).friendlyName << " ";

    if(filter.numFilterConditions == 0)
    {
        // Indicate there's no conditions
        os << "None";
    }
    else
    {
        // Show the filter conditions
        for(size_t conditionCount = 0; conditionCount < filter.numFilterConditions; ++conditionCount)
        {
            os << filter.filterCondition[conditionCount] << " ";
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER_CONDITION& condition)
{
    os << "<";
    os << WfpNameMapper::getName(condition.fieldKey).friendlyName << " ";
    os << WfpNameMapper::getName(condition.matchType).friendlyName << " ";

    // See full list here: https://learn.microsoft.com/en-us/windows/win32/api/fwptypes/ns-fwptypes-fwp_condition_value0
    switch(condition.conditionValue.type)
    {
    case FWP_EMPTY:
        os << "empty";
        break;
    case FWP_UINT8:
        os << static_cast<UINT32>(condition.conditionValue.uint8);
        break;
    case FWP_UINT16:
        os << static_cast<UINT32>(condition.conditionValue.uint16);
        break;
    case FWP_UINT32:
        os << static_cast<UINT32>(condition.conditionValue.uint32);
        break;
    case FWP_UINT64:
        os << *condition.conditionValue.uint64;
        break;
    case FWP_BYTE_BLOB_TYPE:
    {
        std::filesystem::path path = blobToString(*condition.conditionValue.byteBlob);
        os << path.filename().string();

        break;
    }
    case FWP_V4_ADDR_MASK:
    {
        UINT32 ipAddress = condition.conditionValue.v4AddrMask->addr;

        os << ipToString(ntohl(ipAddress));
        os << " / ";
        os <<  ipToString(ntohl(condition.conditionValue.v4AddrMask->mask));

        break;
    }
    case FWP_V6_ADDR_MASK:
    {
        os << ipToString(condition.conditionValue.v6AddrMask->addr);
        os << " / ";
        os <<  static_cast<UINT32>(condition.conditionValue.v6AddrMask->prefixLength);

        break;
    }

    default:
        os << "unknown";
    }

    os << ">";

    return os;
}
}
