#pragma once

#include "wfp_objects.h"
#include <string>
#include <vector>

namespace wfpk {
// Core application class
class WfpKiller
{
public:
    struct Options
    {
        std::vector<std::string> providers;
        std::vector<std::string> layers;
        std::vector<std::string> subLayers;
    };

public:
    void listFilters(const Options &options) const;
    void deleteFilters(const std::vector<FilterId> &filterIds) const;
    void monitor();

private:
    bool deleteSingleFilter(FilterId filterId) const;
    bool isProviderMatched(const std::vector<std::string> &providerMatchers, const GUID &providerKey) const;

private:
    Engine _engine;
};
}
