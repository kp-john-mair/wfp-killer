
#include <winsock2.h>
#include <ws2tcpip.h>

#include "wfp_objects.h"
#include "wfp_ostream_helpers.h"
#include <iostream>
#include <string>

namespace wfpk {
Engine::Engine()
: _handle{}
{

    DWORD result = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &_handle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmEngineOpen failed: " + result};
    }
}

Engine::~Engine()
{
    FwpmEngineClose0(_handle);
}

SingleLayerFilterEnum::SingleLayerFilterEnum(const GUID &layerKey, HANDLE engineHandle)
: _numEntries{0}
, _filters{NULL}
, _engineHandle{engineHandle}
, _enumHandle{}
{
    DWORD result{ERROR_SUCCESS};

    FWPM_FILTER_ENUM_TEMPLATE enumTemplate{};
    enumTemplate.enumType = FWP_FILTER_ENUM_OVERLAPPING;
    enumTemplate.layerKey = layerKey;
    enumTemplate.providerKey = const_cast<GUID*>(&PIA_PROVIDER_KEY);
    enumTemplate.actionMask = 0xFFFFFFFF;

    result = FwpmFilterCreateEnumHandle(_engineHandle, &enumTemplate, &_enumHandle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterCreateEnumHandle failed: " + result};
    }

    result = FwpmFilterEnum(_engineHandle, _enumHandle, MaxFilterCount, &_filters, &_numEntries);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterEnum failed: " + result};
    }
}

auto SingleLayerFilterEnum::filters() const -> std::vector<FWPM_FILTER>
{
    std::vector<FWPM_FILTER> filters;
    filters.reserve(_numEntries);

    forEach([&](const auto &filter) {
        filters.push_back(filter);
    });

    return filters;
}

SingleLayerFilterEnum::~SingleLayerFilterEnum()
{
    // Free the memory allocated for the filters array.
    FwpmFreeMemory(reinterpret_cast<void**>(&_filters));
    // Close the enumeration handle
    DWORD result{ERROR_SUCCESS};
    result = FwpmFilterDestroyEnumHandle(_engineHandle, _enumHandle);
    if(result != ERROR_SUCCESS)
    {
        // Just showing the error - cannot throw in a destructor
        std::cerr << "FwpmFilterDestroyEnumHandle failed: " + result << std::endl;
    }
}

FilterEnum::FilterEnum(const std::vector<GUID> &layerKeys, HANDLE engineHandle)
{
    for(auto const &layerKey : layerKeys)
    {
        _layerEnums.push_back(std::make_unique<SingleLayerFilterEnum>(layerKey, engineHandle));
    }
}
}
