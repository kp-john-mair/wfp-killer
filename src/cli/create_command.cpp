#include "create_command.h"

namespace wfpk {
CreateCommand::CreateCommand(wfpk::WfpKiller *pWfpKiller)
: CliCommand(pWfpKiller)
{
    initOptions("create", "create a WFP filter");
    addOption("h,help", "Display this help message.");
}

void CreateCommand::runCommand(int argc, char **argv)
{
    auto result = parseOptions(argc, argv);

    if(result.count("help"))
    {
        std::cout << help();
        return;
    }

    _pWfpKiller->createFilter();
}
}
