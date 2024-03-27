#include <iostream>
#include <stdexcept>
#include <Shlobj.h> // For IsUserAnAdmin()
#include "wfp_killer.h"
#include "wfp_objects.h"
#include "cxxopts.h"

// Instruct the compiler to link these libs for us
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Shell32.lib")

int main(int argc, char** argv)
{
    if(IsUserAnAdmin() == FALSE)
    {
        std::cerr << "Error: must run with elevated privileges.\n";
        return 1;
    }

    cxxopts::Options options{"wfpkiller", "Introspect and manipulate WFP filters"};

    options.allow_unrecognised_options();
    options.add_options()
        ("h,help", "Display this help message.")
        ("l,list", "List all PIA filters.")
        ("d,delete", "Delete a filter or all filters (takes a filter ID or 'all')", cxxopts::value<std::vector<std::string>>());

    wfpk::WfpKiller wfpKiller;

    try
    {
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

            std::vector<wfpk::FilterId> filterIds;
            filterIds.reserve(filterIdStrs.size());

            for(const auto &filterIdStr : filterIdStrs)
                filterIds.push_back(std::stoull(filterIdStr));

            wfpKiller.deleteFilters(filterIds);

            return 0;
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
