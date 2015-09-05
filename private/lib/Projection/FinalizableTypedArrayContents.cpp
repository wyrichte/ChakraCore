//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "ProjectionPch.h"


namespace Projection
{
    using namespace ProjectionModel;

    void StringReleaser::ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *winrtStringLib)
    {
        FinalizableTypedArrayContents::FinalizeString(winrtStringLib, *((HSTRING *)elementPointer));
    }

    void UnknownReleaser::ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *)
    {
        FinalizableTypedArrayContents::FinalizeUnknown(*((IUnknown **)elementPointer));
    }

    void StructReleaser::Initalize(RtSTRUCTTYPE structType, size_t baseOffset) 
    {
        ImmutableList<RtABIFIELDPROPERTY> * properties = structType->fields;
        while (properties)
        {
            RtABIFIELDPROPERTY prop = properties->First();
            switch(prop->type->typeCode)
            {
            case tcInterfaceType:
            case tcClassType:
            case tcDelegateType:
                {
                    unknownOffsets.Push(baseOffset + prop->fieldOffset);
                }
                break;

            case tcBasicType:
                {
                    RtBASICTYPE basicType = BasicType::From(prop->type);
                    if (basicType->typeCor == ELEMENT_TYPE_OBJECT)
                    {
                        unknownOffsets.Push(baseOffset + prop->fieldOffset);
                    }
                    else if (basicType->typeCor == ELEMENT_TYPE_STRING)
                    {
                        hstringOffsets.Push(baseOffset + prop->fieldOffset);
                    }
                }
                break;

            case tcStructType:
                {
                    Initalize(StructType::From(prop->type), baseOffset + prop->fieldOffset);
                }
                break;
            }
            properties = properties->GetTail();
        }
    }

    void StructReleaser::ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *winrtStringLib)
    {
        while (!unknownOffsets.Empty())
        {
            FinalizableTypedArrayContents::FinalizeUnknown(*((IUnknown **)(elementPointer + unknownOffsets.Pop())));
        }

        while (!hstringOffsets.Empty())
        {
            FinalizableTypedArrayContents::FinalizeString(winrtStringLib, *((HSTRING *)(elementPointer + hstringOffsets.Pop())));
        }
    }

    void FinalizableTypedArrayContents::FinalizeString(Js::DelayLoadWinRtString *winrtStringLib, HSTRING hstring)
    {
        if (hstring)
        {
            winrtStringLib->WindowsDeleteString(hstring);
        }
    }

    void FinalizableTypedArrayContents::FinalizeUnknown(IUnknown *unknown)
    {
        if (unknown)
        {
            unknown->Release();
        }
    }

    FinalizableTypedArrayContents::FinalizableTypedArrayContents(Recycler *recycler, Js::DelayLoadWinRtString *winrtStringLib, RtCONCRETETYPE elementType, UINT32 numberOfElements, byte *typedArrayBuffer, ReleaseBufferType releaseBufferType
#if DBG_DUMP
        , ProjectionMemoryInformation* projectionMemoryInformation
#endif
        )
        : elementType(elementType), numberOfElements(numberOfElements), typedArrayBuffer(typedArrayBuffer), recycler(recycler), typeReleaser(nullptr), winrtStringLib(winrtStringLib), elementStorageSize(elementType->storageSize), releaseBufferType(releaseBufferType)
    {
#if DBG_DUMP
        this->projectionMemoryInformation = projectionMemoryInformation;
        this->projectionMemoryInformation->AddFinalizableArrayContents(this);
#endif
    }


    void FinalizableTypedArrayContents::Initialize()
    {
        if (typedArrayBuffer == nullptr || ConcreteType::IsBlittable(elementType) || numberOfElements < 1)
        {
            return;
        }

        switch(elementType->typeCode)
        {
        case tcInterfaceType:
        case tcClassType:
        case tcDelegateType:
            {
                typeReleaser = new UnknownReleaser;
            }
            break;

        case tcBasicType:
            {
                RtBASICTYPE basicType = BasicType::From(elementType);
                if (basicType->typeCor == ELEMENT_TYPE_OBJECT)
                {
                    typeReleaser = new UnknownReleaser;
                }
                else if (basicType->typeCor == ELEMENT_TYPE_STRING)
                {
                    typeReleaser = new StringReleaser;
                }
            }
            break;

        case tcStructType:
            {
                typeReleaser = new StructReleaser;
                static_cast<StructReleaser *>(typeReleaser)->Initalize(StructType::From(elementType));
            }
            break;

        default:
            {
                Js::Throw::FatalProjectionError();
            }
        }
    }

    void FinalizableTypedArrayContents::Dispose(bool isShutdown)
    {
#if DBG_DUMP
        this->projectionMemoryInformation->DisposeFinalizableArrayContents(this);
#endif

        if (typedArrayBuffer != nullptr  && !isShutdown)
        { 
            if (typeReleaser != nullptr)
            {
                Assert(numberOfElements > 0);
                for (uint32 index = 0; index < numberOfElements; index++)
                {
                    typeReleaser->ReleaseItem(typedArrayBuffer + index * elementStorageSize, winrtStringLib);
                }
            }

            if (releaseBufferType == releaseBufferUsingDeleteArray)
            {
                delete [] typedArrayBuffer;
            }
            else
            {
                Assert(releaseBufferType == releaseBufferUsingCoTaskMemFree);
                CoTaskMemFree(typedArrayBuffer);
            }
        }

        typedArrayBuffer = nullptr;
        numberOfElements = 0;

        if (typeReleaser != nullptr)
        {
            delete typeReleaser;
            typeReleaser = nullptr;
        }
    }

    UINT FinalizableTypedArrayContents::GetHeapObjectOptionalIndexPropertiesSize()
    {
        // We can get the size information only on these types: 
        // RuntimeClass, Interface, Delegate or Object(Inspectable)
        if ((ClassType::Is(elementType) || InterfaceType::Is(elementType) || DelegateType::Is(elementType) || ((BasicType::Is(elementType)) && (BasicType::From(elementType)->typeCor == ELEMENT_TYPE_OBJECT))))
        {
            return ActiveScriptProfilerHeapEnum::GetHeapObjectIndexPropertiesInfoSize(numberOfElements);
        }

        return 0;
    }

    // Info:        Fills the relationship information for the index properties of this typed array buffer
    // Parameters:  heapEnum - The heap enumerator to use for tracking heap objects
    //              optionalInfo - optionalInfo to fill in
    //              scriptContext - if non null, we need to exit script context to query interface
    void FinalizableTypedArrayContents::FillHeapObjectOptionalIndexProperties(
        ActiveScriptProfilerHeapEnum* heapEnum,
        ProfilerHeapObjectOptionalInfo *optionalInfo,
        Js::ScriptContext *scriptContext)
    {
        // We can get the size information only on these types: 
        // RuntimeClass, Interface, Delegate or Object(Inspectable)
        Assert((ClassType::Is(elementType) || InterfaceType::Is(elementType) || DelegateType::Is(elementType) || ((BasicType::Is(elementType)) && (BasicType::From(elementType)->typeCor == ELEMENT_TYPE_OBJECT))));
        Assert(elementType->storageSize == sizeof(IUnknown *));

        optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES;
        optionalInfo->indexPropertyList.count = numberOfElements;

        HRESULT hr = S_OK;
        IUnknown **typedBuffer = (IUnknown **)typedArrayBuffer;
        for (UINT32 i=0; i < numberOfElements; i++)
        {
            IUnknown *unknown = nullptr;
            if (typedBuffer[i] != nullptr)
            {
                if (scriptContext)
                {
                    {
                        hr = typedBuffer[i]->QueryInterface(&unknown);
                        if (SUCCEEDED(hr))
                        {
                            unknown->Release();
                        }
                    }
                }
                else
                {
                    hr = typedBuffer[i]->QueryInterface(&unknown);
                    if (SUCCEEDED(hr))
                    {
                        unknown->Release();
                    }
                }
            }

            // Populate relationshiplist
            optionalInfo->indexPropertyList.elements[i].relationshipId = i;
            heapEnum->SetRelationshipInfo(optionalInfo->indexPropertyList.elements[i], PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT);
            optionalInfo->indexPropertyList.elements[i].relationshipInfo = PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT;
            optionalInfo->indexPropertyList.elements[i].externalObjectAddress = unknown;
        }
    }
}
