//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"
#include "ChakraVersion.h"
#include "Language\JavascriptMathOperators.h"
#include "Language\JavascriptMathOperators.inl"

#define NUMBER_UTIL_INLINE inline
#include "common\NumberUtilities.inl"

namespace JsStaticAPI
{
    HRESULT _stdcall DataConversion::VarToBOOL(Var obj, BOOL* value)
    {
        if (Js::TaggedInt::Is(obj))
        {
            *value = obj != reinterpret_cast<Var>(Js::AtomTag_IntPtr);
            return NOERROR;
        }
#if FLOATVAR
        else if (Js::JavascriptNumber::Is_NoTaggedIntCheck(obj))
        {
            double dValue = Js::JavascriptNumber::GetValue(obj);
            *value = (!Js::JavascriptNumber::IsNan(dValue)) && (!Js::JavascriptNumber::IsZero(dValue));
            return NOERROR;
        }
#endif
        else if (Js::JavascriptBoolean::Is(obj))
        {
            *value =  Js::JavascriptBoolean::UnsafeFromVar(obj)->GetValue();
            return NOERROR;
        }

        HRESULT hr = E_FAIL;
        IActiveScriptDirect* scriptDirect = VarToScriptDirectNoRef(obj);
        if (scriptDirect)
        {
            hr = scriptDirect->VarToBOOL(obj, value);
        }
        return hr;
    }

    IActiveScriptDirect* DataConversion::VarToScriptDirectNoRef(Var obj)
    {
        Assert(Js::RecyclableObject::Is(obj));
        Js::ScriptContext* scriptContext = Js::RecyclableObject::UnsafeFromVar(obj)->GetScriptContext();
        if (scriptContext->IsClosed())
        {
            return NULL;
        }
        return scriptContext->GetActiveScriptDirect();
    }

    HRESULT _stdcall DataConversion::VarToInt(Var obj, int* value)
    {
        HRESULT hr = NOERROR;
        switch (Js::JavascriptOperators::GetTypeId(obj))
        {
        case Js::TypeIds_Undefined:
        case Js::TypeIds_Null:
            *value = 0;
            break;
        case Js::TypeIds_Integer:
            *value = Js::TaggedInt::ToInt32(obj);
            break;
#if FLOATVAR
        case Js::TypeIds_Number:
        {
            double e = Js::JavascriptNumber::GetValue(obj);
            *value = Js::JavascriptMath::ToInt32Core(e);
        }
            break;
#endif
        default:
            {
                IActiveScriptDirect* activeScriptDirect = VarToScriptDirectNoRef(obj);
                hr = activeScriptDirect->VarToInt(obj, value);
            }
            break;
        }
        return hr;
    }

    JavascriptTypeId DataConversion::GetTypeId(Var obj)
    {
        return (JavascriptTypeId)Js::JavascriptOperators::GetTypeId(obj);
    }

    HRESULT DataConversion::VarToExtensionWithTypeIdRange(Var obj, void** buffer, JavascriptTypeId* typeId, JavascriptTypeId beginTypeId, JavascriptTypeId endTypeId)
    {
        if (!Js::RecyclableObject::Is(obj))
        {
            return E_INVALIDARG;
        }
        JavascriptTypeId baseTypeId = Js::JavascriptOperators::GetTypeId(obj);
        if (baseTypeId < beginTypeId ||
            baseTypeId > endTypeId)
        {
            return E_INVALIDARG;
        }
        // To avoid bringing in the whole type system, I'm creating a fake object with matching
        // memory layout as Js::CustomExternalObject.
        *buffer = (void*)(((char*)obj) + sizeof(Js::MockExternalObject));
        *typeId = baseTypeId;
        return S_OK;

    }

    void __stdcall DataConversion::FillInBinaryVerificationData(BinaryVerificationData* binaryVerificationData)
    {
        binaryVerificationData->majorVersion = SCRIPT_ENGINE_MAJOR_VERSION;
        binaryVerificationData->minorVersion = SCRIPT_ENGINE_MINOR_VERSION;
        binaryVerificationData->scriptEngineBaseSize = sizeof(ScriptEngineBase);
        binaryVerificationData->scriptEngineBaseOffset =  (DWORD)(static_cast<ScriptEngineBase*>((IActiveScriptDirect*)0x0));
        binaryVerificationData->scriptContextBaseSize = sizeof(Js::ScriptContextBase);
        binaryVerificationData->scriptContextBaseOffset = (DWORD)((Js::ScriptContext*)0x0)->GetScriptContextBase();
        binaryVerificationData->javascriptLibraryBaseSize = sizeof(Js::JavascriptLibraryBase);
        binaryVerificationData->javascriptLibraryBaseOffset = (DWORD)((Js::JavascriptLibrary*)0x0)->GetLibraryBase();
        binaryVerificationData->customExternalObjectSize = sizeof(Js::MockExternalObject);
        binaryVerificationData->typeOffset = (DWORD)((Js::RecyclableObject*)(0x0))->GetOffsetOfType();
        binaryVerificationData->typeIdOffset = (DWORD)((Js::Type*)(0x0))->GetTypeIdFieldOffset();
        binaryVerificationData->taggedIntSize = sizeof(Js::TaggedInt);
        binaryVerificationData->javascriptNumberSize = sizeof(Js::JavascriptNumber);
        binaryVerificationData->typeIdLimit = TypeIds_Limit;
        binaryVerificationData->numberUtilitiesBaseSize = sizeof(Js::NumberUtilitiesBase);
        binaryVerificationData->numberUtilitiesBaseOffset = (DWORD)((Js::NumberUtilities*)0x0)->GetNumberUtilitiesBase();
        JS_ETW(EventWriteJSCRIPT_HOSTING_BINARY_CONSISTENCY_INFO(
            binaryVerificationData->scriptEngineBaseSize,
            binaryVerificationData->scriptContextBaseSize,
            binaryVerificationData->javascriptLibraryBaseSize,
            binaryVerificationData->customExternalObjectSize,
            binaryVerificationData->scriptEngineBaseOffset,
            binaryVerificationData->scriptContextBaseOffset,
            binaryVerificationData->javascriptLibraryBaseOffset,
            binaryVerificationData->typeOffset,
            binaryVerificationData->typeIdOffset,
            binaryVerificationData->taggedIntSize,
            binaryVerificationData->javascriptNumberSize,
            binaryVerificationData->typeIdLimit,
            binaryVerificationData->numberUtilitiesBaseSize,
            binaryVerificationData->numberUtilitiesBaseOffset
            ));
    }

};