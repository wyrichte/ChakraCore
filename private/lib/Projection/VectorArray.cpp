//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a projection of an ABIMethod in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    bool VectorArray::IsVector(__in SpecialProjection * specialProjection)
    {
        return specialProjection->thisInfo->specialization->specializationType == specVectorSpecialization;
    }

    bool VectorArray::IsVectorView(__in SpecialProjection * specialProjection)
    {
        return specialProjection->thisInfo->specialization->specializationType == specVectorViewSpecialization;
    }

    bool VectorArray::IsVectorOrVectorView(__in SpecialProjection * specialProjection)
    {
        return IsVector(specialProjection) || IsVectorView(specialProjection);
    }


    HRESULT VectorArray::CreateProtypeObject(__in bool fSpecialisedProjection, __in ProjectionContext *projectionContext, __out Var *pPrototypeVar)
    {
        IfNullReturnError(pPrototypeVar, E_POINTER);
        *pPrototypeVar = nullptr;

        Js::JavascriptLibrary *library = projectionContext->GetScriptContext()->GetLibrary();
        Js::DynamicObject* prototypeOfPrototype;
        if (fSpecialisedProjection)
        {
            prototypeOfPrototype = library->GetArrayPrototype();
        }
        else
        {
            prototypeOfPrototype = library->GetObjectPrototype();
        }

        *pPrototypeVar = library->CreateObject(prototypeOfPrototype);
        return S_OK;
    }

    uint32 VectorArray::GetLength(__in SpecialProjection * specialization, Var instance, Js::ScriptContext * scriptContext)
    {
        Var getSizeValues[1] = { instance };
        switch(specialization->thisInfo->specialization->specializationType)
        {
        case specVectorSpecialization:
            return Js::JavascriptConversion::ToUInt32(DoInvoke(specialization->thisInfo, VectorSpecialization::From(specialization->thisInfo->specialization)->length->getSize, true, getSizeValues, 1, specialization->projectionContext), scriptContext);

        case specVectorViewSpecialization:
            return Js::JavascriptConversion::ToUInt32(DoInvoke(specialization->thisInfo, VectorViewSpecialization::From(specialization->thisInfo->specialization)->length->getSize, true, getSizeValues, 1, specialization->projectionContext), scriptContext);
        }
        Js::Throw::FatalProjectionError();
    }

    Var VectorArray::GetAt(__in SpecialProjection * specialization, Var instance, Var index)
    {
        Var getAtValues[2] = {instance, index };

        // If we have not created a function for the getAt ABI call yet, create one now
        Assert(specialization->abiFunctions);
        if (!specialization->abiFunctions->getAtFunction)
        {
            switch(specialization->thisInfo->specialization->specializationType)
            {
            case specVectorSpecialization:
                specialization->abiFunctions->getAtFunction = specialization->projectionContext->GetProjectionWriter()->FunctionOfSignature(VectorSpecialization::From(specialization->thisInfo->specialization)->getAt, nullptr, specialization->thisInfo, false, true);
                Assert(specialization->abiFunctions->getAtFunction != nullptr);
                break;

            case specVectorViewSpecialization:
                specialization->abiFunctions->getAtFunction = specialization->projectionContext->GetProjectionWriter()->FunctionOfSignature(VectorViewSpecialization::From(specialization->thisInfo->specialization)->getAt, nullptr, specialization->thisInfo, false, true);
                Assert(specialization->abiFunctions->getAtFunction != nullptr);
                break;
            }
        }
        Js::VerifyCatastrophic(specialization->abiFunctions && specialization->abiFunctions->getAtFunction);
        Js::CallInfo callInfo(Js::CallFlags_Value, 2);
        return specialization->abiFunctions->getAtFunction->CallFunction(Js::Arguments(callInfo, getAtValues));
    }

    void VectorArray::SetAt(__in SpecialProjection * specialization, Var instance, Var index, Var value)
    {
        Var setAtValues[3] = {instance, index, value };
        // If we have not created a function for the setAt ABI call yet, create one now
        Assert(specialization->abiFunctions);
        if (!specialization->abiFunctions->setAtFunction)
        {
            specialization->abiFunctions->setAtFunction = specialization->projectionContext->GetProjectionWriter()->FunctionOfSignature(VectorSpecialization::From(specialization->thisInfo->specialization)->setAt, nullptr, specialization->thisInfo, false, false);
            Assert(specialization->abiFunctions->setAtFunction != nullptr);
        }
        Js::VerifyCatastrophic(specialization->abiFunctions && specialization->abiFunctions->setAtFunction);
        Js::CallInfo callInfo(Js::CallFlags_Value, 3);
        specialization->abiFunctions->setAtFunction->CallFunction(Js::Arguments(callInfo, setAtValues));
    } 

    void VectorArray::Append(__in SpecialProjection * specialization, Var instance, Var value)
    {
        Var appendValues[2] = {instance, value } ;
        // If we have not created a function for the append ABI call yet, create one now
        Assert(specialization->abiFunctions);
        if (!specialization->abiFunctions->appendFunction)
        {
            specialization->abiFunctions->appendFunction = specialization->projectionContext->GetProjectionWriter()->FunctionOfSignature(VectorSpecialization::From(specialization->thisInfo->specialization)->append, nullptr, specialization->thisInfo, false, false);
            Assert(specialization->abiFunctions->appendFunction != nullptr);
        }
        Js::VerifyCatastrophic(specialization->abiFunctions && specialization->abiFunctions->appendFunction);
        Js::CallInfo callInfo(Js::CallFlags_Value, 2);
        specialization->abiFunctions->appendFunction->CallFunction(Js::Arguments(callInfo, appendValues));
    }
    
    void VectorArray::RemoveAtEnd(__in SpecialProjection * specialization, Var instance)
    {
        // If we have not created a function for the removeAtEnd ABI call yet, create one now
        Assert(specialization->abiFunctions);
        if (!specialization->abiFunctions->removeAtEndFunction)
        {
            specialization->abiFunctions->removeAtEndFunction = specialization->projectionContext->GetProjectionWriter()->FunctionOfSignature(VectorSpecialization::From(specialization->thisInfo->specialization)->removeAtEnd, nullptr, specialization->thisInfo, false, false);
            Assert(specialization->abiFunctions->removeAtEndFunction != nullptr);
        }
        Js::VerifyCatastrophic(specialization->abiFunctions && specialization->abiFunctions->removeAtEndFunction);
        Js::CallInfo callInfo(Js::CallFlags_Value, 1);
        specialization->abiFunctions->removeAtEndFunction->CallFunction(Js::Arguments(callInfo, &instance));
    }

    // Name:        SetLength
    // Info:        Sets the length for the vector as array
    // Parameters:  methodThunk - The method thunk corresponding to this Projection
    //              pParentInterface - ParentInterfaceProjection
    // Return:      Success code
    Var VectorArray::SetLengthOfSpecialProjection(
        __in SpecialProjection * specialization, 
        __in Js::Arguments* arguments)
    {
        // this var + in param
        Assert(arguments->Info.Count == 2);

        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        Assert(scriptContext->GetThreadContext()->IsScriptActive());
        if (VectorArray::IsVector(specialization))
        {
            Var instance = arguments->Values[0];
            int length = static_cast<int>(VectorArray::GetLength(specialization, instance, scriptContext));
            int newLength = length;

            Var varNewLength = arguments->Values[1];
            if (Js::TaggedInt::Is(varNewLength))
            {
                newLength = Js::TaggedInt::ToInt32(varNewLength);
            }
            else
            {
                uint32 uintValue = Js::JavascriptConversion::ToUInt32(varNewLength, scriptContext);
                double dblValue = Js::JavascriptConversion::ToNumber(varNewLength, scriptContext);
                if (dblValue == uintValue)
                {
                    newLength = uintValue;
                }
                else
                {
                    Js::JavascriptError::ThrowRangeError(scriptContext, JSERR_ArrayLengthAssignIncorrect);
                }
            }

            if (newLength < 0)
            {
                Js::JavascriptError::ThrowRangeError(scriptContext, JSERR_ArrayLengthAssignIncorrect);
            }
            else if (newLength > length)
            {
                Js::JavascriptError::ThrowError(scriptContext, VBSERR_ActionNotSupported);
            }

            for ( ; length > newLength; length--)
            {
                VectorArray::RemoveAtEnd(specialization, instance);
            }

            return Js::JavascriptNumber::ToVar(newLength, scriptContext);
        }
        else
        {
            Assert (VectorArray::IsVectorView(specialization));
            Js::JavascriptError::ThrowError(scriptContext, VBSERR_ActionNotSupported);
        }
    }

    HRESULT VectorArray::HasItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *itemPresent, bool fIndexIsUInt32, uint32 uint32Index)
    {
        IfNullReturnError(itemPresent, E_POINTER);
        *itemPresent = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        HRESULT hr = S_OK;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            uint32 length = VectorArray::GetLength(specialization, instance, scriptContext);
            uint32 indexValue = fIndexIsUInt32 ? uint32Index : Js::JavascriptConversion::ToUInt32(index, scriptContext);

            *itemPresent = (indexValue < length) ? TRUE : FALSE;
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT VectorArray::GetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out Var *value, __out BOOL *itemPresent, bool fIndexIsUInt32, uint32 uint32Index)
    {
        IfNullReturnError(value, E_POINTER);
        *value = nullptr;
        IfNullReturnError(itemPresent, E_POINTER);
        *itemPresent = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        HRESULT hr = S_OK;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            Var indexVar = fIndexIsUInt32 ? Js::JavascriptNumber::ToVar(uint32Index, scriptContext) : index;
            *value = VectorArray::GetAt(specialization, instance, indexVar);
            if (Js::JavascriptOperators::GetTypeId(*value) != Js::TypeIds_Undefined)
            {
                *itemPresent = TRUE;
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT VectorArray::SetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __in Var value, __out BOOL *result, bool fIndexIsUInt32, uint32 uint32Index)
    {
        IfNullReturnError(result, E_POINTER);
        *result = false;

        Assert(IsVectorOrVectorView(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;

        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            if (IsVector(specialization))
            {
                uint length = Js::JavascriptConversion::ToUInt32(Js::JavascriptOperators::OP_GetLength(instance, scriptContext), scriptContext);
                uint32 uIndex = fIndexIsUInt32 ? uint32Index : Js::JavascriptConversion::ToUInt32(index, scriptContext);
                if (uIndex < length)
                {
                    VectorArray::SetAt(specialization, instance, index, value);
                    *result = TRUE;
                }
                else if (uIndex == length)
                {
                    VectorArray::Append(specialization, instance, value);
                    *result = TRUE;
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    HRESULT VectorArray::DeleteItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result, bool fIndexIsUInt32, uint32 uint32Index)
    {
        IfNullReturnError(result, E_POINTER);
        *result = FALSE;

        Assert(IsVectorOrVectorView(specialization));

        // Cant delete the item
        return S_OK;
    }

    HRESULT VectorArray::GetEnumerator( __in SpecialProjection * specialization, __in Var instance, __in IVarEnumerator *pPropertyEnumerator, __out IVarEnumerator **enumerator)
    {
        IfNullReturnError(enumerator, E_POINTER);
        *enumerator = nullptr;

        Assert(IsVectorOrVectorView(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();

        HRESULT hr = S_OK;

        // Create the VectorArrayEnumerator
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {  
            VectorArrayEnumerator *pVectorArrayEnumerator = HeapNew(VectorArrayEnumerator, specialization, instance, pPropertyEnumerator); 
            hr = pVectorArrayEnumerator->QueryInterface(__uuidof(IVarEnumerator), (void**)enumerator);            
            Assert(SUCCEEDED(hr));
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }

    // return true if the property corresponds to the 0 to lenght-1 
    HRESULT VectorArray::HasOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result)
    {
        IfNullReturnError(result, E_POINTER);
        *result = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        uint32 index = 0;
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return VectorArray::HasItem(specialization, instance, nullptr, result, true, index);
        }

        return E_BOUNDS;
    }

    HRESULT VectorArray::GetOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out Var *value, __out BOOL *propertyPresent)
    {
        IfNullReturnError(value, E_POINTER);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_POINTER);
        *propertyPresent = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        uint32 index = 0;
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return VectorArray::GetItem(specialization, instance, nullptr, value, propertyPresent, true, index);
        }

        return E_BOUNDS;
    }

    HRESULT VectorArray::SetProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __in Var value, __out BOOL *result)
    {
        IfNullReturnError(result, E_POINTER);
        *result = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        uint32 index = 0;
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return VectorArray::SetItem(specialization, instance, nullptr, value, result, true, index);
        }

        return E_BOUNDS;
    }

    // return true if the property corresponds to the 0 to lenght-1 
    HRESULT VectorArray::IsWritable(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result)
    {
        IfNullReturnError(result, E_POINTER);
        *result = FALSE;

        HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, result);
        if (SUCCEEDED(hr))
        {
            *result = IsVector(specialization);
        }

        return hr;
    }

    HRESULT VectorArray::GetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId, __out BOOL *result)
    {
        IfNullReturnError(result, E_POINTER);
        *result = FALSE;

        Assert(IsVectorOrVectorView(specialization));
        uint32 index = 0;
        if (specialization->projectionContext->GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            // we dont have any getter setter for any numeric properties
            return S_OK;
        }

        return E_BOUNDS;
    }

    HRESULT VectorArray::SetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId)
    {
        Assert(IsVectorOrVectorView(specialization));
        uint32 index = 0;
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            // we dont have any getter setter for any numeric properties
            return VBSERR_ActionNotSupported;
        }

        return E_BOUNDS;
    }
}
