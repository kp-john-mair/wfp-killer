#pragma once

#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <fwpmu.h>

namespace wfpk {
    using FilterId = UINT64;

// PIA-specific data
inline constexpr GUID PIA_PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
inline constexpr GUID PIA_SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };

// Base error
class WfpError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class SingleLayerFilterEnum;
class FilterEnum;

// RAII wrapper around FWPEngine
class Engine
{
public:
    Engine();
    Engine(Engine&&) = delete;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;
    ~Engine();

public:
    // Iterate over all filters for all given layers
    template <typename IterFuncT>
    void enumerateFiltersForLayers(const std::vector<GUID> &layerKeys, IterFuncT func) const
    {
        FilterEnum{layerKeys, _handle}.forEach(func);
    }

    // Iterate over filters for just one layer
    template <typename IterFuncT>
    void enumerateFiltersForLayer(const GUID& layerKey, IterFuncT func) const
    {
        SingleLayerFilterEnum{layerKey, _handle}.forEach(func);
    }

    // Delete a filter by Id
    DWORD deleteFilterById(FilterId filerId) const;

    auto handle() -> const HANDLE& { return _handle; }
    operator HANDLE() { return handle(); }

private:
    HANDLE _handle{};
};

// RAII Wrapper around FWPM_FILTER enumeration classes
class SingleLayerFilterEnum
{
    enum { MaxFilterCount = 5000 };
public:
    SingleLayerFilterEnum(const GUID &layerKey, HANDLE engineHandle);
    SingleLayerFilterEnum(SingleLayerFilterEnum&&) = delete;
    SingleLayerFilterEnum(const SingleLayerFilterEnum&) = delete;
    SingleLayerFilterEnum& operator=(const SingleLayerFilterEnum&) = delete;
    SingleLayerFilterEnum& operator=(SingleLayerFilterEnum&&) = delete;
    ~SingleLayerFilterEnum();

public:
    // Ensure filters are sorted by weight
    struct FilterCompare
    {
        bool operator()(const FWPM_FILTER &lhs, const FWPM_FILTER &rhs) const
        {
            // TODO: uint8 weight is unique to PIA - make this
            // more general.
            return lhs.weight.uint8 > rhs.weight.uint8;
        }
    };

    // Use a multiset so we can have multiple filters of the same weight
    using FilterSet = std::multiset<FWPM_FILTER, FilterCompare>;

public:
    template <typename IterFuncT>
    void forEach(IterFuncT func) const
    {
        const auto &sortedFilters = filters();

        for(const auto &filter : sortedFilters)
        {
            func(filter);
        }
    }

private:
    // Ensure the filters are sorted
    auto filters() const -> FilterSet;

private:
    UINT32 _numEntries{0};
    FWPM_FILTER** _filters{NULL};
    HANDLE _engineHandle{};
    HANDLE _enumHandle{};
};

// Wraps SingleLayerFilterEnum to allow iteration over
// multiple WFP layers at once
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
