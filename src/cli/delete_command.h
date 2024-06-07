#pragma once

#include "cli_command.h"

namespace wfpk {
class DeleteCommand final : public CliCommand
{
public:
    // Non-owning pointer to a WfpKiller instance
    DeleteCommand(wfpk::WfpKiller *pWfpKiller);

private:
    virtual void runCommand(int argc, char **argv) override;
};
}
