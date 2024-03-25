#pragma once

#include <string>
#include <guiddef.h>

class WfpNameMapper
{
public:
    static std::string convertToFriendlyName(const GUID& guidName);
};
