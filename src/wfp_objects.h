#pragma once

#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>
#include <fwpmu.h>

namespace wfpk {

// PIA-specific data
inline constexpr GUID PIA_PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
inline constexpr GUID PIA_SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };

// Base error
class WfpError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

// RAII wrapper around FWPEngine
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

// Core application classs
class WfpKiller
{
public:
    WfpKiller()
    {}

public:
    bool process();

private:
    Engine _engine;
};

// RAII Wrapper around FWPM_FILTER enumeration classes
class SingleLayerFilterEnum
{
    enum { MaxFilterCount = 5000 };
public:
    SingleLayerFilterEnum(const GUID &layerKey, HANDLE engineHandle);
    SingleLayerFilterEnum(SingleLayerFilterEnum&&) = delete;
    SingleLayerFilterEnum(const SingleLayerFilterEnum&) = delete;
    ~SingleLayerFilterEnum();

public:
    template <typename IterFuncT>
    void forEach(IterFuncT func) const
    {
        for(size_t i = 0; i < _numEntries; ++i)
        {
            func(*_filters[i]);
        }
    }

    auto filters() const -> std::vector<FWPM_FILTER>;

private:
    UINT32 _numEntries{0};
    FWPM_FILTER** _filters{NULL};
    HANDLE _engineHandle{};
    HANDLE _enumHandle{};
};

class FilterEnum
{
public:
    FilterEnum(const std::vector<GUID> &layers, HANDLE engineHandle);

public:
    template <typename IterFuncT>
    void forEach(IterFuncT func) const
    {
        for(const auto &layerEnum : _layerEnums)
        {
            layerEnum->forEach([&](const auto &filter) {
                func(filter);
            });
        }
    }

private:
    std::vector<GUID> _layers;
    std::vector<std::unique_ptr<SingleLayerFilterEnum>> _layerEnums;
};
}
