//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

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
        LoadPropertyId(L"implicitCallFlags", &this->implicitCallFlags);
        LoadPropertyId(L"loopCount", &this->loopCount);
        LoadPropertyId(L"loopImplicitCallFlags", &this->loopImplicitCallFlags);
        LoadPropertyId(L"returnTypeInfo", &this->returnTypeInfo);
        LoadPropertyId(L"parameterInfo", &this->parameterInfo);

        LoadPropertyId(L"ValueType", &this->propValueType);
        LoadPropertyId(L"ImplicitCallFlags", &this->propImplicitCallFlags);

        // Create arrays wrapping the profile data.
        CompileAssert(sizeof(ImplicitCallFlags) == sizeof(uint8));
        AddWrappedArray<uint8>(loopImplicitCallFlags, 
            (byte*)funcBody->GetAnyDynamicProfileInfo()->loopImplicitCallFlags, funcBody->GetLoopCount());
        AddWrappedArray<ValueType::TSize>(returnTypeInfo, 
            (byte*)funcBody->GetAnyDynamicProfileInfo()->returnTypeInfo, funcBody->GetProfiledReturnTypeCount());
        AddWrappedArray<ValueType::TSize>(parameterInfo, 
            (byte*)funcBody->GetAnyDynamicProfileInfo()->parameterInfo, funcBody->GetProfiledInParamsCount());

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
        #define VALUE_TYPE_BIT(t, b) SetConstantProperty(L"" STRINGIZEW(t) L"Bit", b, valueTypeObj);
        #include "..\..\Lib\Runtime\Language\ValueTypes.h"
        #undef VALUE_TYPE_BIT

        // Value type bit counts
        SetConstantProperty(L"VALUE_TYPE_COMMON_BIT_COUNT", VALUE_TYPE_COMMON_BIT_COUNT, valueTypeObj);
        SetConstantProperty(L"VALUE_TYPE_NONOBJECT_BIT_COUNT", VALUE_TYPE_NONOBJECT_BIT_COUNT, valueTypeObj);
        SetConstantProperty(L"VALUE_TYPE_OBJECT_BIT_COUNT", VALUE_TYPE_OBJECT_BIT_COUNT, valueTypeObj);

        DynamicObject::SetPropertyWithAttributes(
            propValueType, 
            valueTypeObj,
            PropertyAttributes_All & ~PropertyAttributes_Writable,
            NULL
            );

        // ImplicitCallFlags
        Var icf = jsLibrary->CreateObject();
        SetConstantProperty(L"HasNoInfo", ImplicitCall_HasNoInfo, icf);
        SetConstantProperty(L"None", ImplicitCall_None, icf);
        SetConstantProperty(L"ToPrimitive", ImplicitCall_ToPrimitive, icf);
        SetConstantProperty(L"Accessor", ImplicitCall_Accessor, icf);
        SetConstantProperty(L"External", ImplicitCall_External, icf);
        SetConstantProperty(L"Exception", ImplicitCall_Exception, icf);
        SetConstantProperty(L"All", ImplicitCall_All, icf);
        SetConstantProperty(L"Async", ImplicitCall_AsyncHostOperation, icf);
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

    BOOL ProfileDataObject::HasProperty(PropertyId propertyId)
    {
        if(propertyId == loopCount)
            return TRUE;
        if(propertyId == implicitCallFlags)
            return TRUE;

        return DynamicObject::HasProperty(propertyId);
    }

    BOOL ProfileDataObject::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return ProfileDataObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }
    BOOL ProfileDataObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if(propertyId == loopCount)
        {
            *value = Js::JavascriptNumber::ToVar(funcBody->GetLoopCount(), requestContext);
            return TRUE;
        }
        if(propertyId == implicitCallFlags)
        {
            *value = Js::JavascriptNumber::ToVar(funcBody->GetAnyDynamicProfileInfo()->GetImplicitCallFlags(), requestContext);
            return TRUE;
        }
        return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL ProfileDataObject::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return ProfileDataObject::GetProperty(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
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
