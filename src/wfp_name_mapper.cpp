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
    const std::unordered_map<GUID, std::string> kGuidWfpNameMap {
        // Layer names, see full list here: https://learn.microsoft.com/en-us/windows/win32/fwp/management-filtering-layer-identifiers-
        { FWPM_LAYER_ALE_AUTH_CONNECT_V4, "[Ipv4 outbound]" },
        { FWPM_LAYER_ALE_AUTH_CONNECT_V6, "[Ipv6 outbound]" },
        { FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4, "[Ipv4 inbound]" },
        { FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6, "[Ipv6 inbound]" },
        { FWPM_LAYER_ALE_BIND_REDIRECT_V4, "[Ipv4 bind-redirect]" },
        { FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4, "[Ipv4 flow-established]" },
        { FWPM_LAYER_ALE_CONNECT_REDIRECT_V4, "[Ipv4 connect-redirect]"},
        { FWPM_LAYER_INBOUND_IPPACKET_V4, "[Ipv4 packet-inbound]" },
        { FWPM_LAYER_OUTBOUND_IPPACKET_V4, "[Ipv4 packet-outbound]" },

        // Condition types, see full list here: https://learn.microsoft.com/en-us/windows/win32/fwp/filtering-condition-identifiers-
        { FWPM_CONDITION_ALE_APP_ID, "app_id" },
        { FWPM_CONDITION_IP_REMOTE_ADDRESS, "remote_ip" },
        { FWPM_CONDITION_IP_REMOTE_PORT, "remote_port" },
        { FWPM_CONDITION_IP_LOCAL_ADDRESS, "local_ip" },
        { FWPM_CONDITION_IP_LOCAL_PORT, "local_port" },
        { FWPM_CONDITION_IP_LOCAL_INTERFACE, "local_interface" },
    };

    // Match types, see full list here: https://learn.microsoft.com/en-us/windows/win32/api/fwptypes/ne-fwptypes-fwp_match_type
    const std::unordered_map<FWP_MATCH_TYPE, std::string> kMatchTypeMap {
        { FWP_MATCH_EQUAL, "equal" },
        { FWP_MATCH_GREATER, "greater" },
        { FWP_MATCH_LESS, "less than" },
        { FWP_MATCH_GREATER_OR_EQUAL, "great or equal" },
        { FWP_MATCH_LESS_OR_EQUAL, "less or equal" }
    };

    // Action types, see full list: https://learn.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_action0
    const std::unordered_map<UINT32, std::string> kActionTypeMap {
        { FWP_ACTION_BLOCK, "block" },
        { FWP_ACTION_PERMIT, "permit" },
        { FWP_ACTION_CALLOUT_TERMINATING, "callout" },
        { FWP_ACTION_CALLOUT_INSPECTION, "callout_inspection" },
        { FWP_ACTION_CALLOUT_UNKNOWN, "callout-unknown" }
    };

    // Generic lookup function with fallback
    template <typename MapT, typename KeyT>
    std::string nameLookup(const MapT &map, const KeyT &key, const std::string &unknownFallback)
    {
        auto it = map.find(key);

        if(it != map.end())
        {
            return it->second;
        }
        else
        {
            return unknownFallback;
        }
    }
}

std::string WfpNameMapper::convertToFriendlyName(const GUID &guidName)
{

    return nameLookup(kGuidWfpNameMap, guidName, "UNKNOWN-GUID");
}

std::string WfpNameMapper::convertToFriendlyName(const FWP_MATCH_TYPE &matchType)
{

    return nameLookup(kMatchTypeMap, matchType, "UNKNOWN-MATCHTYPE");
}

template <>
static std::string WfpNameMapper::convertToFriendlyName<WFPK_ACTION_TYPE>(UINT32 value)
{
    return nameLookup(kActionTypeMap, value, "UNKNOWN-ACTIONTYPE");
}
}
