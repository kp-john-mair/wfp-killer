#include <winsock2.h>
#include <ws2tcpip.h>

#include <iomanip>
#include <algorithm>

#include "wfp_ostream_helpers.h"
#include "wfp_name_mapper.h"
#include <fwpmu.h>
#include <format>

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER& filter)
{
    os << std::format("[Id: {}] [Weight: {:2}]", filter.filterId, static_cast<int>(filter.weight.uint8));

    os << std::setw(7);

    switch(filter.action.type)
    {
    case FWP_ACTION_BLOCK:
        os << "BLOCK ";
        break;
    case FWP_ACTION_PERMIT:
        os << "PERMIT ";
        break;
    case FWP_ACTION_CALLOUT_TERMINATING:
        os << "CALLOUT ";
        break;
    default:
        os << "UNKNOWN ";
    }

    os << WfpNameMapper::convertToFriendlyName(filter.layerKey);
    os << " ";

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
            os << filter.filterCondition[conditionCount];
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER_CONDITION& condition)
{
    os << "<";
    os << WfpNameMapper::convertToFriendlyName(condition.fieldKey);
    os << " ";

    switch(condition.matchType)
    {
    case FWP_MATCH_EQUAL:
        os << "equals ";
        break;
    default:
        os << "unknown_match_type";
    }

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

        if (pos != std::string::npos)
        {
            std::string filename = str.substr(pos + 1);
            os << filename;

        } else
        {
           os << str;
        }

        break;
    }
    case FWP_V4_ADDR_MASK:
    {
        UINT32 ipAddress = condition.conditionValue.v4AddrMask->addr;

        auto ipToString = [&](UINT32 ipAddress)
        {
            char str[INET_ADDRSTRLEN]{};
            InetNtopA(AF_INET, &ipAddress, str, INET_ADDRSTRLEN);

            return std::string{str};
        };

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
