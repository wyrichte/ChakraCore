//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// Forward declaration to avoid including robuffer.h/robytestream.h
namespace Windows { namespace Storage { namespace Streams { struct IBuffer; struct IBufferByteAccess; } } }

namespace Js
{
    class ArrayBufferFromIBuffer : public ArrayBuffer
    {
        // Due to the design of IBuffer we need to keep its pointer around despite only really needing IBufferByteAccess.
        Windows::Storage::Streams::IBuffer * m_iBuf;
        Windows::Storage::Streams::IBufferByteAccess * m_iBufByteAcccess;
    protected:
        DEFINE_VTABLE_CTOR(ArrayBufferFromIBuffer, ArrayBuffer);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ArrayBufferFromIBuffer);

        virtual ArrayBufferDetachedStateBase* CreateDetachedState(BYTE* buffer, uint32 bufferLength) override { Assert(UNREACHED); Throw::InternalError(); };

    public:
        static ArrayBuffer* Create(Windows::Storage::Streams::IBuffer * iBuf,
                                   Windows::Storage::Streams::IBufferByteAccess * iBufByteAccess,
                                   BYTE * buffer,
                                   UINT32 capacity,
                                   JavascriptLibrary *library);
        virtual void Dispose(bool isShutdown) override;
        virtual void Finalize(bool isShutdown) override;

    private:
         ArrayBufferFromIBuffer::ArrayBufferFromIBuffer(Windows::Storage::Streams::IBuffer * iBuf,
                                                   Windows::Storage::Streams::IBufferByteAccess * iBufByteAccess,
                                                   byte *buffer,
                                                   UINT32 length,
                                                   DynamicType *type);
    };
}

// Attempts to create an ArrayBuffer instance based on an existing IBuffer ProjectionObjectInstance.
HRESULT AttemptCreateArrayBufferFromIBuffer(__in Projection::ProjectionObjectInstance *prObj,
                                            __out Js::ArrayBuffer **ppArrayBuffer);
