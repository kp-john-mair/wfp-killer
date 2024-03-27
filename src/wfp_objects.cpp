
#include <winsock2.h>
#include <ws2tcpip.h>

#include "wfp_objects.h"
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

DWORD Engine::deleteFilterById(FilterId filterId) const
{
    return FwpmFilterDeleteById(_handle, filterId);
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

// TODO: This is currently very brittle - if/when SingleLayerFilterEnum
// goes out of scope all the filters become invalid as they have
// pointer fields such as providerKey and filterCondition which
// will be cleaned up by the call to FwpmFreeMemory in the SingleLayerFilterEnum destructor.
// We should probably make deep copies of these filters into a new
// Filter class which has the same fields but no pointer members; instead it should
// have deep copies of all the FWPM_FILTER data. This would make the filters
// immune to their parent object (SingleLayerFilterEnum) going out of scope.
// Right now it's all too easy to hold onto a FWPM_FILTER object beyond the
// life-time of SingleLayerFilterEnum.
auto SingleLayerFilterEnum::filters() const -> FilterSet
{
    FilterSet filters;

    for(size_t idx = 0; idx < _numEntries; ++idx)
    {
        // TODO: Make sure each filter object is a deep copy
        // in a new "Filter" object whose life-time is independent
        // of the parent class
        filters.insert(*_filters[idx]);
    }

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
