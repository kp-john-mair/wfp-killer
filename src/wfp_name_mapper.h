#pragma once
#include <windows.h>
#include <fwpmu.h>
#include <string>

namespace wfpk {
// Our own enum mapping of non-enum types such as FWP_ACTION_TYPE
// which is just an alias for a UINT32 - so we map it to an enum value
// so that we can provide a template specialization for looking up its
// 'values' (which are also just plain ints, like FWP_ACTION_BLOCK )
enum WFPK_TYPES
{
    WFPK_ACTION_TYPE
};

class WfpNameMapper
{
public:
    static std::string convertToFriendlyName(const GUID& guidName);
    static std::string convertToFriendlyName(const FWP_MATCH_TYPE &matchType);

    template <WFPK_TYPES type>
    static std::string convertToFriendlyName(UINT32 value);
};
}
