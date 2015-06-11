//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    void MissingPropertyTypeHandler::SetUndefinedPropertySlot(DynamicObject* instance)
    {
        Var * slots = reinterpret_cast<Var*>(reinterpret_cast<size_t>(instance) + sizeof(DynamicObject));
        slots[0] = instance->GetLibrary()->GetUndefined();
    }

    MissingPropertyTypeHandler::MissingPropertyTypeHandler() :
        DynamicTypeHandler(1, 1, (uint16)sizeof(DynamicObject)) {}

    PropertyId MissingPropertyTypeHandler::GetPropertyId(ScriptContext* scriptContext, PropertyIndex index)
    {
        return Constants::NoProperty;
    }

    PropertyId MissingPropertyTypeHandler::GetPropertyId(ScriptContext* scriptContext, BigPropertyIndex index)
    {
        return Constants::NoProperty;
    }

    BOOL MissingPropertyTypeHandler::FindNextProperty(ScriptContext* scriptContext, PropertyIndex& index, __out JavascriptString** propertyStringName,
        __out PropertyId* propertyId, __out PropertyAttributes* attributes, Type* type, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols)
    {
        return FALSE;
    }

    PropertyIndex MissingPropertyTypeHandler::GetPropertyIndex(PropertyRecord const* propertyRecord)
    {
        return 0;
    }

    bool MissingPropertyTypeHandler::GetPropertyEquivalenceInfo(PropertyRecord const* propertyRecord, PropertyEquivalenceInfo& info)
    {
        info.slotIndex = Constants::NoSlot;
        info.isWritable = false;
        return false;
    }

    bool MissingPropertyTypeHandler::IsObjTypeSpecEquivalent(const Type* type, const TypeEquivalenceRecord& record, uint& failedPropertyIndex)
    {
        failedPropertyIndex = 0;
        return false;
    }

    bool MissingPropertyTypeHandler::IsObjTypeSpecEquivalent(const Type* type, const EquivalentPropertyEntry *entry)
    {
        return false;
    }

    BOOL MissingPropertyTypeHandler::HasProperty(DynamicObject* instance, PropertyId propertyId, __out_opt bool *noRedecl)
    {
        if (noRedecl != nullptr)
        {
            *noRedecl = false;
        }

        return false;
    }


    BOOL MissingPropertyTypeHandler::HasProperty(DynamicObject* instance, JavascriptString* propertyNameString)
    {
        return false;
    }

    BOOL MissingPropertyTypeHandler::GetProperty(DynamicObject* instance, Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL MissingPropertyTypeHandler::GetProperty(DynamicObject* instance, Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetProperty(DynamicObject* instance, JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        Throw::FatalInternalError();
        return false;
    }

    DescriptorFlags MissingPropertyTypeHandler::GetSetter(DynamicObject* instance, PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyValueInfo::SetNoCache(info, instance);
        return None;
    }

    DescriptorFlags MissingPropertyTypeHandler::GetSetter(DynamicObject* instance, JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyValueInfo::SetNoCache(info, instance);
        return None;
    }

    BOOL MissingPropertyTypeHandler::DeleteProperty(DynamicObject* instance, PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        Throw::FatalInternalError();
        return false;
    }


    BOOL MissingPropertyTypeHandler::IsEnumerable(DynamicObject* instance, PropertyId propertyId)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::IsWritable(DynamicObject* instance, PropertyId propertyId)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::IsConfigurable(DynamicObject* instance, PropertyId propertyId)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetEnumerable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetWritable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetConfigurable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
        Throw::FatalInternalError();
        return false;
    }

    //
    // Set an attribute bit. Return true if change is made.
    //
    BOOL MissingPropertyTypeHandler::SetAttribute(DynamicObject* instance, int index, PropertyAttributes attribute)
    {
        Throw::FatalInternalError();
        return false;
    }

    //
    // Clear an attribute bit. Return true if change is made.
    //
    BOOL MissingPropertyTypeHandler::ClearAttribute(DynamicObject* instance, int index, PropertyAttributes attribute)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetAccessors(DynamicObject* instance, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::PreventExtensions(DynamicObject* instance)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::Seal(DynamicObject* instance)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::FreezeImpl(DynamicObject* instance, bool isConvertedType)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetPropertyWithAttributes(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::SetAttributes(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attributes)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::GetAttributesWithPropertyIndex(DynamicObject * instance, PropertyId propertyId, BigPropertyIndex index, PropertyAttributes * attributes)
    {
        Throw::FatalInternalError();
        return false;
    }

    BOOL MissingPropertyTypeHandler::AddProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        Throw::FatalInternalError();
        return false;
    }

    void MissingPropertyTypeHandler::SetAllPropertiesToUndefined(DynamicObject* instance, bool invalidateFixedFields)
    {
        Throw::FatalInternalError();
    }

    void MissingPropertyTypeHandler::MarshalAllPropertiesToScriptContext(DynamicObject* instance, ScriptContext* targetScriptContext, bool invalidateFixedFields)
    {
        Throw::FatalInternalError();
    }

    DynamicTypeHandler* MissingPropertyTypeHandler::ConvertToTypeWithItemAttributes(DynamicObject* instance)
    {
        Throw::FatalInternalError();
        return this;
    }

    void MissingPropertyTypeHandler::SetIsPrototype(DynamicObject* instance)
    {
        Throw::FatalInternalError();
    }

#if DBG
    bool MissingPropertyTypeHandler::CanStorePropertyValueDirectly(const DynamicObject* instance, PropertyId propertyId, bool allowLetConst)
    {
        Throw::FatalInternalError();
        return false;
    }
#endif
}