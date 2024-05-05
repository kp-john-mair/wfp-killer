#include <cli/list_command.h>

namespace wfpk {
namespace
{
    // This converts a vector of strings to a vector of regex
    // Since our matchers are case insensitive, we also lowercase them here too.
    std::vector<std::regex> stringVecToMatchers(const std::vector<std::string> &vec)
    {
        std::vector<std::regex> matchers;
        matchers.reserve(vec.size());

        for(const auto &pattern : vec)
            matchers.emplace_back(wfpk::toLowercase(pattern));

        return matchers;
    }
}

ListCommand::ListCommand(wfpk::WfpKiller *pWfpKiller)
: CliCommand(pWfpKiller)
{
    initOptions("list", "List WFP objects");
    addOption("h,help", "Display this help message.");
    addOption("f,filters", "Display all filters.");
    addOption("c,callouts", "Display all callouts.");
    addOption("L,layers", "Display layers.");
    addOption("pia", "Display PIA filters.");
    addOption("s,search", "Display filters that match the regex.", cxxopts::value<std::vector<std::string>>()->default_value({}));
    addOption("sublayers", "Display sublayers");
}

void ListCommand::runCommand(int argc, char **argv)
{
    auto result = parseOptions(argc, argv);

    if(result.count("help"))
    {
        std::cout << help();
        return;
    }
    else if(result.count("search"))
    {
        const auto &listValues = result["search"].as<std::vector<std::string>>();
        bool shouldShowAllFilters = std::ranges::find(listValues, "all") != listValues.end();

        std::vector<std::regex> providers, subLayers, layers;

        if(!shouldShowAllFilters)
        {
            // We match the 'names' against both providers and subLayers
            // so that both fields are searched
            providers = stringVecToMatchers(listValues);
            subLayers = providers;
            layers = stringVecToMatchers({result["search"].as<std::vector<std::string>>()});
        }

        _pWfpKiller->listFilters({
            .providerMatchers = providers,
            .layerMatchers = layers,
            .subLayerMatchers = subLayers
        });
    }
    else
    {
        std::cout << "Options are required.\n";
        std::cout << help();
        return;
    }
}
}
