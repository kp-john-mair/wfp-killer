#include <format>
#include <ranges>
#include <algorithm>
#include <regex>
#include <string>
#include <windows.h>
#include <stdlib.h>
#include "wfp_killer.h"
#include "wfp_ostream_helpers.h"
#include "wfp_name_mapper.h"

namespace wfpk {

namespace
{
    const std::vector kPiaLayers = {
        FWPM_LAYER_ALE_AUTH_CONNECT_V4,
        FWPM_LAYER_ALE_AUTH_CONNECT_V6,
        FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4,
        FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6,
        FWPM_LAYER_ALE_BIND_REDIRECT_V4,
        FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4,
        FWPM_LAYER_ALE_CONNECT_REDIRECT_V4,
        FWPM_LAYER_INBOUND_IPPACKET_V4,
        FWPM_LAYER_OUTBOUND_IPPACKET_V4
    };

    using Options = WfpKiller::Options;
}

void WfpKiller::listFilters(const Options &options) const
{
    size_t filterCount{0};

    for(const auto &layerKey : kPiaLayers)
    {
        std::cout << std::format("\nLayer: {}\n", WfpNameMapper::getName(layerKey).rawName);
        _engine.enumerateFiltersForLayer(layerKey, [&](const auto &pFilter) {
            if(options.providerMatchers.size())
            {
                // Skip empty providers - we only want to show filters
                // with a provider and whose provider matches one in our list
                if(!pFilter->providerKey)
                    return;

                if(!isProviderMatched(options.providerMatchers, *pFilter->providerKey))
                    return;
            }

            std::cout << *pFilter << std::endl;
            ++filterCount;
        });
    }

    std::cout << std::format("\nTotal number of filters: {}\n", filterCount);
}

void WfpKiller::deleteFilters(const std::vector<FilterId> &filterIds) const
{
    uint32_t deleteCount{0};

    if(filterIds.size() > 0)
    {
        std::cout << std::format("Will delete {} filters\n", filterIds.size());
        for(const auto &filterId : filterIds)
        {
            if(deleteSingleFilter(filterId))
                ++deleteCount;
        }
    }
    else
    {
        std::cout << "This action will delete ALL PIA filters\nAre you sure? (y/n)\n";
        char userDecision{};
        std::cin >> userDecision;
        if(userDecision == 'y')
        {
            _engine.enumerateFiltersForLayers(kPiaLayers, [&](const auto &pFilter) {
                if(deleteSingleFilter(pFilter->filterId))
                    ++deleteCount;
            });
        }
    }

    std::cout << std::format("Deleted {} filters.\n", deleteCount);
}

void WfpKiller::monitor()
{
    std::cout << "Monitoring network events - press enter or Ctrl+C to stop.\n";
    _engine.monitorEvents([](void *context, const FWPM_NET_EVENT *event) {
        std::cout << *event << "\n";
    });

    std::cin.get();
}

bool WfpKiller::deleteSingleFilter(FilterId filterId) const
{
    DWORD result = _engine.deleteFilterById(filterId);
    if(result != ERROR_SUCCESS)
    {
        std::cerr << std::format("Error: Failed to delete filter with id {}: {}\n", filterId, getErrorString(result));
        return false;
    }
    else
    {
        std::cout << std::format("Successfully deleted filter with id {}.\n", filterId);
        return true;
    }
}

bool WfpKiller::isProviderMatched(const std::vector<std::regex> &providerMatchers, const GUID &providerKey) const
{
    std::unique_ptr<FWPM_PROVIDER, WfpDeleter> pProvider{_engine.getProviderByKey(providerKey)};

    // lowercase the provider name - as we're doing a case insensitive compare
    const std::string providerName = toLowercase(wideStringToString(pProvider->displayData.name));

    bool isProviderMatch = std::ranges::any_of(providerMatchers, [&](const auto &providerMatcher) {
        return std::regex_search(providerName, providerMatcher);
    });

    return isProviderMatch;
}

}
