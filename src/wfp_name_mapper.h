#pragma once
#include <windows.h>
#include <fwpmu.h>
#include <string>

namespace wfpk {
    // The names of a WFP entity.
    // Both the friendlyName (human readable) as well as the
    // raw WFP name (often more arcane).
    struct WfpName
    {
        std::string friendlyName;
        std::string rawName;
    };

// Our own enum mapping of non-enum types such as FWP_ACTION_TYPE
// which is just an alias for a UINT32 - so we map it to an enum value
// so that we can provide a template specialization for looking up its
// 'values' (which are also just plain ints, like FWP_ACTION_BLOCK )
enum WFPK_TYPES
{
    WFPK_ACTION_TYPE,
    // Maps to the IPPROTO_* fields found here: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
    WFPK_IPPROTO_TYPE
};

// Maps a WFP name to something human readable
class WfpNameMapper
{
public:
    static WfpName getName(const GUID& guidName);
    static WfpName getName(const FWP_MATCH_TYPE &matchType);
    static WfpName getName(const FWPM_NET_EVENT_TYPE &eventType);
    static WfpName getName(const FWP_IP_VERSION &ipVersion);

    // Primary template (not implemented)
    template <WFPK_TYPES type>
    static WfpName getName(UINT32 value);

    // Full specializations of above
    template <>
    static WfpName getName<WFPK_ACTION_TYPE>(UINT32 value);

    template <>
    static WfpName getName<WFPK_IPPROTO_TYPE>(UINT32 value);
};
}
