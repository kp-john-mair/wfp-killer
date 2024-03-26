#pragma once

#include "wfp_objects.h"

namespace wfpk {
// Core application class
class WfpKiller
{
public:
    WfpKiller()
    {}

public:
    bool process();

private:
    Engine _engine;
};
}
