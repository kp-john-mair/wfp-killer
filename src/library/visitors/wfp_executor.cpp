#include <visitors/wfp_executor.h>

namespace wfpk {
void WfpExecutor::visit(const RulesetNode &ruleset) const
{
    for(const auto &node : ruleset.children())
        node->accept(*this);
}

void WfpExecutor::visit(const FilterNode &filterNode) const
{
    FWPM_FILTER filter{};
    std::unique_ptr<FWPM_PROVIDER, WfpDeleter> pProvider{_engine.getProviderByKey(PIA_PROVIDER_KEY)};

   // Basic filter setup
    filter.providerKey = &PIA_PROVIDER_KEY;
    filter.subLayerKey = PIA_SUBLAYER_KEY;
    filter.flags = FWPM_FILTER_FLAG_PERSISTENT | FWPM_FILTER_FLAG_INDEXED;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = 10;
    filter.displayData = pProvider->displayData;

    if(filterNode.layer() == FilterNode::Layer::AUTH_CONNECT_V4)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    else if(filterNode.layer() == FilterNode::Layer::AUTH_CONNECT_V6)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
    else if(filterNode.layer() == FilterNode::Layer::AUTH_RECV_V4)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
    else if(filterNode.layer() == FilterNode::Layer::AUTH_RECV_V6)
        filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;

    if(filterNode.action() == FilterNode::Action::Permit)
        filter.action.type = FWP_ACTION_PERMIT;
    else if(filterNode.action() == FilterNode::Action::Block)
        filter.action.type = FWP_ACTION_BLOCK;



}
}
