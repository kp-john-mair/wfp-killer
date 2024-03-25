#pragma once
#include <iostream>
#include <fwpmu.h>

// << overloads for ostream to allow easy string representations of wfp objects
std::ostream& operator<<(std::ostream& os, const FWPM_FILTER& filter);
std::ostream& operator<<(std::ostream& os, const FWPM_FILTER_CONDITION& condition);
