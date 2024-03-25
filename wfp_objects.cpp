
#include <winsock2.h>
#include <ws2tcpip.h>

#include "wfp_objects.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>

namespace
{
    // Replace these GUIDs with the actual provider and sublayer GUIDs you want to target
    constexpr GUID PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
    constexpr GUID SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };
}

Engine::Engine()
: _handle{}
{

    DWORD result = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &_handle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmEngineOpen failed: " + result};
    }
}

Engine::~Engine()
{
    FwpmEngineClose0(_handle);
}

FilterEnum::FilterEnum(FWPM_FILTER_ENUM_TEMPLATE enumTemplate, HANDLE engineHandle)
: _numEntries{0}
, _filters{NULL}
, _engineHandle{engineHandle}
, _enumHandle{}
{
    DWORD result{ERROR_SUCCESS};

    result = FwpmFilterCreateEnumHandle(_engineHandle, &enumTemplate, &_enumHandle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterCreateEnumHandle failed: " + result};
    }

    result = FwpmFilterEnum(_engineHandle, _enumHandle, MaxFilterCount, &_filters, &_numEntries);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterEnum failed: " + result};
    }
}

FilterEnum::~FilterEnum()
{
    // Free the memory allocated for the filters array.
    FwpmFreeMemory(reinterpret_cast<void**>(&_filters));
    // Close the enumeration handle
    DWORD result{ERROR_SUCCESS};
    result = FwpmFilterDestroyEnumHandle(_engineHandle, _enumHandle);
    if(result != ERROR_SUCCESS)
    {
        // Just showing the error - cannot throw in a destructor
        std::cerr << "FwpmFilterDestroyEnumHandle failed: " + result << std::endl;
    }
}

bool WfpContext::process()
{
    FWPM_FILTER_ENUM_TEMPLATE enumTemplate = {0};
    enumTemplate.enumType = FWP_FILTER_ENUM_OVERLAPPING;
    enumTemplate.providerKey = const_cast<GUID*>(&PROVIDER_KEY);
    enumTemplate.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    enumTemplate.actionMask = 0xFFFFFFFF;

    FilterEnum filterEnum{enumTemplate, _engine};

    filterEnum.forEach([](const auto &filter) {
        std::cout << filter << std::endl;
    });

    //         result = FwpmFilterDeleteById(_engine, filterId);

    return true;
}

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER& filter)
{
    // Access the FWPM_FILTER's members and print them. Just as an example, let's say it has a name and description.
    os << "[Id: " << filter.filterId << "]" << " [Weight: " << std::setw(2) << static_cast<int>(filter.weight.uint8) << "] ";

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

    if(filter.layerKey == FWPM_LAYER_ALE_AUTH_CONNECT_V4)
    {
        os << "[Ipv4 outbound]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_AUTH_CONNECT_V6)
    {
        os << "[Ipv6 outbound]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4)
    {
        os << "[Ipv4 inbound]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6)
    {
        os << "[Ipv6 inbound]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_BIND_REDIRECT_V4)
    {
        os << "[Ipv4 bind-redirect]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4)
    {
        os << "[Ipv4 flow-established]";
    }
    else if(filter.layerKey == FWPM_LAYER_ALE_CONNECT_REDIRECT_V4)
    {
        os << "[Ipv4 connect-redirect]";
    }
    else if(filter.layerKey == FWPM_LAYER_INBOUND_IPPACKET_V4)
    {
        os << "[Ipv4 packet-inbound]";
    }
    else if(filter.layerKey == FWPM_LAYER_OUTBOUND_IPPACKET_V4)
    {
        os << "[Ipv4 packet-outbound]";
    }

    for(size_t conditionCount = 0; conditionCount < filter.numFilterConditions; ++conditionCount)
    {
        os << filter.filterCondition[conditionCount];
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const FWPM_FILTER_CONDITION& condition)
{
    os << " <";
    if(condition.fieldKey == FWPM_CONDITION_ALE_APP_ID)
    {
        os << "app_id ";
    }
    else if(condition.fieldKey == FWPM_CONDITION_IP_REMOTE_ADDRESS)
    {
        os << "remote_ip ";
    }
    else if(condition.fieldKey == FWPM_CONDITION_IP_REMOTE_PORT)
    {
        os << "remote_port ";
    }
    else if(condition.fieldKey == FWPM_CONDITION_IP_LOCAL_PORT)
    {
        os << "local_port ";
    }
    else if(condition.fieldKey == FWPM_CONDITION_IP_LOCAL_INTERFACE)
    {
        os << "local_interface ";
    }
    else
    {
        os << "unknown condition ";
    }

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

    default:
        os << "unknown";
    }

    os << ">";

    return os;
}
