//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#include <ProjectionPch.h>
#include "Library\JavascriptBooleanObject.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "JavascriptWinRTDate.h"

namespace Projection
{

#if DBG
    int invokeCount = 0;
    void IncrementInvokeCount()
    {
        ++Projection::invokeCount;
        Projection::invokeCount; // You can set a break point here to see the post-increment count.
    }

    class EnsureRefCountChange
    {
        ULONG expectedChange;
        ULONG originalCount;
        IUnknown * u;
    public:
        EnsureRefCountChange(IUnknown * u, ULONG expectedChange) 
            : u(u), expectedChange(expectedChange)
        {
            originalCount = u->AddRef() - 1;
        }

        ~EnsureRefCountChange()
        {
            auto newCount = u->Release();
            Assert((newCount-originalCount)==expectedChange);
        }
    };
#endif

    struct ParameterInfo 
    {
    };
    
    struct RuntimeClassParameterInfo : ParameterInfo
    {
#if DEBUG
        static const int staticSignature = 0xd980e99;
        int instanceSignature; 
#endif
        MetadataStringId typeNameId;
        const IID & iidDefault;

        RuntimeClassParameterInfo(MetadataStringId typeNameId, const IID & iidDefault) :
            typeNameId(typeNameId), iidDefault(iidDefault)
#if DBG
            ,instanceSignature(staticSignature)
#endif
        { }

        static RuntimeClassParameterInfo * From(ParameterInfo * parameterInfo)
        {
            auto runtimeClassParameterInfo = static_cast<RuntimeClassParameterInfo*>(parameterInfo);
            Assert(runtimeClassParameterInfo->instanceSignature == RuntimeClassParameterInfo::staticSignature);
            return runtimeClassParameterInfo;
        }
    };

    struct InterfaceParameterInfo : ParameterInfo
    {
#if DEBUG
        static const int staticSignature = 0xe980e99;
        int instanceSignature; 
#endif

        MetadataStringId typeNameId;
        RtEXPR expr;
        const IID & iidInterface;
        RtSPECIALIZATION specialization;

        InterfaceParameterInfo(MetadataStringId typeNameId, const IID & iidInterface, RtSPECIALIZATION specialization, RtEXPR expr) :
            typeNameId(typeNameId), iidInterface(iidInterface), specialization(specialization), expr(expr)
#if DBG
            ,instanceSignature(staticSignature)
#endif
        { }

        static InterfaceParameterInfo * From(ParameterInfo * parameterInfo)
        {
            auto interfaceParameterInfo = static_cast<InterfaceParameterInfo*>(parameterInfo);
            Assert(interfaceParameterInfo->instanceSignature == InterfaceParameterInfo::staticSignature);
            return interfaceParameterInfo;
        }
    };

    template<bool throwErrorOnTypeMismatch>
    IUnknown * TryGetInterface(Var thisArg, MetadataStringId expectedTypeId, const IID & iid, MetadataStringId methodNameId, Js::ScriptContext * scriptContext, bool * isDefaultInterface)
    {
        if (Js::TaggedNumber::Is(thisArg))
        {
            if (throwErrorOnTypeMismatch)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }
            return nullptr;
        }

        if (Js::CustomExternalObject::Is(thisArg))
        {
            auto ceo = Js::CustomExternalObject::FromVar(thisArg);
            if (ceo->GetTypeNameId() == expectedTypeId)
            {
                auto p = (ProjectionObjectInstance*)(thisArg);
                Assert(p != nullptr);

                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase,
                    _u("TryGetInterface(%s (#%d) :: %s (#%d)) invoked\n"),
                    p->GetProjectionContext()->StringOfId(expectedTypeId),
                    expectedTypeId,
                    p->GetProjectionContext()->StringOfId(methodNameId),
                    methodNameId);

                if (p->GetUnknown() == nullptr)
                {
                    if (methodNameId != MetadataStringIdNil)
                    {
                        Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, StringOfId(scriptContext, methodNameId));
                    }
                    Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject);
                }
                IUnknown* retValue = p->GetInterfaceOfNativeABI(iid, scriptContext, isDefaultInterface);

                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase,
                    _u("TryGetInterface(%s (#%d) :: %s (#%d)) returned IUnknown=0x%08X\n"),
                    p->GetProjectionContext()->StringOfId(expectedTypeId),
                    expectedTypeId,
                    p->GetProjectionContext()->StringOfId(methodNameId),
                    methodNameId,
                    retValue);

                return retValue;
            }
            else if (throwErrorOnTypeMismatch)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }
        }

        if (throwErrorOnTypeMismatch)
        {
            Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(thisArg);
            if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined)
            {
                return nullptr;
            }

            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
        }

        return nullptr;
    }

    INT64 GetDateTimeOfJavascriptDate(Var varInput, ProjectionContext *projectionContext)
    {
        HRESULT hr = S_OK;
        INT64 ticks;
        if (Js::JavascriptWinRTDate::Is(varInput))
        {
            // Handle it getting dirty as well as just a regular date object being passed in
            Js::JavascriptWinRTDate* pWinRTDate = (Js::JavascriptWinRTDate*) varInput;

            if (pWinRTDate->AreTicksValid())
            {
                ticks = pWinRTDate->GetTicks();
            }
            else
            {
                // If the ticks are not valid, then the double value is the true date within this object
                // so get that instead
                hr = Js::DateUtilities::ES5DateToWinRTDate(pWinRTDate->GetTime(), &ticks);
                if (FAILED(hr))
                {
                    // If conversion failed, double value is outside the range of WinRT DateTime
                    Js::JavascriptError::ThrowRangeError(projectionContext->GetScriptContext(), JSERR_OutOfDateTimeRange);
                }
            }
        }
        else if (Js::JavascriptDate::Is(varInput))
        {
            Js::JavascriptDate* pDate = Js::JavascriptDate::FromVar(varInput);

            if (Js::JavascriptNumber::IsNan(pDate->GetTime()))
            {
                Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), JSERR_NeedDate);
            }

            hr = Js::DateUtilities::ES5DateToWinRTDate(pDate->GetTime(), &ticks);
            if (FAILED(hr))
            {
                // If conversion failed, double value is outside the range of WinRT DateTime
                Js::JavascriptError::ThrowRangeError(projectionContext->GetScriptContext(), JSERR_OutOfDateTimeRange);
            }
        }
        else
        {
            Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), JSERR_NeedDate);
        }

        return ticks;
    }

    template<typename TBase, typename TContext = void*>
    struct BaseTraits
    {
        typedef TBase TBase;
        typedef uint32 TLength;
        typedef TContext TContext;
        static const bool signatureRequiresLength = false;
        static const bool requiresContext = false;
        static const bool toVarRequiresRuntimeClassName = false;
        inline static TContext CreateParameterContext() { return TContext(); }
        inline static __declspec(noreturn) uint32 GetLength(TContext*) { Js::Throw::FatalInternalError(); }
        inline static void ReleaseInAfterCall(TBase, TContext*, ProjectionContext *) { }
        inline static TBase Default() { return TBase(); }
        inline static HRESULT GetRuntimeClassName(TBase v, HSTRING *className) { Js::Throw::FatalInternalError(); }
        inline static bool IsAsync(ParameterInfo* parameterInfo) { return false; }
    };

    struct UInt32Traits : public BaseTraits<unsigned __int32>
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            return (TBase)Js::JavascriptConversion::ToInt32(var, projectionContext->GetScriptContext());
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            return Js::JavascriptNumber::ToVar(v, projectionContext->GetScriptContext());
        }
    };

    struct Int32Traits : public BaseTraits<__int32> 
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            return (TBase)Js::JavascriptConversion::ToInt32(var, projectionContext->GetScriptContext());
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            return Js::JavascriptNumber::ToVar(v, projectionContext->GetScriptContext());
        }
    };

    struct SingleTraits : public BaseTraits<float>
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            return (TBase)Js::JavascriptConversion::ToNumber(var, projectionContext->GetScriptContext());
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            return Js::JavascriptNumber::ToVarWithCheck(v, projectionContext->GetScriptContext());
        }
    };

    struct DoubleTraits : public BaseTraits<double>
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            return (TBase)Js::JavascriptConversion::ToNumber(var, projectionContext->GetScriptContext());
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            return Js::JavascriptNumber::ToVarWithCheck(v, projectionContext->GetScriptContext());
        }
    };

    struct BoolTraits : public BaseTraits<bool>
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            if (Js::JavascriptOperators::GetTypeId(var) == Js::TypeIds_BooleanObject)
            {
                return Js::JavascriptBooleanObject::FromVar(var)->GetValue() ? true: false; 
            }
            return Js::JavascriptConversion::ToBoolean(var, projectionContext->GetScriptContext()) ? true: false;
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            return Js::JavascriptBoolean::ToVar(v, projectionContext->GetScriptContext());
        }
    };

    struct DateTimeTraits : public BaseTraits<Windows::Foundation::DateTime>
    {
        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            TBase dateTime;
            dateTime.UniversalTime = GetDateTimeOfJavascriptDate(var, projectionContext);
            return dateTime;
        }
        inline static Js::Var ToVar(ParameterInfo *, TBase v, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            // Not Implemented. Only +Date signatures included for now
            Js::Throw::FatalInternalError();
        }
    };

    struct HStringReferenceContext
    {
        HSTRING_HEADER header;
        RecyclerRootPtr<Js::JavascriptString> pinned;
    };

    template<bool pinVar = true>
    struct HStringReferenceTraits : public BaseTraits<HSTRING, HStringReferenceContext>
    {
        static const bool requiresContext = true;

        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext * context, ProjectionContext *projectionContext) 
        {
            auto scriptContext = projectionContext->GetScriptContext();
            auto str = Js::JavascriptConversion::ToString(var, scriptContext);

            if (pinVar) // Built-in strings are not in the recycler
            {
                auto recycler = scriptContext->GetRecycler();

                if (recycler->IsValidObject(str))
                {
                    context->pinned.Root(str, recycler);
                }
            }

            HSTRING hs;
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            HRESULT hr = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsCreateStringReference(str->GetSz(), str->GetLength(), &context->header, &hs);
            if (FAILED(hr))
            {
                ReleaseInAfterCall(hs, context, projectionContext);
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            }
            return hs;
        }
        // This method assumes ownership of the HSTRING
        inline static Js::Var ToVar(ParameterInfo *, TBase hs, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            UINT32 length;
            auto stringMethods = projectionContext->GetThreadContext()->GetWinRTStringLibrary();
            auto sz = stringMethods->WindowsGetStringRawBuffer(hs, &length); 
            auto result = Js::JavascriptString::NewCopyBuffer(sz, length, projectionContext->GetScriptContext()); // NOTE: We could make a new basic type for HSTRINGs and avoid this copy.
            stringMethods->WindowsDeleteString(hs);
            return result;
        }

        inline static void ReleaseInAfterCall(TBase hs, TContext * context, ProjectionContext *projectionContext)
        {
            projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsDeleteString(hs);
            if (pinVar && context && context->pinned) 
            {
                context->pinned.Unroot(projectionContext->GetScriptContext()->GetRecycler());
            }
        }
    };

    // Assumes unknown * is ownership
    Var FastPathProjectionObjectCreate(IUnknown *raw, PropertyId typeNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
    {
        Assert(raw != nullptr);
        projectionContext->TypeInstanceCreated(raw); 
        auto projectionWriter = projectionContext->GetProjectionWriter();
        auto scriptContext = projectionContext->GetScriptContext();
        Var result = nullptr;
        CComPtr<IUnknown> unknown;
        HRESULT hr;

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = raw->QueryInterface(IID_IUnknown, (void**)&unknown);
            raw->Release();
        }
        END_LEAVE_SCRIPT(scriptContext)

        Js::VerifyOkCatastrophic(hr);

        if (!projectionWriter->TryGetTypedInstanceFromCache(unknown, &result))
        {
            HTYPE htype = nullptr;
            RuntimeClassTypeInformation * typeInformation = nullptr;
            Js::VerifyCatastrophic(projectionWriter->GetRuntimeClassTypeInformation(typeNameId, &htype, &typeInformation));
            result = projectionWriter->CreateProjectionObjectTypedInstance(typeNameId, htype, typeInformation, unknown, true, false, constructorArguments);
        }

        return result;
    }

    struct RuntimeClassTraits : public BaseTraits<IUnknown*>
    {
        inline static TBase FromVar(ParameterInfo * parameterInfo, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            auto runtimeClassParameterInfo = RuntimeClassParameterInfo::From(parameterInfo);
            return TryGetInterface<true>(var, runtimeClassParameterInfo->typeNameId, runtimeClassParameterInfo->iidDefault, MetadataStringIdNil, projectionContext->GetScriptContext(), nullptr);
        }

        // ToVar assumes ownership of the instance
        inline static Js::Var ToVar(ParameterInfo * parameterInfo, TBase raw, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            auto runtimeClassParameterInfo = RuntimeClassParameterInfo::From(parameterInfo);
            Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

            if(raw) 
            {
                return FastPathProjectionObjectCreate(raw, runtimeClassParameterInfo->typeNameId, projectionContext);
            }
            return scriptContext->GetLibrary()->GetNull();
        }
    };

    struct InterfaceTraits : public BaseTraits<IUnknown*>
    {
        static const bool toVarRequiresRuntimeClassName = true;

        inline static TBase FromVar(ParameterInfo * parameterInfo, Js::Var var, TContext, ProjectionContext *projectionContext) 
        {
            auto interfaceParameterInfo = InterfaceParameterInfo::From(parameterInfo);
            ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
            return marshal.GetFastPathInterfaceFromExpr(var, interfaceParameterInfo->expr);
        }

        // ToVar assumes ownership of the instance
        inline static Js::Var ToVar(ParameterInfo * parameterInfo, TBase raw, HSTRING className, MetadataStringId methodNameId, ProjectionContext *projectionContext, ConstructorArguments* constructorArguments = nullptr)
        {
            auto interfaceParameterInfo = InterfaceParameterInfo::From(parameterInfo);
            Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

            if(raw) 
            {
                uint32 lengthClassName;
                auto winrtStringLib = scriptContext->GetThreadContext()->GetWinRTStringLibrary();
                LPCWSTR strClassName = winrtStringLib->WindowsGetStringRawBuffer(className, &lengthClassName);
                // PropertyValue marshaling would have to go through marhaler
                if ((interfaceParameterInfo->specialization == nullptr || interfaceParameterInfo->specialization->specializationType != specPropertyValueSpecialization) && 
                    (className == nullptr || interfaceParameterInfo->typeNameId == IdOfString(scriptContext, strClassName)))
                {
                    // Fast Path = same String name as Interface expected
                    winrtStringLib->WindowsDeleteString(className);
                    return FastPathProjectionObjectCreate(raw, interfaceParameterInfo->typeNameId, projectionContext, constructorArguments);
                }

                // Project using runtimeClassName string
                ProjectionMarshaler marshal(CalleeTransfersOwnership, projectionContext, false);
                Var result = marshal.TransferOwnershipAndReadInterfaceFromClassName((IInspectable *)raw, className, methodNameId, constructorArguments);
                if (Js::JavascriptOperators::GetTypeId(result) != Js::TypeIds_Undefined)
                {
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        raw->Release();
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                    return result;
                }
                // If unable to marshal based on runtimeClassName string, use typeId
                return FastPathProjectionObjectCreate(raw, interfaceParameterInfo->typeNameId, projectionContext, constructorArguments);
            }
            return scriptContext->GetLibrary()->GetNull();
        }

        inline static HRESULT GetRuntimeClassName(TBase v, HSTRING *className)
        {
            if (v)
            {
                return ((IInspectable *)v)->GetRuntimeClassName(className);
            }
            return S_OK;
        }

        inline static void ReleaseInAfterCall(TBase v, TContext * context, ProjectionContext *projectionContext)
        {
            if (v)
            {
                v->Release();
            }
        }

        inline static bool IsAsync(ParameterInfo* parameterInfo) 
        { 
            auto interfaceParameterInfo = InterfaceParameterInfo::From(parameterInfo);
            if (interfaceParameterInfo->specialization && interfaceParameterInfo->specialization->specializationType == specPromiseSpecialization)
            {
                return true;
            }

            return false;
        }
    };

    template<typename TElementTraits>
    struct ArrayContext
    {
        typedef typename TElementTraits::TBase TElement;
        typedef typename TElementTraits::TContext TElementContext;

        bool isArray:1;
        bool isArrayProjection:1;
        bool isJavascriptArray:1;
        bool isNull:1;
        bool isUndefined:1;
        unsigned int length;
        TElement * elements;
        TElementContext * elementContext;
        ProjectionContext * projectionContext;

        ArrayContext()
            : isArray(false), isArrayProjection(false), isJavascriptArray(false), isNull(false), isUndefined(false), length(0), elements(nullptr), elementContext(nullptr), projectionContext(nullptr)
        { }

        ~ArrayContext()
        {
            if (!isArrayProjection)
            {
                if (elements)
                {
                    Assert(projectionContext);
                    if (projectionContext)
                    {
                        for (uint index = 0; index < length; index++)
                        {
                            if (elementContext)
                            {
                                TElementTraits::ReleaseInAfterCall(elements[index], &elementContext[index], projectionContext);
                            }
                            else
                            {
                                TElementTraits::ReleaseInAfterCall(elements[index], nullptr, projectionContext);
                            }
                        }
                    }
                    delete [] elements;
                }

                if (TElementTraits::requiresContext)
                {
                    delete [] elementContext;
                }
            }
        }

        void Analyze(Js::Var var, Js::ScriptContext * scriptContext)
        {
            Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(var);
            isNull = typeId == Js::TypeIds_Null;
            isUndefined = typeId == Js::TypeIds_Undefined;
            if (isNull || isUndefined)
            {
                length = 0;
                return;
            }
            isArray = ArrayAsCollection::IsArrayInstance(var) ? true : false; 
            isArrayProjection = ArrayProjection::Is(var) ? true : false; 
            auto isTypedArray = Js::TypedArrayBase::Is(var) ? true : false; 
            isJavascriptArray = Js::JavascriptArray::Is(var) ? true : false;

            if (!isArray && !isArrayProjection && !isTypedArray)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayObject);
            }

            if (isArray)
            {
                length = ArrayAsCollection::GetLength(static_cast<Js::JavascriptArray *>(var));
            } 
            else if (isTypedArray)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }
            else
            {
                auto instance = ArrayProjection::GetArrayObjectInstance(var);
                Assert(instance != NULL);
                if (instance->GetPropertyId() != IdOfString(scriptContext, _u("String")))
                {
                    Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
                }

                length = instance->GetLength();
            }
        }

        void AllocateElements(ProjectionContext * projContext)
        {
            projectionContext = projContext;
            elements = new TElement[length];
            IfNullThrowOutOfMemory(elements);
            ::memset(elements, 0, sizeof(TElement) * length);
        }

        void AllocateElementContexts()
        {
            elementContext = new TElementContext[length];
            IfNullThrowOutOfMemory(elementContext);
            ::memset(elementContext, 0, sizeof(TElementContext) * length);
        }
    };
    
    template<typename TElementTraits>
    struct ArrayTraits : BaseTraits<typename TElementTraits::TBase *, ArrayContext<typename TElementTraits> >
    {
        typedef typename TElementTraits::TBase TElement;

        static const bool signatureRequiresLength = true;
        static const bool requiresContext = true;

        inline static TContext CreateParameterContext()
        {
            return TContext();
        }

        inline static TBase FromVar(ParameterInfo *, Js::Var var, TContext * context, ProjectionContext *projectionContext) 
        {
            auto scriptContext = projectionContext->GetScriptContext();
            context->Analyze(var, scriptContext);

            if (context->isArrayProjection)
            {
                auto instance = ArrayProjection::GetArrayObjectInstance(var);
                context->elements = (TBase)(instance->GetArrayBlock());
            }
            else
            {
                context->AllocateElements(projectionContext);
                if (context->length == 0)
                {
                    return context->elements;
                }

                if (TElementTraits::requiresContext)
                {
                    context->AllocateElementContexts();
                }

                if (context->isJavascriptArray) 
                {
                    // Fast-path for JavascriptArray
                    auto arr = Js::JavascriptArray::FromVar(var);

                    arr->WalkExisting([&](uint32 index, Var element) {
                        if (TElementTraits::requiresContext)
                            context->elements[index] = TElementTraits::FromVar(nullptr, element, &context->elementContext[index], projectionContext);
                        else
                            context->elements[index] = TElementTraits::FromVar(nullptr, element, nullptr, projectionContext);
                    });
                }
                else
                {
                    // General path
                    for (uint index = 0; index < context->length; index++)
                    {
                        Var varIndex = Js::JavascriptNumber::ToVar(index, scriptContext);
                        Var varElement = Js::JavascriptOperators::OP_GetElementI(var, varIndex, scriptContext);
                        if (TElementTraits::requiresContext)
                            context->elements[index] = TElementTraits::FromVar(nullptr, varElement, &context->elementContext[index], projectionContext);
                        else
                            context->elements[index] = TElementTraits::FromVar(nullptr, varElement, nullptr, projectionContext);                    
                    }
                }
            }
            return context->elements;
        }

        inline static uint32 GetLength(TContext * context) 
        {
            return context->length;
        }
    };

    struct FastPathSignature
    {
        Signature * dynamicCallSignature; // The non-fastpath signature
        const IID & iid;
        int vtableIndex;
        ParameterInfo ** parameters;
        void* fastPathFunction;

        FastPathSignature(Signature * signature, const IID & iid, int vtableIndex)
            : dynamicCallSignature(signature), iid(iid), vtableIndex(vtableIndex), parameters(nullptr), fastPathFunction(nullptr)
        { }
    };

    Var FastPath(Js::Var method, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
        auto dynamicCallSignature = signature->dynamicCallSignature;
        Assert(callInfo.Count > 0);
        auto a0 = args[0];
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();

        bool isDefaultInterface;
        auto unknown = TryGetInterface<false>(a0, dynamicCallSignature->thisInfo->GetTypeId(), signature->iid, dynamicCallSignature->method->nameId, scriptContext, &isDefaultInterface);
        if (unknown)
        {
            typedef HRESULT fn(IUnknown*);
            auto call = (fn*)((*((LPVOID**)unknown))[signature->vtableIndex]);
            IncrementInvokeCount();
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            HRESULT hr;
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = (*call)(unknown);
                // If the interface is a default interface, we don't need to release since we already don't add ref the cached default interface for this call
                if (!isDefaultInterface)
                {
                    unknown->Release();
                }
            }
            END_LEAVE_SCRIPT(scriptContext)
            if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        return DynamicCall(dynamicCallSignature, method, args, callInfo);
    }

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
    template<typename T1>
    Var FastPathIn(Js::Var method, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
        auto dynamicCallSignature = signature->dynamicCallSignature;
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();
        if (callInfo.Count > 1)
        {
            auto a0 = args[0];
            bool isDefaultInterface;
            auto unknown = TryGetInterface<false>(a0, dynamicCallSignature->thisInfo->GetTypeId(), signature->iid, dynamicCallSignature->method->nameId, scriptContext, &isDefaultInterface);
            if (unknown)
            {
                auto projectionContext = dynamicCallSignature->projectionContext;

                auto a1 = args[1];
                auto c1 = T1::CreateParameterContext();
                auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
                auto vptr = ((*((LPVOID**)unknown))[signature->vtableIndex]);
                IncrementInvokeCount();
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                HRESULT hr;
                BEGIN_LEAVE_SCRIPT(scriptContext);
                if (T1::signatureRequiresLength)
                {
                    typedef HRESULT fn(IUnknown*, T1::TLength, T1::TBase);
                    auto call = (fn*)vptr;
                    auto c1Length = T1::GetLength(&c1);
                    hr = (*call)(unknown, c1Length, p1);
                }
                else
                {
                    typedef HRESULT fn(IUnknown*, T1::TBase);
                    auto call = (fn*)vptr;
                    hr = (*call)(unknown, p1);
                }
                // If the interface is a default interface, we don't need to release since we already don't add ref the cached default interface for this call
                if (!isDefaultInterface)
                {
                    unknown->Release();
                }
                T1::ReleaseInAfterCall(p1, &c1, projectionContext);
                END_LEAVE_SCRIPT(scriptContext);
                if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
                {
                    return scriptContext->GetLibrary()->GetUndefined();
                }
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                return scriptContext->GetLibrary()->GetUndefined();
            }
            return DynamicCall(dynamicCallSignature, method, args, callInfo);
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, dynamicCallSignature->method->nameId));
    }
#pragma warning(pop)

    template<typename T1>
    Var FastPathOut(Js::Var method, Js::CallInfo callInfo, ...)
    {
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
        ARGUMENTS(args, callInfo);
        Assert(callInfo.Count > 0);
        auto a0 = args[0];
        auto dynamicCallSignature = signature->dynamicCallSignature;
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();

        bool isDefaultInterface;
        auto unknown = TryGetInterface<false>(a0, dynamicCallSignature->thisInfo->GetTypeId(), signature->iid, dynamicCallSignature->method->nameId, scriptContext, &isDefaultInterface);
        if (unknown)
        {
            auto projectionContext = signature->dynamicCallSignature->projectionContext;
            auto p1 = T1::Default();
            HSTRING className = nullptr;
            auto isAsyncDebuggingEnabled = AsyncDebug::IsAsyncDebuggingEnabled(scriptContext) && T1::IsAsync(signature->parameters[0]);
            AsyncDebug::AsyncOperationId asyncOperationId = AsyncDebug::InvalidAsyncOperationId;

            if (isAsyncDebuggingEnabled)
            {
                asyncOperationId = projectionContext->GetProjectionAsyncDebug()->BeginAsyncMethodCall(signature->dynamicCallSignature->thisInfo->GetTypeId(), signature->dynamicCallSignature->method->nameId);
            }

            typedef HRESULT fn(IUnknown*, T1::TBase *);
            auto call = (fn*)((*((LPVOID**)unknown))[signature->vtableIndex]);
            IncrementInvokeCount();
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            HRESULT hr;
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = (*call)(unknown, &p1);
                // If the interface is a default interface, we don't need to release since we already don't add ref the cached default interface for this call
                if (!isDefaultInterface)
                {
                    unknown->Release();
                }
                if (T1::toVarRequiresRuntimeClassName && SUCCEEDED(hr))
                {
                    if (FAILED(T1::GetRuntimeClassName(p1, &className)))
                    {
                        className = nullptr;
                    }
                }
            }
            END_LEAVE_SCRIPT(scriptContext)
            if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            Var result;
            
            if (isAsyncDebuggingEnabled)
            {
                PromiseConstructorArguments constructorArguments(asyncOperationId);
                result = T1::ToVar(signature->parameters[0], p1, className, signature->dynamicCallSignature->method->nameId, projectionContext, &constructorArguments);
            }
            else 
            {
                result = T1::ToVar(signature->parameters[0], p1, className, signature->dynamicCallSignature->method->nameId, projectionContext);
            }
            return result;
        }
        return DynamicCall(dynamicCallSignature, method, args, callInfo);
    }

    template<typename T1, typename T2>
    Var FastPathInIn(Js::Var method, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
        auto dynamicCallSignature = signature->dynamicCallSignature;
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();
        if (callInfo.Count > 2)
        {
            auto a0 = args[0];
            bool isDefaultInterface;
            auto unknown = TryGetInterface<false>(a0, dynamicCallSignature->thisInfo->GetTypeId(), signature->iid, dynamicCallSignature->method->nameId, scriptContext, &isDefaultInterface);
            if (unknown)
            {
                auto projectionContext = dynamicCallSignature->projectionContext;
                auto a1 = args[1];
                auto c1 = T1::CreateParameterContext();
                auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
                auto a2 = args[2];
                auto c2 = T2::CreateParameterContext();
                auto p2 = T2::FromVar(signature->parameters[1], a2, &c2, projectionContext);
                typedef HRESULT fn(IUnknown*, T1::TBase, T2::TBase);
                auto call = (fn*)((*((LPVOID**)unknown))[signature->vtableIndex]);
                IncrementInvokeCount();
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                HRESULT hr;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = (*call)(unknown, p1, p2);
                    // If the interface is a default interface, we don't need to release since we already don't add ref the cached default interface for this call
                    if (!isDefaultInterface)
                    {
                        unknown->Release();
                    }
                    T1::ReleaseInAfterCall(p1, &c1, projectionContext);
                    T2::ReleaseInAfterCall(p2, &c2, projectionContext);
                }
                END_LEAVE_SCRIPT(scriptContext)
                if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
                {
                    return scriptContext->GetLibrary()->GetUndefined();
                }
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                return scriptContext->GetLibrary()->GetUndefined();
            }
            return DynamicCall(dynamicCallSignature, method, args, callInfo);
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, signature->dynamicCallSignature->method->nameId));
    }

    
    template<typename T1, typename T2>
    Var FastPathInOut(Js::Var method, Js::CallInfo callInfo, ...)
    {
        typedef HRESULT fn(IUnknown*, T1::TBase, T2::TBase *);

        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
        auto dynamicCallSignature = signature->dynamicCallSignature;
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();
        HSTRING className = nullptr;

        if (callInfo.Count > 1)
        {
            auto a0 = args[0];
            bool isDefaultInterface;
            auto unknown = TryGetInterface<false>(a0, dynamicCallSignature->thisInfo->GetTypeId(), signature->iid, dynamicCallSignature->method->nameId, scriptContext, &isDefaultInterface);
            if (unknown)
            {
                auto projectionContext = dynamicCallSignature->projectionContext;
                auto a1 = args[1];
                auto c1 = T1::CreateParameterContext();
                auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
                auto p2 = T2::Default();
                auto call = (fn*)((*((LPVOID**)unknown))[signature->vtableIndex]);
                auto isAsyncDebuggingEnabled = AsyncDebug::IsAsyncDebuggingEnabled(scriptContext) && T2::IsAsync(signature->parameters[1]);
                AsyncDebug::AsyncOperationId asyncOperationId = AsyncDebug::InvalidAsyncOperationId;

                if (isAsyncDebuggingEnabled)
                {
                    asyncOperationId = projectionContext->GetProjectionAsyncDebug()->BeginAsyncMethodCall(signature->dynamicCallSignature->thisInfo->GetTypeId(), signature->dynamicCallSignature->method->nameId);
                }

                IncrementInvokeCount();
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                HRESULT hr = S_OK;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = (*call)(unknown, p1, &p2);
                    // If the interface is a default interface, we don't need to release since we already don't add ref the cached default interface for this call
                    if (!isDefaultInterface)
                    {
                        unknown->Release();
                    }
                    if (T2::toVarRequiresRuntimeClassName && SUCCEEDED(hr))
                    {
                        if (FAILED(T2::GetRuntimeClassName(p2, &className)))
                        {
                            className = nullptr;
                        }
                    }
                    T1::ReleaseInAfterCall(p1, &c1, projectionContext);
                }
                END_LEAVE_SCRIPT(scriptContext)
                if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
                {
                    return scriptContext->GetLibrary()->GetUndefined();
                }
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                Var result;
            
                if (isAsyncDebuggingEnabled)
                {
                    PromiseConstructorArguments constructorArguments(asyncOperationId);
                    result = T2::ToVar(signature->parameters[1], p2, className, signature->dynamicCallSignature->method->nameId, projectionContext, &constructorArguments);
                }
                else 
                {
                    result = T2::ToVar(signature->parameters[1], p2, className, signature->dynamicCallSignature->method->nameId, projectionContext);
                }

                return result; 
            }
            return DynamicCall(dynamicCallSignature, method, args, callInfo);
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, signature->dynamicCallSignature->method->nameId));
    }

    void FastPathPopulateRuntimeClassThis(RuntimeClassThis * runtimeClassThis, ProjectionContext * projectionContext)
    {
        Assert(runtimeClassThis->factory==nullptr);

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        // Figure out the metadata
        RtEXPR expr = nullptr;
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        Assert(projectionContext->IdOfString(runtimeClassThis->fullClassName) == runtimeClassThis->typeId);
        auto hr = projectionContext->GetExpr(runtimeClassThis->typeId, runtimeClassThis->typeId, nullptr, nullptr, &expr);
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        // Get the factory
        CComPtr<IActivationFactory> factory;
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        hr = projectionContext->CreateTypeFactoryInstance(projectionContext->StringOfId(runtimeClassThis->typeId), __uuidof(IActivationFactory), (IUnknown**)&factory); 
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr); 

        // This is the last step, to preserve error behaviors above
        runtimeClassThis->factory = factory.Detach(); 
        projectionContext->GetProjectionWriter()->AddRuntimeClassThisToCleanupOnClose(runtimeClassThis);
    }

    struct StaticFastPathSignature : public FinalizableObject
    {
        Signature * dynamicCallSignature; // The non-fastpath signature
        IUnknown * factory; 
        LPVOID vtableEntry;
        ParameterInfo ** parameters;
        void* fastPathFunction;

        StaticFastPathSignature(Signature * signature, IUnknown * factory, LPVOID vtableEntry)
            : dynamicCallSignature(signature), factory(factory), vtableEntry(vtableEntry), parameters(nullptr),
            fastPathFunction(nullptr)
        { }


        void Finalize(bool isShutdown) override {}
        void Dispose(bool isShutdown) override 
        {
            if (factory  && !isShutdown)
            {
                factory->Release();
                factory = nullptr;
            }
        }
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }
    };

    Var StaticFastPath(Js::Var method, Js::CallInfo callInfo, ...)
    {
        typedef HRESULT fn(IUnknown*);
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
        Assert(callInfo.Count > 0);
        auto scriptContext = function->GetScriptContext();
        auto call = (fn*)(signature->vtableEntry);
        HRESULT hr;
        IncrementInvokeCount();
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = (*call)(signature->factory);
        }
        END_LEAVE_SCRIPT(scriptContext)
        if(hr == E_BOUNDS && signature->dynamicCallSignature->boundsToUndefined)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    template<bool callGRCN, typename T1>
    Var StaticFastPathOut(Js::Var method, Js::CallInfo callInfo, ...)
    {
        typedef HRESULT fn(IUnknown*, T1::TBase*);
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
        auto projectionContext = signature->dynamicCallSignature->projectionContext;
        auto scriptContext = projectionContext->GetScriptContext();
        HSTRING className = nullptr;
        T1::TBase p1 = T1::Default();
        auto call = (fn*)(signature->vtableEntry);
        HRESULT hr;
        auto isAsyncDebuggingEnabled = AsyncDebug::IsAsyncDebuggingEnabled(scriptContext) && T1::IsAsync(signature->parameters[0]);
        AsyncDebug::AsyncOperationId asyncOperationId = AsyncDebug::InvalidAsyncOperationId;

        if (isAsyncDebuggingEnabled)
        {
            asyncOperationId = projectionContext->GetProjectionAsyncDebug()->BeginAsyncMethodCall(signature->dynamicCallSignature->thisInfo->GetTypeId(), signature->dynamicCallSignature->method->nameId);
        }

        IncrementInvokeCount();
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = (*call)(signature->factory, &p1);
            if (T1::toVarRequiresRuntimeClassName && SUCCEEDED(hr))
            {
                if (FAILED(T1::GetRuntimeClassName(p1, &className)))
                {
                    className = nullptr;
                }
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
        if(hr == E_BOUNDS && signature->dynamicCallSignature->boundsToUndefined)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        Var result;
            
        if (isAsyncDebuggingEnabled)
        {
            PromiseConstructorArguments constructorArguments(asyncOperationId);
            result = T1::ToVar(signature->parameters[0], p1, className, signature->dynamicCallSignature->method->nameId, projectionContext, &constructorArguments);
        }
        else 
        {
            result = T1::ToVar(signature->parameters[0], p1, className, signature->dynamicCallSignature->method->nameId, projectionContext);
        }

        return result;
    }

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
    template<typename T1>
    Var StaticFastPathIn(Js::Var method, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
        auto projectionContext = signature->dynamicCallSignature->projectionContext;
        auto scriptContext = projectionContext->GetScriptContext();
        if (callInfo.Count > 1)
        {
            auto a1 = args[1];
            auto c1 = T1::CreateParameterContext();
            auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
            HRESULT hr;
            IncrementInvokeCount();
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                if (T1::signatureRequiresLength)
                {
                    typedef HRESULT fn(IUnknown*, T1::TLength, T1::TBase);
                    auto call = (fn*)signature->vtableEntry;
                    auto c1Length = T1::GetLength(&c1);
                    hr = (*call)(signature->factory, c1Length, p1);
                }
                else
                {
                    typedef HRESULT fn(IUnknown*, T1::TBase);
                    auto call = (fn*)signature->vtableEntry;
                    hr = (*call)(signature->factory, p1);
                }
                T1::ReleaseInAfterCall(p1, &c1, projectionContext);
            }
            END_LEAVE_SCRIPT(scriptContext)
            if(hr == E_BOUNDS && signature->dynamicCallSignature->boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, signature->dynamicCallSignature->method->nameId));
    }
#pragma warning(pop)

    template<typename T1, typename T2>
    Var StaticFastPathInIn(Js::Var method, Js::CallInfo callInfo, ...)
    {
        typedef HRESULT fn(IUnknown*, T1::TBase, T2::TBase);
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
        auto projectionContext = signature->dynamicCallSignature->projectionContext;
        auto scriptContext = projectionContext->GetScriptContext();
        if (callInfo.Count > 2)
        {
            auto a1 = args[1];
            auto c1 = T1::CreateParameterContext();
            auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
            auto a2 = args[2];
            auto c2 = T2::CreateParameterContext();
            auto p2 = T2::FromVar(signature->parameters[1], a2, &c2, projectionContext);
            auto call = (fn*)(signature->vtableEntry);
            HRESULT hr;
            IncrementInvokeCount();
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = (*call)(signature->factory, p1, p2);
                T1::ReleaseInAfterCall(p1, &c1, projectionContext);
                T2::ReleaseInAfterCall(p2, &c2, projectionContext);
            }
            END_LEAVE_SCRIPT(scriptContext)
            if(hr == E_BOUNDS && signature->dynamicCallSignature->boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, signature->dynamicCallSignature->method->nameId));
    }


    template<typename T1, typename T2>
    Var StaticFastPathInOut(Js::Var method, Js::CallInfo callInfo, ...)
    {
        typedef HRESULT fn(IUnknown*, T1::TBase, T2::TBase*);
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
        auto dynamicCallSignature = signature->dynamicCallSignature;
        auto projectionContext = dynamicCallSignature->projectionContext;
        auto scriptContext = projectionContext->GetScriptContext();

        HSTRING className = nullptr;
        if (callInfo.Count > 1)
        {
            auto a1 = args[1];
            auto c1 = T1::CreateParameterContext();
            auto p1 = T1::FromVar(signature->parameters[0], a1, &c1, projectionContext);
            auto p2 = T2::Default();
            auto call = (fn*)(signature->vtableEntry);
            HRESULT hr;
            auto isAsyncDebuggingEnabled = AsyncDebug::IsAsyncDebuggingEnabled(scriptContext) && T2::IsAsync(signature->parameters[1]);
            AsyncDebug::AsyncOperationId asyncOperationId = AsyncDebug::InvalidAsyncOperationId;

            if (isAsyncDebuggingEnabled)
            {
                asyncOperationId = projectionContext->GetProjectionAsyncDebug()->BeginAsyncMethodCall(signature->dynamicCallSignature->thisInfo->GetTypeId(), signature->dynamicCallSignature->method->nameId);
            }

            IncrementInvokeCount();
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = (*call)(signature->factory, p1, &p2);
                if (T2::toVarRequiresRuntimeClassName && SUCCEEDED(hr))
                {
                    if (FAILED(T2::GetRuntimeClassName(p2, &className)))
                    {
                        className = nullptr;
                    }
                }
                T1::ReleaseInAfterCall(p1, &c1, projectionContext);
            }
            END_LEAVE_SCRIPT(scriptContext)
            if(hr == E_BOUNDS && dynamicCallSignature->boundsToUndefined)
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

            Var result;
            
            if (isAsyncDebuggingEnabled)
            {
                PromiseConstructorArguments constructorArguments(asyncOperationId);
                result = T2::ToVar(signature->parameters[1], p2, className, signature->dynamicCallSignature->method->nameId, projectionContext, &constructorArguments);
            }
            else 
            {
                result = T2::ToVar(signature->parameters[1], p2, className, signature->dynamicCallSignature->method->nameId, projectionContext);
            }

            return result;
        }
        Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, dynamicCallSignature->method->nameId));
    }

    // 'null' indicates that there was at least one unsupported parameter
    ParameterInfo ** TryGenerateParameterInfo(ProjectionContext * projectionContext, ImmutableList<RtPARAMETER> * parameters)
    {
        auto scriptContext = projectionContext->GetScriptContext();
        auto projectionWriter = projectionContext->GetProjectionWriter();
        auto recycler = scriptContext->GetRecycler();
        auto parameterCount = parameters->Count();
        auto parameterInfos = RecyclerNewArray(recycler, ParameterInfo*, parameterCount);
        bool allSupportedParameters = true;
        parameters->IterateN([&](int index, RtPARAMETER parameter) {
            bool isByRef = ByRefType::Is(parameter->type);
            RtTYPE type = isByRef ? ByRefType::From(parameter->type)->pointedTo : parameter->type;
            if (ClassType::Is(type) || InterfaceType::Is(type))
            {
                auto typeDefType = TypeDefinitionType::From(type);
                HTYPE htype = nullptr;
                RuntimeClassTypeInformation * typeInformation = nullptr;
                
                bool runtimeClassTypeExists;
                {
                    // In fast path case, the signature and all parameters should be resloved before calling the winRT function
                    // at this time, we should allow parsing metadata
#if DBG
                    ProjectionModel::AllowParsingMetadata allow;
#endif
                    runtimeClassTypeExists = projectionWriter->TryEnsureRuntimeClassTypeExists(typeDefType, &htype, &typeInformation);
                }

                if (!runtimeClassTypeExists)
                {
                    allSupportedParameters = false;
                    parameterInfos[index] = nullptr;
                }
                else
                {
                    if (ClassType::Is(type))
                    {
                        parameterInfos[index] = RecyclerNew(recycler, RuntimeClassParameterInfo, typeDefType->typeId, typeInformation->GetThisInfo()->defaultInterface->instantiated);
                    }
                    else
                    {
                        RtEXPR expr = nullptr;
                        HRESULT hr = projectionContext->GetExpr(typeDefType->typeId, typeDefType->typeDef->id, nullptr, typeDefType->genericParameters, &expr);
                        if (FAILED(hr))
                        {
                            Js::VerifyOkCatastrophic(hr);
                        }
                        parameterInfos[index] = RecyclerNew(recycler, InterfaceParameterInfo, typeDefType->typeId, typeInformation->GetThisInfo()->defaultInterface->instantiated, 
                            typeInformation->GetThisInfo()->GetSpecialization(), expr);
                    }
                }
            }
            else
            {
                parameterInfos[index] = nullptr;
            }
        });

        return allSupportedParameters ? parameterInfos : nullptr;
    }


    Js::JavascriptWinRTFunction * TryGetProjectionFastPath(PropertyId nameId, RtABIMETHODSIGNATURE method, Signature * thunkSignature, ThisInfo * thisInfo, ProjectionContext * projectionContext, bool fConstructor)
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.NoWinRTFastSig)
        {
            return nullptr;
        }
#endif
        auto scriptContext = projectionContext->GetScriptContext();
        auto projectionWriter = projectionContext->GetProjectionWriter();
        auto recycler = scriptContext->GetRecycler();

        auto metadataParameters = method->GetParameters()->allParameters;
        auto parameters = TryGenerateParameterInfo(projectionContext, metadataParameters);
        if (!metadataParameters->IsEmpty() // Empty is the void() case which will legitimately have parameters==nullptr
            && nullptr == parameters)
        {
            // One of the parameters was not fast-path-able.
            return nullptr;
        }

        // conditions we need to generate thunks. We need to check these conditions in the thunk as well.
        auto NeedFastPathThunk = [&]() ->BOOL {
            if (method->deprecatedAttributes != nullptr)
            {
                return TRUE;
            }
            return FALSE;
        };

        auto make = [&](void * fn) -> Js::JavascriptWinRTFunction * {
            // Non-static signature
            auto staticSignature = RecyclerNew(recycler, FastPathSignature, thunkSignature, method->iid->instantiated, method->vtableIndex+6);
            BOOL needFastPathThunk = FALSE;

            if (NeedFastPathThunk())
            {
                needFastPathThunk = TRUE;
                staticSignature->fastPathFunction = fn;
            }
            staticSignature->parameters = parameters;
            return projectionWriter->BuildDirectFunction((Var)staticSignature, needFastPathThunk? DefaultFastPathThunk<false> :fn, nameId, fConstructor);                                                                    
        };
        auto makeStatic = [&](void * fn) -> Js::JavascriptWinRTFunction * {
            // Static 'factory'
            auto runtimeClassThis = (RuntimeClassThis*)thisInfo;
            if(runtimeClassThis->factory==nullptr)
            {
                FastPathPopulateRuntimeClassThis(runtimeClassThis, projectionContext);
            }

            StaticFastPathSignature * staticSignature = nullptr; 
            IUnknown * factory = nullptr;
            HRESULT hr = S_OK;
            BOOL needFastPathThunk = FALSE;

            if (NeedFastPathThunk())
            {
                needFastPathThunk = TRUE;
            }
            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
#pragma warning(suppress: 6011) // runtimeClassThis->factory set by FastPathPopulateRuntimeClassThis()
                hr = runtimeClassThis->factory->QueryInterface(method->iid->instantiated, (void **)&factory);

                if (SUCCEEDED(hr))
                {
                    // Vtable offset
                    auto vtableEntry = ((*((LPVOID**)factory))[method->vtableIndex+6]);

                    // Static signature
                    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
                    staticSignature = RecyclerNewFinalized(recycler, StaticFastPathSignature, thunkSignature, factory, vtableEntry);
                    staticSignature->parameters = parameters;
                    if (needFastPathThunk)
                    {
                        staticSignature->fastPathFunction = fn;
                    }
                    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
                    
                    if (factory!=nullptr && FAILED(hr))
                    {
                        factory->Release();
                    }
                }
            }
            END_LEAVE_SCRIPT(scriptContext)
            IfFailedMapAndThrowHr(scriptContext, hr);
            return projectionWriter->BuildDirectFunction((Var)staticSignature, needFastPathThunk? DefaultFastPathThunk<true> : fn, nameId, fConstructor);                                                                    
        };

        auto callPattern = method->GetParameters()->callPattern;
        if (thisInfo->thisType == thisRuntimeClass)
        {
            // Static---------------
            if (wcscmp(callPattern,_u("-Class"))==0)
            {
                auto fn = StaticFastPathOut<false, RuntimeClassTraits>;
                return makeStatic(fn);
            } 
            else if (wcscmp(callPattern,_u("-String"))==0)
            {
                auto fn = StaticFastPathOut<false, HStringReferenceTraits<false> >;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-Int32"))==0)
            {
                auto fn = StaticFastPathOut<false, Int32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+String"))==0)
            {
                auto fn = StaticFastPathIn<HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-UInt32"))==0)
            {
                auto fn = StaticFastPathOut<false, UInt32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-UInt32"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,UInt32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u(""))==0)
            {
                auto fn = StaticFastPath;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-Boolean"))==0)
            {
                auto fn = StaticFastPathOut<false, BoolTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Int32"))==0)
            {
                auto fn = StaticFastPathIn<Int32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32"))==0)
            {
                auto fn = StaticFastPathIn<UInt32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Boolean"))==0)
            {
                auto fn = StaticFastPathIn<BoolTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-Single"))==0)
            {
                auto fn = StaticFastPathOut<false, SingleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-Double"))==0)
            {
                auto fn = StaticFastPathOut<false, DoubleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Single"))==0)
            {
                auto fn = StaticFastPathIn<SingleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Int32-String"))==0)
            {
                auto fn = StaticFastPathInOut<Int32Traits,HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+String-Boolean"))==0)
            {
                auto fn = StaticFastPathInOut<HStringReferenceTraits<false>,BoolTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Double"))==0)
            {
                auto fn = StaticFastPathIn<DoubleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+String-Class"))==0)
            {
                auto fn = StaticFastPathInOut<HStringReferenceTraits<false>,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Int32-Class"))==0)
            {
                auto fn = StaticFastPathInOut<Int32Traits,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Class"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Class"))==0)
            {
                auto fn = StaticFastPathIn<RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Class+Class"))==0)
            {
                auto fn = StaticFastPathInIn<RuntimeClassTraits,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Class-Class"))==0)
            {
                auto fn = StaticFastPathInOut<RuntimeClassTraits,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+String-String"))==0)
            {
                auto fn = StaticFastPathInOut<HStringReferenceTraits<false>,HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("-Interface"))==0)
            {
                auto fn = StaticFastPathOut<true, InterfaceTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+Interface"))==0)
            {
                auto fn = StaticFastPathIn<InterfaceTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+[String]"))==0)
            {
                auto fn = StaticFastPathIn<ArrayTraits<HStringReferenceTraits<true>>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+DateTime-String"))==0)
            {
                auto fn = StaticFastPathInOut<DateTimeTraits,HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-String"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Int32"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,Int32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Boolean"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,BoolTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Single"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,SingleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Double"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,DoubleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Interface"))==0)
            {
                auto fn = StaticFastPathInOut<UInt32Traits,InterfaceTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+String"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,HStringReferenceTraits<false>>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+UInt32"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,UInt32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Int32"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,Int32Traits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Boolean"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,BoolTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Single"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,SingleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Double"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,DoubleTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Class"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,RuntimeClassTraits>;
                return makeStatic(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Interface"))==0)
            {
                auto fn = StaticFastPathInIn<UInt32Traits,InterfaceTraits>;
                return makeStatic(fn);
            }
        }
        else
        {
            // Nonstatic---------------
            if (wcscmp(callPattern,_u("-Class"))==0)
            {
                auto fn = FastPathOut<RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-String"))==0)
            {
                auto fn = FastPathOut<HStringReferenceTraits<false> >;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-Int32"))==0)
            {
                auto fn = FastPathOut<Int32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+String"))==0)
            {
                auto fn = FastPathIn<HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-UInt32"))==0)
            {
                auto fn = FastPathOut<UInt32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-UInt32"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,UInt32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u(""))==0)
            {
                auto fn = FastPath;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-Boolean"))==0)
            {
                auto fn = FastPathOut<BoolTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Int32"))==0)
            {
                auto fn = FastPathIn<Int32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32"))==0)
            {
                auto fn = FastPathIn<UInt32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Boolean"))==0)
            {
                auto fn = FastPathIn<BoolTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-Single"))==0)
            {
                auto fn = FastPathOut<SingleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-Double"))==0)
            {
                auto fn = FastPathOut<DoubleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Single"))==0)
            {
                auto fn = FastPathIn<SingleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Int32-String"))==0)
            {
                auto fn = FastPathInOut<Int32Traits,HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+String-Boolean"))==0)
            {
                auto fn = FastPathInOut<HStringReferenceTraits<false>,BoolTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Double"))==0)
            {
                auto fn = FastPathIn<DoubleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+String-Class"))==0)
            {
                auto fn = FastPathInOut<HStringReferenceTraits<false>,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Int32-Class"))==0)
            {
                auto fn = FastPathInOut<Int32Traits,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Class"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Class"))==0)
            {
                auto fn = FastPathIn<RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Class+Class"))==0)
            {
                auto fn = FastPathInIn<RuntimeClassTraits,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Class-Class"))==0)
            {
                auto fn = FastPathInOut<RuntimeClassTraits,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+String-String"))==0)
            {
                auto fn = FastPathInOut<HStringReferenceTraits<false>,HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("-Interface"))==0)
            {
                auto fn = FastPathOut<InterfaceTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+Interface"))==0)
            {
                auto fn = FastPathIn<InterfaceTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+[String]"))==0)
            {
                auto fn = FastPathIn<ArrayTraits<HStringReferenceTraits<true>>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+DateTime-String"))==0)
            {
                auto fn = FastPathInOut<DateTimeTraits,HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-String"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Int32"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,Int32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Boolean"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,BoolTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Single"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,SingleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Double"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,DoubleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32-Interface"))==0)
            {
                auto fn = FastPathInOut<UInt32Traits,InterfaceTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+String"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,HStringReferenceTraits<false>>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+UInt32"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,UInt32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Int32"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,Int32Traits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Boolean"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,BoolTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Single"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,SingleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Double"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,DoubleTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Class"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,RuntimeClassTraits>;
                return make(fn);
            } else if (wcscmp(callPattern,_u("+UInt32+Interface"))==0)
            {
                auto fn = FastPathInIn<UInt32Traits,InterfaceTraits>;
                return make(fn);
            }
        }
        return nullptr;
    }

    template<bool isStaticMethod>
    Js::Var DefaultFastPathThunk(Js::RecyclableObject* method, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction* function = Js::JavascriptWinRTFunction::FromVar(method);
        Signature* dynamicCallSignature;
        void* fastPathMethod;
        if (isStaticMethod)
        {
            StaticFastPathSignature* signature = reinterpret_cast<StaticFastPathSignature*>(function->GetSignature());
            dynamicCallSignature = signature->dynamicCallSignature;
            fastPathMethod = signature->fastPathFunction;
        }
        else
        {
            FastPathSignature* signature = reinterpret_cast<FastPathSignature*>(function->GetSignature());
            dynamicCallSignature = signature->dynamicCallSignature;
            fastPathMethod = signature->fastPathFunction;
        }
        Js::ScriptContext *scriptContext = dynamicCallSignature->projectionContext->GetScriptContext();
        VerifyDeprecatedAttributeOnce(dynamicCallSignature->method, scriptContext, 
            (callInfo.Flags & CallFlags_New) ? DeprecatedInvocation_Class : DeprecatedInvocation_Method);
        return Js::JavascriptFunction::CallFunction<true>(function, reinterpret_cast<Js::JavascriptMethod>(fastPathMethod), args);
    }

}

