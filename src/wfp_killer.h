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

        bool isEmpty() const
        {
            return providerMatchers.empty() && layerMatchers.empty() && subLayerMatchers.empty();
        }
    };

public:
    void listFilters(const Options &options) const;
    void deleteFilters(const std::vector<FilterId> &filterIds) const;
    void monitor();

private:
    bool deleteSingleFilter(FilterId filterId) const;
    bool isNameMatched(const std::vector<std::regex> &matchers, const std::string &name) const;
    bool isFilterNameMatched(const std::vector<std::regex> &matchers, const std::shared_ptr<FWPM_FILTER> &pFilter) const;

private:
    Engine _engine;
};
}
