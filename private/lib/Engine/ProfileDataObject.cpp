//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
#if JS_PROFILE_DATA_INTERFACE
    ProfileDataObject::ProfileDataObject(DynamicType *type, FunctionBody *funcBody) : DynamicObject(type), funcBody(funcBody)
    {
        ScriptContext *scriptContext = this->GetScriptContext();

        // The function must have profile data to use this API.
        if(!funcBody->HasDynamicProfileInfo())
            Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);

        // Load the builtin property IDs for this object.
        LoadPropertyId(_u("implicitCallFlags"), &this->implicitCallFlags);
        LoadPropertyId(_u("loopCount"), &this->loopCount);
        LoadPropertyId(_u("loopImplicitCallFlags"), &this->loopImplicitCallFlags);
        LoadPropertyId(_u("returnTypeInfo"), &this->returnTypeInfo);
        LoadPropertyId(_u("parameterInfo"), &this->parameterInfo);

        LoadPropertyId(_u("ValueType"), &this->propValueType);
        LoadPropertyId(_u("ImplicitCallFlags"), &this->propImplicitCallFlags);

        // Create arrays wrapping the profile data.
        CompileAssert(sizeof(ImplicitCallFlags) == sizeof(uint8));
        AddWrappedArray<uint8>(loopImplicitCallFlags, 
            (byte*)PointerValue(funcBody->GetAnyDynamicProfileInfo()->loopImplicitCallFlags), funcBody->GetLoopCount());
        AddWrappedArray<ValueType::TSize>(returnTypeInfo, 
            (byte*)PointerValue(funcBody->GetAnyDynamicProfileInfo()->returnTypeInfo), funcBody->GetProfiledReturnTypeCount());
        AddWrappedArray<ValueType::TSize>(parameterInfo, 
            (byte*)PointerValue(funcBody->GetAnyDynamicProfileInfo()->parameterInfo), funcBody->GetProfiledInParamsCount());

        // Build the constants
        BuildConstants();
    }

    template<class T> 
    void ProfileDataObject::AddWrappedArray(PropertyId propertyId, byte *buffer, uint32 length)
    {
        JavascriptLibrary* jsLibrary = this->GetScriptContext()->GetLibrary();

        ArrayBuffer *arrayBuffer = RecyclerNew(
                        this->GetScriptContext()->GetRecycler(),
                        Js::ExternalArrayBuffer,
                        buffer,
                        length * sizeof(T),
                        jsLibrary->GetArrayBufferType());
        
        Var value = 
            TypedArray<T>::Create(arrayBuffer, 0, length, jsLibrary);

        // Set the array on the profile data object as a readonly property.
        DynamicObject::SetPropertyWithAttributes(
            propertyId, 
            value, 
            PropertyAttributes_All & ~PropertyAttributes_Writable,
            NULL
            );
    }

    void ProfileDataObject::LoadPropertyId(LPCWSTR str, PropertyId *propertyId)
    {
        ThreadContext *threadContext = this->GetScriptContext()->GetThreadContext();
        const PropertyRecord *propRecord;
        threadContext->GetOrAddPropertyId(str, wcslen(str), &propRecord);
        *propertyId = propRecord->GetPropertyId();
    }

    void ProfileDataObject::BuildConstants()
    {
        JavascriptLibrary* jsLibrary = this->GetScriptContext()->GetLibrary();

        // ValueType
        Var valueTypeObj = jsLibrary->CreateObject();
        ValueType::MapInitialValueTypesUntil([&](const ValueType valueType, const size_t) -> bool
        {
            wchar valueTypeStr[VALUE_TYPE_MAX_STRING_SIZE];
            valueType.ToString(valueTypeStr);
            SetConstantProperty(valueTypeStr, valueType.GetRawData(), valueTypeObj);
            return false;
        });

        // Value type bits
        #define VALUE_TYPE_BIT(t, b) SetConstantProperty(_u("") STRINGIZEW(t) _u("Bit"), b, valueTypeObj);
        #include "..\..\Lib\Runtime\Language\ValueTypes.h"
        #undef VALUE_TYPE_BIT

        // Value type bit counts
        SetConstantProperty(_u("VALUE_TYPE_COMMON_BIT_COUNT"), VALUE_TYPE_COMMON_BIT_COUNT, valueTypeObj);
        SetConstantProperty(_u("VALUE_TYPE_NONOBJECT_BIT_COUNT"), VALUE_TYPE_NONOBJECT_BIT_COUNT, valueTypeObj);
        SetConstantProperty(_u("VALUE_TYPE_OBJECT_BIT_COUNT"), VALUE_TYPE_OBJECT_BIT_COUNT, valueTypeObj);

        DynamicObject::SetPropertyWithAttributes(
            propValueType, 
            valueTypeObj,
            PropertyAttributes_All & ~PropertyAttributes_Writable,
            NULL
            );

        // ImplicitCallFlags
        Var icf = jsLibrary->CreateObject();
        SetConstantProperty(_u("HasNoInfo"), ImplicitCall_HasNoInfo, icf);
        SetConstantProperty(_u("None"), ImplicitCall_None, icf);
        SetConstantProperty(_u("ToPrimitive"), ImplicitCall_ToPrimitive, icf);
        SetConstantProperty(_u("Accessor"), ImplicitCall_Accessor, icf);
        SetConstantProperty(_u("External"), ImplicitCall_External, icf);
        SetConstantProperty(_u("Exception"), ImplicitCall_Exception, icf);
        SetConstantProperty(_u("All"), ImplicitCall_All, icf);
        SetConstantProperty(_u("Async"), ImplicitCall_AsyncHostOperation, icf);
        DynamicObject::SetPropertyWithAttributes(
            propImplicitCallFlags, 
            icf,
            PropertyAttributes_All & ~PropertyAttributes_Writable,
            NULL
            );
    }

    void ProfileDataObject::SetConstantProperty(LPCWSTR str, uint16 value, Var object)
    {
        ThreadContext *threadContext = this->GetScriptContext()->GetThreadContext();
        const PropertyRecord *propRecord;
        threadContext->GetOrAddPropertyId(str, wcslen(str), &propRecord);

        DynamicObject::FromVar(object)->SetPropertyWithAttributes(
            propRecord->GetPropertyId(), 
            JavascriptNumber::ToVar(value, this->GetScriptContext()), 
            PropertyAttributes_All & ~PropertyAttributes_Writable,
            NULL
            );
    }

    BOOL ProfileDataObject::IsEnumerable(PropertyId propertyId)
    {
        if(propertyId == loopCount)
            return TRUE;
        if(propertyId == implicitCallFlags)
            return TRUE;

        return DynamicObject::IsEnumerable(propertyId);
    }

    PropertyQueryFlags ProfileDataObject::HasPropertyQuery(PropertyId propertyId)
    {
        if(propertyId == loopCount)
            return PropertyQueryFlags::Property_Found;
        if(propertyId == implicitCallFlags)
            return PropertyQueryFlags::Property_Found;

        return DynamicObject::HasPropertyQuery(propertyId);
    }

    PropertyQueryFlags ProfileDataObject::GetPropertyReferenceQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return ProfileDataObject::GetPropertyQuery(originalInstance, propertyId, value, info, requestContext);
    }
    PropertyQueryFlags ProfileDataObject::GetPropertyQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if(propertyId == loopCount)
        {
            *value = Js::JavascriptNumber::ToVar(funcBody->GetLoopCount(), requestContext);
            return PropertyQueryFlags::Property_Found;
        }
        if(propertyId == implicitCallFlags)
        {
            *value = Js::JavascriptNumber::ToVar(funcBody->GetAnyDynamicProfileInfo()->GetImplicitCallFlags(), requestContext);
            return PropertyQueryFlags::Property_Found;
        }
        return DynamicObject::GetPropertyQuery(originalInstance, propertyId, value, info, requestContext);
    }

    PropertyQueryFlags ProfileDataObject::GetPropertyQuery(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return ProfileDataObject::GetPropertyQuery(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
    }

    BOOL ProfileDataObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        // TypedArrays provide a convenient way to give JS access to an array, however there's no easy way
        // to access a data member.  So, data members are hooked up directly via GetProperty/SetProperty.
        if(propertyId == loopCount)
            return FALSE;
        if(propertyId == implicitCallFlags)
        {
            if(TaggedInt::Is(value))
            {
                funcBody->GetAnyDynamicProfileInfo()->implicitCallFlags = (ImplicitCallFlags)TaggedInt::ToInt32(value);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        return DynamicObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL ProfileDataObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return ProfileDataObject::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
    }
#endif
}
