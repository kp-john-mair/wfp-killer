#pragma once

#include "wfp_objects.h"
#include <string>
#include <vector>
#include <regex>

namespace wfpk {
// Core application class
class WfpKiller
{
public:
    struct Options
    {
        std::vector<std::regex> providerMatchers;
        std::vector<std::regex> layerMatchers;
        std::vector<std::regex> subLayerMatchers;
    };

public:
    void listFilters(const Options &options) const;
    void deleteFilters(const std::vector<FilterId> &filterIds) const;
    void monitor();

private:
    bool deleteSingleFilter(FilterId filterId) const;
    bool isProviderMatched(const std::vector<std::regex> &providerMatchers, const GUID &providerKey) const;

private:
    Engine _engine;
};
}
