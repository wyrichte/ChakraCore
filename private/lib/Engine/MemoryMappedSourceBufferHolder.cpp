//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    MemoryMappedSourceBufferHolder::MemoryMappedSourceBufferHolder(Streams::IByteChunk* memoryMappedBuffer) :
        memoryMappedBuffer(memoryMappedBuffer),
        shouldFreeBuffer(true),
        mappedSourceByteLength(memoryMappedBuffer->GetFilledSizeInBytes())
    {
    }

    bool MemoryMappedSourceBufferHolder::IsEmpty()
    {
        // REVIEW: What is the purpose of IsEmpty
        // Debugger tests appear to rely on this always returning false??
        // return mappedSourceByteLength == 0;
        return false;
    }

    // Following two methods are calls to EnsureSource before attempting to get the source
    // If EnsureSource returns false, these methods will return nullptr and 0 respectively.
    LPCUTF8 MemoryMappedSourceBufferHolder::GetSource(const char16* reasonString)
    {
        AssertMsg(memoryMappedBuffer != nullptr, "Mapped buffer is null, this means that this object has been finalized.");

        return memoryMappedBuffer->GetRawBuffer();
    }

    size_t MemoryMappedSourceBufferHolder::GetByteLength(const char16* reasonString)
    {
        AssertMsg(memoryMappedBuffer != nullptr, "Mapped buffer is null, this means that this object has been finalized.");

        // REVIEW
        return (DWORD)mappedSourceByteLength;
    }

    void MemoryMappedSourceBufferHolder::Dispose(bool isShutdown)
    {
        Unload();
    }

    void MemoryMappedSourceBufferHolder::Unload()
    {
        if (shouldFreeBuffer)
        {
            memoryMappedBuffer->Free();
            memoryMappedBuffer = nullptr;
            shouldFreeBuffer = false;
        }
    }

    bool MemoryMappedSourceBufferHolder::Equals(ISourceHolder* other)
    {
        return this == other ||
            (this->GetByteLength(_u("Equal Comparison")) == other->GetByteLength(_u("Equal Comparison"))
                && memcmp(this->GetSource(_u("Equal Comparison")), other->GetSource(_u("Equal Comparison")), this->GetByteLength(_u("Equal Comparison"))));
    }

    hash_t MemoryMappedSourceBufferHolder::GetHashCode()
    {
        LPCUTF8 source = GetSource(_u("Hash Code Calculation"));
        size_t byteLength = GetByteLength(_u("Hash Code Calculation"));
        Assert(byteLength < MAXUINT32);
        return JsUtil::CharacterBuffer<utf8char_t>::StaticGetHashCode(source, (charcount_t)byteLength);
    }

    bool MemoryMappedSourceBufferHolder::IsDeferrable()
    {
        return CONFIG_FLAG(DeferLoadingAvailableSource);
    }
}