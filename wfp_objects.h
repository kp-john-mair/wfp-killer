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
    enum { MaxFilterCount = 100 };
public:
    FilterEnum(FWPM_FILTER_ENUM_TEMPLATE enumTemplate, HANDLE engineHandle)
    : _numEntries{0}
    , _filters{NULL}
    , _enumTemplate{std::move(_enumTemplate)}
    , _engineHandle{engineHandle}
    , _enumHandle{}
    {
        DWORD result{ERROR_SUCCESS};

        result = FwpmFilterCreateEnumHandle(_engineHandle, &enumTemplate, &_enumHandle);
        if(result != ERROR_SUCCESS)
        {
            throw WfpError{"FwpmFilterCreateEnumHandle failed: " + result};
        }

        result = FwpmFilterEnum(_engineHandle, _enumHandle, MaxFilterCount, &_filters, &_numEntries);
        if(result != ERROR_SUCCESS)
        {
            throw WfpError{"FwpmFilterEnum failed: " + result};
        }
    }

    ~FilterEnum()
    {
        // Free the memory allocated for the filters array.
        FwpmFreeMemory(reinterpret_cast<void**>(&_filters));
        // Close the enumeration handle and engine session.
        DWORD result{ERROR_SUCCESS};
        result = FwpmFilterDestroyEnumHandle(_engineHandle, _enumHandle);
        if(result != ERROR_SUCCESS)
        {
            std::cerr << "FwpmFilterDestroyEnumHandle failed: " + result << std::endl;
        }
    }

public:
    template <typename IterFuncT>
    void forEach(IterFuncT func) const
    {
        for(UINT32 i = 0; i < _numEntries; ++i)
        {
            func(*_filters[i]);
        }
    }

private:
    UINT32 _numEntries{0};
    FWPM_FILTER** _filters{NULL};
    FWPM_FILTER_ENUM_TEMPLATE _enumTemplate{};
    HANDLE _engineHandle{};
    HANDLE _enumHandle{};
};
