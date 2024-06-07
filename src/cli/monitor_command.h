#pragma once

#include "cli_command.h"

namespace wfpk {
class MonitorCommand final : public CliCommand
{
public:
    MonitorCommand(wfpk::WfpKiller *pWfpKiller);

private:
    virtual void runCommand(int argc, char **argv) override;
};
}
