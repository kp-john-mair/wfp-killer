#pragma once

#include <guiddef.h>
#include <fwpmu.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <format>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <ranges>
#include <concepts>
#include <assert.h>
#include <filesystem>
#include <magic_enum.h>

// Allow GUID to be used as a key in a hash
template <>
struct std::hash<GUID>
{
    size_t operator()(const GUID& guid) const
    {
        const uint64_t *p = reinterpret_cast<const uint64_t*>(&guid);
        return std::hash<uint64_t>{}(p[0]) ^ std::hash<uint64_t>{}(p[1]);
    }
};

namespace wfpk {
// CRTP singleton mixin
template<class Derived>
class Singleton
{
public:
    Singleton() { assert(_instance == nullptr); _instance = this; }
    ~Singleton() { _instance = nullptr; }
    static Derived* instance() { return static_cast<Derived*>(_instance); }
private:
    inline static Singleton* _instance = nullptr;
};

// Given an enum value, return its name as a string
template <typename T>
std::string enumName(T enumValue)
{
    return std::string{magic_enum::enum_name(enumValue)};
}

// Split a std::string based on a delim
auto splitString(const std::string &str, char delim) -> std::vector<std::string>;

// Helpers to represent Ip addresses as strings
// Ipv4
std::string ipToString(UINT32 ipAddress);
// Ipv6 - Ensure we get an array of UINT8[16] - prevent decay to pointer
std::string ipToString(const UINT8 (&ipAddress)[16]);
// blobs here represent appIds
std::string blobToString(const FWP_BYTE_BLOB &blob);
// Convert a std::wstring to a std::string
std::string wideStringToString(const std::wstring &wstr);
// Convert a GUID to a std::string
std::string guidToString(const GUID& guid);
// Lowercase a string
std::string toLowercase(const std::string& input);

std::string getErrorString(DWORD errorCode);

// Validate a string contains an ipv4 address
bool isIpv4(const std::string &ipAddress);
// Validate a string contains an ipv6 address
bool isIpv6(const std::string &ipAddress);

// Join together a vector of elements of type T as a string
template <typename T>
std::string joinVec(const std::vector<T>& ports) {
    std::ostringstream oss;

    for(size_t i = 0; i < ports.size(); ++i)
    {
        if(i != 0)
            oss << ", ";

        oss << ports[i];
    }

    return oss.str();
}

// void fwpConditionValueHandler(const FWP_CONDITION_VALUE &value, auto handlerFunc)
// {
//     switch(value.type)
//     {
//     case FWP_EMPTY:
//         break;
//     case FWP_UINT8:
//         handlerFunc(value.uint8);
//         break;
//     case FWP_UINT16:
//         handlerFunc(value.uint16);
//         break;
//     case FWP_UINT32:
//         handlerFunc(value.uint32);
//         break;
//     case FWP_UINT64:
//         handlerFunc(value.uint64);
//         break;
//     case FWP_INT8:
//         handlerFunc(value.int8);
//         break;
//     case FWP_INT16:
//         handlerFunc(value.int16);
//         break;
//     case FWP_INT32:
//         handlerFunc(value.int32);
//         break;
//     case FWP_INT64:
//         handlerFunc(value.int64);
//         break;
//     case FWP_FLOAT:
//         handlerFunc(value.float32);
//         break;
//     case FWP_DOUBLE:
//         handlerFunc(value.double64);
//         break;
//     case FWP_BYTE_ARRAY16_TYPE:
//         handlerFunc(value.byteArray16);
//         break;
//     case FWP_BYTE_BLOB_TYPE:
//         handlerFunc(value.byteBlob);
//         break;
//     case FWP_SID:
//         handlerFunc(value.sid);
//         break;
//     case FWP_SECURITY_DESCRIPTOR_TYPE:
//         handlerFunc(value.sd);
//         break;
//     case FWP_TOKEN_INFORMATION_TYPE:
//         handlerFunc(value.tokenInformation);
//         break;
//     case FWP_TOKEN_ACCESS_INFORMATION_TYPE:
//         handlerFunc(value.tokenAccessInformation);
//         break;
//     case FWP_UNICODE_STRING_TYPE:
//         handlerFunc(value.unicodeString);
//         break;
//     case FWP_BYTE_ARRAY6_TYPE:
//         handlerFunc(value.byteArray6);
//         break;
//     case FWP_BITMAP_INDEX_TYPE:
//         handlerFunc(value.bitmapArray64);
//         break;
//     case FWP_BITMAP_ARRAY64_TYPE:
//         handlerFunc(value.bitmapArray64);
//         break;
//     case FWP_SINGLE_DATA_TYPE_MAX:
//         break;
//     case FWP_V4_ADDR_MASK:
//         handlerFunc(value.v4AddrMask);
//         break;
//     case FWP_V6_ADDR_MASK:
//         handlerFunc(value.v6AddrMask);
//         break;
//     case FWP_RANGE_TYPE:
//         handlerFunc(value.rangeValue);
//         break;
//     case FWP_DATA_TYPE_MAX:
//         break;
//     }
// }
}

