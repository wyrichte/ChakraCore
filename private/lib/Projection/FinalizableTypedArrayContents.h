//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Projection
{
    enum ProjectionTypeReleaserType
    {
        eStringReleaser,
        eUnknownReleaser,
        eStructReleaser
    };

    struct ProjectionTypeReleaser
    {
        ProjectionTypeReleaserType releaserType;

        ProjectionTypeReleaser(ProjectionTypeReleaserType releaserType) : releaserType(releaserType) 
        {
        }

        virtual ~ProjectionTypeReleaser() {};

        virtual void ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *winrtStringLib) = 0;
    };

    struct StringReleaser : ProjectionTypeReleaser
    {
        StringReleaser() : ProjectionTypeReleaser(eStringReleaser)
        {
        }

        ~StringReleaser()
        {
        }

        void ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *winrtStringLib);
    };

    struct UnknownReleaser : ProjectionTypeReleaser
    {
        UnknownReleaser() : ProjectionTypeReleaser(eUnknownReleaser)
        {
        }

        ~UnknownReleaser()
        {
        }

        void ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *);
    };

    struct StructReleaser : ProjectionTypeReleaser
    {
        SList<size_t, HeapAllocator> unknownOffsets;
        SList<size_t, HeapAllocator> hstringOffsets;

        StructReleaser() : ProjectionTypeReleaser(eStructReleaser), unknownOffsets(&HeapAllocator::Instance), hstringOffsets(&HeapAllocator::Instance)
        {
        }

        ~StructReleaser()
        {
        }

        void Initalize(RtSTRUCTTYPE structType, size_t baseOffset = 0);
        void ReleaseItem(byte *elementPointer, Js::DelayLoadWinRtString *);
    };

    enum ReleaseBufferType
    {
        releaseBufferUsingDeleteArray,
        releaseBufferUsingCoTaskMemFree
    };

    class FinalizableTypedArrayContents : public FinalizableObject
    {
        friend struct StructReleaser;
        friend class ProjectionMemoryInformation;

    public:
        FinalizableTypedArrayContents(Recycler *recycler, Js::DelayLoadWinRtString *winrtStringLib, RtCONCRETETYPE elementType, UINT32 numberOfElements, byte *typedArrayBuffer, ReleaseBufferType releaseBufferType
#if DBG_DUMP
            , ProjectionMemoryInformation* projectionMemoryInformation
#endif
            ); 

        void Initialize();

        RtCONCRETETYPE elementType;
        UINT32 numberOfElements;
        byte *typedArrayBuffer;

        static void FinalizeString(Js::DelayLoadWinRtString *winrtStringLib, HSTRING hstring);
        static void FinalizeUnknown(IUnknown *unknown);

        virtual void Finalize(bool isShutdown) { }
        // Call after sweeping is done.  
        // Can call other script or cause another collection.
        virtual void Dispose(bool isShutdown) override;
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

        UINT GetHeapObjectOptionalIndexPropertiesSize();
        void FillHeapObjectOptionalIndexProperties(
            ActiveScriptProfilerHeapEnum* heapEnum,
            ProfilerHeapObjectOptionalInfo *optionalInfo,
            Js::ScriptContext *scriptContext = nullptr);

    private:
        Recycler *recycler;
        Js::DelayLoadWinRtString *winrtStringLib;

        // Store this because when we are going to free the elements we dont cant use the elementType when script context is closing as the types are in arena and they are gone before all recycler items are released
        size_t elementStorageSize; 

        ProjectionTypeReleaser *typeReleaser;
        ReleaseBufferType releaseBufferType;

#if DBG_DUMP
        ProjectionMemoryInformation* projectionMemoryInformation;
#endif
    };
}
