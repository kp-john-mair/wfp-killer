#include <winsock2.h>
#include <ws2tcpip.h>
#include <fwpmu.h>

#include "utils.h"

namespace wfpk {
std::string ipToString(UINT32 ipAddress)
{
    char str[INET_ADDRSTRLEN]{};
    InetNtopA(AF_INET, &ipAddress, str, INET_ADDRSTRLEN);

    return std::string{str};
}

std::string ipToString(const FWP_BYTE_ARRAY16 &ipAddress)
{
    char str[INET6_ADDRSTRLEN]{};
    InetNtopA(AF_INET6, &ipAddress.byteArray16, str, INET6_ADDRSTRLEN);

    return str;
}

std::string blobToString(const FWP_BYTE_BLOB &blob)
{
    UINT8* data = blob.data;
    size_t numChars = blob.size / sizeof(wchar_t) - 1;

    std::string str;
    std::wstring wstr(reinterpret_cast<const wchar_t*>(data), numChars);

    // hack to convert wide strings to strings
    std::transform(wstr.begin(), wstr.end(), std::back_inserter(str), [] (wchar_t c) {
        return static_cast<char>(c);
    });

    return str;
}
}
