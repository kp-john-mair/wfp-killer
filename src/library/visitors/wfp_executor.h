#pragma once

#include <parser/nodes.h>
#include <wfp_objects.h>

namespace wfpk {
class WfpExecutor
{
public:
    WfpExecutor(Engine &engine) : _engine{engine} {}

public:
    void visit(const RulesetNode &ruleset) const;
    void visit(const FilterNode &filterNode) const;

private:
    Engine &_engine;
};
}
