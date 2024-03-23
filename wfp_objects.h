#pragma once

#include <windows.h>
#include <fwpmu.h>
#include <stdexcept>

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
