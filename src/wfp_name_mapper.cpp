#include "wfp_name_mapper.h"
#include <unordered_map>
#include <iostream>

namespace std
{
    template <>
    struct hash<GUID>
    {
        size_t operator()(const GUID& guid) const
        {
            const uint64_t *p = reinterpret_cast<const uint64_t*>(&guid);
            return hash<uint64_t>{}(p[0]) ^ hash<uint64_t>{}(p[1]);
        }
    };
}

namespace wfpk {
namespace
{
    #define WFP_NAME(rawName, friendlyName) { rawName, { friendlyName, #rawName } }

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
    };

    // Match types, see full list here: https://learn.microsoft.com/en-us/windows/win32/api/fwptypes/ne-fwptypes-fwp_match_type
    const std::unordered_map<FWP_MATCH_TYPE, WfpName> kMatchTypeMap {
        WFP_NAME(FWP_MATCH_EQUAL, "equal"),
        WFP_NAME(FWP_MATCH_GREATER, "greater"),
        WFP_NAME(FWP_MATCH_LESS, "less than"),
        WFP_NAME(FWP_MATCH_GREATER_OR_EQUAL, "great or equal"),
        WFP_NAME(FWP_MATCH_LESS_OR_EQUAL, "less or equal")
    };

    // Action types, see full list: https://learn.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_action0
    const std::unordered_map<UINT32, WfpName> kActionTypeMap {
        WFP_NAME(FWP_ACTION_BLOCK, "block"),
        WFP_NAME(FWP_ACTION_PERMIT, "permit"),
        WFP_NAME(FWP_ACTION_CALLOUT_TERMINATING, "callout"),
        WFP_NAME(FWP_ACTION_CALLOUT_INSPECTION, "callout_inspection"),
        WFP_NAME(FWP_ACTION_CALLOUT_UNKNOWN, "callout-unknown")
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
            return { unknownFallback, unknownFallback };
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

 WfpName WfpNameMapper::getName(const FWPM_NET_EVENT_TYPE &eventType)
 {
     return nameLookup(kEventTypeMap, eventType, "UNKNOWN-EVENTTYPE");
 }

template <>
WfpName WfpNameMapper::getName<WFPK_ACTION_TYPE>(UINT32 value)
{
    return nameLookup(kActionTypeMap, value, "UNKNOWN-ACTIONTYPE");
}

#undef WFP_NAME
}
