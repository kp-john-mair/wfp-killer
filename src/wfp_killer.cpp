#include <iostream>
#include <vector>
#include <stdexcept>
#include "wfp_objects.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "fwpuclnt.lib")

int main()
{
    try
    {
        WfpContext wfpContext;
        wfpContext.process();
    }
    catch(const WfpError& ex)
    {
        std::cerr << "Fatal WFP error: " << ex.what() << " Unable to continue." << std::endl;
        return 1;
    }
    catch(const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << " Unable to continue." << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error occurred, unable to continue." << std::endl;
        return 1;
    }

    return 0;
}
