//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Projection
{
    class ProjectionObjectInstance;
    class ArrayProjection;
    class UnknownsReleaser;
    class FinalizableTypedArrayContents;
    using namespace ProjectionModel;

    enum ResourceCleanup
    {
        CalleeTransfersOwnership,
        CalleeRetainsOwnership
    };

    enum ThisType
    {
        thisUnknown,
        thisUnknownEventHandling,
        thisPrototype,
        thisNamespace,
        thisRuntimeClass,
        thisDelegate
    };

    struct ThisInfo : public FinalizableObject
    {
        ThisType thisType;
        RtSPECIALIZATION specialization;
        RtIID defaultInterface;
        MetadataStringId typeId;

        bool CanHoldEventCookies()
        {
            return thisType==thisRuntimeClass || thisType==thisUnknownEventHandling;
        }

        bool IsArray();

        RtSPECIALIZATION GetSpecialization()
        {
            return specialization;
        }

        RtIID GetDefaultInterface()
        {
            return defaultInterface;
        }      

        MetadataStringId GetTypeId()
        {
            return typeId;
        }

        void Finalize(bool isShutdown) override {}
        void Dispose(bool isShutdown) override {}
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

    protected:
        ThisInfo(ThisType thisType, MetadataStringId typeId, RtIID defaultInterface, RtSPECIALIZATION specialization)
            : thisType(thisType), typeId(typeId), defaultInterface(defaultInterface), specialization(specialization)
        { }
    };

    // IUnknown is held in var extension
    struct UnknownThis : ThisInfo
    {
        UnknownThis(MetadataStringId typeId, RtIID defaultInterface, RtSPECIALIZATION specialization) 
            : ThisInfo(thisUnknown, typeId, defaultInterface, specialization)
        { 
        }
    };

    // IUnknown is held in var extension
    struct UnknownEventHandlingThis : ThisInfo
    {
        UnknownEventHandlingThis(MetadataStringId typeId, RtIID defaultInterface, RtSPECIALIZATION specialization) 
            : ThisInfo(thisUnknownEventHandling, typeId, defaultInterface, specialization)
        { 
        }
    };

    // 'this' is a prototype instance
    struct PrototypeThis : ThisInfo
    {
        PrototypeThis() 
            : ThisInfo(thisPrototype, MetadataStringIdNil, nullptr, nullptr)
        { }
    };

    // 'this' is a namespace
    struct NamespaceThis : ThisInfo
    {
        NamespaceThis() 
            : ThisInfo(thisNamespace, MetadataStringIdNil, nullptr, nullptr)
        { }
    };

    // 'this' is a delegate
    struct DelegateThis : ThisInfo
    {
        IUnknown * _this; 
        RtDELEGATECONSTRUCTOR constructor;

        DelegateThis(RtDELEGATECONSTRUCTOR constructor, IUnknown * _this) 
            : ThisInfo(thisDelegate, MetadataStringIdNil, nullptr, nullptr), constructor(constructor), _this(_this)
        {
            Assert(_this);
            if (_this != nullptr)
            {
                _this->AddRef();
            }
        }
        ~DelegateThis()
        {   }

        void Dispose(bool isShutdown) override
        {
            if (_this  && !isShutdown)
            {
                _this->Release();
                _this = nullptr;
            }
        }
    };

    struct RuntimeClassThis : ThisInfo
    {
#if DBG
        LPCWSTR fullClassName; 
#endif
        IActivationFactory * factory;  
        EventProjectionHandler eventProjectionHandler;

        EventProjectionHandler * GetEventProjectionHandler() { return &eventProjectionHandler; }
        uint GetEventAndEventHandlerCount();
        void PopulateProfilerEventInfo(ActiveScriptProfilerHeapEnum* heapEnum, PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList);

#if DBG
        RuntimeClassThis(LPCWSTR fullClassName, MetadataStringId typeId)
            : fullClassName(fullClassName),
#else
        RuntimeClassThis(MetadataStringId typeId) :
#endif
            ThisInfo(thisRuntimeClass, typeId, nullptr, nullptr), factory(nullptr)
        { 
        }

        void Dispose(bool isShutdown) override
        {
            if (factory && !isShutdown)
            {
                factory->Release();
                factory = nullptr;
            }
        }
    };

    template<typename TBaggage>
    struct ThunkSignature
    {
        ProjectionContext * projectionContext;
        TBaggage baggage;
        ThunkSignature(ProjectionContext * projectionContext, TBaggage baggage)
            : projectionContext(projectionContext), baggage(baggage)
        { } 
    };

    struct Signature
    {
        ProjectionContext * projectionContext;
        ThisInfo * thisInfo;
        bool boundsToUndefined;
        RtABIMETHODSIGNATURE method;
        Signature(ProjectionContext * projectionContext, ThisInfo * thisInfo, RtABIMETHODSIGNATURE method, bool boundsToUndefined = false)
            : projectionContext(projectionContext), thisInfo(thisInfo), method(method), boundsToUndefined(boundsToUndefined)
        { }
    };

    struct EventsSignature
    {
        ProjectionContext * projectionContext;
        ThisInfo * thisInfo;
        regex::ImmutableList<RtEVENT> * events;
        EventsSignature(ProjectionContext * projectionContext, ThisInfo * thisInfo, regex::ImmutableList<RtEVENT> * events)
            : projectionContext(projectionContext), thisInfo(thisInfo), events(events)
        { }
    };

    struct EventHandlerSignature
    {
        ProjectionContext * projectionContext;
        ThisInfo * thisInfo;
        RtEVENT abiEvent;
        MetadataStringId eventPropertyNameId;
        EventHandlerSignature(ProjectionContext * projectionContext, ThisInfo * thisInfo, RtEVENT abiEvent, MetadataStringId eventPropertyNameId)
            : projectionContext(projectionContext), thisInfo(thisInfo), abiEvent(abiEvent), eventPropertyNameId(eventPropertyNameId)
        { }
    };

    struct MemoryCleanupRecord
    {
        byte * mem;
        size_t memsize;
    };

    enum ConstructorArgumentType
    {
        ConstructorArgumentType_Promise     = 0
    };

    struct ConstructorArguments
    {
        ConstructorArgumentType type;

        ConstructorArguments(ConstructorArgumentType type) 
            : type(type) 
        { 
        }
    };

    struct PromiseConstructorArguments : ConstructorArguments
    {
        AsyncDebug::AsyncOperationId asyncOperationId;

        PromiseConstructorArguments(AsyncDebug::AsyncOperationId asyncOperationId)
            : ConstructorArguments(ConstructorArgumentType_Promise), asyncOperationId(asyncOperationId)
        {
        }
    };

    enum ParameterMarkerType
    {
        ParameterMarkerType_Async         = 0
    };

    struct ParameterMarker
    {
        ParameterMarkerType type;

        ParameterMarker(ParameterMarkerType type)
            : type(type)
        {
        }
    };

    struct AsyncParameterMarker : ParameterMarker
    {
        RtABIPARAMETER parameter;
        AsyncDebug::AsyncOperationId asyncOperationId;

        AsyncParameterMarker(RtABIPARAMETER parameter, AsyncDebug::AsyncOperationId asyncOperationId)
            : ParameterMarker(ParameterMarkerType_Async), parameter(parameter), asyncOperationId(asyncOperationId)
        {
        }
    };

    enum DeprecatedInvocationType 
    {
        DeprecatedInvocation_Class,
        DeprecatedInvocation_Method,
        DeprecatedInvocation_Event,
        DeprecatedInvocation_Delegate
    };

    PropertyId IdOfString(Js::ScriptContext *scriptContext, LPCWSTR propertyName);
    LPCWSTR StringOfId(Js::ScriptContext *scriptContext, PropertyId id);
    size_t GetApproximateSizeForGCPressure(INT32 gcPressure);
    void VerifyDeprecatedAttributeOnce(RtABIMETHODSIGNATURE rtmethod, Js::ScriptContext* scriptContext, DeprecatedInvocationType methodType);

    Var InvokeMethodByThisInfo(ThisInfo * thisInfo, RtABIMETHODSIGNATURE rtmethod, bool boundsToUndefined, Var _this, Js::Arguments args, ProjectionContext *projectionContext);
    ProjectionObjectInstance * GetProjectionObjectInstanceFromVar(Js::ScriptContext * scriptContext, Var instance);
    ProjectionObjectInstance * GetProjectionObjectInstanceFromVarNoThrow(Var instance);
    void GetUnknownOfVarExtension(Js::ScriptContext * scriptContext, Var instance, const IID & iid, void ** pp, bool * isDefaultInterface = nullptr, bool addRefDefault = false, MetadataStringId methodNameId = MetadataStringIdNil, MetadataStringId expectedTypeId = MetadataStringIdNil);
    HRESULT QueryInterfaceAfterLeaveScript(Js::ScriptContext *scriptContext, IUnknown *unknown, const IID & iid, void ** pp);
    Var DoInvoke(ThisInfo *thisInfo, RtABIMETHODSIGNATURE signature, bool boundsToUndefined, Var * args, ushort argCount, ProjectionContext *projectionContext);

    Var DynamicCall(Signature * signature, Var method, Js::Arguments & args, Js::CallInfo & callInfo);
    Var MethodSignatureThunk(Var method, Js::CallInfo callInfo, ...);


    class ProjectionMarshaler
    {
        friend class ScriptEngine;
        friend class ProjectionWriter;
        friend class ProjectionMethodInvoker;

        Js::TempArenaAllocatorObject * allocatorObject;
        ArenaAllocator * alloc;
        ProjectionContext * projectionContext;
        regex::ImmutableList<HSTRING> * hstrings;
        regex::ImmutableList<IUnknown*> * addrefs;
        regex::ImmutableList<Var> * recyclerVars;

        regex::ImmutableList<HSTRING *> *outHstrings;
        regex::ImmutableList<IUnknown **> *outUnknowns;

        regex::ImmutableList<HSTRING> *delegateOutHstrings;
        regex::ImmutableList<IUnknown *> *delegateOutUnknowns;
        regex::ImmutableList<MemoryCleanupRecord> *delegateOutMemory;
        regex::ImmutableList<FinalizableTypedArrayContents *> *delegateOutArrayContents;


        ResourceCleanup resourceCleanup;
        bool fReleaseExistingResource; // This flag when set means that we need to release the already present resource before writing new value over it.
        bool fReleaseOutResources;
        bool fReleaseDelegateOutResources;
        bool fInAsyncInterface;

#if DBG
        UnknownsReleaser *unknownsReleaser;
#endif
         
        byte * WriteInPointer(__in void * pointer, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInUnknown(__in IUnknown * unknown, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInBasicType(__in Var arg, __in RtBASICTYPE type, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInCorElementType(__in Var arg, __in CorElementType typeCor, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInGuid(Var varInput, bool fStructsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInDate(Var varInput, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInTimeSpan(Var varInput, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteStructConstructorTypeInParameter(Var varInput, RtSTRUCTCONSTRUCTOR constructor, bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteDelegateConstructorTypeInParameter(Var varInput, RtDELEGATECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInterfaceInParameter(Var varInput, const IID & iid, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteRuntimeInterfaceConstructor(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteRuntimeClassConstructor(Var varInput, RtRUNTIMECLASSCONSTRUCTOR rtc, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteFunctionTypeInParameter(Var varInput, RtFUNCTION function, __in_bcount(memSize) byte * mem, __in size_t memSize, bool structsByValue);
        byte * WriteExprTypeInParameter(Var varInput, RtEXPR expr, __in_bcount(memSize) byte * mem, __in size_t memSize, bool structsByValue);
        byte * WriteTypeNameTypeInParameter(Var varInput, MetadataStringId typeId, MetadataStringId typeNameId, regex::ImmutableList<RtTYPE> * genericParameters, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue);
        byte * WriteOutType(__in Js::Var inOutArg, RtTYPE type, __in_bcount(memSize) byte * mem, __in size_t memSize);
        byte * WriteInBoolean(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize);
        byte * WriteInString(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize);

        void SetReleaseOutResources() { fReleaseOutResources = true; }

        bool CanMarshalExpr(RtEXPR expr);
        bool CanMarshalType(RtTYPE type, bool allowMissingTypes = false, bool *outWasMissingType = nullptr);


        HRESULT GetTypedArrayAsPropertyValue(
            __in size_t arraySize, 
            __in_bcount(arraySize) byte * typedArrayBuffer, 
            __in CorElementType type, 
            __out IInspectable **propertyValue);
        HRESULT GetTypedArrayAsPropertyValue(
            __in size_t arraySize, 
            __in_bcount(arraySize) byte * typedArrayBuffer, 
            __in RtCONCRETETYPE type, 
            __out IInspectable **propertyValue);
        HRESULT GetBuiltinTypedArrayAsPropertyValue(
            __in Var varValue,
            __out IInspectable **propertyValue);

        HRESULT GetNonArrayTypeAsPropertyValue(
            __in size_t arraySize,
            __in_bcount(arraySize) byte * typeBuffer, 
            __in CorElementType type, 
            __out IInspectable **propertyValue);
        HRESULT GetNonArrayTypeAsPropertyValue(
            __in size_t arraySize,
            __in_bcount(arraySize) byte * typeBuffer, 
            __in RtCONCRETETYPE type, 
            __out IInspectable **propertyValue);

        byte * WriteInspectableObject(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize, bool boxInterface = false);
        byte * WriteIReference(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize);
        byte * WriteIReferenceArray(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize);

        Js::Var ReadEmptyRuntimeClassNameUnknown(IUnknown *unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadInspectableUnknown(__in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions);
        Js::Var ReadDelegateConstructorOutParameter(RtDELEGATECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, bool allowIdentity, bool allowExtensions);
        Js::Var ReadRuntimeClassConstructorOutParameter(RtRUNTIMECLASSCONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, bool allowIdentity, bool allowExtensions);
        Js::Var ReadRuntimeClassConstructorOutParameterFromUnknown(RtRUNTIMECLASSCONSTRUCTOR constructor, IUnknown * unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadStructConstructorOut(RtSTRUCTCONSTRUCTOR constructor, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowExtensions);

        Js::Var TryReadInspectable(__in IUnknown *unknown, __in IInspectable * inspectable, MetadataStringId methodNameId = MetadataStringIdNil, bool allowIdentity = true, bool allowExtensions = false, bool allowEmptyRuntimeClassName = true, ConstructorArguments* constructorArguments = nullptr);
        Js::Var TryReadInspectableUnknown(__in IUnknown * unknown, __in IInspectable *interfacePtr, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, bool allowEmptyRuntimeClassName, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadInterfaceConstructorOutParameterFromUnknown(RtRUNTIMEINTERFACECONSTRUCTOR constructor, IUnknown * unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadInterfaceConstructorOutParameter(bool definitelyNotRuntimeClass, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadTypeFunctionOut(bool definitelyNotRuntimeClass, RtFUNCTION function, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadExprTypeOut(bool definitelyNotRuntimeClass, RtEXPR expr, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var TryReadTypeNameOutParameter(bool definitelyNotRuntimeClass, MetadataStringId typeId, MetadataStringId typeNameId, regex::ImmutableList<RtTYPE> * genericParameters, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadTypeNameOutParameter(bool definitelyNotRuntimeClass, MetadataStringId typeId, MetadataStringId typeNameId, regex::ImmutableList<RtTYPE> * genericParameters, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadTypeDefinitionTypeOutParameter(RtTYPEDEFINITIONTYPE type, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadVarFromRuntimeClassName(AutoHSTRING &runtimeClassHString, __in IUnknown *unknown, __in IInspectable *inspectable, bool allowIdentity, bool allowExtensions, bool allowEmptyRuntimeClassName, MetadataStringId methodNameId, ConstructorArguments* constructorArguments = nullptr);

        Js::Var ReadOutBasicType(CorElementType typeCor, __in_bcount(memSize) byte * mem, __in size_t memSize, MetadataStringId methodNameId, bool allowIdentity = true, bool allowExtensions = false);
        Js::Var ReadOutSystemGuidType(bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize);
        Js::Var ReadOutWindowsFoundationDateTimeType(__in_bcount(memSize) byte * mem, __in size_t memSize);
        Js::Var ReadOutString(__in_bcount(memSize) byte *mem, __in size_t memSize);
        Js::Var ReadPropertyValueVarFromRuntimeClassName(AutoHSTRING &runtimeClassHString, __in IUnknown *unknown, __in Windows::Foundation::IPropertyValue *propertyValue, bool allowIdentity, bool allowExtensions, MetadataStringId methodNameId = MetadataStringIdNil);
        Js::Var ReadOutWindowsFoundationTimeSpanType(__in_bcount(memSize) byte * mem, __in size_t memSize);

        void RecordToReleaseExistingString(HSTRING hs);
        void RecordToReleaseExistingUnknown(IUnknown * unknown);
        void RecordToUndo(HSTRING hs, bool forcedRecord = false);
        void RecordToUndo(IUnknown * unknown, bool forceRelease = false);

        void RecordRecyclerVar(Var recyclerVar, bool forcedRecord = false);

        typedef void (ProjectionMarshaler::*fnTypeRecordString)(HSTRING *hstring);
        typedef void (ProjectionMarshaler::*fnTypeRecordUnknown)(IUnknown **unknown);

        void RecordTypeToUndo(fnTypeRecordString stringRecorder, fnTypeRecordUnknown unknownsRecorder, RtCONCRETETYPE concrete, __in_bcount(memSize) byte * mem, __in size_t memSize);
        void RecordStructTypeToUndo(fnTypeRecordString stringRecorder, fnTypeRecordUnknown unknownsRecorder, RtSTRUCTTYPE structType, __in_bcount(memSize) byte * mem, __in size_t memSize);

        void RecordOutString(HSTRING *hstring)
        {
            outHstrings = outHstrings->Prepend(hstring, alloc);
        }

        void RecordOutUnknown(IUnknown **unknown)
        {
            outUnknowns = outUnknowns->Prepend(unknown, alloc);
        }

        void RecordDelegateOutString(HSTRING *hstring)
        {
            if (fReleaseDelegateOutResources)
            {
                delegateOutHstrings = delegateOutHstrings->Prepend(*hstring, alloc);
            }
        }

        void RecordDelegateOutUnknown(IUnknown **unknown)
        {
            if (fReleaseDelegateOutResources && (*unknown != nullptr))
            {
                delegateOutUnknowns = delegateOutUnknowns->Prepend(*unknown, alloc);
            }
        }

        void RecordDelegateOutMemory(MemoryCleanupRecord & record)
        {
            delegateOutMemory = delegateOutMemory->Prepend(record, alloc);
        }

        void RecordDelegateReceiveArrayContents(FinalizableTypedArrayContents * contents)
        {
            delegateOutArrayContents = delegateOutArrayContents->Prepend(contents, alloc);
        }

        void ClearString(HSTRING *hString)
        {
            hstrings = hstrings->Prepend(*hString, alloc);
            *hString = nullptr;
        }

        void ClearUnknown(IUnknown **unknown)
        {
            addrefs = addrefs->Prepend(*unknown, alloc);
            *unknown = nullptr;
        }

        PropertyId IdOfString(LPCWSTR propertyName)
        {
            return Projection::IdOfString(projectionContext->GetScriptContext(), propertyName);
        }
        LPCWSTR StringOfId(PropertyId id)
        {
            return Projection::StringOfId(projectionContext->GetScriptContext(), id);
        }

        static HRESULT NumberToWinRTTimeSpanV6(double span, __out INT64* pResult);
        static HRESULT WinRTTimeSpanToNumberV6(INT64 span, __out double* pResult);

    public:
        ProjectionMarshaler(ResourceCleanup resourceCleanup, ProjectionContext *projectionContext, bool fReleaseExistingResource);
        ~ProjectionMarshaler();
        byte * WriteOutParameter(__in Js::Var inOutArg, __in RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId);
        byte * WriteInParameter(__in Var arg, __in RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize);
        void WriteInArrayTypeIndividual(Var arg, RtARRAYTYPE type, bool isByRef, __in_bcount(sizeof(LPVOID)) byte * lengthPointer, __in_bcount(sizeof(LPVOID)) byte * arrayPointer, bool isOut, bool hasLength = false, uint32 lengthValue = 0);
        byte * WriteInType(Var arg, RtTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue);

        Js::Var ReadOutType(Js::Var inOutArgument, RtTYPE type, bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize, MetadataStringId methodNameId, bool allowIdentity = true, bool allowExtensions = false, ConstructorArguments* constructorArguments = nullptr);
        Js::Var ReadOutParameter(Js::Var inOutArgument, RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity = true, bool allowExtensions = false, ConstructorArguments* constructorArguments = nullptr);

        Js::Var ReadOutArrayType(Js::Var inOutArgument, RtARRAYTYPE arrayType, __in_bcount(memSize1) byte * mem1, __in size_t memSize1,  
            __in_bcount(memSize2) byte * mem2, __in size_t memSize2, MetadataStringId methodNameId, bool isByRef, bool isOut, bool allowExtensions = false, bool hasLength = false, uint32 lengthValue = 0);
        Var TransferOwnershipAndReadInterfaceFromClassName(IInspectable *inspectable, HSTRING className, MetadataStringId methodNameId, ConstructorArguments* constructorArguments = nullptr);
        IUnknown * GetFastPathInterfaceFromExpr(Var varInput, RtEXPR expr);
        bool InitializeDelegateOutType(RtTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize);
        bool InitializeDelegateOutArrayType(RtARRAYTYPE arrayType, __in_bcount(memSize1) byte * mem1, __in size_t memSize1,  
            __in_bcount(memSize2) byte * mem2, __in size_t memSize2, bool isFillArray, bool hasLength, uint32 lengthValue);

        void SetReleaseDelegateOutResources() { fReleaseDelegateOutResources = true; }
        void TransferOwnershipOfDelegateOutTypes();

        bool IsInAsyncInterface() const { return fInAsyncInterface; }
        void SetIsInAsyncInterface() { fInAsyncInterface = true; }

#ifdef PROJECTION_METADATA_TRACE
        static void Trace(const char16 *form, ...) 
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(_u("ProjectionMarshaler: "));
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif
    };
}
