// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>

namespace clarisma {

class HttpResponse
{
public:
    HttpResponse() : hRequest_(nullptr) {}
    ~HttpResponse() { close(); }
    explicit HttpResponse(const HINTERNET hRequest) : hRequest_(hRequest) {}
    int status() const;
    size_t read(void* buf, size_t size);
    void close();

private:
    HINTERNET hRequest_;
};

class HttpClient 
{
public:
    explicit HttpClient(const char* host);
    ~HttpClient() { close(); };
    void setUserAgent(const char* name);
    void setRedirects(int max);
    void setTimeout(int ms);
    void open();
    void close();
    bool isOpen() { return hConnect_ != nullptr; }
    HttpResponse get(const char* url);

private:
    static std::wstring toWideString(std::string_view s);

    HINTERNET hSession_;
    HINTERNET hConnect_;
    std::wstring host_;
    std::wstring userAgent_;
};

} // namespace clarisma