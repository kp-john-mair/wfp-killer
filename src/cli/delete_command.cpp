#include <cli/delete_command.h>

namespace wfpk {
DeleteCommand::DeleteCommand(wfpk::WfpKiller *pWfpKiller)
: CliCommand(pWfpKiller)
{
    initOptions("delete", "Delete WFP objects");
    addOption("h,help", "Display this help message.");
    addOption("f,filter", "Delete a filter.", cxxopts::value<std::vector<std::string>>()->default_value({}));
}

void DeleteCommand::runCommand(int argc, char **argv)
{
    auto result = parseOptions(argc, argv);

    if(result.count("help"))
    {
        std::cout << help();
        return;
    }
    else if(result.count("filter"))
    {
        const auto &filterIdStrs = result["filter"].as<std::vector<std::string>>();

        // Whether we should delete all pia filters (did the user pass in '-f all' ?)
        bool shouldDeleteAllFilters = std::ranges::find(filterIdStrs, "pia") != filterIdStrs.end();

        std::vector<wfpk::FilterId> filterIds;
        filterIds.reserve(result.count("filter"));

        // Empty filterIds vector means we delete all PIA filters
        // If we shouldn't delete all - then we fill it with ids
        //
        // TODO: Currently this just deletes ALL filters not just PIA
        // change the code so it filters based on PIA filters
        if(!shouldDeleteAllFilters)
        {
            filterIds.reserve(filterIdStrs.size());
            for(const auto &filterIdStr : filterIdStrs)
                filterIds.push_back(std::stoull(filterIdStr));
        }
        _pWfpKiller->deleteFilters(filterIds);
    }
    else
    {
        std::cout << "Options are required.\n";
        std::cout << help();
        return;
    }
}
}
