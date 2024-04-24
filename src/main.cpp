#include <iostream>
#include <stdexcept>
#include <ranges>
#include <Shlobj.h> // For IsUserAnAdmin()
#include "wfp_killer.h"
#include "wfp_objects.h"
#include "utils.h"
#include "cxxopts.h"
#include <stdlib.h>
#include <regex>

// Instruct the compiler to link these libs for us
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Rpcrt4.lib")

namespace {

BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType)
{
    switch(ctrlType)
    {
    case CTRL_C_EVENT:
        std::cout << "Ctrl+C detected, exiting...\n";
        exit(0);
        return TRUE;
    }
    return FALSE;
}

// This converts a vector of strings to a vector of regex
// Since our matchers are case insensitive, we also lowercase them here too.
std::vector<std::regex> stringVecToMatchers(const std::vector<std::string> &vec)
{
    std::vector<std::regex> matchers;
    matchers.reserve(vec.size());

    for(const auto &pattern : vec)
    {
        matchers.emplace_back(wfpk::toLowercase(pattern));
    }

    return matchers;
}
}

int main(int argc, char** argv)
{
    if(IsUserAnAdmin() == FALSE)
    {
        std::cerr << "Error: must run with elevated privileges.\n";
        return 1;
    }

    if(!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))
    {
        // If the handler cannot be installed, exit
        std::cerr <<  "Error: Unable to install Ctrl+C handler\n";
        return 1;
    }

    cxxopts::Options options{"wfpkiller", "Introspect and manipulate WFP filters"};

    try
    {
        //options.allow_unrecognised_options();
        options.add_options()
            ("h,help", "Display this help message.")
            ("l,list", "List all filters across all layers (also accepts an optional partial name match for a provider or 'all').", cxxopts::value<std::vector<std::string>>())
            ("m,monitor", "Monitor WFP events.")
            ("p,provider", "Limit output to a specific provider (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("L,layer", "Limit output to a specific layer (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("s,sublayer", "Limit output to a specified sublayer (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("d,delete", "Delete a filter or all filters (takes a filter ID or 'all').", cxxopts::value<std::vector<std::string>>())
            ("c,create", "Create a dummy filter for testing");

        wfpk::WfpKiller wfpKiller;

        auto result = options.parse(argc, argv);

        if(result.count("create"))
        {
            std::cout << "Trying to create a dummy callout filter\n";
            wfpKiller.createFilter();
            return 0;
        }

        else if(result.count("help"))
        {
          std::cout << options.help();
          return 0;
        }
        else if(result.count("list"))
        {
            const auto &listValues = result["list"].as<std::vector<std::string>>();
            bool shouldShowAllFilters = std::ranges::find(listValues, "all") != listValues.end();

            std::vector<std::regex> providers, subLayers;

            if(!shouldShowAllFilters)
            {
                // We match the 'names' against both providers and subLayers
                // so that both fields are searched
                providers = stringVecToMatchers(listValues);
                subLayers = providers;
            }

            const auto layers = stringVecToMatchers({result["layer"].as<std::vector<std::string>>()});
            wfpKiller.listFilters({
                .providerMatchers = providers,
                .layerMatchers = layers,
                .subLayerMatchers = subLayers
             });
            return 0;
        }
        else if(result.count("delete"))
        {
            const auto &filterIdStrs = result["delete"].as<std::vector<std::string>>();

            // Whether we should delete all filters (did the user pass in '-d all' ?)
            bool shouldDeleteAllFilters = std::ranges::find(filterIdStrs, "all") != filterIdStrs.end();

            std::vector<wfpk::FilterId> filterIds;
            filterIds.reserve(result.count("delete"));

            // Empty filterIds vector means we delete all filters
            // If we shouldn't delete all - then we fill it with ids
            if(!shouldDeleteAllFilters)
            {
                filterIds.reserve(filterIdStrs.size());
                for(const auto &filterIdStr : filterIdStrs)
                    filterIds.push_back(std::stoull(filterIdStr));
            }

            wfpKiller.deleteFilters(filterIds);

            return 0;
        }
        else if(result.count("monitor"))
        {
            wfpKiller.monitor();
        }
        else
        {
            std::cout << "Options are required.\n\n";
            std::cout << options.help();
            return 1;
        }
    }
    catch(const wfpk::WfpError& ex)
    {
        std::cerr << "Fatal WFP error: " << ex.what() << "\n";
        return 1;
    }
    catch(const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error occurred, unable to continue.\n";
        return 1;
    }

    return 0;
}
