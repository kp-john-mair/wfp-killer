#include <iostream>
#include <stdexcept>
#include <Shlobj.h> // For IsUserAnAdmin()
#include "wfp_killer.h"
#include "argh.h"

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

    argh::parser cmdl(argv);

    wfpk::WfpKiller wfpKiller;

    try
    {
        if(cmdl[{ "-l", "--list"}])
        {
            wfpKiller.listFilters();
            return 0;
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
