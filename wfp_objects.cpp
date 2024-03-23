#include "wfp_objects.h"
#include <iostream>

// Replace these GUIDs with the actual provider and sublayer GUIDs you want to target
constexpr GUID PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
constexpr GUID SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };

Engine::Engine()
: _handle{}
{

    auto result = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &_handle);
    if(result != ERROR_SUCCESS)
    {
        std::cerr << "FwpmEngineOpen failed. Error: " << result << std::endl;
        throw WfpError{"FwpmEngineOpen failed"};
    }
}

Engine::~Engine()
{
    FwpmEngineClose0(_handle);
}

bool WfpContext::process()
{
    DWORD result = ERROR_SUCCESS;
    HANDLE enumHandle = NULL;
    FWPM_FILTER_ENUM_TEMPLATE enumTemplate = {0};
    enumTemplate.enumType = FWP_FILTER_ENUM_OVERLAPPING;
    enumTemplate.providerKey = (GUID*)&PROVIDER_KEY;
    enumTemplate.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    enumTemplate.actionMask = 0xFFFFFFFF;

    // Create an enumeration handle. Here, we don't specify any conditions so all filters will be enumerated.
    result = FwpmFilterCreateEnumHandle(_engine, &enumTemplate, &enumHandle);
    if (result != ERROR_SUCCESS) {
        std::cerr << "FwpmFilterCreateEnumHandle0 failed. Error: " << result << std::endl;
        FwpmEngineClose0(_engine);
        return 1;
    }

    FWPM_FILTER0** filters = NULL;
    UINT32 numEntriesReturned = 0;

    // Retrieve all filters. Adjust numEntriesRequested as needed.
    result = FwpmFilterEnum(_engine, enumHandle, 100, &filters, &numEntriesReturned);
    if (result != ERROR_SUCCESS) {
        std::cerr << "FwpmFilterEnum0 failed. Error: " << result << std::endl;
    } else {
        for(UINT32 i = 0; i < numEntriesReturned; i++) {
            const auto &filterId = filters[i]->filterId;
            // Process each filter. For example, print filter ID.
            std::cout << "Deleting Filter ID: " << filterId << std::endl;

            result = FwpmFilterDeleteById(_engine, filterId);

            if(result == ERROR_SUCCESS) {
                std::cout << "Successfully deleted filter with ID: " << filterId << std::endl;
            }
            else {
                std::cerr << "Failed to delete filter. Error: " << result << std::endl;
            }
        }
    }

    // Free the memory allocated for the filters array.
    FwpmFreeMemory((void**)&filters);

    // Close the enumeration handle and engine session.
    FwpmFilterDestroyEnumHandle(_engine, enumHandle);

}
