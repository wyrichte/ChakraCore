//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include <ImmutableList.h>
#include <Shlwapi.h>
#include <atlbase.h>
#include <XmlLite.h>

using namespace regex;

namespace XmlReader
{
    // A class representing a parameter element in the xml documentation
    class Param
    {
    public:
        LPCWSTR name = nullptr;
        LPCWSTR description = nullptr;

        static Param* ReadParam(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc);
    };

    // A class representing a member element in the xml documentation
    class Member
    {
    public:
        LPCWSTR summary = nullptr;
        LPCWSTR returns = nullptr;
        LPCWSTR deprecated = nullptr;
        LPCWSTR name = nullptr;
        ImmutableList<Param*>* params = nullptr;

        void AddParameter(_In_ Param* addedParam, _Inout_ ArenaAllocator* alloc)
        {
            params = params->Prepend(addedParam, alloc);
        }

        static Member* ReadMember(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc);
    };

    class MemberComparer : virtual public Comparer < Member* >
    {
    public:
        bool Equals(Member* item1, Member* item2)
        {
            return wcscmp(item1->name, item2->name) == 0;
        }
        int GetHashCode(Member* item)
        {
            return 0;
        }
        int Compare(Member* item1, Member* item2)
        {
            return wcscmp(item1->name, item2->name);
        }
    };

    // A Heap to store the members in the xml document for fast retrieval
    class Heap
    {
    public:

        ImmutableList<Member*>* values = nullptr;
        Member** valuesArr = nullptr;
        size_t size;
        bool isInitialized = false;

        void AddValue(_In_ Member* value, _Inout_ ArenaAllocator* alloc)
        {
            values = values->Prepend(value, alloc);
            isInitialized = false;
        }

        void Initialize(_Inout_ ArenaAllocator* alloc)
        {
            size = values->Count();
            valuesArr = AnewArray(alloc, Member*, size);
            int index = 0;
            values->Iterate(
                [&](_In_ Member* documentation){
                valuesArr[index++] = documentation;
            });

            QuickSort<Member*, true>::Sort(valuesArr, valuesArr + size - 1, &MemberComparer());
            isInitialized = true;
        }

        Member* GetValue(_In_ LPCWSTR key, _Inout_ ArenaAllocator* alloc)
        {
            if (!isInitialized)
            {
                Initialize(alloc);
            }

            int start = 0, end = (int)size - 1, result = -1;
            while (start <= end)
            {
                int middle = (start + end) >> 1;
                if (wcscmp(valuesArr[middle]->name, key) <= 0)
                {
                    result = middle;
                    start = middle + 1;
                }
                else
                {
                    end = middle - 1;
                }
            }

            if (result == -1 || wcscmp(valuesArr[result]->name, key) != 0)
            {
                return nullptr;
            }

            return valuesArr[result];
        }
    };

    class XmlReader
    {
    public:
        Heap* heap = nullptr;
        ArenaAllocator* alloc = nullptr;

        void Initialize(_In_ ArenaAllocator* alloc)
        {
            this->alloc = alloc;
            this->heap = Anew(alloc, Heap);
        }
        void Initialize(_In_ LPCWSTR xmlFilePath, _In_ ArenaAllocator* alloc);
        void Initialize(_In_ ImmutableList<LPCWSTR>* xmlFilePaths, _In_ ArenaAllocator* alloc);
        Member* GetValue(_In_ LPCWSTR typePrefix,_In_ LPCWSTR key)
        {
            LPCWSTR fullKey = StringUtils::Concat(typePrefix, key, alloc);
            return this->heap->GetValue(fullKey, alloc);
        }
        static LPCWSTR GetElementName(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc);
        static LPCWSTR GetElementValue(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc);
        void AddXmlDataToHeap(_In_ LPCWSTR xmlFilePath);
        static XmlNodeType* Read(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc);
        static LPCWSTR GetLocalName(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc);
        static LPCWSTR GetValue(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc);
        static UINT GetDepth(_In_ IXmlReader* reader);
    };
}