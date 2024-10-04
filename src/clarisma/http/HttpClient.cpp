// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <clarisma/http/HttpClient.h>
#include <iostream>

namespace clarisma {

std::wstring HttpClient::toWideString(std::string_view s)
{
    int size = static_cast<int>(s.size());
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, s.data(), size, NULL, 0);
    std::wstring wideString(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), size, &wideString[0], sizeNeeded);
    return wideString;
}

HttpClient::HttpClient(const char* host) :
    host_(toWideString(host)),
    hSession_(NULL),
    hConnect_(NULL)
{
}

void HttpClient::open()
{
    hSession_ = WinHttpOpen(userAgent_.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession_)
    {
        std::cerr << "WinHttpOpen failed: " << GetLastError() << std::endl;
        return;
    }

    hConnect_ = WinHttpConnect(hSession_, host_.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect_)
    {
        std::cerr << "WinHttpConnect failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession_);
        hSession_ = NULL;
        return;
    }
}

HttpResponse HttpClient::get(const char* url)
{
    std::wstring urlW = toWideString(url);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect_, L"GET", urlW.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest)
    {
        std::cerr << "WinHttpOpenRequest failed: " << GetLastError() << std::endl;
        return HttpResponse();
    }

    // Send the request
    BOOL bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
        0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResult)
    {
        std::cerr << "WinHttpSendRequest failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        return HttpResponse();
    }

    // Receive the response
    bResult = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResult)
    {
        std::cerr << "WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
        return HttpResponse();
    }
    return HttpResponse(hRequest);
}

void HttpClient::close()
{
    if(hConnect_)
    {
        WinHttpCloseHandle(hConnect_);
        hConnect_ = NULL;
    }
    if(hSession_)
    {
        WinHttpCloseHandle(hSession_);
        hSession_ = NULL;
    }
}

size_t HttpResponse::read(void* buf, size_t size)
{
    std::byte* p = reinterpret_cast<std::byte*>(buf);
    DWORD bytesRead = 0;
    DWORD totalBytesToRead = static_cast<DWORD>(size);
    DWORD totalBytesRead = 0;

    // Keep reading until all requested bytes are read or an error occurs
    while (totalBytesRead < totalBytesToRead)
    {
        if (!WinHttpReadData(hRequest_, p, totalBytesToRead - totalBytesRead, &bytesRead))
        {
            std::cerr << "Error reading data: " << GetLastError() << std::endl;
            return 0;
        }

        // If no bytes were read, we've reached the end of the response
        if (bytesRead == 0)
        {
            break;
        }
        totalBytesRead += bytesRead;
        p += bytesRead;
    }
    return totalBytesRead;
}

void HttpResponse::close()
{
    if(hRequest_)
    {
        WinHttpCloseHandle(hRequest_);
        hRequest_ = NULL;
    }
}

} // namespace clarisma