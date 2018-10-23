#include "StdAfx.h"

JsHostMemoryMappedBuffer::JsHostMemoryMappedBuffer(LPCWSTR strScriptFilePath):
    _strScriptFilePath(strScriptFilePath),
    _hFile(INVALID_HANDLE_VALUE),
    _hMapping(INVALID_HANDLE_VALUE)
{
}

// Causes the buffer to be mapped in and returns the pointer to the raw bytes
BYTE* JsHostMemoryMappedBuffer::GetRawBuffer() const
{
    if (HostConfigFlags::flags.TraceMemoryBuffer)
    {
        printf("GetRawBuffer called\n");
    }

    Assert(_pBuffer);
    return _pBuffer;
}

// Causes the view on the memory mapped buffer to get recreated
// This causes the file to be paged out
HRESULT JsHostMemoryMappedBuffer::RecreateView()
{
    if (HostConfigFlags::flags.TraceMemoryBuffer)
    {
        printf("View recreated\n");
    }

    ::UnmapViewOfFile(_pBuffer);
    _pBuffer = (BYTE*) ::MapViewOfFile(_hMapping, FILE_MAP_READ, 0, 0, 0);
    if (_pBuffer == NULL)
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    return S_OK;
}

void JsHostMemoryMappedBuffer::Free()
{
    if (HostConfigFlags::flags.TraceMemoryBuffer)
    {
        printf("Close called\n");
    }

    if (_pBuffer)
    {
        ::UnmapViewOfFile(_pBuffer);
        _pBuffer = NULL;
        _cbFileSize = (size_t) -1;
    }

    if (_hMapping != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(_hMapping);
    }

    if (_hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(_hFile);
    }

    // A bit shady, but fine for test code
    HeapFree(GetProcessHeap(), 0, (void*)this);
}

#define VALIDATE_HANDLE_OR_RETURN_HR(handle) \
    if (handle == INVALID_HANDLE_VALUE) { \
        return HRESULT_FROM_WIN32(::GetLastError()); \
    } \
    _##handle = handle;

HRESULT JsHostMemoryMappedBuffer::Initialize()
{
    // To map a file, we need to do the following:
    // 1. Open the file with CreateFile
    // 2. Create a file mapping object
    // 3. Create a file view to access the shared memory

    HANDLE hFile = ::CreateFile(_strScriptFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    VALIDATE_HANDLE_OR_RETURN_HR(hFile);

    LARGE_INTEGER fileSize;
    if (::GetFileSizeEx(hFile, &fileSize) == FALSE)
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    LONGLONG fs = static_cast<LONGLONG>(static_cast<size_t>(fileSize.QuadPart));

    if (fs != fileSize.QuadPart)
    {
        return E_OUTOFMEMORY;
    }

    _cbFileSize = static_cast<size_t>(fs);

    HANDLE hMapping = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    VALIDATE_HANDLE_OR_RETURN_HR(hMapping);

    _pBuffer = (BYTE*) ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (_pBuffer == NULL)
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    return S_OK;
}

JsHostMemoryMappedBuffer* JsHostMemoryMappedBuffer::Create(LPCWSTR strScriptFilePath)
{
    void* mem = HeapAlloc(GetProcessHeap(), 0, sizeof(JsHostMemoryMappedBuffer));

    if (!mem)
    {
        return nullptr;
    }

    JsHostMemoryMappedBuffer* pMemoryMappedBuffer = new (mem) JsHostMemoryMappedBuffer(strScriptFilePath);

    if (FAILED(pMemoryMappedBuffer->Initialize()))
    {
        pMemoryMappedBuffer->Free();
        pMemoryMappedBuffer = nullptr;
    }

    return pMemoryMappedBuffer;
}