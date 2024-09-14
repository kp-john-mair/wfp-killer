#include <cli/load_command.h>

namespace wfpk
{
LoadCommand::LoadCommand(wfpk::WfpKiller *pWfpKiller)
    : CliCommand(pWfpKiller)
{
    initOptions("load", "load wfp filters from a file");
    addOption("h,help", "Display this help message.");
    addOption("f,file", "The file containing WFP rules.",
              cxxopts::value<std::string>()->default_value({}));
}

void LoadCommand::runCommand(int argc, char **argv)
{
    auto result = parseOptions(argc, argv);

    if(result.count("help"))
    {
        std::cout << help();
        return;
    }
    else if(result.count("file"))
    {
        std::string sourceFile{result["file"].as<std::string>()};
        std::cout << "Got a file param of: " << sourceFile << "\n";

        _pWfpKiller->loadFilters(sourceFile);
    }
    else
    {
        std::cout << "Didn't get any options!\n";
    }
}
}
