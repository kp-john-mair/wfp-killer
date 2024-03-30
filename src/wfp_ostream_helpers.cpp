#include <winsock2.h>
#include <ws2tcpip.h>

#include <iomanip>
#include <algorithm>

#include "wfp_ostream_helpers.h"
#include "wfp_name_mapper.h"
#include <fwpmu.h>
#include <format>

namespace wfpk {
namespace
{
    std::string ipToString(UINT32 ipAddress)
    {
        char str[INET_ADDRSTRLEN]{};
        InetNtopA(AF_INET, &ipAddress, str, INET_ADDRSTRLEN);

        return std::string{str};
    }

    std::string blobToString(const FWP_BYTE_BLOB &blob)
    {
        UINT8* data = blob.data;
        size_t numChars = blob.size / sizeof(wchar_t) - 1;

        std::string str;
        std::wstring wstr(reinterpret_cast<const wchar_t*>(data), numChars);

        // hack to convert wide strings to strings
        std::transform(wstr.begin(), wstr.end(), std::back_inserter(str), [] (wchar_t c) {
            return static_cast<char>(c);
        });

        return str;
    }
}

std::ostream& operator<<(std::ostream& os, const FWPM_NET_EVENT& event)
{
    const FWPM_NET_EVENT_HEADER &header = event.header;

    if(header.ipProtocol == AF_INET6)
    {
        os << "IPv6 not yet supported";
        return os;
    }

    std::string eventType = WfpNameMapper::getName(event.type).friendlyName;

    os << std::format("{} {} {}:{} -> {}:{}", eventType, blobToString(header.appId), ipToString(ntohl(header.localAddrV4)), header.localPort, ipToString(ntohl(header.remoteAddrV4)), header.remotePort);

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
        UINT8* data = condition.conditionValue.byteBlob->data;

        size_t numChars = condition.conditionValue.byteBlob->size / sizeof(wchar_t) - 1;

        std::string str;
        std::wstring wstr(reinterpret_cast<const wchar_t*>(data), numChars);

        // hack to convert wide strings to strings
        std::transform(wstr.begin(), wstr.end(), std::back_inserter(str), [] (wchar_t c) {
            return static_cast<char>(c);
        });

        auto pos = str.find_last_of('\\');

        if(pos != std::string::npos)
        {
            std::string filename = str.substr(pos + 1);
            os << filename;

        }
        else
        {
           os << str;
        }

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
        char str[INET6_ADDRSTRLEN]{};
        InetNtopA(AF_INET6, &condition.conditionValue.v6AddrMask->addr, str, INET6_ADDRSTRLEN);

        os << str;
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
