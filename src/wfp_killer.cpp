#include "wfp_killer.h"
#include "wfp_ostream_helpers.h"

namespace wfpk {
bool WfpKiller::process()
{
    std::vector layerKeys = {FWPM_LAYER_ALE_AUTH_CONNECT_V4, FWPM_LAYER_ALE_AUTH_CONNECT_V6};
    _engine.enumerateFiltersForLayers(layerKeys, [](const auto &filter) {
        std::cout << filter << std::endl;
    });

    //         result = FwpmFilterDeleteById(_engine, filterId);

    return true;
}
}
