#pragma once

#include <cli/cli_command.h>

namespace wfpk
{
class LoadCommand final : public CliCommand
{
public:
    LoadCommand(wfpk::WfpKiller *pWfpKiller);

private:
    virtual void runCommand(int argc, char **argv) override;
};
}
