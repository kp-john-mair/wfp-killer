#include <winsock2.h>
#include <ws2tcpip.h>
#include "wfp_name_mapper.h"
#include <unordered_map>
#include <iostream>
#include "utils.h"

namespace wfpk {
namespace
{
    #define WFP_NAME(guid, friendlyName) { guid, { friendlyName, #guid } }

    const std::unordered_map<GUID, WfpName> kGuidWfpNameMap {
        // Layer names, see full list here: https://learn.microsoft.com/en-us/windows/win32/fwp/management-filtering-layer-identifiers-
        WFP_NAME(FWPM_LAYER_ALE_AUTH_CONNECT_V4, "[Ipv4 outbound]"),
        WFP_NAME(FWPM_LAYER_ALE_AUTH_CONNECT_V6, "[Ipv6 outbound]"),
        WFP_NAME(FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4, "[Ipv4 inbound]"),
        WFP_NAME(FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6, "[Ipv6 inbound]"),
        WFP_NAME(FWPM_LAYER_ALE_BIND_REDIRECT_V4, "[Ipv4 bind-redirect]"),
        WFP_NAME(FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4, "[Ipv4 flow-established]"),
        WFP_NAME(FWPM_LAYER_ALE_CONNECT_REDIRECT_V4, "[Ipv4 connect-redirect]"),
        WFP_NAME(FWPM_LAYER_INBOUND_IPPACKET_V4, "[Ipv4 packet-inbound]"),
        WFP_NAME(FWPM_LAYER_OUTBOUND_IPPACKET_V4, "[Ipv4 packet-outbound]"),

        // Condition types, see full list here: https://learn.microsoft.com/en-us/windows/win32/fwp/filtering-condition-identifiers-
        WFP_NAME(FWPM_CONDITION_ALE_APP_ID, "app_id"),
        WFP_NAME(FWPM_CONDITION_IP_REMOTE_ADDRESS, "remote_ip"),
        WFP_NAME(FWPM_CONDITION_IP_REMOTE_PORT, "remote_port"),
        WFP_NAME(FWPM_CONDITION_IP_LOCAL_ADDRESS, "local_ip"),
        WFP_NAME(FWPM_CONDITION_IP_LOCAL_PORT, "local_port"),
        WFP_NAME(FWPM_CONDITION_IP_LOCAL_INTERFACE, "local_interface"),
        WFP_NAME(FWPM_CONDITION_IP_PROTOCOL, "protocol"),
        WFP_NAME(FWPM_CONDITION_INTERFACE_TYPE, "interface_type"),
        WFP_NAME(FWPM_CONDITION_INTERFACE_INDEX, "interface_index")
    };

    // Match types, see full list here: https://learn.microsoft.com/en-us/windows/win32/api/fwptypes/ne-fwptypes-fwp_match_type
    const std::unordered_map<FWP_MATCH_TYPE, WfpName> kMatchTypeMap {
        WFP_NAME(FWP_MATCH_EQUAL, "equal"),
        WFP_NAME(FWP_MATCH_GREATER, "greater"),
        WFP_NAME(FWP_MATCH_LESS, "less than"),
        WFP_NAME(FWP_MATCH_GREATER_OR_EQUAL, "great or equal"),
        WFP_NAME(FWP_MATCH_LESS_OR_EQUAL, "less or equal"),
        WFP_NAME(FWP_MATCH_RANGE, "range match"),
        WFP_NAME(FWP_MATCH_NOT_EQUAL, "not equal"),
        WFP_NAME(FWP_MATCH_PREFIX, "prefix equal"),
        WFP_NAME(FWP_MATCH_NOT_PREFIX, "prefix not equal")
    };

    // IP Version types, see full list here: https://learn.microsoft.com/en-us/windows/win32/api/fwptypes/ne-fwptypes-fwp_ip_version
    const std::unordered_map<FWP_IP_VERSION, WfpName> kIpVersionMap {
        WFP_NAME(FWP_IP_VERSION_V4, "Ipv4"),
        WFP_NAME(FWP_IP_VERSION_V6, "Ipv6"),
        WFP_NAME(FWP_IP_VERSION_NONE, "Invalid (No Ip version)"),
        WFP_NAME(FWP_IP_VERSION_MAX, "Invalid (Max Ip Version)"),
    };

    // Action types, see full list: https://learn.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_action0
    const std::unordered_map<UINT32, WfpName> kActionTypeMap {
        WFP_NAME(FWP_ACTION_BLOCK, "block"),
        WFP_NAME(FWP_ACTION_PERMIT, "permit"),
        WFP_NAME(FWP_ACTION_CALLOUT_TERMINATING, "callout"),
        WFP_NAME(FWP_ACTION_CALLOUT_INSPECTION, "callout_inspection"),
        WFP_NAME(FWP_ACTION_CALLOUT_UNKNOWN, "callout_unknown")
    };

    // See IPPROTO_* section of https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
    const std::unordered_map<UINT32, WfpName> kIpProtoTypeMap {
        WFP_NAME(IPPROTO_ICMP, "icmp"),
        WFP_NAME(IPPROTO_IGMP, "igmp"),
        WFP_NAME(IPPROTO_TCP, "tcp"),
        WFP_NAME(IPPROTO_UDP, "udp"),
        WFP_NAME(IPPROTO_ICMPV6, "ipv6 icmp"),
    };

    // Event types, see full list: https://learn.microsoft.com/en-us/windows/win32/api/fwpmtypes/ne-fwpmtypes-fwpm_net_event_type
    const std::unordered_map<FWPM_NET_EVENT_TYPE, WfpName> kEventTypeMap {
        WFP_NAME(FWPM_NET_EVENT_TYPE_CLASSIFY_DROP, "drop"),
        WFP_NAME(FWPM_NET_EVENT_TYPE_CLASSIFY_ALLOW, "allow"),
    };

    // Generic lookup function with fallback
    template <typename MapT, typename KeyT>
    WfpName nameLookup(const MapT &map, const KeyT &key, const std::string &unknownFallback)
    {
        auto it = map.find(key);

        if(it != map.end())
        {
            return it->second;
        }
        else
        {
            // Show the unknown key in the return value
            std::string unknownMessage{unknownFallback + ": "};
            if constexpr(std::is_same_v<std::decay_t<KeyT>, GUID>)
                unknownMessage += guidToString(key);
            else
                unknownMessage += std::to_string(static_cast<uint32_t>(key));

            return { unknownMessage, unknownMessage };
        }
    }
}

WfpName WfpNameMapper::getName(const GUID &guidName)
{
    return nameLookup(kGuidWfpNameMap, guidName, "UNKNOWN-GUID");
}

WfpName WfpNameMapper::getName(const FWP_MATCH_TYPE &matchType)
{

    return nameLookup(kMatchTypeMap, matchType, "UNKNOWN-MATCHTYPE");
}

static WfpName getName(const FWP_IP_VERSION &ipVersion)
{
    return nameLookup(kIpVersionMap, ipVersion, "UNKNOWN-IPVERSION");
}

 WfpName WfpNameMapper::getName(const FWPM_NET_EVENT_TYPE &eventType)
 {
     return nameLookup(kEventTypeMap, eventType, "UNKNOWN-EVENTTYPE");
 }

template <>
WfpName WfpNameMapper::getName<WFPK_ACTION_TYPE, UINT32>(UINT32 value)
{
    return nameLookup(kActionTypeMap, value, "UNKNOWN-ACTIONTYPE");
}

template <>
WfpName WfpNameMapper::getName<WFPK_IPPROTO_TYPE, UINT8>(UINT8 value)
{
    return nameLookup(kIpProtoTypeMap, value, "UNKNOWN-IPPROTOTYPE");
}

#undef WFP_NAME
}
