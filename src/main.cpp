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

using CommandMapT = std::map<std::string, std::shared_ptr<wfpk::CliCommand>>;

void showHelp(const std::string &programName, const CommandMapT &commandMap)
{
    std::cout << "A tool to inspect and manipulate the WFP firewall.\n\n";
    std::cout << std::format("Usage: {} <subcommand>\n\n", programName);
    std::cout << "Subcommands:\n";
    for(const auto &[name, cmd] : commandMap)
        std::cout << std::format("  {:10} {}\n", name, cmd->description());

    std::cout << std::format("\nSee {} <subcommand> -h for detailed help.\n", programName);
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

    wfpk::WfpKiller wfpKiller;

    // Use a map so we can enforce order for display purposes
    // Only using a std::shared_ptr so can use the initializer list form of initialization
    // which requires the elements to be copyable (and so won't work with a std::unique_ptr)
    CommandMapT CliCommands {
        {"list", std::make_shared<wfpk::ListCommand>(&wfpKiller)},
        {"delete", std::make_shared<wfpk::DeleteCommand>(&wfpKiller)},
        {"create", std::make_shared<wfpk::CreateCommand>(&wfpKiller)},
        {"monitor", std::make_shared<wfpk::MonitorCommand>(&wfpKiller)}
    };

    cxxopts::Options options{"wfpkiller", "Introspect and manipulate WFP filters"};

    try
    {
        if(argc < 2)
        {
            std::cerr << "Options are required!";
            return 1;
        }

        // Extract out just the filename for the process (not the full path)
        const std::string &programName{std::filesystem::path{argv[0]}.filename().string()};
        // Name of command (not program name)
        const std::string &commandName{argv[1]};

        if(commandName == "-h" || commandName == "help")
        {
            showHelp(programName, CliCommands);
            return 0;
        }
        else if(CliCommands.contains(commandName))
        {
            CliCommands[commandName]->run(argc - 1, argv + 1);
            return 0;
        }
        else
        {
            std::cerr << "Unrecognized command\n";
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
