#pragma once

#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <optional>
#include <fwpmu.h>
#include <concepts>
#include <source_location>
#include "utils.h"

namespace wfpk {
// Convenient alias for a Filter ID
using FilterId = UINT64;

// Zero GUID
inline constexpr GUID ZeroGuid = {};

// PIA-specific data
inline constinit GUID PIA_PROVIDER_KEY = { 0x8de3850, 0xa416, 0x4c47, { 0xb3, 0xad, 0x65, 0x7c, 0x5e, 0xf1, 0x40, 0xfb } };
inline constinit GUID PIA_SUBLAYER_KEY = { 0xf31e288d, 0xde5a, 0x4522, { 0x94, 0x58, 0xde, 0x14, 0xeb, 0xd0, 0xa3, 0xf8 } };

// Base error
class WfpError : public std::runtime_error
{
public:
    WfpError(const std::string &message, DWORD code, const std::source_location& loc = std::source_location::current())
    : std::runtime_error(formatErrorMessage(message, code, loc))
    {}

private:
    static std::string formatErrorMessage(const std::string &message, DWORD code, const std::source_location &loc)
    {
        return std::format("{} {} @ (file: {}, line: {})", message, getErrorString(code), loc.file_name(), loc.line());
    }
};

// Generic deleter for WFP objects
struct WfpDeleter
{
    template <typename WfpPtr>
    void operator()(WfpPtr *ptr) const
    {
        if(ptr)
            FwpmFreeMemory(reinterpret_cast<void**>(&ptr));
    }
};

// Ensure filters are sorted by weight
struct FilterCompare
{
    // Make this a PtrT so it can work with raw pointers
    // as well as smart ptrs
    template <typename PtrT>
    bool operator()(const PtrT &lhs, const PtrT &rhs) const
    {
        // See here (weight section) for explanation of types
        // https://learn.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_filter0
        switch(lhs->weight.type)
        {
        // we don't check for this as the deref causes crashes sometimes - use u8 instead
        case FWP_UINT64:
            // if(lhs->weight.uint64 && rhs->weight.uint64)
            //     return *(lhs->weight.uint64) > *(rhs->weight.uint64);
            // else
            // {
            //     std::cerr << "invalid weight got null uint64!" << std::endl;
            //     return true;
            // }
        case FWP_EMPTY: // Weight determined by BFE
        case FWP_UINT8:
        default:
            return lhs->weight.uint8 > rhs->weight.uint8;
        }
    }
};

// Use a multiset so we can have multiple filters of the same weight
using FilterSet = std::multiset<std::shared_ptr<FWPM_FILTER>, FilterCompare>;

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

private:
    // Build the enumeration template
    auto createEnumTemplate(const GUID &layerKey) const -> FWPM_FILTER_ENUM_TEMPLATE;

public:
    template <typename IterFuncT>
        requires std::invocable<IterFuncT, std::shared_ptr<FWPM_FILTER>>
    void forEach(IterFuncT func) const
    {
        for(const auto &filter : _pFilters)
        {
            func(filter);
        }
    }

    const FilterSet& filters() const { return _pFilters; }

private:
    HANDLE _engineHandle{};
    FilterSet _pFilters;
};

// Wraps SingleLayerFilterEnum to allow iteration over
// multiple WFP layers at once
class FilterEnum
{
public:
    FilterEnum(const std::vector<GUID> &layers, HANDLE engineHandle)
    : _engineHandle{engineHandle}
    , _layers{layers}
    {}

public:
    template <typename IterFuncT>
        requires std::invocable<IterFuncT, std::shared_ptr<FWPM_FILTER>>
    void forEach(IterFuncT func) const
    {
        for(const auto &layer : _layers)
        {
            SingleLayerFilterEnum{layer, _engineHandle}.forEach([&](const auto &pFilter) {
                func(pFilter);
            });
        }
    }

private:
    HANDLE _engineHandle{};
    std::vector<GUID> _layers;
};

// Monitor live WFP events
class EventMonitor
{
public:
    EventMonitor(HANDLE engineHandle)
    : _engineHandle{engineHandle}
    , _eventSubscriptionHandle{}
    {
        assert(_engineHandle); // pre-condition
    }

    template <typename FuncT>
        requires std::invocable<FuncT, void*, const FWPM_NET_EVENT*>
    void start(FuncT callbackFunc)
    {
        // Template that determines the events we're interested in
        FWPM_NET_EVENT_ENUM_TEMPLATE eventEnumTemplate{};
        // Set startTime for our event gathering to current time ('now')
        GetSystemTimeAsFileTime(&eventEnumTemplate.startTime);
        // Maximum possible date for endTime - since we don't want an end date
        eventEnumTemplate.endTime.dwLowDateTime = eventEnumTemplate.startTime.dwHighDateTime + 10000;
        eventEnumTemplate.endTime.dwHighDateTime = eventEnumTemplate.startTime.dwHighDateTime + 1000;
        // No conditions for our template - we want everything.
        eventEnumTemplate.numFilterConditions = 0;

        // Stores information used to subscribe to notifications about a
        // network event. Just a light wrapper around the template above.
        FWPM_NET_EVENT_SUBSCRIPTION subscription{};
        subscription.enumTemplate = &eventEnumTemplate;

        DWORD result = FwpmNetEventSubscribe(
            _engineHandle,
            &subscription,
            callbackFunc,
            NULL, // Context for the callback; can be NULL
            &_eventSubscriptionHandle
        );


        if(result != ERROR_SUCCESS)
            throw WfpError{"FwpmNetEventSubscribe failed:", result};
    }

    ~EventMonitor()
    {
        if(!_eventSubscriptionHandle)
            return;

        DWORD result{ERROR_SUCCESS};
        result = FwpmNetEventUnsubscribe(_engineHandle, _eventSubscriptionHandle);
        if(result != ERROR_SUCCESS)
        {
            std::cerr << "FwpmNetEventUnsubscribe failed: " << result;
        }
    }

private:
    HANDLE _engineHandle{};
    HANDLE _eventSubscriptionHandle{};
};

// RAII wrapper around FWPEngine - also a singleton that can be
// accessed via Engine::instance()
class Engine : public Singleton<Engine>
{
public:
    Engine();
    Engine(Engine&&) = delete;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;
    ~Engine();

public:

    FilterId add(const FWPM_FILTER &filter);
    // template <typename WfpObjectType, typename LookupType>
    // WfpObjectType *getWfpObject(const LookupType &lookup)
    // {
    //     WfpObjectType *pObject{nullptr};
    //     DWORD result =
    // }

    // Returns an owning pointer to a FWPM_FILTER.
    // Caller is responsible for the life-time of this object
    FWPM_FILTER* getFilterById(UINT64 filterId) const
    {
        FWPM_FILTER *pFilter{nullptr};
        DWORD result = FwpmFilterGetById(_handle, filterId, &pFilter);

        if(result != ERROR_SUCCESS)
        {
            std::cerr << std::format("FwpmFilterGetById failed: {}\n", getErrorString(result));
            return nullptr;
        }

        return pFilter;
    }

    FWP_BYTE_BLOB* getAppIdFromFileName(const std::wstring &appPath)
    {
        FWP_BYTE_BLOB *pBlob{nullptr};

        DWORD result = FwpmGetAppIdFromFileName(appPath.c_str(), &pBlob);

        if(result != ERROR_SUCCESS)
        {
            std::cerr << "FwpmGetAppIdFromFileName failed: " << getErrorString(result);
            return nullptr;
        }

        return pBlob;
    }

    // Returns an owning pointer to a FWPM_SUBLAYER
    // Caller is responsible for the life-time of this object
    FWPM_SUBLAYER* getSubLayerByKey(const GUID &subLayerKey) const
    {
        FWPM_SUBLAYER *pSublayer{nullptr};
        DWORD result = FwpmSubLayerGetByKey(_handle, &subLayerKey, &pSublayer);

        if(result != ERROR_SUCCESS)
        {
            std::cerr << std::format("FwpmSubLayerGetByKey failed: {}\n", getErrorString(result));
            return nullptr;
        }

        return pSublayer;
    }

    FWPM_PROVIDER* getProviderByKey(const GUID &providerKey) const
    {
        FWPM_PROVIDER *pProvider{nullptr};
        DWORD result = FwpmProviderGetByKey(_handle, &providerKey, &pProvider);

        if(result != ERROR_SUCCESS)
        {
            std::cerr << std::format("FwpmProviderGetByKey failed: {}\n", getErrorString(result));
            return nullptr;
        }

        return pProvider;
    }

    // Iterate over all filters for all given layers
    template <typename IterFuncT>
        requires std::invocable<IterFuncT, std::shared_ptr<FWPM_FILTER>>
    void enumerateFiltersForLayers(const std::vector<GUID> &layerKeys, IterFuncT func) const
    {
        FilterEnum{layerKeys, _handle}.forEach(func);
    }

    // Iterate over filters for just one layer
    template <typename IterFuncT>
        requires std::invocable<IterFuncT, std::shared_ptr<FWPM_FILTER>>
    void enumerateFiltersForLayer(const GUID& layerKey, IterFuncT func) const
    {
        SingleLayerFilterEnum{layerKey, _handle}.forEach(func);
    }

    auto filtersForLayer(const GUID &layerKey) const
        -> FilterSet
    {
        return SingleLayerFilterEnum{layerKey, _handle}.filters();
    }

    template <typename CallbackFuncT>
        requires std::invocable<CallbackFuncT, void*, const FWPM_NET_EVENT*>
    void monitorEvents(CallbackFuncT callbackFunc)
    {
        _pMonitor = std::make_unique<EventMonitor>(_handle);
        _pMonitor->start(callbackFunc);
    }

    // Delete a filter by Id
    DWORD deleteFilterById(FilterId filerId) const;

    auto handle() -> HANDLE { return _handle; }
    operator HANDLE() { return handle(); }

private:
    HANDLE _handle{};
    std::unique_ptr<EventMonitor> _pMonitor;
};
}
