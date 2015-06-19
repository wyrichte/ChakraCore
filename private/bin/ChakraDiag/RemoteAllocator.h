//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once
namespace JsDiag 
{
    class RemoteAllocator
    {
    private:
        struct AllocationChunk
        {
        private:
            AllocationChunk* nextChunk;
            BYTE* m_baseAddress;
            BYTE* m_currentAddress;
            size_t m_allocatedSize;
        public:
            static const size_t DefaultSize = 64 * 1024;
            static const size_t Alignment = 16;

            AllocationChunk(BYTE* baseAddress, size_t allocatedSize, AllocationChunk* next) : 
                nextChunk(next),
                m_baseAddress(baseAddress),
                m_currentAddress(baseAddress),
                m_allocatedSize(allocatedSize)
            {
            }

            size_t GetUsedBytes()
            {
                return (m_currentAddress - m_baseAddress);
            }

            size_t GetFreeBytes()
            {
                return m_allocatedSize - GetUsedBytes();
            }

            void* Allocate(size_t size);

            BYTE* GetBaseAddress()
            {
                return m_baseAddress;
            }

            AllocationChunk* GetNext()
            {
                return nextChunk;
            }
        };
        IVirtualReader* m_reader;
        AllocationChunk* m_headChunk;
    public:
        RemoteAllocator(IVirtualReader* reader) :
            m_reader(reader),
            m_headChunk(NULL)
        {
        }

        ~RemoteAllocator()
        {
            // TODO: Figure out when the memory gets released
            //this->ReleaseAll();
        }

        template<class T>
        T* Allocate()
        {
            return (T*)Allocate(sizeof(T));
        }

        void* Allocate(size_t size);
        void* SnailAllocate(size_t size);
        void ReleaseAll();
    };
}