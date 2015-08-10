//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    PROPERTY_INFO::PROPERTY_INFO(const CString& name, Js::Var data, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::DATA_PROPERTY),
        data(data),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, Js::Var data, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::DATA_PROPERTY),
        data(data),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(Js::Var data, JS_PROPERTY_ATTRIBUTES attr):
        type(PROPERTY_INFO::DATA_PROPERTY),
        data(data),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, Js::Var getter, Js::Var setter, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::ACCESSOR_PROPERTY),
        getter(getter),
        setter(setter),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, Js::Var getter, Js::Var setter, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::ACCESSOR_PROPERTY),
        getter(getter),
        setter(setter),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const AutoMSHTMLDAC_PROPERTY& domProperty)
    {
        this->name = FromBSTR(domProperty.bstrName);
        this->attr = JS_PROPERTY_READONLY; //TODO: Update interface if needed

        switch (domProperty.dwType)
        {
        case MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_INT:
            this->type = INT_VALUE;
            this->i4Value = domProperty.i4Value;
            break;

        case MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_UINT:
            this->type = UINT_VALUE;
            this->ui4Value = domProperty.ui4Value;
            break;

        case MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_DBL:
            this->type = DOUBLE_VALUE;
            this->dblValue = domProperty.dblValue;
            break;

        case MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_BSTR:
            this->type = STRING_VALUE;
            this->strValue = FromBSTR(domProperty.bstrValue);
            break;

        case MSHTMLDAC_PROPERTY_TYPE::MSHTMLDAC_PROPERTY_TYPE_VAR:
            this->type = VAR_VALUE;
            this->data = domProperty.varValue;
            break;

        default:
            AssertMsg(false, "Unknown MSHTMLDAC_PROPERTY_TYPE");
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::UNKNOWN_MSHTMLDAC_PROPERTY_TYPE);
            break;
        }
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, bool value, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::BOOL_VALUE),
        boolValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, bool value, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::BOOL_VALUE),
        boolValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, int value, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::INT_VALUE),
        i4Value(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, UINT value, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::UINT_VALUE),
        ui4Value(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, double value, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::DOUBLE_VALUE),
        dblValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, const CString& value, JS_PROPERTY_ATTRIBUTES attr):
        name(name),
        type(PROPERTY_INFO::STRING_VALUE),
        strValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, int value, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::INT_VALUE),
        i4Value(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, double value, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::DOUBLE_VALUE),
        dblValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(uint name, const CString& value, JS_PROPERTY_ATTRIBUTES attr):
        name(FromUINT(name)),
        type(PROPERTY_INFO::STRING_VALUE),
        strValue(value),
        attr(attr)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(IJsDebugPropertyInternal* value):
        type(PROPERTY_INFO::DEBUG_PROPERTY),
        debugProperty(value)
    {
    }

    PROPERTY_INFO::PROPERTY_INFO(const CString& name, const PROPERTY_INFO& other)
    {
        *this = other;
        this->name = name;
    }

    CString PROPERTY_INFO::FromUINT(uint name)
    {
        const int MAX_LENGTH = 20; // maximum length of 64-bit value converted to base 10 string

        CString str;
        LPWSTR buf = str.GetBufferSetLength(MAX_LENGTH + 3); // Leading/ending [] and Null terminator
        *buf++ = L'[';
        if (_ui64tow_s(name, buf, MAX_LENGTH + 1, 10) == 0)
        {
            str.ReleaseBuffer();
            str += L']';
            return str;
        }
        
        return L"[.]";
    }
    
    bool RemoteDynamicTypeHandler::IsObjectHeaderInlinedTypeHandler()
    {
        return this->m_offsetOfInlineSlots == offsetof(DynamicObject, auxSlots);
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetPropertyName(const PropertyRecord* propertyRecord, CString* name, Js::PropertyId* propertyId)
    {
        if (propertyRecord)
        {
            RemotePropertyRecord remotePropertyRecord(m_reader, propertyRecord);
            Js::PropertyId recordPropertyId = remotePropertyRecord.GetPropertyId();
            if (Js::IsInternalPropertyId(recordPropertyId))
            {
                return false; // Skip internal property
            }

            if (propertyId != nullptr)
            {
                *propertyId = recordPropertyId;
            }

            *name = ReadPropertyName(propertyRecord);
        }

        return true;
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetPropertyInfo(const CString& name, Js::Var data, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId*)
    {
        *prop = PROPERTY_INFO(name, data);
        return true;
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetPropertyInfo(const CString& name, uint slotIndex, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId*)
    {
        *prop = PROPERTY_INFO(name, ReadSlot(slotIndex));
        return true;
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetPropertyInfo(const PropertyRecord* propertyRecord, Js::Var data, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        CString name;
        return GetPropertyName(propertyRecord, &name, propertyId)
            && GetPropertyInfo(name, data, prop);
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetPropertyInfo(const PropertyRecord* propertyRecord, uint slotIndex, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        CString name;
        return GetPropertyName(propertyRecord, &name, propertyId)
            && GetPropertyInfo(name, slotIndex, prop);
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetInternalPropertyInfo(Js::Var data, _Out_ PROPERTY_INFO* prop)
    {
        *prop = PROPERTY_INFO(data);
        return true;
    }

    template <class T>
    bool RemoteTypeHandler<T>::GetInternalPropertyInfo(uint slotIndex, _Out_ PROPERTY_INFO* prop)
    {
        return GetInternalPropertyInfo(ReadSlot(slotIndex), prop);
    }

    template<size_t size>
    template <class Fn>
    bool RemoteSimpleTypeHandler<size>::MapUntil(Fn fn)
    {
        const uint propertyCount = ToTargetPtr()->propertyCount;
        const auto& descriptors = ToTargetPtr()->descriptors;

        for(uint index = 0; index < propertyCount; ++index)
        {
            if (fn(descriptors[index], index))
            {
                return true;
            }
        }

        return false;
    }

    template<size_t size>
    void RemoteSimpleTypeHandler<size>::EnumProperties(IPropertyListener* listener, bool requireEnumerable)
    {
        MapUntil([=](const DescriptorType& descriptor, uint slotIndex) -> bool
        {
            if (HasProperty(descriptor, requireEnumerable))
            {
                PROPERTY_INFO prop;
                Js::PropertyId propertyId = Js::Constants::NoProperty;
                if (GetPropertyInfo(descriptor.Id, slotIndex, &prop, &propertyId))
                {
                    return !listener->EnumProperty(propertyId, prop);
                }
            }

            return false; // do not stop
        });
    }

    template<size_t size>
    bool RemoteSimpleTypeHandler<size>::GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop)
    {
        uint index = 0;
        return MapUntil([=](const DescriptorType& descriptor, uint slotIndex) -> bool
        {
            if (HasProperty(descriptor))
            {
                if (RemotePropertyRecord(m_reader, descriptor.Id)->GetPropertyId() == propertyId)
                {
                    return GetPropertyInfo(NULL, slotIndex, prop);
                }
            }

            return false;
        });
    }

    template<size_t size>
    bool RemoteSimpleTypeHandler<size>::GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop)
    {
        uint index = 0;
        return MapUntil([=](const DescriptorType& descriptor, uint slotIndex) -> bool
        {
            if (HasProperty(descriptor))
            {
                if (descriptor.Id == propertyRecord)
                {
                    return GetInternalPropertyInfo(slotIndex, prop);
                }
            }

            return false;
        });
    }

    template class RemoteSimpleTypeHandler<1>;
    template class RemoteSimpleTypeHandler<2>;
    template class RemoteSimpleTypeHandler<3>;

    template <class T>
    void RemotePathTypeHandlerBase<T>::EnumProperties(IPropertyListener* listener, bool requireEnumerable)
    {
        const uint pathLength = ToTargetPtr()->GetPathLength();
        for (uint index = 0; index < pathLength; index++)
        {
            PROPERTY_INFO prop;
            Js::PropertyId propertyId = Js::Constants::NoProperty;
            const PropertyRecord* propertyRecord = m_typePath[index].GetRemoteAddr();
            if (GetPropertyInfo(propertyRecord, index, &prop, &propertyId))
            {
                if (!listener->EnumProperty(propertyId, prop))
                {
                    break;
                }
            }
        }
    }

    template <class T>
    bool RemotePathTypeHandlerBase<T>::GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop)
    {
        const uint pathLength = ToTargetPtr()->GetPathLength();
        PropertyIndex index;
        if (m_typePath.TryLookup(propertyId, pathLength, &index))
        {
            return GetPropertyInfo(NULL, index, prop);
        }

        return false;
    }

    template <class T>
    bool RemotePathTypeHandlerBase<T>::GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop)
    {
        RemoteScriptContext scriptContext(m_reader, m_obj.GetScriptContext());
        RemotePropertyRecord remotePropertyRecord(m_reader, propertyRecord);
        const uint pathLength = ToTargetPtr()->GetPathLength();
        PropertyIndex index;
        if (m_typePath.TryLookup(remotePropertyRecord->GetPropertyId(), pathLength, &index))
        {
            return GetInternalPropertyInfo(index, prop);
        }

        return false;
    }

    template class RemotePathTypeHandlerBase<PathTypeHandlerBase>;

    template <class T, class Sub>
    void RemoteDictionaryTypeHandlerCommon<T, Sub>::EnumProperties(IPropertyListener* listener, bool requireEnumerable)
    {
        const uint propertyCount = m_propertyMap->Count();
        for (uint index = 0; index < propertyCount; ++index)
        {
            auto entry = m_propertyMap.Item(index);
            PROPERTY_INFO prop;

            Js::PropertyId propertyId = Js::Constants::NoProperty;
            if (pThis()->HasProperty(entry.value, requireEnumerable)
                && pThis()->GetPropertyInfo(entry.key, entry.value, &prop, &propertyId))
            {
                if (!listener->EnumProperty(propertyId, prop))
                {
                    break;
                }
            }
        }
    }

    template <class T, class Sub>
    void RemoteDictionaryTypeHandlerCommon<T, Sub>::EnumLetConstGlobals(IPropertyListener* listener)
    {
        const uint propertyCount = m_propertyMap->Count();
        for (uint index = 0; index < propertyCount; ++index)
        {
            auto entry = m_propertyMap.Item(index);
            PROPERTY_INFO prop;

            Js::PropertyId propertyId = Js::Constants::NoProperty;
            if (HasLetConstGlobal(entry.value)
                && pThis()->GetLetConstGlobalPropertyInfo(entry.key, entry.value, &prop, &propertyId))
            {
                if (!listener->EnumProperty(propertyId, prop))
                {
                    break;
                }
            }
        }
    }

    template <class T, class Sub>
    bool RemoteDictionaryTypeHandlerCommon<T, Sub>::GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop)
    {
        RemoteScriptContext scriptContext(m_reader, m_obj.GetScriptContext());
        RemotePropertyRecord propertyRecord(m_reader, scriptContext.GetPropertyName(propertyId));

        DescriptorType descriptor;
        if (m_propertyMap.TryGetValue(propertyRecord, &descriptor)
            && pThis()->HasProperty(descriptor))
        {
            return pThis()->GetPropertyInfo((const PropertyRecord*)NULL, descriptor, prop);
        }

        return false;
    }

    template <class T, class Sub>
    bool RemoteDictionaryTypeHandlerCommon<T, Sub>::GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop)
    {
        RemoteScriptContext scriptContext(m_reader, m_obj.GetScriptContext());
        RemotePropertyRecord remotePropertyRecord(m_reader, propertyRecord);

        DescriptorType descriptor;
        if (m_propertyMap.TryGetValue(remotePropertyRecord, &descriptor)
            && pThis()->HasProperty(descriptor))
        {
            return pThis()->GetInternalPropertyInfo(descriptor, prop);
        }

        return false;
    }

    template <class TPropertyIndex, class TMapKey, bool IsNotExtensibleSupported>
    template <class PropertyKey>
    bool RemoteSimpleDictionaryTypeHandlerBase<TPropertyIndex, TMapKey, IsNotExtensibleSupported>::GetPropertyInfo(
        PropertyKey propertyKey, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        Assert(!(descriptor.Attributes & PropertyLetConstGlobal));
        auto key = ReadJavascriptStringKey(m_context, propertyKey);
        if (descriptor.propertyIndex != TargetType::NoSlots)
        {
            return __super::GetPropertyInfo(key, descriptor.propertyIndex, prop, propertyId);
        }
        else
        {
            RemoteJavascriptLibrary lib(m_reader, m_obj.GetLibrary());
            return __super::GetPropertyInfo(key, lib.GetUndefined(), prop, propertyId);
        }
    }

    template <class TPropertyIndex, class TMapKey, bool IsNotExtensibleSupported>
    bool RemoteSimpleDictionaryTypeHandlerBase<TPropertyIndex, TMapKey, IsNotExtensibleSupported>::GetInternalPropertyInfo(
        const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop)
    {
        if (descriptor.propertyIndex != TargetType::NoSlots)
        {
            return __super::GetInternalPropertyInfo(descriptor.propertyIndex, prop);
        }
        else
        {
            RemoteJavascriptLibrary lib(m_reader, m_obj.GetLibrary());
            return __super::GetInternalPropertyInfo(lib.GetUndefined(), prop);
        }
    }

    template <class TPropertyIndex, class TMapKey, bool IsNotExtensibleSupported>
    template <class PropertyKey>
    bool RemoteSimpleDictionaryTypeHandlerBase<TPropertyIndex, TMapKey, IsNotExtensibleSupported>::GetLetConstGlobalPropertyInfo(
        PropertyKey propertyKey, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        Assert(descriptor.Attributes & PropertyLetConstGlobal);
        if (descriptor.propertyIndex != TargetType::NoSlots)
        {
            auto key = ReadJavascriptStringKey(m_context, propertyKey);
            return __super::GetPropertyInfo(key, descriptor.propertyIndex, prop, propertyId);
        }
        Assert(false);
        return false;
    }

    template class RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, false>;
    template class RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, true>;
    template class RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, false>;
    template class RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, true>;

    template class RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, JavascriptString*, false>;
    template class RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, JavascriptString*, true>;
    template class RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, JavascriptString*, false>;
    template class RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, JavascriptString*, true>;

    template <class TPropertyIndex>
    bool RemoteDictionaryTypeHandlerBase<TPropertyIndex>::GetPropertyInfo(const PropertyRecord* propertyRecord, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        Assert(!(descriptor.Attributes & PropertyLetConstGlobal) || descriptor.IsShadowed);
        uint dataSlotIndex = descriptor.GetDataPropertyIndex<false>();
        if (dataSlotIndex != TargetType::NoSlots)
        {
            return __super::GetPropertyInfo(propertyRecord, dataSlotIndex, prop, propertyId);
        }
        else
        {
            CString name;
            if (GetPropertyName(propertyRecord, &name, propertyId))
            {
                uint getterSlotIndex = descriptor.GetGetterPropertyIndex();
                uint setterSlotIndex = descriptor.GetSetterPropertyIndex();

                Js::Var getter = (getterSlotIndex != TargetType::NoSlots ? ReadSlot(getterSlotIndex) : NULL);
                Js::Var setter = (setterSlotIndex != TargetType::NoSlots ? ReadSlot(setterSlotIndex) : NULL);

                if (getter != NULL && setter != NULL)
                {
                    *prop = PROPERTY_INFO(name, getter, setter);
                    return true;
                }
                else
                {
                    RemoteJavascriptLibrary lib(m_reader, m_obj.GetLibrary());
                    return __super::GetPropertyInfo(name, lib.GetUndefined(), prop);
                }
            }
            
            return false;
        }
    }

    template <class TPropertyIndex>
    bool RemoteDictionaryTypeHandlerBase<TPropertyIndex>::GetInternalPropertyInfo(const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop)
    {
        if (descriptor.Data != TargetType::NoSlots)
        {
            return __super::GetInternalPropertyInfo(descriptor.Data, prop);
        }
        else
        {
            // Internal properties do not hold getters/setters
            Assert(false);
            return false;
        }
    }

    template <class TPropertyIndex>
    bool RemoteDictionaryTypeHandlerBase<TPropertyIndex>::GetLetConstGlobalPropertyInfo(const PropertyRecord* propertyRecord, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId)
    {
        Assert(descriptor.Attributes & PropertyLetConstGlobal);
        uint slotIndex = descriptor.Data;
        if (slotIndex != TargetType::NoSlots)
        {
            // Let/const globals are always stored in the Data slot
            return __super::GetPropertyInfo(propertyRecord, slotIndex, prop, propertyId);
        }
        Assert(false);
        return false;
    }

    template class RemoteDictionaryTypeHandlerBase<PropertyIndex>;
    template class RemoteDictionaryTypeHandlerBase<BigPropertyIndex>;

} // namespace JsDiag.

namespace Js
{
    // template specialization for remote dictionary lookup
    template<>
    struct PropertyRecordStringHashComparer<JsDiag::RemotePropertyRecord>
    {
        __inline static bool Equals(JavascriptString* str1, const JsDiag::RemotePropertyRecord& str2)
        {
            JsDiag::RemoteData<JavascriptString> remoteStr1(str2.GetReader(), str1);

            if (remoteStr1->m_charLength != str2->GetLength())
            {
                return false;
            }

            // We can read the string directly here because it is finalized when put
            // into the dictionary as a key.  This allows us to get away with making
            // the string comparison here without requiring an InspectionContext
            AssertMsg(remoteStr1->m_pszValue != NULL, "Expected strings stored as keys to be finalized");
            CString cstr1 = JsDiag::InspectionContext::ReadStringLen(str2.GetReader(), remoteStr1->m_pszValue, remoteStr1->m_charLength);
            CString cstr2 = JsDiag::InspectionContext::ReadStringLen(str2.GetReader(), str2.GetRemoteAddr()->GetBuffer(), str2->GetLength());

            return JsUtil::CharacterBuffer<WCHAR>::StaticEquals(cstr1, cstr2, remoteStr1->m_charLength);
        }

        __inline static bool Equals(const PropertyRecord* str1, const JsDiag::RemotePropertyRecord& str2)
        {
            return str1 == str2.GetRemoteAddr();
        }

        __inline static uint GetHashCode(const JsDiag::RemotePropertyRecord& str)
        {
            return str->GetHashCode();
        }
    };
}
