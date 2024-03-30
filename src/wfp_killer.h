#pragma once

#include "wfp_objects.h"

namespace wfpk {

void CALLBACK foo(void *context, const FWPM_NET_EVENT3 *event);
// Core application class
class WfpKiller
{
public:
    void listFilters() const;
    void deleteFilters(const std::vector<FilterId> &filterIds) const;
    void monitor();

private:
    bool deleteSingleFilter(FilterId filterId) const;

private:
    Engine _engine;
};
}
