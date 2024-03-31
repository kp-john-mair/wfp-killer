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

struct WfpDeleter
{
    template <typename WfpPtr>
    void operator()(WfpPtr *ptr) const
    {
        if(ptr)
            FwpmFreeMemory(reinterpret_cast<void**>(&ptr));
    }
};

class SingleLayerFilterEnum;
class FilterEnum;
class EventMonitor;

class EventMonitor
{
public:
    static uint64_t context;

public:
    EventMonitor(HANDLE engineHandle, const GUID& sessionKey)
    : _engineHandle{engineHandle}
    , _eventSubscriptionHandle{}
    , _sessionKey{sessionKey}
    {
    }

    template <typename FuncT>
        requires std::invocable<FuncT, void*, const FWPM_NET_EVENT*>
    void start(FuncT callbackFunc)
    {
        HANDLE eventHandle{};
        FWPM_FILTER_CONDITION condition{};
        condition.fieldKey = FWPM_CONDITION_IP_PROTOCOL;
        condition.matchType = FWP_MATCH_EQUAL;
        condition.conditionValue.type = FWP_UINT8;
        condition.conditionValue.uint8 = 6;
        FWPM_NET_EVENT_ENUM_TEMPLATE eventEnumTemplate{};
        // Set startTime to current time ('now')
        GetSystemTimeAsFileTime(&eventEnumTemplate.startTime);
        // Maximum possible date for endTime - since we don't want an end date
        eventEnumTemplate.endTime.dwLowDateTime = eventEnumTemplate.startTime.dwHighDateTime + 10000;
        eventEnumTemplate.endTime.dwHighDateTime = eventEnumTemplate.startTime.dwHighDateTime + 1000;
        eventEnumTemplate.filterCondition = &condition;
        eventEnumTemplate.numFilterConditions = 0;

        FWPM_NET_EVENT_SUBSCRIPTION subscription{};
        subscription.enumTemplate = &eventEnumTemplate;
        subscription.sessionKey = _sessionKey ;
        subscription.flags = {};

        DWORD result = FwpmNetEventSubscribe(
            _engineHandle,
            &subscription,
            callbackFunc,
            &EventMonitor::context, // Context for the callback; can be NULL
            &_eventSubscriptionHandle // Receives the subscription handle
        );

        if(result != ERROR_SUCCESS)
            throw WfpError{"FwpmNetEventSubscribe failed, code: " + result};
    }

    ~EventMonitor()
    {
        if(!_engineHandle || !_eventSubscriptionHandle)
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
    GUID _sessionKey;

};

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

    template <typename CallbackFuncT>
        requires std::invocable<CallbackFuncT, void*, const FWPM_NET_EVENT*>
    void monitorEvents(CallbackFuncT callbackFunc)
    {
        _pMonitor = std::make_unique<EventMonitor>(_handle, _session.sessionKey);
        _pMonitor->start(callbackFunc);
    }

    // Delete a filter by Id
    DWORD deleteFilterById(FilterId filerId) const;

    auto handle() -> HANDLE { return _handle; }
    operator HANDLE() { return handle(); }

private:
    HANDLE _handle{};
    FWPM_SESSION0 _session{};
    std::unique_ptr<EventMonitor> _pMonitor;
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

private:
    // Ensure filters are sorted by weight
    struct FilterCompare
    {
        template <typename PtrT>
        bool operator()(const PtrT &lhs, const PtrT &rhs) const
        {
            // TODO: uint8 weight is unique to PIA - make this
            // more general.
            return lhs->weight.uint8 > rhs->weight.uint8;
        }
    };

    // Use a multiset so we can have multiple filters of the same weight
    using FilterSet = std::multiset<std::shared_ptr<FWPM_FILTER>, FilterCompare>;

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
}
