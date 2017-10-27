//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"
#include "Library\JavascriptProxy.h"

namespace Projection
{
    // *******************************************************
    // Represents a projection of an ArrayProjection in JavaScript
    // *******************************************************

    // Name:        ArrayProjection
    // Info:        Constructor for ArrayProjection
    // Parameters:  elementType - type of elements of the array
    // Return:      A new ArrayProjection  
    ArrayProjection::ArrayProjection(ProjectionContext* projectionContext, RtCONCRETETYPE elementType) :
        ProjectionTypeOperations(),
        elementType(elementType),
        projectionContext(projectionContext),
        m_hTypeRef(NULL),
        getItemAtId(projectionContext->getItemAtId)
    {
    }

    // Name:        ~ArrayProjection
    // Info:        Destructor for ArrayProjection
    // Parameters:
    // Return:
    ArrayProjection::~ArrayProjection()
    {
    }

    BOOL ArrayProjection::Is(Var instance)
    {
        Js::CustomExternalObject* externalObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(instance);
        if (!externalObject)
        {
            return FALSE;
        }
        
        Js::ExternalType * externalType = (Js::ExternalType *)externalObject->GetType();
        ProjectionTypeOperations* projectionTypeOperations = NULL;
        if (SUCCEEDED(externalType->GetTypeOperations()->QueryInterface(IID_IProjectionTypeOperations, (void**)&projectionTypeOperations)))
        {
            ProjectionType projectionType = projectionTypeOperations->GetProjectionType();
            projectionTypeOperations->Release();
            return projectionType == ArrayProjectionType;
        }

        return FALSE;
    }

    HRESULT ArrayProjection::EnsureProjection(
        __in RtCONCRETETYPE elementType,
        __in ProjectionContext* projectionContext, 
        __out ArrayProjection** ppNewInstance)
    {
        IfNullReturnError(ppNewInstance, E_INVALIDARG);
        *ppNewInstance = nullptr;

        Recycler* recycler = projectionContext->GetScriptContext()->GetRecycler();
        PropertyId propertyId = elementType->fullTypeNameId;

        ProjectionWriter *projectionWriter = projectionContext->GetProjectionWriter();
        ArrayProjection *pArrayProjection = projectionWriter->GetArrayProjection(propertyId);
        if (pArrayProjection == nullptr)
        {
            pArrayProjection = RecyclerNew(recycler, ArrayProjection, projectionContext, elementType);
            HRESULT hr = pArrayProjection->Initialize();
            if (FAILED(hr))
            {
                pArrayProjection->Release();
                return hr;
            }

            projectionWriter->AddArrayProjection(propertyId, pArrayProjection);
        }

        *ppNewInstance = pArrayProjection;
        return S_OK;
    }

    HRESULT ArrayProjection::Initialize()
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        LPCWSTR pszElementTypeName = StringOfId(scriptContext, elementType->fullTypeNameId);            

        size_t lenElementTypeName = wcslen(pszElementTypeName) + wcslen(_u("Array")) + 1;
        WCHAR *pszArrayName = new WCHAR[lenElementTypeName ];
        IfNullReturnError(pszArrayName, E_OUTOFMEMORY);
        wcscpy_s(pszArrayName, lenElementTypeName, pszElementTypeName);
        wcscat_s(pszArrayName, lenElementTypeName, _u("Array"));
        PropertyId propertyId = IdOfString(scriptContext, pszArrayName);
        delete [] pszArrayName;

        // We change the prototype of typed array projection from array to object. This make it consistent with the
        // library's typed array so we can change to use typed array in the future.
        // We are using null so that the CreateTypeFromScript will use the object prototype as prototype

        ScriptEngine *pScriptEngine = projectionContext->GetScriptEngine();
        IfNullMapAndThrowHr(scriptContext, pScriptEngine, E_ACCESSDENIED);

        return pScriptEngine->CreateTypeFromScript((::TypeId)scriptContext->CreateTypeId(), NULL, NULL, static_cast<ITypeOperations*>(this), FALSE, propertyId, TRUE, &m_hTypeRef);
    }


    HRESULT ArrayProjection::CreateArrayProjectionObject(
            __in RtCONCRETETYPE elementType, 
            __in ProjectionContext* projectionContext, 
            __in_xcount(length * elementType->storageSize) byte* pArrayBlockPointer,
            __in uint length,
            __in uint32 readArrayLength,
            __in bool fOwnBuffer,
            __out Var* pNewInstance,
            __in bool validContents)
    {
        IfNullReturnError(pNewInstance, E_INVALIDARG);
        *pNewInstance = nullptr;

        // Ensure we have projection
        HRESULT hr = NOERROR;
        ArrayProjection* pArrayProjection = NULL;
        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();
        Js::JavascriptLibrary* javascriptLibrary = scriptContext->GetLibrary();
        if (BasicType::Is(elementType))
        {
            RtBASICTYPE basicType = BasicType::From(elementType);
            Js::PFNCreateTypedArray createTypedArrayFunc = NULL;
            switch(basicType->typeCor)
            {
            case ELEMENT_TYPE_BOOLEAN:
                createTypedArrayFunc = Js::BoolArray::Create;
                break;
            case ELEMENT_TYPE_I8:
                createTypedArrayFunc = Js::Int64Array::Create;
                break;
            case ELEMENT_TYPE_U8:
                createTypedArrayFunc = Js::Uint64Array::Create;
                break;
            case ELEMENT_TYPE_U1:
                createTypedArrayFunc = Js::Uint8Array::Create;
                break;
            case ELEMENT_TYPE_I2:
                createTypedArrayFunc = Js::Int16Array::Create;
                break;
            case ELEMENT_TYPE_U2:
                createTypedArrayFunc = Js::Uint16Array::Create;
                break;
            case ELEMENT_TYPE_CHAR:
                createTypedArrayFunc = Js::CharArray::Create;
                break;
            case ELEMENT_TYPE_I4:
                createTypedArrayFunc = Js::Int32Array::Create;
                break;
            case ELEMENT_TYPE_U4:
                createTypedArrayFunc = Js::Uint32Array::Create;
                break;
            case ELEMENT_TYPE_R4:
                createTypedArrayFunc = Js::Float32Array::Create;
                break;
            case ELEMENT_TYPE_R8:
                createTypedArrayFunc = Js::Float64Array::Create;
                break;
            default:
                break;
            }

            if (createTypedArrayFunc)
            {
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    Output::Print(_u("    Creating Typed Array : %s\n"), TypedArrayName(createTypedArrayFunc));
                    Output::Flush();
                }
#endif
                Js::ArrayBuffer* arrayBuffer;
                auto elementSize = elementType->storageSize;
                auto requestedSize = length * elementSize;
                if (requestedSize > UINT32_MAX)         // Previous behavior would be to underflow sizeOfBlock without a check.
                    return E_OUTOFMEMORY;               // Instead we raise a responsible out-of-memory exception.

                uint32 sizeOfBlock = (uint32)(length * elementSize);
                Assert(Js::JavascriptConversion::ToUInt32((double)sizeOfBlock) >= length);
                // Create a instance that stores pointer to memory block
                if (!fOwnBuffer)
                {
                    // Duplicate the arrayblock and use that instead.
                    arrayBuffer = scriptContext->GetLibrary()->CreateProjectionArraybuffer(sizeOfBlock);
                    byte *pDuplicateArrayBlock = (byte *)arrayBuffer->GetBuffer();
                    
                    if (sizeOfBlock != 0 && pDuplicateArrayBlock == nullptr)
                    {
                        return E_OUTOFMEMORY;
                    }
                    // Copy the elements
                    Assert(ConcreteType::IsBlittable(elementType));
                    if (validContents)
                    {
#if DBG_DUMP
                        if (Js::Configuration::Global.flags.TraceWin8Allocations)
                        {
                            Output::Print(_u("    Copying %u Elements\n"), readArrayLength);
                            Output::Flush();
                        }
#endif
                    
                        // directly copy the buffer - no addref, strings copy involved
                        memcpy_s(pDuplicateArrayBlock, sizeOfBlock, pArrayBlockPointer, readArrayLength * elementSize);
                        if (length > readArrayLength)
                        {
#if DBG_DUMP
                            if (Js::Configuration::Global.flags.TraceWin8Allocations)
                            {
                                Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                                Output::Flush();
                            }
#endif
                            memset(pDuplicateArrayBlock + (readArrayLength * elementSize), 0, (length - readArrayLength) * elementSize); 
                        }
                    }
                    else
                    {
#if DBG_DUMP
                        if (Js::Configuration::Global.flags.TraceWin8Allocations)
                        {
                            Output::Print(_u("    Setting all elements to 0\n"));
                            Output::Flush();
                        }
#endif
                        memset(pDuplicateArrayBlock, 0, sizeOfBlock);
                    }
                }
                else
                {
                    Assert(validContents);

                    if (length > readArrayLength)
                    {
#if DBG_DUMP
                        if (Js::Configuration::Global.flags.TraceWin8Allocations)
                        {
                            Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                            Output::Flush();
                        }
#endif
                        memset(pArrayBlockPointer + (readArrayLength * elementSize), 0, (length - readArrayLength) * elementSize);
                    }
                    arrayBuffer = scriptContext->GetLibrary()->CreateProjectionArraybuffer(pArrayBlockPointer, sizeOfBlock);
                }

                BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
                {
                    *pNewInstance = createTypedArrayFunc(arrayBuffer, 0, length, javascriptLibrary);
                }
                END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_INSCRIPT(hr);

                return hr;
            }
        }

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("    Creating ArrayProjection : "));
            Output::Flush();
        }
#endif

        hr = ArrayProjection::EnsureProjection(elementType, projectionContext, &pArrayProjection);
        IfFailedReturn(hr);

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("%s\n"), StringOfId(scriptContext, pArrayProjection->GetPropertyId()));
            Output::Flush();
        }
#endif

        auto elementSize = elementType->storageSize;

        // Create a instance that stores pointer to memory block
        if (!fOwnBuffer)
        {
#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceWin8Allocations)
            {
                Output::Print(_u("    Create new buffer\n"));
                Output::Flush();
            }
#endif

            // Duplicate the arrayblock and use that instead.
            size_t sizeOfBlock = length * elementSize;
            Assert(Js::JavascriptConversion::ToUInt32((double)sizeOfBlock) >= length);
            byte *pDuplicateArrayBlock = (byte *)CoTaskMemAlloc(sizeOfBlock);
            IfNullReturnError(pDuplicateArrayBlock, E_OUTOFMEMORY);

            // Copy the elements if valid contents
            if (validContents)
            {
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    Output::Print(_u("    Copying %u Elements\n"), readArrayLength);
                    Output::Flush();
                }
#endif

                if (ConcreteType::IsBlittable(elementType))
                {
                    // directly copy the buffer - no addref, strings copy involved
                    memcpy_s(pDuplicateArrayBlock, sizeOfBlock, pArrayBlockPointer, readArrayLength * elementSize);
                    memset(pDuplicateArrayBlock + (readArrayLength * elementSize), 0, (length - readArrayLength) * elementSize); 
                }
                else
                {
                    // we need to project out each element and then marshal the content into the new buffer
                    for (uint index = 0; index < readArrayLength; index++)
                    {
                        Var elementValue = nullptr;
                        BOOL result;

                        ArrayObjectInstance::GetItemAt(pArrayBlockPointer, length, projectionContext, elementType, index, pArrayProjection->getItemAtId, &elementValue, &result);
                        Assert(result == TRUE);
                        ArrayObjectInstance::SetItemAt(pDuplicateArrayBlock, length, projectionContext, elementType, index, elementValue, false, &result);
                        Assert(result == TRUE);
                    }

                    if (length > readArrayLength)
                    {
#if DBG_DUMP
                        if (Js::Configuration::Global.flags.TraceWin8Allocations)
                        {
                            Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                            Output::Flush();
                        }
#endif
                        memset(pDuplicateArrayBlock + (readArrayLength * elementSize), 0, (length - readArrayLength) * elementSize); 
                    }
                }
            }
            else
            {
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    Output::Print(_u("    Clear the buffer\n"));
                    Output::Flush();
                }
#endif

                memset(pDuplicateArrayBlock, 0, sizeOfBlock);
            }

            pArrayBlockPointer = pDuplicateArrayBlock;
        }
        else
        {
            Assert(validContents);
            if (length > readArrayLength)
            {
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                    Output::Flush();
                }
#endif
                memset(pArrayBlockPointer + (readArrayLength * elementSize), 0, (length - readArrayLength) * elementSize);
            }
        }

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("    Creates Finalizer\n"));
            Output::Flush();
        }
#endif

        ArrayObjectInstance* pArrayObjectInstance = nullptr;
        HTYPE hTypeRef = pArrayProjection->GetTypeRef();
        hr = ArrayObjectInstance::Create(hTypeRef, pArrayProjection->getItemAtId, elementType, pArrayBlockPointer, length, projectionContext, &pArrayObjectInstance);
        IfFailedReturn(hr);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_TYPEDARRAY_OBJECT(pArrayObjectInstance, StringOfId(scriptContext, pArrayObjectInstance->GetTypeNameId())));
       
        // Create the length value and set it
        Var varLength = Js::JavascriptNumber::ToVar(length, scriptContext);
        pArrayObjectInstance->SetPropertyWithAttributes(Js::PropertyIds::length, varLength, PropertyNone, NULL, Js::PropertyOperation_None, Js::SideEffects_None);

        *pNewInstance = pArrayObjectInstance;
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *pNewInstance = Js::JavascriptProxy::AutoProxyWrapper(*pNewInstance);
        }
#endif
        return S_OK;
    }

    ArrayObjectInstance * ArrayProjection::GetArrayObjectInstance(Var instance)
    {
        Assert(ArrayProjection::Is(instance));
        return (ArrayObjectInstance*)instance;
    }

    uint32 ArrayProjection::GetLength(__in Var instance)
    {
        ArrayObjectInstance *pInstance = ArrayProjection::GetArrayObjectInstance(instance);
        Assert(pInstance != NULL);

        return pInstance->GetLength();
    }

    HRESULT ArrayProjection::GetNumericOwnProperty(__in Var instance, __in uint32 index, __out Var *value, __out BOOL *result, bool fIndexByVar, Var indexVar)
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        ArrayObjectInstance *pInstance = ArrayProjection::GetArrayObjectInstance(instance);
        Assert(pInstance != NULL);

        return pInstance->GetItemAt(projectionContext, index, value, result, fIndexByVar, indexVar);
    }

    HRESULT ArrayProjection::HasNumericOwnProperty(__in Var instance, __in uint32 index, __out BOOL *result, bool fIndexByVar, Var indexVar)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 indexValue = index;
        if (fIndexByVar)
        {
            BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(projectionContext->GetScriptContext())
            {
                indexValue = Js::JavascriptConversion::ToUInt32(indexVar, projectionContext->GetScriptContext());
            }
            END_JS_RUNTIME_CALL(projectionContext->GetScriptContext());
        }
        *result = (indexValue < ArrayProjection::GetLength(instance)) ? TRUE : FALSE;
        return S_OK;
    }

    HRESULT ArrayProjection::SetNumericOwnProperty(__in Var instance, __in uint32 index, __in Var value, __out BOOL *result, bool fIndexByVar, Var indexVar)
    {
        IfNullReturnError(value, E_INVALIDARG);
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        ArrayObjectInstance *pInstance = ArrayProjection::GetArrayObjectInstance(instance);
        Assert(pInstance != NULL);

        return pInstance->SetItemAt(projectionContext, index, value, result, fIndexByVar, indexVar);
    }

    HRESULT ArrayProjection::DeleteNumericOwnProperty(__in Var instance, __in uint32 index, __out BOOL *result, bool fIndexByVar, Var indexVar)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        Assert(ArrayProjection::GetArrayObjectInstance(instance) != NULL);

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::HasOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return HasNumericOwnProperty(instance, index, result);
        }
        return __super::HasOwnProperty(scriptDirect, instance, propertyId, result);
    }


    HRESULT STDMETHODCALLTYPE ArrayProjection::GetOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return GetNumericOwnProperty(instance, index, value, propertyPresent);
        }

        return __super::GetOwnProperty(scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetPropertyReference(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return GetNumericOwnProperty(instance, index, value, propertyPresent);
        }

        return __super::GetPropertyReference(scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return SetNumericOwnProperty(instance, index, value, result);
        }

        return __super::SetProperty(scriptDirect, instance, propertyId, value, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::DeleteProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

#if DBG
        uint32 index = 0;
        Assert(!projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index));
#endif

        return __super::DeleteProperty(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(itemPresent, E_INVALIDARG);
        *itemPresent = FALSE;

        return GetNumericOwnProperty(instance, 0, value, itemPresent, true, index);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        return SetNumericOwnProperty(instance, 0, value, result, true, index);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::DeleteItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        return DeleteNumericOwnProperty(instance, 0, result, true, index);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetEnumerator(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ BOOL enumNonEnumerable,
        /* [in] */ BOOL enumSymbols,
        /* [out] */ IVarEnumerator **enumerator) 
    {
        IfNullReturnError(enumerator, E_INVALIDARG);
        *enumerator = nullptr;

        CComPtr<IVarEnumerator> pDefaultOperatationEnumerator;
        HRESULT hr = __super::GetEnumerator(scriptDirect, instance, enumNonEnumerable, enumSymbols, &pDefaultOperatationEnumerator);
        IfFailedReturn(hr);

        // Create the ArrayProjectionEnumerator
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(projectionContext->GetScriptContext())
        {  
            ArrayProjectionEnumerator *pArrayProjectionEnumerator = HeapNew(ArrayProjectionEnumerator, this, instance, pDefaultOperatationEnumerator); 
            hr = pArrayProjectionEnumerator->QueryInterface(__uuidof(IVarEnumerator), (void**)enumerator);
            Assert(SUCCEEDED(hr));
        }
        END_JS_RUNTIME_CALL(projectionContext->GetScriptContext());

        return hr;
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::IsEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // check if it is one of the index that we have values for - then it is enumerable
            return HasNumericOwnProperty(instance, index, result);
        }

        return __super::IsEnumerable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::IsWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // check if it is one of the index that we have values for - then it is writable
            return HasNumericOwnProperty(instance, index, result);
        }

        return __super::IsWritable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::IsConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        { 
            // Non configurable
            return S_OK;
        }

        return __super::IsConfigurable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // no change if its one of our own index
            return S_OK;
        }

        return __super::SetEnumerable(scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // no change if its one of our own index
            return S_OK;
        }

        return __super::SetWritable(scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // no change if its one of our own index
            return S_OK;
        }

        return __super::SetConfigurable(scriptDirect, instance, propertyId, value);
    }


    HRESULT STDMETHODCALLTYPE ArrayProjection::GetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* getter,
        /* [out] */ Var* setter,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(getter, E_INVALIDARG);
        *getter = nullptr;
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return S_OK;
        }

        return __super::GetAccessors(scriptDirect, instance, propertyId, getter, setter, result);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter) 
    {
        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return VBSERR_ActionNotSupported;
        }

        return __super::SetAccessors(scriptDirect, instance, propertyId, getter, setter);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetSetter( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = ::DescriptorFlags_None;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            BOOL hasProperty;
            HRESULT hr = this->HasNumericOwnProperty(instance, index, &hasProperty);
            IfFailedReturn(hr);

            *flags = (hasProperty == TRUE) ? DescriptorFlags_Writable : DescriptorFlags_None;
            return S_OK;
        }

        return __super::GetSetter(scriptDirect, instance, propertyId, setter, flags);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetHeapObjectInfo(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ ProfilerHeapObjectInfoFlags flags,
        /* [out] */ HostProfilerHeapObject** result,
        /* [out] */ HeapObjectInfoReturnResult* returnResult)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = nullptr;
        IfNullReturnError(returnResult, E_INVALIDARG);
        *returnResult = HeapObjectInfoReturnResult_NoResult;
		
        // We need to report the information about internal indices
        if (flags == ProfilerHeapObjectInfoFull)
        {
            return GetArrayObjectInstance(instance)->GetFullHeapObjectInfo(result, returnResult);
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::HasOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ Var index,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        return HasNumericOwnProperty(instance, 0, result, true, index);
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::GetItemSetter(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index, 
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = ::DescriptorFlags_None;

        BOOL hasItem;
        HRESULT hr = this->HasOwnItem(scriptDirect, instance, index, &hasItem);
        IfFailedReturn(hr);

        *flags = (hasItem == TRUE) ? DescriptorFlags_Writable : DescriptorFlags_None;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE ArrayProjection::SetPropertyWithAttributes(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out]*/ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        uint32 index = 0;
        if (projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            return SetNumericOwnProperty(instance, index, value, result);
        }

        return __super::SetPropertyWithAttributes(scriptDirect, instance, propertyId, value, attributes, effects, result);
    }


    ArrayObjectInstance::ArrayObjectInstance(FinalizableTypedArrayContents *finalizableTypedArrayContents, HTYPE hTypeRef, MetadataStringId getItemAtId) 
        : Js::CustomExternalObject((Js::CustomExternalType *)hTypeRef),
        getItemAtId(getItemAtId),
        finalizableTypedArrayContents(finalizableTypedArrayContents)
    {
        finalizableTypedArrayContents->Initialize();
    }

    HRESULT ArrayObjectInstance::Create(
        __in HTYPE hTypeRef,
        __in MetadataStringId getItemAtId,
        __in RtCONCRETETYPE elementType, 
        __in byte *pArrayBlockPointer, 
        __in uint length,
        __in ProjectionContext *projectionContext, 
        __out ArrayObjectInstance** ppNewArrayInstance)
    {
        IfNullReturnError(projectionContext, E_INVALIDARG);
        IfNullReturnError(ppNewArrayInstance, E_INVALIDARG);
        *ppNewArrayInstance = nullptr;

        Recycler* recycler = projectionContext->GetScriptContext()->GetRecycler();
        
#if DBG_DUMP
        FinalizableTypedArrayContents *finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, length, pArrayBlockPointer, releaseBufferUsingCoTaskMemFree, (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation());
#else
        FinalizableTypedArrayContents *finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, length, pArrayBlockPointer, releaseBufferUsingCoTaskMemFree);
#endif

        *ppNewArrayInstance = RecyclerNew(recycler, ArrayObjectInstance, finalizableTypedArrayContents, hTypeRef, getItemAtId);

        return S_OK;
    }

    HRESULT ArrayObjectInstance::GetItemAt(
        __in ProjectionContext *projectionContext, 
        __in uint32 index, 
        __out Var *value, 
        __out BOOL *result,
        bool fIndexByVar, 
        Var indexVar)
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        HRESULT hr = S_OK;

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            uint32 indexValue = fIndexByVar ? Js::JavascriptConversion::ToUInt32(indexVar, scriptContext) : index;
            hr = ArrayObjectInstance::GetItemAt(finalizableTypedArrayContents->typedArrayBuffer, finalizableTypedArrayContents->numberOfElements, projectionContext, finalizableTypedArrayContents->elementType, indexValue, getItemAtId, value, result);
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }
    
    HRESULT ArrayObjectInstance::SetItemAt(
        __in ProjectionContext *projectionContext, 
        __in uint32 index, 
        __in Var value, 
        __out BOOL *result,
        bool fIndexByVar, 
        Var indexVar)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        HRESULT hr = S_OK;

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            uint32 indexValue = fIndexByVar ? Js::JavascriptConversion::ToUInt32(indexVar, scriptContext) : index;
            hr = ArrayObjectInstance::SetItemAt(finalizableTypedArrayContents->typedArrayBuffer, finalizableTypedArrayContents->numberOfElements, projectionContext, finalizableTypedArrayContents->elementType, indexValue, value, true, result);
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT ArrayObjectInstance::GetItemAt(
        __in_xcount(length * elementType->storageSize) byte *pArrayBlockPointer, 
        __in uint length, 
        __in ProjectionContext *projectionContext, 
        __in RtCONCRETETYPE elementType,
        __in uint32 index, 
        __in MetadataStringId getItemAtId,
        __out Var *value, 
        __out BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;

        if (index >= length)
        {
            return S_OK;
        }

        // Get item  at the index
        AssertMsg(elementType->storageSize < INT_MAX, "Invalid metadata: Max storage size should never exceed 2gb.");
        uint uElementTypeSize = (uint)elementType->storageSize;

        auto elementPointer = pArrayBlockPointer + (index * uElementTypeSize);
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        *value = marshal.ReadOutType(nullptr, elementType, true, elementPointer, uElementTypeSize, getItemAtId);
        *result = TRUE;
        return S_OK;
    }
    
    HRESULT ArrayObjectInstance::SetItemAt(
        __in_xcount(length * elementType->storageSize) byte *pArrayBlockPointer, 
        __in uint length, 
        __in ProjectionContext *projectionContext, 
        __in RtCONCRETETYPE elementType,
        __in uint32 index, 
        __in Var value, 
        __in bool fReleaseExistingItem,
        __out BOOL *result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (index >= length)
        {

            return S_OK;
        }

        // Get item  at the index
        AssertMsg(elementType->storageSize < INT_MAX, "Invalid metadata: Max storage size should never exceed 2gb.");
        uint uElementTypeSize = (uint)elementType->storageSize;
        auto elementPointer = pArrayBlockPointer + (index * uElementTypeSize);
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, fReleaseExistingItem);
        auto final = marshal.WriteInType(value, elementType, elementPointer, uElementTypeSize, true);
        Assert(static_cast<size_t>(final - elementPointer) == elementType->storageSize);

        *result = TRUE;
        return S_OK;
    }
    
    HRESULT ArrayObjectInstance::GetFullHeapObjectInfo(HostProfilerHeapObject** result, HeapObjectInfoReturnResult* returnResult)
    {
        Assert(this->finalizableTypedArrayContents != nullptr);
        AssertMsg(returnResult != nullptr, "A return result must be supplied.");
        UINT indexPropertiesSize = finalizableTypedArrayContents->GetHeapObjectOptionalIndexPropertiesSize();
        if (indexPropertiesSize == 0)
        {
            // This means its one of the types that we dont want to report index properties on
            *returnResult = HeapObjectInfoReturnResult_NoResult;
            return S_OK;
        }

        // Calculate size for HostHeapObjectInfo
        UINT headerSize = offsetof(HostProfilerHeapObject, optionalInfo);
        UINT allocSize = headerSize + indexPropertiesSize;

        *result = (HostProfilerHeapObject*)CoTaskMemAlloc(allocSize);
        IFNULLMEMRET(*result);
        memset(*result, 0, allocSize);

        // HeapObject information
        (*result)->typeNameId = this->GetNameId();
        (*result)->optionalInfoCount = 1;

        // WinRT instance related information
        ProfilerHeapObjectOptionalInfo *optionalInfo = (ProfilerHeapObjectOptionalInfo *)((byte *)(*result) + headerSize);

        ActiveScriptProfilerHeapEnum* heapEnum = reinterpret_cast<ActiveScriptProfilerHeapEnum*>(this->GetScriptContext()->GetHeapEnum());
        finalizableTypedArrayContents->FillHeapObjectOptionalIndexProperties(heapEnum, optionalInfo);
        *returnResult = HeapObjectInfoReturnResult_Success;
        return S_OK;
    }

    BOOL ArrayProjection::NeedConversion(RtCONCRETETYPE elementType, Js::Var typedArray)
    {
        Assert(Js::TypedArrayBase::Is(typedArray));
        Assert(BasicType::Is(elementType));
        if (!BasicType::Is(elementType))
        {
            return TRUE;
        }
        CorElementType corType = BasicType::From(elementType)->typeCor;
        switch (Js::JavascriptOperators::GetTypeId(typedArray))
        {
        case Js::TypeIds_Int8Array:
            return TRUE; // no native I1 array support.
        case Js::TypeIds_Uint8Array:
        case Js::TypeIds_Uint8ClampedArray:
            return corType != ELEMENT_TYPE_U1;
        case Js::TypeIds_Int16Array:
            return corType != ELEMENT_TYPE_I2;
        case Js::TypeIds_Uint16Array:
            return corType != ELEMENT_TYPE_U2;
        case Js::TypeIds_Int32Array:
            return corType != ELEMENT_TYPE_I4;
        case Js::TypeIds_Uint32Array:
            return corType != ELEMENT_TYPE_U4;
        case Js::TypeIds_Int64Array:
            return corType != ELEMENT_TYPE_I8;
        case Js::TypeIds_Uint64Array:
            return corType != ELEMENT_TYPE_U8;
        case Js::TypeIds_Float32Array:
            return corType != ELEMENT_TYPE_R4;
        case Js::TypeIds_Float64Array:
            return corType != ELEMENT_TYPE_R8;
        case Js::TypeIds_CharArray:
            return corType != ELEMENT_TYPE_CHAR;
        case Js::TypeIds_BoolArray:
            return corType != ELEMENT_TYPE_BOOLEAN;
        default:
            AssertMsg(FALSE, "invalid typed array type");
            return TRUE;
        }
    }

#if DBG_DUMP
    LPCWSTR ArrayProjection::TypedArrayName(Js::PFNCreateTypedArray createTypedArrayFunc)
    {
        if (createTypedArrayFunc == Js::BoolArray::Create)
            return _u("BoolArray");
        if (createTypedArrayFunc == Js::Int64Array::Create)
            return _u("Int64Array");
        if (createTypedArrayFunc == Js::Uint64Array::Create)
            return _u("Uint64Array");
        if (createTypedArrayFunc == Js::Int8Array::Create)
            return _u("Int8Array");
        if (createTypedArrayFunc == Js::Uint8Array::Create)
            return _u("Uint8Array");
        if (createTypedArrayFunc == Js::Uint8ClampedArray::Create)
            return _u("Uint8ClampedArray");
        if (createTypedArrayFunc == Js::Int16Array::Create)
            return _u("Int16Array");
        if (createTypedArrayFunc == Js::Uint16Array::Create)
            return _u("Uint16Array");
        if (createTypedArrayFunc == Js::CharArray::Create)
            return _u("CharArray");
        if (createTypedArrayFunc == Js::Int32Array::Create)
            return _u("Int32Array");
        if (createTypedArrayFunc == Js::Uint32Array::Create)
            return _u("Uint32Array");
        if (createTypedArrayFunc == Js::Float32Array::Create)
            return _u("Float32Array");
        if (createTypedArrayFunc == Js::Float64Array::Create)
            return _u("Float64Array");
        
        Js::Throw::FatalProjectionError();
    }

    LPCWSTR ArrayProjection::TypedArrayName(Js::Var typedArray)
    {
        Assert(Js::TypedArrayBase::Is(typedArray));
        switch (Js::JavascriptOperators::GetTypeId(typedArray))
        {
        case Js::TypeIds_Int8Array:
            return _u("Int8Array");
        case Js::TypeIds_Uint8Array:
            return _u("Uint8Array");
        case Js::TypeIds_Uint8ClampedArray:
            return _u("Uint8ClampedArray");
        case Js::TypeIds_Int16Array:
            return _u("Int16Array");
        case Js::TypeIds_Uint16Array:
            return _u("Uint16Array");
        case Js::TypeIds_Int32Array:
            return _u("Int32Array");
        case Js::TypeIds_Uint32Array:
            return _u("Uint32Array");
        case Js::TypeIds_Int64Array:
            return _u("Int64Array");
        case Js::TypeIds_Uint64Array:
            return _u("Uint64Array");
        case Js::TypeIds_Float32Array:
            return _u("Float32Array");
        case Js::TypeIds_Float64Array:
            return _u("Float64Array");
        case Js::TypeIds_CharArray:
            return _u("CharArray");
        case Js::TypeIds_BoolArray:
            return _u("BoolArray");
        default:
            Js::Throw::FatalProjectionError();
        }
    }
#endif
};
