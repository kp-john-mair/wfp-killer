#pragma once

#include <windows.h>
#include <fwpmu.h>
#include <stdexcept>
#include <iostream>

class WfpError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class Engine
{
public:
    Engine();
    ~Engine();

public:
    auto handle() -> const HANDLE& { return _handle; }
    operator HANDLE() { return handle(); }

private:
    HANDLE _handle{};
};

class WfpContext
{
public:
    WfpContext()
    {}

public:
    bool process();

private:
    Engine _engine;
};

class FilterEnum
{
public:
    FilterEnum(FWPM_FILTER_ENUM_TEMPLATE enumTemplate, HANDLE engineHandle)
    : _enumTemplate{std::move(_enumTemplate)}
    , _engineHandle{engineHandle}
    {
        HANDLE enumHandle{NULL};
        DWORD result = FwpmFilterCreateEnumHandle(_engineHandle, &enumTemplate, &enumHandle);
        if(result != ERROR_SUCCESS)
        {
            throw WfpError{"FwpmFilterCreateEnumHandle failed: " + result};
        }
    }

private:
    FWPM_FILTER_ENUM_TEMPLATE _enumTemplate{};
    HANDLE _engineHandle;
};
