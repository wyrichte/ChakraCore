/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"
#include "codex\Utf8Codex.h"

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
    if (_wfopen_s(&file, filename, _u("rb")) != 0)
    {
        if (printFileOpenError)
        {
            char16 wszBuff[512];
            fwprintf(stderr, _u("_wfopen of %s failed"), filename);
            wszBuff[0] = 0;
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                0,
                wszBuff,
                _countof(wszBuff),
                NULL))
            {
                fwprintf(stderr, _u(": %s"), wszBuff);
            }
            fwprintf(stderr, _u("\n"));
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
        fwprintf(stderr, _u("out of memory"));
        IfFailGo(E_OUTOFMEMORY);
    }

    //
    // Read the entire content as a binary block.
    //
    fread((void*) contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);
    *(WCHAR*)((byte*)contentsRaw + lengthBytes) = _u('\0'); // Null terminate it. Could be LPCWSTR.

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
        fwprintf(stderr, _u("unsupported file encoding"));
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
        utf8::DecodeOptions decodeOptions = utf8::doAllowInvalidWCHARs;

        UINT cUtf16Chars = utf8::ByteIndexIntoCharacterIndex(pRawBytes, lengthBytes, decodeOptions);
        contents = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, (cUtf16Chars + 1) * sizeof(WCHAR)); // Simulate Trident buffer, allocate by HeapAlloc
        if (NULL == contents)
        {
            fwprintf(stderr, _u("out of memory"));
            IfFailGo(E_OUTOFMEMORY);
        }

        utf8::DecodeIntoAndNullTerminate((char16*) contents, pRawBytes, cUtf16Chars, decodeOptions);
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

HRESULT JsHostLoadBinaryFile(LPCWSTR filename, LPCWSTR& contents, UINT& lengthBytes, bool printFileOpenError)
{
    HRESULT hr = S_OK;
    contents = nullptr;
    lengthBytes = 0;
    FILE * file;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //
    if (_wfopen_s(&file, filename, _u("rb")) != 0)
    {
        if (printFileOpenError)
        {
            DWORD lastError = GetLastError();
            char16 wszBuff[512];
            fwprintf(stderr, _u("Error in opening file '%s' "), filename);
            wszBuff[0] = 0;
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              nullptr,
                              lastError,
                              0,
                              wszBuff,
                              _countof(wszBuff),
                              nullptr))
            {
                fwprintf(stderr, _u(": %s"), wszBuff);
            }
            fwprintf(stderr, _u("\n"));
            IfFailGo(E_FAIL);
        }
        else
        {
            return E_FAIL;
        }
    }
    // file will not be nullptr if _wfopen_s succeeds
    __analysis_assume(file != nullptr);

    //
    // Determine the file length, in bytes.
    //
    fseek(file, 0, SEEK_END);
    lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    contents = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, lengthBytes);
    if (nullptr == contents)
    {
        fwprintf(stderr, _u("out of memory"));
        IfFailGo(E_OUTOFMEMORY);
    }
    //
    // Read the entire content as a binary block.
    //
    size_t result = fread((void*)contents, sizeof(char), lengthBytes, file);
    if (result != lengthBytes)
    {
        fwprintf(stderr, _u("Read error"));
        IfFailGo(E_FAIL);
    }
    fclose(file);

Error:
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

    LPWSTR pchFile = wcsrchr(pchUrl, _u('/'));
    if (pchFile == NULL)
    {
        pchFile = wcsrchr(pchUrl, _u('\\'));
    }

    LPCWSTR pchToCopy = pchUrl;

    if (pchFile != NULL)
    {
        pchToCopy = pchFile+1;
    }

    wcscpy_s(pchShortName, cchBuffer, pchToCopy);
}


HRESULT PrivateCoCreate(
    HINSTANCE hInstModule,
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID iid,
    LPVOID* ppunk
    )
{
    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    pProc = (FN_DllGetClassObject)GetProcAddress(hInstModule, "DllGetClassObject");
    if (pProc == NULL) return E_FAIL;

    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(pUnkOuter, iid, ppunk));
Error:
    return hr;
}

HRESULT LoadPDM(HINSTANCE* phInstPdm, IProcessDebugManager ** ppPDM)
{
    // Only try loading from the same path as jshost.  Do not try to fallback and load
    // whatever pdm is registered with the system.  We want to ensure that we always
    // run with our version of pdm.  Therefore, fail fast if it is not found beside jshost.
    char16 pdmPath[_MAX_PATH];
    char16 pdmDir[_MAX_PATH];
    char16 pdmDrive[_MAX_DRIVE];
    char16 *pdmFilename = _u("pdm.dll");

    if (phInstPdm != nullptr)
    {
        *phInstPdm = nullptr;
    }

    DWORD len = GetModuleFileName(nullptr, pdmPath, _MAX_PATH);

    if (len == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (len == _MAX_PATH && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    _wsplitpath_s(pdmPath, pdmDrive, _MAX_DRIVE, pdmDir, _MAX_PATH, nullptr, 0, nullptr, 0);

    wcscpy_s(pdmPath, pdmDrive);
    wcscat_s(pdmPath, pdmDir);

    size_t strLen = wcslen(pdmPath);
    size_t localLen = wcslen(pdmFilename);

    if (strLen + localLen >= _MAX_PATH)
    {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    wcscat_s(pdmPath, pdmFilename);
    HINSTANCE hInstPdm = LoadLibrary(pdmPath);

    if (hInstPdm == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (phInstPdm != nullptr)
    {
        *phInstPdm = hInstPdm;
    }

    return PrivateCoCreate(hInstPdm, __uuidof(ProcessDebugManager), NULL, CLSCTX_INPROC_SERVER, _uuidof(IProcessDebugManager), (LPVOID*) ppPDM);
}
