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

namespace
{
    const std::unordered_map<GUID, std::string> kWfpNameMap {
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
}

std::string WfpNameMapper::convertToFriendlyName(const GUID& guidName)
{
    auto it = kWfpNameMap.find(guidName);

    if(it != kWfpNameMap.end())
    {
        return it->second;
    }
    else
    {
        return "UNKNOWN-GUID";
    }
}
