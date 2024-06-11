#include <visitors/wfp_executor.h>

namespace wfpk {
void WfpExecutor::visit(const RulesetNode &ruleset) const
{
    for(const auto &node : ruleset.children())
        node->accept(*this);
}

void WfpExecutor::visit(const FilterNode &filterNode) const
{

}
}
