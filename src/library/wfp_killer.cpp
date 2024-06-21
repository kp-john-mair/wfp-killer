#include <format>
#include <ranges>
#include <algorithm>
#include <regex>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <wfp_killer.h>
#include <wfp_ostream_helpers.h>
#include <wfp_name_mapper.h>
#include <parser/parser.h>
#include <visitors/wfp_executor.h>

// We only need a minimal windows.h
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace wfpk {

namespace
{
    // The layers we care about in wfpk - there's many more layers than this
    // but these are the ones we're interested in for now.
    const std::vector kLayers = {
        FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
        FWPM_LAYER_OUTBOUND_TRANSPORT_V6,
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

void WfpKiller::loadFilters(const std::string &sourceFile)
{
    std::ifstream file{sourceFile};

    if(!file.is_open())
        throw std::runtime_error{std::format("Could not open file: {}", sourceFile)};

    std::stringstream buffer;
    buffer << file.rdbuf();

    auto ast = Parser{buffer.str()}.parse();
    WfpExecutor wfpExecutor{_engine};

    ast->accept(wfpExecutor);
}

// creates a dummy conditional filter that filters on the chrome app
void WfpKiller::createFilter()
{
    FWPM_FILTER filter{};

    std::unique_ptr<FWPM_PROVIDER, WfpDeleter> pProvider{_engine.getProviderByKey(PIA_PROVIDER_KEY)};

    // Basic filter setup
    filter.providerKey = &PIA_PROVIDER_KEY;
    filter.subLayerKey = PIA_SUBLAYER_KEY;
    filter.flags = FWPM_FILTER_FLAG_PERSISTENT | FWPM_FILTER_FLAG_INDEXED;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = 5;
    filter.displayData = pProvider->displayData;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    filter.action.type = FWP_ACTION_PERMIT;

    // Add a condition for an application id constraint
    FWPM_FILTER_CONDITION condition{};
    condition.fieldKey = FWPM_CONDITION_ALE_APP_ID;
    condition.matchType = FWP_MATCH_EQUAL;
    condition.conditionValue.type = FWP_BYTE_BLOB_TYPE;

    filter.filterCondition = &condition;
    filter.numFilterConditions = 1;

    // Add the application byte blob
    // Ensure we assign a wide string
    std::wstring appPath = L"c:/program files/google/chrome/application/chrome.exe";

    // Get the byte blob
    std::unique_ptr<FWP_BYTE_BLOB, WfpDeleter> pBlob{_engine.getAppIdFromFileName(appPath)};
    condition.conditionValue.byteBlob = pBlob.get();

    std::cout << "Trying to add new dummy filter to engine" << std::endl;

    auto id =  _engine.add(filter);
    if(id != 0)
    {
        std::cout << "Should have created a filter: " << id << std::endl;
    }
}

void WfpKiller::listFilters(const Options &options) const
{
    size_t filterCount{0};

    for(const auto &layerKey : kLayers)
    {
        const auto &filters = _engine.filtersForLayer(layerKey);

        std::unordered_map<GUID, FilterSet> filtersBySubLayer;
        FilterSet filtersWithoutSubLayer;

        for(const auto &pFilter : filters)
        {
            if(!options.isEmpty() && !isFilterNameMatched(options.providerMatchers, pFilter))
                continue;

             // Non-existent sublayers are represented as ZeroGuid
            if(pFilter->subLayerKey != ZeroGuid)
            {
                auto &set = filtersBySubLayer[pFilter->subLayerKey];
                set.insert(pFilter);
            }
            else
            {
                filtersWithoutSubLayer.insert(pFilter);
            }
        }

        if(filtersBySubLayer.empty() && filtersWithoutSubLayer.empty())
            continue;

        std::cout << std::format("\nLayer: {}\n", WfpNameMapper::getName(layerKey).rawName);

        for(const auto &[subLayerKey, filterSet] : filtersBySubLayer)
        {
            std::unique_ptr<FWPM_SUBLAYER, WfpDeleter> pSubLayer{_engine.getSubLayerByKey(subLayerKey)};

            if(pSubLayer)
            {
                std::string subLayerName = toLowercase(wideStringToString(pSubLayer->displayData.name));
                std::cout << std::format("SubLayer: {}\n\n", wideStringToString(pSubLayer->displayData.name));
            }
            else
            {
                std::cerr << std::format("Got an invalid subLayer GUID: {}\n",
                    WfpNameMapper::getName(subLayerKey).rawName);
            }

            for(const auto &pFilter : filterSet)
            {
                std::cout << *pFilter << "\n";
                ++filterCount;
            }

            if(!filterSet.empty())
                std::cout << "\n";
        }

        if(!filtersWithoutSubLayer.empty())
        {
            std::cout << "No SubLayer\n\n";
            for(const auto &pFilter : filtersWithoutSubLayer)
                std::cout << *pFilter << "\n";
        }
    }

    std::cout << std::format("\nTotal number of filters: {}\n", filterCount);
}

// TODO: make this more efficient - it's currently loading (sublayers and providers) objects every time
bool WfpKiller::isFilterNameMatched(const std::vector<std::regex> &matchers,
    const std::shared_ptr<FWPM_FILTER> &pFilter) const
{
    std::unique_ptr<FWPM_SUBLAYER, WfpDeleter> pSubLayer;
    std::unique_ptr<FWPM_PROVIDER, WfpDeleter> pProvider;

    if(pFilter->subLayerKey != ZeroGuid)
    {
        pSubLayer.reset(_engine.getSubLayerByKey(pFilter->subLayerKey));
    }
    if(pFilter->providerKey)
    {
        pProvider.reset(_engine.getProviderByKey(*pFilter->providerKey));
    }

    if(pSubLayer)
    {
        std::string subLayerName = toLowercase(wideStringToString(pSubLayer->displayData.name));
        if(isNameMatched(matchers, subLayerName))
            return true;
    }
    if(pProvider)
    {
        std::string providerName = toLowercase(wideStringToString(pProvider->displayData.name));
        if(isNameMatched(matchers, providerName))
            return true;
    }

    return false;
}

void WfpKiller::deleteFilters(const std::vector<FilterId> &filterIds) const
{
    uint32_t deleteCount{0};

    // Returns true if a given GUID ptr points to a PIA filter.
    // Null provider pointers are ignored.
    auto isPiaProvider = [&](const GUID *pProviderKey)
    {
        return pProviderKey && IsEqualGUID(*pProviderKey, PIA_PROVIDER_KEY);
    };

    // Delete a specific subset of filters
    if(filterIds.size() > 0)
    {
        std::cout << std::format("Will delete {} filters\n", filterIds.size());
        for(const auto &filterId : filterIds)
        {
            if(deleteSingleFilter(filterId))
                ++deleteCount;
        }
    }
    // Delete ALL PIA filters
    else
    {
        // Get all PIA filters
        std::vector<std::shared_ptr<FWPM_FILTER>> piaFilters;
         _engine.enumerateFiltersForLayers(kLayers, [&](const auto &pFilter) {
                if(isPiaProvider(pFilter->providerKey))
                    piaFilters.push_back(pFilter);
        });

        std::cout << std::format("This action will delete ALL PIA filters\nAre you sure? (y/n) (will delete {} filters)\n", piaFilters.size());
        char userDecision{};
        std::cin >> userDecision;
        if(userDecision == 'y')
        {
            // Delete all PIA filters.
            for(const auto &pFilter : piaFilters)
            {
                if(deleteSingleFilter(pFilter->filterId))
                    ++deleteCount;
            }
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

bool WfpKiller::isNameMatched(const std::vector<std::regex> &matchers, const std::string &name) const
{
    bool matched = std::ranges::any_of(matchers, [&](const auto &providerMatcher) {
        return std::regex_search(name, providerMatcher);
    });

    return matched;
}
}
