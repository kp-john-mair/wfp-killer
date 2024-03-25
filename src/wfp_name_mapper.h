#pragma once
#include <windows.h>
#include <fwpmu.h>
#include <string>

class WfpNameMapper
{
public:
    static std::string convertToFriendlyName(const GUID& guidName);
    static std::string convertToFriendlyName(const FWP_MATCH_TYPE &matchType);
};
