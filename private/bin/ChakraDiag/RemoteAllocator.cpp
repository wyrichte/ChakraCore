//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#include "stdafx.h"
namespace JsDiag 
{
    void* RemoteAllocator::AllocationChunk::Allocate(size_t size)
    {
        Assert(GetFreeBytes() >= size);
        void* allocation = m_currentAddress;
        m_currentAddress += size;
        return allocation;
    }

    void* RemoteAllocator::Allocate(size_t size)
    {
        Assert(size != 0);

        size_t alignedSize = AllocSizeMath::Align(size, AllocationChunk::Alignment);
        if(m_headChunk && m_headChunk->GetFreeBytes() >= alignedSize)
        {
            return m_headChunk->Allocate(size);
        }
        return SnailAllocate(alignedSize);
    }

    void* RemoteAllocator::SnailAllocate(size_t size)
    {
        Assert(m_headChunk == nullptr || m_headChunk->GetFreeBytes() <= size);

        size_t chunkSize = max(size, AllocationChunk::DefaultSize);
        //TODO: align allocated Address to Virtual memory allocation granularity
        ULONG64 allocatedAddress;
        HRESULT hr = m_reader->AllocateVirtualMemory(NULL, chunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, &allocatedAddress);
        CheckHR(hr, DiagErrorCode::ALLOCATE_VIRTUAL);
        AllocationChunk* chunk = new(nothrow) AllocationChunk((BYTE*)allocatedAddress, chunkSize, m_headChunk);
        if(chunk == nullptr)
        {
            m_reader->FreeVirtualMemory(allocatedAddress, /*size=*/ 0, MEM_RELEASE);
            DiagException::ThrowOOM();
        }
        m_headChunk = chunk;
        return m_headChunk->Allocate(size);
    }

    void RemoteAllocator::ReleaseAll()
    {
        AllocationChunk* chunk = m_headChunk;
        while(chunk != nullptr)
        {
            AllocationChunk* currentChunk = chunk;
            HRESULT hr = m_reader->FreeVirtualMemory((UINT64)currentChunk->GetBaseAddress(), /*size=*/ 0, MEM_RELEASE);
            Assert(SUCCEEDED(hr));
            chunk = currentChunk->GetNext();
            delete currentChunk;
        }
        m_headChunk = nullptr;
    }
}