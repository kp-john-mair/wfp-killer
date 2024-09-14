#include <cli/monitor_command.h>

namespace wfpk
{
MonitorCommand::MonitorCommand(wfpk::WfpKiller *pWfpKiller)
    : CliCommand(pWfpKiller)
{
    initOptions("monitor", "Monitor WFP events");
    addOption("h,help", "Display this help message.");
}

void MonitorCommand::runCommand(int argc, char **argv)
{
    auto result = parseOptions(argc, argv);

    if(result.count("help"))
    {
        std::cout << help();
        return;
    }

    _pWfpKiller->monitor();
}
}
