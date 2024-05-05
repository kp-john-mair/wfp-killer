#pragma once

#include <cli/cli_command.h>

namespace wfpk {
class ListCommand final : public CliCommand
{
public:
    // Non-owning pointer to a WfpKiller instance
    ListCommand(wfpk::WfpKiller *pWfpKiller);

private:
    virtual void runCommand(int argc, char **argv) override;
};
}
