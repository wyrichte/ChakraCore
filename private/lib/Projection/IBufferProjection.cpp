//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "ProjectionPch.h"

#include <robytestream.h>
#include <robuffer.h>
#include "IBufferProjection.h"

namespace Js
{
     ArrayBufferFromIBuffer::ArrayBufferFromIBuffer(Windows::Storage::Streams::IBuffer * iBuf,
                                                    Windows::Storage::Streams::IBufferByteAccess * iBufByteAccess,
                                                    BYTE *buffer,
                                                    UINT32 length,
                                                    DynamicType *type)
                                                    : m_iBuf(nullptr),
                                                      m_iBufByteAcccess(nullptr),
                                                      ArrayBuffer(buffer, length, type)
    {
        BEGIN_LEAVE_SCRIPT(GetScriptContext())
        {
            m_iBufByteAcccess = iBufByteAccess;
            m_iBufByteAcccess->AddRef();
            m_iBuf = iBuf;
            m_iBuf->AddRef();
        }
        END_LEAVE_SCRIPT(GetScriptContext())
    }


    ArrayBuffer* ArrayBufferFromIBuffer::Create(Windows::Storage::Streams::IBuffer * iBuf,
                                                Windows::Storage::Streams::IBufferByteAccess * iBufByteAccess,
                                                BYTE * buffer,
                                                UINT32 capacity,
                                                JavascriptLibrary *library)
    {
        Assert(iBuf != nullptr && iBufByteAccess != nullptr && buffer != nullptr && library != nullptr);

        // Only the ArrayBufferFromIBuffer class itself needs to know about the specialization.
        // We should only pass around an ArrayBuffer.
        OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("Creating ArrayBuffer using IBuffer projection with capacity/length/byteLength %u bytes\n"), capacity);
        return static_cast<ArrayBuffer*>(RecyclerNewFinalized(library->GetRecycler(),
                                                              ArrayBufferFromIBuffer,
                                                              iBuf,
                                                              iBufByteAccess,
                                                              buffer,
                                                              capacity,
                                                              library->GetArrayBufferType()));
    }

    void ArrayBufferFromIBuffer::Finalize(bool isShutdown)
    {
        // In debugger scenario, ScriptAuthor can create scriptContxt and delete scriptContext
        // explicitly. So for the builtin, while javascriptLibrary is still alive fine, the
        // matching scriptContext might have been deleted and the javascriptLibrary->scriptContext
        // field reset (but javascriptLibrary is still alive).
        // Use the recycler field off library instead of scriptcontext to avoid av.
        Recycler* recycler = GetType()->GetLibrary()->GetRecycler();
        recycler->ReportExternalMemoryFree(bufferLength);
        OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("ArrayBufferFromIBuffer finalize call freed %u external bytes\n"), bufferLength);
    }

    void ArrayBufferFromIBuffer::Dispose(bool isShutdown)
    {
        if (m_iBufByteAcccess != nullptr)
        {
            m_iBufByteAcccess->Release();
            m_iBufByteAcccess = nullptr;
        }
        if (m_iBuf != nullptr)
        {
            m_iBuf->Release();
            m_iBuf = nullptr;
        }
    }
} // namespace Js

HRESULT AttemptCreateArrayBufferFromIBuffer(__in Projection::ProjectionObjectInstance *prObj,
                                            __out Js::ArrayBuffer **ppArrayBuffer)
{
    Assert(prObj != nullptr && ppArrayBuffer != nullptr);
    HRESULT hr = S_OK;
    CComPtr<Windows::Storage::Streams::IBuffer> iBuf = nullptr;
    CComPtr<Windows::Storage::Streams::IBufferByteAccess> iBufByteAccess = nullptr;
    Js::JavascriptLibrary *library = prObj->GetLibrary();
    Js::ScriptContext *scriptContext = prObj->GetScriptContext();
    UINT32 capacity = 0;
    BYTE* buffer = nullptr;

    // While the IBuffer instance may be valid, the ScriptSite may have already been closed.
    Assert(scriptContext != nullptr);
    IfFailGo(scriptContext->IsClosed() ? E_FAIL : S_OK);
    Assert(library != nullptr);

    IUnknown * unknown = prObj->GetUnknown();
    // If we have released the WinRT object (e.g. msReleaseWinRTObject) then we could get a nullptr here.
    IfFailGo(unknown == nullptr ? E_FAIL : S_OK);

    // Check for IBuffer.
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        do // We want to avoid using goto Error while out of script context, so we use break for readibility.
        {
            hr = unknown->QueryInterface(Windows::Storage::Streams::IID_IBuffer, (void **)&iBuf);
            // Not finding an IBuffer is not a failure; we use S_FALSE to notify the caller.
            if (FAILED(hr))
            {
                hr = S_FALSE;
                // Not really an error, but we still need to do cleanup as if it was.
                break;
            }

            // Since we are doing byte access and the user is explicitly passing size, we assume that all bytes are used.
            hr = iBuf->get_Capacity(&capacity);
            if (FAILED(hr)) break;

            hr = iBuf->put_Length(capacity);
            if (FAILED(hr)) break;

            // We need to get IBufferByteAccess to get direct access to the buffer.
            hr = iBuf->QueryInterface(__uuidof(Windows::Storage::Streams::IBufferByteAccess), reinterpret_cast<void**>(&iBufByteAccess));
            if (FAILED(hr)) break;

            hr = iBufByteAccess->Buffer(&buffer);
        }
        while (0); // Any breaks will allow END_LEAVE_SCRIPT to do its job before using IfFailGo as normal.
    }
    END_LEAVE_SCRIPT(scriptContext)
    if (hr != S_OK) 
        goto Error;

    // Notify Recycler of IBuffer's memory
    Recycler *recycler = library->GetRecycler();
    recycler->AddExternalMemoryUsage(capacity);

    // With a valid IBuffer, we can now attempt to create the ArrayBufferFromIBuffer.
    *ppArrayBuffer = Js::ArrayBufferFromIBuffer::Create(iBuf, iBufByteAccess, buffer, capacity, library);
    Assert(*ppArrayBuffer != nullptr);

Error:
    if (hr != S_OK)
    {
        *ppArrayBuffer = nullptr;
    }

    return hr;
}
