//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a projection of an ABIMethod in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    bool MapWithStringKey::IsMapWithStringKey(__in SpecialProjection * specialProjection)
    {
        return (specialProjection != nullptr && specialProjection->thisInfo->specialization->specializationType == specMapSpecialization);
    }

    bool MapWithStringKey::IsMapViewWithStringKey(__in SpecialProjection * specialProjection)
    {
        return (specialProjection != nullptr && specialProjection->thisInfo->specialization->specializationType == specMapViewSpecialization);
    }

    bool MapWithStringKey::IsMapOrMapViewWithStringKey(__in SpecialProjection * specialProjection)
    {
        return IsMapWithStringKey(specialProjection) || IsMapViewWithStringKey(specialProjection);
    }

    BOOL MapWithStringKey::HasKey(__in SpecialProjection *specialization, Var instance, Var key)
    {
        Var hasKeyValues[2] = {instance, key };

        Var result = nullptr;

        switch(specialization->thisInfo->specialization->specializationType)
        {
        case specMapSpecialization:
            result = DoInvoke(specialization->thisInfo,  MapSpecialization::From(specialization->thisInfo->specialization)->hasKey, true, hasKeyValues, 2, specialization->projectionContext);
            break;

        case specMapViewSpecialization:
            result = DoInvoke(specialization->thisInfo,  MapViewSpecialization::From(specialization->thisInfo->specialization)->hasKey, true, hasKeyValues, 2, specialization->projectionContext);
            break;

        default:
            Js::Throw::FatalProjectionError();
        }

        Assert(Js::JavascriptOperators::GetTypeId(result) == Js::TypeIds_Boolean);
        return Js::JavascriptConversion::ToBoolean(result, specialization->projectionContext->GetScriptContext());
    }

    void MapWithStringKey::Insert(__in SpecialProjection *specialization, Var instance, Var key, Var value)
    {
        Var insertValues[3] = {instance, key, value };
        Assert(MapWithStringKey::IsMapWithStringKey(specialization));
        DoInvoke(specialization->thisInfo, MapSpecialization::From(specialization->thisInfo->specialization)->insert, true, insertValues, 3, specialization->projectionContext);
    }

    Var MapWithStringKey::Lookup(__in SpecialProjection *specialization, Var instance, Var key)
    {
        Var lookupValues[2] = {instance, key };
        switch(specialization->thisInfo->specialization->specializationType)
        {
        case specMapSpecialization:
            return DoInvoke(specialization->thisInfo, MapSpecialization::From(specialization->thisInfo->specialization)->lookup, true, lookupValues, 2, specialization->projectionContext);

        case specMapViewSpecialization:
            return DoInvoke(specialization->thisInfo, MapViewSpecialization::From(specialization->thisInfo->specialization)->lookup, true, lookupValues, 2, specialization->projectionContext);

        default:
            Js::Throw::FatalProjectionError();
        }
    }

    void MapWithStringKey::Remove(__in SpecialProjection *specialization, Var instance, Var key)
    {
        Var removeValues[2] = {instance, key };
        Assert(MapWithStringKey::IsMapWithStringKey(specialization));

        DoInvoke(specialization->thisInfo, MapSpecialization::From(specialization->thisInfo->specialization)->remove, true, removeValues, 2, specialization->projectionContext);
    }

    Var MapWithStringKey::First(__in SpecialProjection *specialization, Var instance)
    {
        Var firstValues[1] = {instance };

        switch(specialization->thisInfo->specialization->specializationType)
        {
        case specMapSpecialization:
            return DoInvoke(specialization->thisInfo, MapSpecialization::From(specialization->thisInfo->specialization)->first, false, firstValues, 1, specialization->projectionContext);

        case specMapViewSpecialization:
            return DoInvoke(specialization->thisInfo, MapViewSpecialization::From(specialization->thisInfo->specialization)->first, false, firstValues, 1, specialization->projectionContext);

        default:
            Js::Throw::FatalProjectionError();
        }
    }

    Var MapWithStringKey::GetPropertyIdVar(PropertyId propertyId, Js::ScriptContext * scriptContext)
    {
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return Js::JavascriptNumber::ToVar(index, scriptContext);
        }
        else
        {
            Js::PropertyRecord const *stringProperty = scriptContext->GetPropertyName(propertyId);
            return Js::JavascriptString::NewCopyBuffer(stringProperty->GetBuffer(), stringProperty->GetLength(), scriptContext); 
        }
    }

    Var MapWithStringKey::GetVarFromHSTRING(HSTRING hstring, __in SpecialProjection * specialization)
    {
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        UINT32 length;
        PCWSTR keyString = specialization->projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(hstring, &length);
        return Js::JavascriptString::NewCopyBuffer(keyString, length, scriptContext); 
    }

    BOOL MapWithStringKey::HasPrototypeProperty(__in SpecialProjection * specialization, Js::PropertyId propertyId)
    {
        return Js::JavascriptOperators::HasProperty(specialization->prototypeObject, propertyId);
    }

    BOOL MapWithStringKey::HasPrototypeItem(__in SpecialProjection * specialization, uint32 index)
    {
        return Js::JavascriptOperators::HasItem(specialization->prototypeObject, index);
    }

    BOOL MapWithStringKey::HasOwnPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar)
    {
        return MapWithStringKey::HasKey(specialization, instance, propertyIdVar);
    }

    // Returns own property value if present
    BOOL MapWithStringKey::GetOwnPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar, __out Var *value)
    {
        BOOL propertyPresent = MapWithStringKey::HasKey(specialization, instance, propertyIdVar);
        if (propertyPresent)
        {
            *value = MapWithStringKey::Lookup(specialization, instance, propertyIdVar);
        }
        else
        {
            *value = nullptr;
        }

        return propertyPresent;
    }

    // Set the value into the map down there
    void MapWithStringKey::SetPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar, __in Var value)
    {
        MapWithStringKey::Insert(specialization, instance, propertyIdVar, value);
    }

    BOOL MapWithStringKey::DeletePropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar)
    {
        BOOL propertyPresent = MapWithStringKey::HasKey(specialization, instance, propertyIdVar);
        if (propertyPresent)
        {
            MapWithStringKey::Remove(specialization, instance, propertyIdVar);
        }
        return propertyPresent;
    }

    HRESULT MapWithStringKey::HasOwnKey(__in SpecialProjection * specialization, __in Var instance, __in HSTRING key, __out BOOL *result)
    {
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        Js::JavascriptString *jsString = nullptr;
        Js::PropertyRecord const * propertyRecord;
        Js::PropertyId propertyId = Js::PropertyIds::_none;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            UINT32 length;
            PCWSTR keyString = specialization->projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(key, &length);
            jsString = Js::JavascriptString::NewCopyBuffer(keyString, length, scriptContext); 

            scriptContext->GetOrAddPropertyRecord(keyString, length, &propertyRecord);
            Assert(Js::Constants::NoProperty != propertyRecord->GetPropertyId());
            propertyId = propertyRecord->GetPropertyId();
        }
        END_JS_RUNTIME_CALL(scriptContext);

        uint32 numericPropertyId;
        if (scriptContext->IsNumericPropertyId(propertyId, &numericPropertyId))
        {
            // This is uint32 value so need to use Item and not property
            return MapWithStringKey::HasOwnItem(specialization, instance, jsString, result);
        }
        
        // Treat as string and get propertyId
        return MapWithStringKey::HasOwnProperty(specialization, instance, propertyId, result);
    }

    HRESULT MapWithStringKey::GetOwnValue(__in SpecialProjection * specialization, __in Var instance, __in HSTRING key, __out Var *value, __out BOOL *propertyPresent)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        *propertyPresent = FALSE;

        BOOL fHasOwnKey;
        HRESULT hr = MapWithStringKey::HasOwnKey(specialization, instance, key, &fHasOwnKey);
        IfFailedReturn(hr);

        if (fHasOwnKey)
        {
            Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
            BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
            {
                *propertyPresent = MapWithStringKey::GetOwnPropertyCore(specialization, instance, MapWithStringKey::GetVarFromHSTRING(key, specialization), value);
            }
            END_JS_RUNTIME_CALL(scriptContext);
        }
        return hr;
    }

    // return true if the property is present
    HRESULT MapWithStringKey::HasOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeProperty(specialization, propertyId);
            if (!isPrototypeProperty)
            {
                *result = MapWithStringKey::HasOwnPropertyCore(specialization, instance, MapWithStringKey::GetPropertyIdVar(propertyId, scriptContext));
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    // Returns own property value if present
    HRESULT MapWithStringKey::GetOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out Var *value, __out BOOL *propertyPresent)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *propertyPresent = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeProperty(specialization, propertyId);
            if (!isPrototypeProperty)
            {
                *propertyPresent = MapWithStringKey::GetOwnPropertyCore(specialization, instance, MapWithStringKey::GetPropertyIdVar(propertyId, scriptContext), value);
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    // Set the value into the map down there
    HRESULT MapWithStringKey::SetProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __in Var value, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            if (IsMapWithStringKey(specialization))
            {
                BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeProperty(specialization, propertyId);
                if (!isPrototypeProperty)
                {
                    MapWithStringKey::SetPropertyCore(specialization, instance, MapWithStringKey::GetPropertyIdVar(propertyId, scriptContext), value);
                    *result = TRUE;
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::DeleteProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            if (IsMapWithStringKey(specialization))
            {
                BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeProperty(specialization, propertyId);
                if (!isPrototypeProperty)
                {
                    *result = MapWithStringKey::DeletePropertyCore(specialization, instance, MapWithStringKey::GetPropertyIdVar(propertyId, scriptContext));
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::HasOwnItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeItem(specialization, Js::JavascriptConversion::ToInt32(index, scriptContext));
            if (!isPrototypeProperty)
            {
                *result = MapWithStringKey::HasOwnPropertyCore(specialization, instance, index);
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::GetOwnItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out Var *value, __out BOOL *itemPresent)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *itemPresent = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeItem(specialization, Js::JavascriptConversion::ToInt32(index, scriptContext));
            if (!isPrototypeProperty)
            {
                *itemPresent = MapWithStringKey::GetOwnPropertyCore(specialization, instance, index, value);
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::SetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __in Var value, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            if (IsMapWithStringKey(specialization))
            {
                BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeItem(specialization, Js::JavascriptConversion::ToInt32(index, scriptContext));
                if (!isPrototypeProperty)
                {
                    MapWithStringKey::SetPropertyCore(specialization, instance, index, value);
                    *result = TRUE;
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::DeleteItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result)
    {
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        *result = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            if (IsMapWithStringKey(specialization))
            {
                BOOL isPrototypeProperty = MapWithStringKey::HasPrototypeItem(specialization, Js::JavascriptConversion::ToInt32(index, scriptContext));
                if (!isPrototypeProperty)
                {
                    *result = MapWithStringKey::DeletePropertyCore(specialization, instance, index);
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT MapWithStringKey::GetEnumerator(__in SpecialProjection * specialization, __in Var instance, __out IVarEnumerator **enumerator)
    {
        // Get the iterator instance
        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        CComPtr<Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, IInspectable *> *>> iterator = nullptr;
        MapWithStringKeyEnumerator *pMapWithStringKeyEnumerator = nullptr;
        *enumerator = nullptr;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            Var iteratorInstance = MapWithStringKey::First(specialization, instance);

            RtTYPE returnType = nullptr;

            switch(specialization->thisInfo->specialization->specializationType)
            {
            case specMapSpecialization:
                {
#if DBG
                    AllowHeavyOperation allow;
#endif
                    returnType = ByRefType::From(MapSpecialization::From(specialization->thisInfo->specialization)->first->GetParameters()->returnType)->pointedTo;
                }
                break;

            case specMapViewSpecialization:
                {
#if DBG
                    AllowHeavyOperation allow;
#endif
                    returnType = ByRefType::From(MapViewSpecialization::From(specialization->thisInfo->specialization)->first->GetParameters()->returnType)->pointedTo;
                }
                break;

            default:
                Js::Throw::FatalProjectionError();
            }

            auto interfaceType = InterfaceType::From(returnType);
            Assert(wcscmp(StringOfId(scriptContext, interfaceType->typeDef->id), _u("Windows.Foundation.Collections.IIterator`1")) == 0);
            Assert(wcsncmp(StringOfId(scriptContext, interfaceType->typeId), _u("Windows.Foundation.Collections.IIterator`1<Windows.Foundation.Collections.IKeyValuePair`2<String,"), 97 /* length of string :  "Windows.Foundation.Collections.IIterator`1<Windows.Foundation.Collections.IKeyValuePair`2<String," */) == 0);

            RtEXPR iteratorExpr = nullptr;
            HRESULT hr = specialization->projectionContext->GetExpr(interfaceType->typeId, interfaceType->typeDef->id, nullptr, interfaceType->genericParameters, &iteratorExpr); 
            IfFailedMapAndThrowHr(scriptContext, hr);

            auto icIterator = RuntimeInterfaceConstructor::From(iteratorExpr);
            GetUnknownOfVarExtension(scriptContext, iteratorInstance, icIterator->iid->instantiated, (void **)&iterator, nullptr, true);

            // Create the MapWithStringKeyEnumerator
            pMapWithStringKeyEnumerator = HeapNew(MapWithStringKeyEnumerator, specialization, instance); 
        }
        END_JS_RUNTIME_CALL(scriptContext);

        // InitializeKeys - gets the list of keys currently in map. Its a no-throw function to make sure we can get the HRESULT and cleanup 
        // memory in case of failures
        HRESULT hr = pMapWithStringKeyEnumerator->InitializeKeys(iterator);
        if (FAILED(hr))
        {
            HeapDelete(pMapWithStringKeyEnumerator);
            IfFailedMapAndThrowHrAfterScriptEnter(scriptContext, hr);
        }

        hr = pMapWithStringKeyEnumerator->QueryInterface(__uuidof(IVarEnumerator), (void**)enumerator);            
        Assert(SUCCEEDED(hr));

        return hr;
    }

    HRESULT MapWithStringKey::GetAccessors(__in SpecialProjection *, __in Js::PropertyId , __out BOOL *result)
    {
        // we dont have any getter setter for any of the properties
        *result = FALSE;
        return S_OK;
    }

    HRESULT MapWithStringKey::SetAccessors(__in SpecialProjection *, __in Js::PropertyId)
    {
        // we dont have any getter setter for any of the properties
        return VBSERR_ActionNotSupported;
    }
}