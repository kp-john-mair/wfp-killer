#include <iostream>
#include <stdexcept>
#include <ranges>
#include <Shlobj.h> // For IsUserAnAdmin()
#include "wfp_killer.h"
#include "wfp_objects.h"
#include "cxxopts.h"
#include <stdlib.h>

// Instruct the compiler to link these libs for us
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Rpcrt4.lib")


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
        options.allow_unrecognised_options();
        options.add_options()
            ("h,help", "Display this help message.")
            ("l,list", "List all PIA filters.")
            ("m,monitor", "Monitor WFP events.")
            ("d,delete", "Delete a filter or all filters (takes a filter ID or 'all').", cxxopts::value<std::vector<std::string>>());

        wfpk::WfpKiller wfpKiller;

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
          std::cout << options.help();
          return 0;
        }
        else if(result.count("list"))
        {
            wfpKiller.listFilters();
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
        std::cerr << "Fatal WFP error: " << ex.what() << " Unable to continue.\n";
        return 1;
    }
    catch(const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << " Unable to continue.\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error occurred, unable to continue.\n";
        return 1;
    }

    return 0;
}
