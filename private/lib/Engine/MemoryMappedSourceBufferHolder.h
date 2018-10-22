//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include <streamspublished.h>

class ISourceHolder;
namespace Js
{
    class MemoryMappedSourceBufferHolder : public ISourceHolder
    {
    private:
        Streams::IByteChunk* memoryMappedBuffer;
        LONGLONG mappedSourceByteLength;
        bool shouldFreeBuffer;

    public:
        MemoryMappedSourceBufferHolder(Streams::IByteChunk* memoryMappedBuffer);
        bool Equals(ISourceHolder* other);
        hash_t GetHashCode();

        virtual bool IsEmpty() override;
        virtual LPCUTF8 GetSource(const char16* reasonString) override;
        virtual size_t GetByteLength(const char16* reasonString) override;
        virtual void Dispose(bool isShutdown) override;
        virtual void Unload() override;
        virtual bool IsDeferrable() override;

        virtual void Finalize(bool isShutdown) override
        {
        }
        virtual void Mark(Recycler * recycler) override
        {
        }
    };
}