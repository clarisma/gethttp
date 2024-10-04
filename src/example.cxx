#include <windows.h>
#include <winhttp.h>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

int main()
{
    // Initialize WinHTTP session with a custom User-Agent
    HINTERNET hSession = WinHttpOpen(L"MyApp/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession)
    {
        std::cerr << "WinHttpOpen failed: " << GetLastError() << std::endl;
        return 1;
    }

    // Set the maximum number of redirects
    DWORD maxRedirects = 3;
    WinHttpSetOption(hSession, WINHTTP_OPTION_MAX_HTTP_AUTOMATIC_REDIRECTS,
                     &maxRedirects, sizeof(maxRedirects));

    // Enable automatic redirection
    DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY,
                     &redirectPolicy, sizeof(redirectPolicy));

    // Connect to the server (HTTPS)
    HINTERNET hConnect = WinHttpConnect(hSession, L"www.example.com",
                                        INTERNET_DEFAULT_HTTPS_PORT, 0);

    if (!hConnect)
    {
        std::cerr << "WinHttpConnect failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // Create an HTTP request handle
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/largefile",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);

    if (!hRequest)
    {
        std::cerr << "WinHttpOpenRequest failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // Send the request
    BOOL bResult = WinHttpSendRequest(hRequest,
                                      WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0,
                                      0, 0);

    if (!bResult)
    {
        std::cerr << "WinHttpSendRequest failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // Receive the response
    bResult = WinHttpReceiveResponse(hRequest, NULL);

    if (!bResult)
    {
        std::cerr << "WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
    }
    else
    {
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        BYTE* pBuffer = nullptr;

        // Read the response in chunks
        do
        {
            // Check how much data is available
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            {
                std::cerr << "WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
                break;
            }

            if (dwSize == 0)
            {
                // No more data available
                break;
            }

            // Allocate buffer for the data
            pBuffer = new BYTE[dwSize];

            if (!pBuffer)
            {
                std::cerr << "Out of memory" << std::endl;
                break;
            }

            // Read the data
            ZeroMemory(pBuffer, dwSize);
            if (!WinHttpReadData(hRequest, pBuffer, dwSize, &dwDownloaded))
            {
                std::cerr << "WinHttpReadData failed: " << GetLastError() << std::endl;
                delete[] pBuffer;
                break;
            }

            // Process the chunk (for example, write to console or file)
            std::cout.write(reinterpret_cast<char*>(pBuffer), dwDownloaded);

            // Clean up
            delete[] pBuffer;

        } while (dwSize > 0);
    }

    // Close handles
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return 0;
}
