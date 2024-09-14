#pragma once

#include <cli/cli_command.h>

namespace wfpk
{
class CreateCommand final : public CliCommand
{
public:
    CreateCommand(wfpk::WfpKiller *pWfpKiller);

private:
    virtual void runCommand(int argc, char **argv) override;
};
}
