//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "XmlReader.h"

// Reads and returns a parameter element from the xml documentation file
XmlReader::Param* XmlReader::Param::ReadParam(IXmlReader* reader, _Inout_ ArenaAllocator* alloc)
{
    Param* resultingParam = Anew(alloc, Param);
    resultingParam->name = XmlReader::GetElementName(reader, alloc);
    resultingParam->description = XmlReader::GetElementValue(reader, alloc);

    return resultingParam;
}

// Reads and returns a member element from the xml documentation file
XmlReader::Member* XmlReader::Member::ReadMember(IXmlReader* reader, _Inout_ ArenaAllocator* alloc)
{
    Member* resultingMember = Anew(alloc, Member);
    UINT initialDepth = XmlReader::GetDepth(reader);

    while (true)
    {
        XmlNodeType* nodeType = XmlReader::Read(reader, alloc);
        if (!nodeType)
        {
            break;
        }
        UINT currentDepth = XmlReader::GetDepth(reader);

        if (currentDepth < initialDepth)
        {
            break;
        }

        LPCWSTR localName = XmlReader::GetLocalName(reader, alloc);

        if (*nodeType == XmlNodeType::XmlNodeType_Element)
        {
            if (wcscmp(localName, L"summary") == 0)
            {
                resultingMember->summary = XmlReader::GetElementValue(reader, alloc);
            }
            else if (wcscmp(localName, L"returns") == 0)
            {
                resultingMember->returns = XmlReader::GetElementValue(reader, alloc);
            }
            else if (wcscmp(localName, L"deprecated") == 0)
            {
                resultingMember->deprecated = XmlReader::GetElementValue(reader, alloc);
            }
            else if (wcscmp(localName, L"param") == 0)
            {
                resultingMember->AddParameter(Param::ReadParam(reader, alloc), alloc);
            }
        }
        if (*nodeType == XmlNodeType::XmlNodeType_EndElement && wcscmp(localName, L"member") == 0)
        {
            break;
        }
    }

    return resultingMember;
}

// returns the name of the element the reader is stopped at in the xml documentation file
LPCWSTR XmlReader::XmlReader::GetElementName(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc)
{
    LPCWSTR elementName = nullptr;
    reader->MoveToAttributeByName(L"name", nullptr);
    elementName = GetValue(reader, alloc);

    return elementName;
}

// returns the value of the element the reader is stopped at in the xml documentation file
LPCWSTR XmlReader::XmlReader::GetElementValue(_In_ IXmlReader* reader, _Inout_ ArenaAllocator* alloc)
{
    XmlNodeType* nodeType;
    while (*(nodeType = Read(reader, alloc)) != XmlNodeType::XmlNodeType_Text) {}
    LPCWSTR value = GetValue(reader, alloc);

    return value;
}

// reads a given xml documentation file and populations the heap with the data read from it
void XmlReader::XmlReader::AddXmlDataToHeap(_In_ LPCWSTR xmlFilePath)
{
    IXmlReader* reader;
    CComPtr<IStream> fileStream;
    HRESULT hr = ::SHCreateStreamOnFileEx(xmlFilePath, STGM_READ | STGM_SHARE_DENY_WRITE, 0, FALSE, nullptr, &fileStream);

    if (hr != S_OK)
    {
        wprintf(L"<ignored-missing-xmlfile filename='%s'/>\n", xmlFilePath);
        return;
    }

    ::CreateXmlReader(__uuidof(IXmlReader), (void**)&reader, nullptr);
    reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
    reader->SetInput(fileStream);

    while (true)
    {
        XmlNodeType* nodeType = XmlReader::Read(reader, alloc);
        if (!nodeType)
        {
            break;
        }
        LPCWSTR localName = GetLocalName(reader, alloc);
        if (*nodeType == XmlNodeType::XmlNodeType_Element && wcscmp(localName, L"member") == 0)
        {
            LPCWSTR memberName = XmlReader::GetElementName(reader, alloc);
            Member* member = Member::ReadMember(reader, alloc);
            member->name = memberName;
            heap->AddValue(member, alloc);
        }
    }
}

void XmlReader::XmlReader::Initialize(_In_ LPCWSTR xmlFilePath, _In_ ArenaAllocator* alloc)
{
    this->alloc = alloc;
    this->heap = Anew(alloc, Heap);
    this->AddXmlDataToHeap(xmlFilePath);
}

void XmlReader::XmlReader::Initialize(_In_ ImmutableList<LPCWSTR>* xmlFilePaths, _In_ ArenaAllocator* alloc)
{
    this->alloc = alloc;
    this->heap = Anew(alloc, Heap);

    xmlFilePaths->Iterate(
        [&](_In_ LPCWSTR xmlFilePath)
        {
            AddXmlDataToHeap(xmlFilePath);
        });
}

XmlNodeType* XmlReader::XmlReader::Read(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc)
{
    XmlNodeType tmpRef;
    HRESULT hr = reader->Read(&tmpRef);

    if (hr != S_OK)
    {
        return nullptr;
    }

    XmlNodeType* output = Anew(alloc, XmlNodeType);
    *output = tmpRef;
    return output;
}

LPCWSTR XmlReader::XmlReader::GetValue(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc)
{
    LPCWSTR tmpRef;
    reader->GetValue(&tmpRef, nullptr);
    return StringUtils::GetCopy(tmpRef, alloc);
}

LPCWSTR XmlReader::XmlReader::GetLocalName(_In_ IXmlReader* reader, _In_ ArenaAllocator* alloc)
{
    LPCWSTR tmpRef;
    reader->GetLocalName(&tmpRef, nullptr);
    return StringUtils::GetCopy(tmpRef, alloc);
}

UINT XmlReader::XmlReader::GetDepth(_In_ IXmlReader* reader)
{
    UINT res;
    reader->GetDepth(&res);
    return res;
}