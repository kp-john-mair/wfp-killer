
#include <winsock2.h>
#include <ws2tcpip.h>

#include "wfp_objects.h"
#include <iostream>
#include <string>
#include <optional>
#include <memory>
#include <guiddef.h>
#include <Rpc.h>

namespace wfpk {

Engine::Engine()
: _handle{}
{
    DWORD result = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &_handle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmEngineOpen failed, code: " + result};
    }
}

DWORD Engine::deleteFilterById(FilterId filterId) const
{
    return FwpmFilterDeleteById(_handle, filterId);
}

Engine::~Engine()
{
    DWORD result{ERROR_SUCCESS};
    result = FwpmEngineClose(_handle);
    if(result != ERROR_SUCCESS)
    {
        // Cannot throw in a destructor
        std::cerr << "FwpmEngineClose failed, code: " << result;
    }
}

auto SingleLayerFilterEnum::createEnumTemplate(const GUID &layerKey) const
    -> FWPM_FILTER_ENUM_TEMPLATE
{
    FWPM_FILTER_ENUM_TEMPLATE enumTemplate{};
    enumTemplate.enumType = FWP_FILTER_ENUM_OVERLAPPING;
    enumTemplate.layerKey = layerKey;
    enumTemplate.providerKey = const_cast<GUID*>(&PIA_PROVIDER_KEY);
    enumTemplate.actionMask = 0xFFFFFFFF;

    return enumTemplate;
}

SingleLayerFilterEnum::SingleLayerFilterEnum(const GUID &layerKey, HANDLE engineHandle)
: _engineHandle{engineHandle}
{
    DWORD result{ERROR_SUCCESS};

    FWPM_FILTER_ENUM_TEMPLATE enumTemplate{createEnumTemplate(layerKey)};

    HANDLE enumHandle{};
    result = FwpmFilterCreateEnumHandle(_engineHandle, &enumTemplate, &enumHandle);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterCreateEnumHandle failed: " + result};
    }

    FWPM_FILTER **pFilters{nullptr};

    UINT32 numEntries{0};
    result = FwpmFilterEnum(_engineHandle, enumHandle, MaxFilterCount, &pFilters, &numEntries);
    if(result != ERROR_SUCCESS)
    {
        throw WfpError{"FwpmFilterEnum failed: " + result};
    }

    for(size_t i = 0; i < numEntries; ++i)
    {
        FWPM_FILTER *pFilter{nullptr};

        // Grab a new filter so that we can carefully manage its lifetime
        DWORD result = FwpmFilterGetById(_engineHandle, pFilters[i]->filterId, &pFilter);
        if(result == ERROR_SUCCESS)
            _pFilters.insert(std::shared_ptr<FWPM_FILTER>{pFilter, WfpDeleter{}});
        else
            std::cerr << "FwpmFilterGetById failed, code: " << std::to_string(result) << std::endl;
    }

    // Free the enum filters since we have shared_ptrs to new ones now anyway
    FwpmFreeMemory(reinterpret_cast<void**>(&pFilters));
    // Close the enumeration handle
    result = FwpmFilterDestroyEnumHandle(_engineHandle, enumHandle);
    if(result != ERROR_SUCCESS)
    {
        // Just showing the error - we have the filters already, so let's give it a chance
        std::cerr << "FwpmFilterDestroyEnumHandle failed, code: " + result << std::endl;
    }
}
}
