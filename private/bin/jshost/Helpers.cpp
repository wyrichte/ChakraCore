/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"

HRESULT JsHostLoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out, LPCWSTR* contentsRawOut, UINT* lengthBytesOut, bool printFileOpenError)
{
    HRESULT hr = S_OK;
    LPCWSTR contentsRaw = nullptr;
    UINT lengthBytes = 0;
    bool isUtf8 = false;
    contents = nullptr;
    FILE * file;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //
    if (_wfopen_s(&file, filename, L"rb") != 0)
    {
        if (printFileOpenError)
        {
            wchar_t wszBuff[512];
            fwprintf(stderr, L"_wfopen of %s failed", filename);
            wszBuff[0] = 0;
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                0,
                wszBuff,
                _countof(wszBuff),
                NULL))
            {
                fwprintf(stderr, L": %s", wszBuff);
            }
            fwprintf(stderr, L"\n");
            IfFailGo(E_FAIL);
        }
        else
        {
            return E_FAIL;
        }
    }

    //
    // Determine the file length, in bytes.
    //
    fseek(file, 0, SEEK_END);
    lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    contentsRaw = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, lengthBytes + sizeof(WCHAR)); // Simulate Trident buffer, allocate by HeapAlloc
    if (NULL == contentsRaw)
    {
        fwprintf(stderr, L"out of memory");
        IfFailGo(E_OUTOFMEMORY);
    }

    //
    // Read the entire content as a binary block.
    //
    fread((void*) contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);
    *(WCHAR*)((byte*)contentsRaw + lengthBytes) = L'\0'; // Null terminate it. Could be LPCWSTR.

    //
    // Read encoding, handling any conversion to Unicode.
    //
    // Warning: The UNICODE buffer for parsing is supposed to be provided by the host.
    // this is temporary code to read from Unicode and ANSI files.
    // It is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
    // wrongly classified as ANSI
    //
    byte * pRawBytes = (byte*)contentsRaw;
    if( (0xEF == *pRawBytes && 0xBB == *(pRawBytes+1) && 0xBF == *(pRawBytes+2)))
    {
        isUtf8 = true;
    }
    else if (0xFFFE == *contentsRaw || 0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw+1))
    {
        // unicode unsupported
        fwprintf(stderr, L"unsupported file encoding");
        IfFailGo(E_UNEXPECTED);
    }
    else if (0xFEFF == *contentsRaw)
    {
        // unicode LE
        contents = contentsRaw;
    }
    else
    {
        // Assume UTF8
        isUtf8 = true;
    }


    if (isUtf8)
    {        
        UINT cAnsiChars = lengthBytes + 1;
        contents = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, cAnsiChars * sizeof(WCHAR)); // Simulate Trident buffer, allocate by HeapAlloc
        if (NULL == contents)
        {
            fwprintf(stderr, L"out of memory");
            IfFailGo(E_OUTOFMEMORY);
        }

        // Covert to Unicode.
        if (0 == MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)contentsRaw, cAnsiChars,
            (LPWSTR)contents, cAnsiChars))
        {
            fwprintf(stderr, L"failed MultiByteToWideChar conversion");
            IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

Error:
    if (SUCCEEDED(hr) && isUtf8Out)
    {
        Assert(contentsRawOut);
        Assert(lengthBytesOut);
        *isUtf8Out = isUtf8;
        *contentsRawOut = contentsRaw;
        *lengthBytesOut = lengthBytes;
    }
    else if (contentsRaw && (contentsRaw != contents)) // Otherwise contentsRaw is lost. Free it if it is different to contents.
    {
        HeapFree(GetProcessHeap(), 0, (void*)contentsRaw);
    }

    if (contents && FAILED(hr))
    {
        HeapFree(GetProcessHeap(), 0, (void*)contents);
        contents = nullptr;
    }

    return hr;
}

void GetShortNameFromUrl(__in LPCWSTR pchUrl, __in LPWSTR pchShortName, __in size_t cchBuffer)
{
    // Note : We can use help from the wininet for cracking the url properly. but for now below logic will just do.

    LPWSTR pchFile = wcsrchr(pchUrl, L'/');
    if (pchFile == NULL)
    {
        pchFile = wcsrchr(pchUrl, L'\\');
    }

    LPCWSTR pchToCopy = pchUrl;

    if (pchFile != NULL)
    {
        pchToCopy = pchFile+1;
    }

    wcscpy_s(pchShortName, cchBuffer, pchToCopy);
}


HRESULT PrivateCoCreate(
    LPCWSTR strModule,
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID iid,
    LPVOID* ppunk
    )
{
    HRESULT hr = NOERROR;
    HINSTANCE hInstance = NULL;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    hInstance = LoadLibraryEx(strModule, NULL, 0);
    if (hInstance == NULL) return E_FAIL;

    pProc = (FN_DllGetClassObject)GetProcAddress(hInstance, "DllGetClassObject");
    if (pProc == NULL) return E_FAIL;

    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(pUnkOuter, iid, ppunk));
Error:
    return hr;
}

HRESULT LoadPDM(__deref_out_z WCHAR ** ppPdmPath, IProcessDebugManager ** ppPDM)
{
    // First try from the .local.

    WCHAR * pdmPath = new WCHAR[_MAX_PATH];
    if (ppPdmPath)
    {
        *ppPdmPath = pdmPath;
    }

    GetModuleFileName(NULL, pdmPath, _MAX_PATH);
    size_t strLen = wcslen(pdmPath);
    size_t localLen = wcslen(L".local\\pdm.dll");

    if (strLen+localLen < _MAX_PATH)
    {
        wcsncpy_s(pdmPath+strLen, _MAX_PATH-strLen, L".local\\pdm.dll", localLen);
        if (_waccess_s(pdmPath, 0) == 0)
        {
            // Load it from here.
            return PrivateCoCreate(pdmPath, CLSID_ProcessDebugManager, NULL, CLSCTX_INPROC_SERVER, _uuidof(IProcessDebugManager),(LPVOID*)ppPDM);
        }
    }

    // Now from the _NTTREE

    DWORD envVarLen = GetEnvironmentVariable(L"_NTTREE", NULL, 0);
    if (envVarLen)
    {
        size_t dlllen = wcslen(L"\\pdm.dll");
        ZeroMemory(pdmPath, _MAX_PATH);

        if (envVarLen + dlllen < _MAX_PATH)
        {
            GetEnvironmentVariable(L"_NTTREE", pdmPath, envVarLen);
            envVarLen = wcslen(pdmPath);

            wcsncpy_s(pdmPath+envVarLen, _MAX_PATH-envVarLen, L"\\pdm.dll", dlllen);

            if (_waccess_s(pdmPath, 0) == 0)
            {
                return PrivateCoCreate(pdmPath, CLSID_ProcessDebugManager, NULL, CLSCTX_INPROC_SERVER, _uuidof(IProcessDebugManager),(LPVOID*)ppPDM);
            }
        }
    }

    delete [] pdmPath;
    if (ppPdmPath)
    {
        *ppPdmPath = NULL;
    }

    // Try from registry 
    return CoCreateInstance(CLSID_ProcessDebugManager, NULL, CLSCTX_INPROC_SERVER, _uuidof(IProcessDebugManager),(LPVOID*)ppPDM);
}


