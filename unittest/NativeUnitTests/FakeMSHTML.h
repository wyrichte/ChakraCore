#include "StdAfx.h"

/*
Fake mshtml which implements IActiveScriptDataCache and IActiveScriptContext.
*/
class FakeMSHTML : public IActiveScriptDataCache, public IActiveScriptContext
{
private:
    ULONG mRefCount;
    LPWSTR mOutputCacheFile;
    LPWSTR mInputCacheFile;
    UINT mGetWriteDataStreamCounter;
    UINT mSaveWriteDataStreamCounter;
    BOOL mFailGetWriteDataStream;
    BOOL mFailSaveWriteDataStream;
    BOOL mNullGetReadDataStream;
    BOOL mNullGetWriteDataStream;
public:
    //IActiveScriptDataCache
    HRESULT STDMETHODCALLTYPE GetReadDataStream(__out IStream **ppStream);
    HRESULT STDMETHODCALLTYPE GetWriteDataStream(__out IStream **ppStream);
    HRESULT STDMETHODCALLTYPE SaveWriteDataStream(__in IStream *pStream);

    //IActiveScriptContext
    HRESULT STDMETHODCALLTYPE GetUrl(__out BSTR *pbstrUrl);
    HRESULT STDMETHODCALLTYPE GetLineColumn(__out ULONG *pulLine, __out ULONG *pulColumn);
    HRESULT STDMETHODCALLTYPE GetOffset(__out ULONG *pulOffset);
    HRESULT STDMETHODCALLTYPE IsDynamicDocument(__out BOOL *pfDynamicDocument);
    HRESULT STDMETHODCALLTYPE GetSourceMapUrl(__out BSTR *pbstrSourceMapUrl);

    //IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    //Getter
    UINT GetWriteDataStreamCounter();
    UINT SaveWriteDataStreamCounter();

    //Setter
    void SetFailGetWriteDataStream(BOOL fail);   //Flag to tell fake mshtml to fail the GetWriteDataStream call
    void SetFailSaveWriteDataStream(BOOL fail);  //Flag to tell fake mshtml to fail the SaveWriteDataStream call
    void SetNullGetWriteDataStream(BOOL fail);   //Flag to tell fake mshtml to return NULL for GetWriteDataStream call
    void SetNullGetReadDataStream(BOOL fail);    //Flag to tell fake mshtml to return NULL for GetReadDataStream call

    void Init()
    {
        mRefCount = 0;
        mGetWriteDataStreamCounter = 0;
        mSaveWriteDataStreamCounter = 0;
        mFailGetWriteDataStream = FALSE;
        mFailSaveWriteDataStream = FALSE;
        mNullGetReadDataStream = FALSE;
        mNullGetWriteDataStream = FALSE;
    }
    //ctor
    FakeMSHTML()
    {
        Init();
    }
    FakeMSHTML(__in LPWSTR inputCacheFile, __in LPWSTR outputCacheFile)
    {
        Init();
        mInputCacheFile = inputCacheFile;
        mOutputCacheFile = outputCacheFile;
    }
    FakeMSHTML(__in LPWSTR inputCacheFile, __in LPWSTR outputCacheFile, BOOL failGetWriteDataStream, BOOL failSaveWriteDataStream)
    {
        Init();
        mInputCacheFile = inputCacheFile;
        mOutputCacheFile = outputCacheFile;
        mFailGetWriteDataStream = failGetWriteDataStream;
        mFailSaveWriteDataStream = failSaveWriteDataStream;
    }
};

