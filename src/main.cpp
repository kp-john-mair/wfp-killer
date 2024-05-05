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

#include "cli/commands.h"

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

    wfpk::WfpKiller g_wfpKiller;

    // Use a map so we can enforce order for display purposes
    // Only using a std::shared_ptr so can use the initializer list form of initialization
    // which requires the elements to be copyable (and so won't work with a std::unique_ptr)
    std::map<std::string, std::shared_ptr<wfpk::CliCommand>> CliCommands {
        {"list", std::make_shared<wfpk::ListCommand>(&g_wfpKiller)}
    };

    cxxopts::Options options{"wfpkiller", "Introspect and manipulate WFP filters"};

    try
    {
        options.add_options()
            ("h,help", "Display this help message.")
            ("l,list", "List all filters across all layers (also accepts an optional partial name match for a provider or 'all').", cxxopts::value<std::vector<std::string>>())
            ("m,monitor", "Monitor WFP events.")
            ("p,provider", "Limit output to a specific provider (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("L,layer", "Limit output to a specific layer (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("s,sublayer", "Limit output to a specified sublayer (accepts partial name match).", cxxopts::value<std::vector<std::string>>()->default_value({}))
            ("d,delete", "Delete a filter or all filters (takes a filter ID or 'all').", cxxopts::value<std::vector<std::string>>())
            ("c,create", "Create a dummy filter for testing");

        auto result = options.parse(argc, argv);

        if(argc < 2)
        {
            std::cerr << "Options are required!";
            return 1;
        }

        const std::string &commandName{argv[1]};

        if(CliCommands.contains(commandName))
        {
            CliCommands[commandName]->run(argc, argv);
            return 0;
        }
        else
        {
            std::cerr << "Unrecognized command\n";
            return 1;
        }

        if(commandName == "list")
        {
            std::cout << "got a list!";
            //wfpk::ListCommand{std::move(pWfpKiller)}.run(argc, argv);
            return 0;
        }
        else
        {
            std::cerr << "didn't get a list!";
            return 1;
        }

        if(result.count("create"))
        {
            std::cout << "Trying to create a dummy callout filter\n";
          //  pWfpKiller->createFilter();
            return 0;
        }

        else if(result.count("help"))
        {
          std::cout << options.help();
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

            //pWfpKiller->deleteFilters(filterIds);

            return 0;
        }
        else if(result.count("monitor"))
        {
            //pWfpKiller->monitor();
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
