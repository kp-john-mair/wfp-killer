#include <visitors/wfp_executor.h>
#include <utils.h>

namespace wfpk {
void WfpExecutor::visit(const RulesetNode &ruleset) const
{
    for(const auto &node : ruleset.children())
        node->accept(*this);
}

void WfpExecutor::visit(const FilterNode &filterNode) const
{
    using Action = FilterNode::Action;
    using Direction = FilterNode::Direction;

    std::cout << "Adding rule: " << filterNode << std::endl;

    FWPM_FILTER filter{};
    std::unique_ptr<FWPM_PROVIDER, WfpDeleter> pProvider{_engine.getProviderByKey(PIA_PROVIDER_KEY)};

   // Basic filter setup
    filter.providerKey = &PIA_PROVIDER_KEY;
    filter.subLayerKey = PIA_SUBLAYER_KEY;
    filter.flags = FWPM_FILTER_FLAG_PERSISTENT | FWPM_FILTER_FLAG_INDEXED;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = 10;
    filter.displayData = pProvider->displayData;

    if(filterNode.direction() == Direction::Out)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    else if(filterNode.direction() == Direction::In)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;

    if(filterNode.action() == FilterNode::Action::Permit)
        filter.action.type = FWP_ACTION_PERMIT;
    else if(filterNode.action() == FilterNode::Action::Block)
        filter.action.type = FWP_ACTION_BLOCK;

    const auto filterConditions = filterNode.filterConditions();

    if(filterConditions == wfpk::NoFilterConditions)
    {
        filter.filterCondition = nullptr;
        filter.numFilterConditions = 0;

        //_engine.add(filter);
    }
    else
    {
        for(const auto &addressStr : filterConditions.destIps.v4)
        {
            // Add a condition for an application id constraint
            FWPM_FILTER_CONDITION condition{};
            FWP_V4_ADDR_AND_MASK addressWithMask;

            if(std::ranges::find(addressStr, '/') != addressStr.end())
            {
                auto parts = splitString(addressStr, '/');
                uint32_t address = stringToIp4(parts.front());
                uint32_t prefix = atoi(parts.back().c_str());

                addressWithMask.addr = address;
                addressWithMask.mask = ~0UL << (32 - prefix);
            }
            else
            {
                uint32_t address = stringToIp4(addressStr);
                addressWithMask.addr = address;
                // /32 subnet contains all 1s
                addressWithMask.mask = ~0UL;
            }

            condition.fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
            condition.matchType = FWP_MATCH_EQUAL;
            condition.conditionValue.type = FWP_V4_ADDR_MASK;
            condition.conditionValue.v4AddrMask = &addressWithMask;

            filter.filterCondition = &condition;
            filter.numFilterConditions = 1;

            //_engine.add(filter);
        }
    }
}
}
