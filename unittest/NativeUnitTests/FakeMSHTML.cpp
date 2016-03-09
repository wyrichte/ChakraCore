#include "StdAfx.h"
#include "ScriptDirectUnitTests.h"

HRESULT FakeMSHTML::GetReadDataStream(__out IStream **ppStream)
{
    printf("GetReadDataStream is called\n");

    //Set IStream to NULL and return E_FAIL on GetReadDataStream call
    if(mNullGetReadDataStream)
    {
        printf("GetReadDataStream will return NULL IStream now...\n");
        *ppStream = NULL;
        return E_FAIL;
    }
    //Reading the input file for the profile cache and put it in IStream
    if(mInputCacheFile != NULL)
    {
        FILE* file;
        if(!_wfopen_s(&file, mInputCacheFile, _u("rb")))
        {
            fseek(file, 0, SEEK_END);
            UINT lengthBytes = ftell(file);
            fseek(file, 0, SEEK_SET);
            byte *contentsRaw = (byte*) calloc(lengthBytes, sizeof(byte));
            if (NULL == contentsRaw)
            {
                fwprintf(stderr, _u("out of memory"));
                return E_OUTOFMEMORY;
            }
            fread((void*) contentsRaw, sizeof(byte), lengthBytes, file);
            fclose(file);
            IStream *outStream = SHCreateMemStream(nullptr, 0);
            ULONG written;
            outStream->Write(contentsRaw, lengthBytes, &written);
            LARGE_INTEGER position = {0};
            outStream->Seek(position, STREAM_SEEK_SET, NULL);
            *ppStream = outStream;

            return S_OK;
        }
    }
    return E_FAIL;
}
HRESULT FakeMSHTML::GetWriteDataStream(__out IStream **ppStream)
{
    printf("GetWriteDataStream is called\n");

    mGetWriteDataStreamCounter++;
    //Set IStream to NULL and return E_FAIL on GetWriteDataStream call
    if(mNullGetWriteDataStream)
    {
        printf("GetWriteDataStream will return NULL IStream now...\n");
        *ppStream = NULL;
        return E_FAIL;
    }
    //Failing the GetReadDataStream call
    if(mFailGetWriteDataStream)
    {
        printf("GetWriteDataStream will return E_FAIL now...\n");
        return E_FAIL;
    }

    //Positive case, returning actual IStream
    IStream *pWriteStream = SHCreateMemStream(nullptr, 0);
    if (pWriteStream)
    {
        *ppStream = pWriteStream;  
        return S_OK;
    }
    return E_FAIL;
}
HRESULT FakeMSHTML::SaveWriteDataStream(__in IStream *pStream)
{
    printf("SaveWriteDataStream is called\n");

    mSaveWriteDataStreamCounter++;

    //Failing the SaveWriteDataStream call
    if(mFailSaveWriteDataStream)
    {
        printf("SaveWriteDataStream will return E_FAIL now...\n");
        return E_FAIL;
    }

    STATSTG statInfo;
    pStream->Stat(&statInfo, STATFLAG_NONAME);

    LARGE_INTEGER position = {0};
    pStream->Seek(position, STREAM_SEEK_SET, NULL);

    //Positive case, saving IStream to profile cache file
    IStream *outStream;
    LPWSTR outCacheFileName = _u("output.out");
    if(mOutputCacheFile != NULL)
    {
        outCacheFileName = this->mOutputCacheFile;
    }
    if(SUCCEEDED(SHCreateStreamOnFile(outCacheFileName, STGM_WRITE | STGM_CREATE, &outStream)))
    {
        //Copying content of the IStream to file
        pStream->CopyTo(outStream,statInfo.cbSize,NULL,NULL);
        outStream->Commit(STGC_DEFAULT);
        outStream->Release();
    }
    return S_OK;
}
HRESULT FakeMSHTML::GetUrl(__out BSTR *pbstrUrl)
{
    return S_OK;
}
HRESULT FakeMSHTML::GetLineColumn(__out ULONG *pulLine, __out ULONG *pulColumn)
{
    return S_OK;
}
HRESULT FakeMSHTML::GetOffset(__out ULONG *pulOffset)
{
    return S_OK;
}
HRESULT FakeMSHTML::IsDynamicDocument(__out BOOL *pfDynamicDocument)
{
    *pfDynamicDocument = FALSE;
    return S_OK;
}
HRESULT FakeMSHTML::GetSourceMapUrl(__out BSTR *pbstrSourceMapUrl)
{
    return S_OK;
}
STDMETHODIMP FakeMSHTML::QueryInterface(REFIID riid, void ** ppvObj)
{
    if(riid == __uuidof(IActiveScriptDataCache))
    {
        *ppvObj = static_cast<IActiveScriptDataCache*>(this);
    }
    else if(riid == __uuidof(IActiveScriptContext))
    {
        *ppvObj = static_cast<IActiveScriptContext*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}
STDMETHODIMP_(ULONG) FakeMSHTML::AddRef(void)
{
    return InterlockedIncrement(&mRefCount);
}

STDMETHODIMP_(ULONG) FakeMSHTML::Release(void)
{
    LONG res = InterlockedDecrement(&mRefCount);

    if (res == 0)
    {
        delete this;
    }

    return res;

}

UINT FakeMSHTML::GetWriteDataStreamCounter()
{
    return mGetWriteDataStreamCounter;
}

UINT FakeMSHTML::SaveWriteDataStreamCounter()
{
    return mSaveWriteDataStreamCounter;
}

void FakeMSHTML::SetFailGetWriteDataStream(BOOL fail)
{
    mFailGetWriteDataStream = fail;
}
void FakeMSHTML::SetFailSaveWriteDataStream(BOOL fail)
{
    mFailSaveWriteDataStream = fail;
}
void FakeMSHTML::SetNullGetWriteDataStream(BOOL nullstream)
{
    mNullGetWriteDataStream = nullstream;
}
void FakeMSHTML::SetNullGetReadDataStream(BOOL nullstream)
{
    mNullGetReadDataStream = nullstream;
}
