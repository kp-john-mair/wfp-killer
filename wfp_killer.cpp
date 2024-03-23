#include <windows.h>
#include <fwpmu.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "fwpuclnt.lib")

// Replace these GUIDs with the actual provider and sublayer GUIDs you want to target
const GUID PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
const GUID SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };

int main() {
    HANDLE engineHandle = NULL;
    DWORD result = ERROR_SUCCESS;

    // Open a session to the filter engine.
    result = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &engineHandle);
    if (result != ERROR_SUCCESS) {
        std::cerr << "FwpmEngineOpen0 failed. Error: " << result << std::endl;
        return 1;
    }

    HANDLE enumHandle = NULL;
    FWPM_FILTER_ENUM_TEMPLATE0 enumTemplate = {0};
    enumTemplate.enumType = FWP_FILTER_ENUM_OVERLAPPING;
    enumTemplate.providerKey = (GUID*)&PROVIDER_KEY;
    enumTemplate.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    enumTemplate.actionMask = 0xFFFFFFFF;

    // Create an enumeration handle. Here, we don't specify any conditions so all filters will be enumerated.
    result = FwpmFilterCreateEnumHandle0(engineHandle, &enumTemplate, &enumHandle);
    if (result != ERROR_SUCCESS) {
        std::cerr << "FwpmFilterCreateEnumHandle0 failed. Error: " << result << std::endl;
        FwpmEngineClose0(engineHandle);
        return 1;
    }

    FWPM_FILTER0** filters = NULL;
    UINT32 numEntriesReturned = 0;

    // Retrieve all filters. Adjust numEntriesRequested as needed.
    result = FwpmFilterEnum0(engineHandle, enumHandle, 100, &filters, &numEntriesReturned);
    if (result != ERROR_SUCCESS) {
        std::cerr << "FwpmFilterEnum0 failed. Error: " << result << std::endl;
    } else {
        for (UINT32 i = 0; i < numEntriesReturned; i++) {
            // Process each filter. For example, print filter ID.
            std::cout << "Filter ID: " << filters[i]->filterId << std::endl;
        }
    }

    // Free the memory allocated for the filters array.
    FwpmFreeMemory0((void**)&filters);

    // Close the enumeration handle and engine session.
    FwpmFilterDestroyEnumHandle0(engineHandle, enumHandle);
    FwpmEngineClose0(engineHandle);

    return 0;
}
