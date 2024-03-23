#include <iostream>
#include <vector>
#include <stdexcept>
#include "wfp_objects.h"

#pragma comment(lib, "fwpuclnt.lib")

int main() {

    WfpContext wfpContext;

    wfpContext.process();

    return 0;
}
