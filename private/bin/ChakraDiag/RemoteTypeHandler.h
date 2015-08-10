//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct PROPERTY_INFO
    {
        CString name;
        enum
        {
            EMPTY = 0,

            DATA_PROPERTY,
            ACCESSOR_PROPERTY,

            BOOL_VALUE,
            INT_VALUE,
            UINT_VALUE,
            DOUBLE_VALUE,
            STRING_VALUE,
            POINTER_VALUE,

            // Most objects maintain simple PROPERTY_INFO lists and lazily create IJsDebugProperty objects when needed.
            // In case the info can't be recorded easily, create and wrap an IJsDebugProperty object so we can still put into PROPERTY_INFO lists.
            DEBUG_PROPERTY,

            VAR_VALUE = DATA_PROPERTY, // alias
        } type;
                
        union
        {
            struct
            {
                Js::Var data;   // Used for DATA_PROPERTY or VAR_VALUE
            };
            struct
            {
                Js::Var getter;
                Js::Var setter;
            };

            bool boolValue;
            int i4Value;
            unsigned int ui4Value;
            double dblValue;
            void* pointer;
        };
        CString strValue;       // Used for STRING_VALUE
        CComPtr<IJsDebugPropertyInternal> debugProperty;    // Used for DEBUG_PROPERTY, wraps an existing IJsDebugProperty object
        DWORD attr;

    public:
        PROPERTY_INFO(): type(EMPTY) {}

        explicit PROPERTY_INFO(const CString& name, Js::Var data, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(uint name, Js::Var data, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(Js::Var data, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, Js::Var getter, Js::Var setter, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(uint name, Js::Var getter, Js::Var setter, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const AutoMSHTMLDAC_PROPERTY& domProperty);

        explicit PROPERTY_INFO(uint name, bool value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, bool value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, int value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, UINT value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, double value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(const CString& name, const CString& value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(uint name, int32 value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(uint name, double value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(uint name, const CString& value, JS_PROPERTY_ATTRIBUTES attr = JS_PROPERTY_ATTRIBUTE_NONE);
        explicit PROPERTY_INFO(IJsDebugPropertyInternal* value);

        explicit PROPERTY_INFO(const CString& name, const PROPERTY_INFO& other);

        bool IsEmpty() const { return type == EMPTY; }
        bool HasData() const { return type == DATA_PROPERTY; }
        static CString FromBSTR(BSTR bstr) { return CString(bstr, ::SysStringLen(bstr)); }
        static CString FromUINT(uint name);
    };

    class ITypeHandler
    {
    public:
        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) = 0;
        virtual void EnumLetConstGlobals(IPropertyListener* listener) = 0;
        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) = 0;
        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop) = 0;
    };

    class ITypeHandlerFactory
    {
    public:
        virtual ITypeHandler* Create(InspectionContext* context, const DynamicTypeHandler* typeHandler, const DynamicObject* obj) = 0;
    };

    template <class T, class TDAC>
    class TypeHandlerFactory:
        public ITypeHandlerFactory
    {
    public:
        virtual ITypeHandler* Create(InspectionContext* context, const DynamicTypeHandler* typeHandler, const DynamicObject* obj)
        {
            return new(oomthrow) TDAC(context, static_cast<const T*>(typeHandler), obj);
        }

        static TypeHandlerFactory s_instance;
    };

    template <class T, class TDAC>
    TypeHandlerFactory<T, TDAC> TypeHandlerFactory<T, TDAC>::s_instance;

    //
    // Base class for type handlers
    //
    template <class T>
    class RemoteTypeHandler:
        public RemoteData<T>,
        public ITypeHandler
    {
    protected:
        RemoteDynamicObject m_obj; //TODO: not sure if this needs concreate type. How about ES5Array?
        const uint m_inlineSlotCapacity;
        const uint m_offsetOfInlineSlots;

    public:
        RemoteTypeHandler(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteData(context->GetReader(), remoteAddr),
            m_obj(context->GetReader(), obj),
            m_inlineSlotCapacity(ToTargetPtr()->GetInlineSlotCapacity()),
            m_offsetOfInlineSlots(ToTargetPtr()->GetOffsetOfInlineSlots())
        {
        }

        Js::Var ReadSlot(uint index)
        {
            return index < m_inlineSlotCapacity ?
                ReadSlot(m_obj.GetFieldAddr<Js::Var>(m_offsetOfInlineSlots), index) :
                ReadSlot(m_obj->auxSlots, index - m_inlineSlotCapacity);
        }

        virtual void EnumLetConstGlobals(IPropertyListener* listener) override
        {
            // Only legal on SimpleDictionaryTypeHandler and DictionaryTypeHandler
            Assert(false);
        }

    protected:
        CString ReadPropertyName(const PropertyRecord* propertyRecord) const
        {
            return InspectionContext::ReadPropertyName(GetReader(), propertyRecord);
        }

        template <class Descriptor>
        bool HasEnumerableProperty(const Descriptor& descriptor)
        {
            return HasPropertyCommon</*requireEnumerable*/true>(descriptor);
        }

        template <class Descriptor>
        bool HasProperty(const Descriptor& descriptor)
        {
            return HasPropertyCommon</*requireEnumerable*/false>(descriptor);
        }

        template <class Descriptor>
        bool HasProperty(const Descriptor& descriptor, bool requireEnumerable)
        {
            return requireEnumerable ? HasEnumerableProperty(descriptor) : HasProperty(descriptor);
        }

        bool IsInternalPropertyId(const PropertyRecord* propertyRecord);
        bool GetPropertyName(const PropertyRecord* propertyRecord, CString* name, Js::PropertyId* propertyId = nullptr);
        bool GetPropertyInfo(const CString& name, Js::Var data, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* = nullptr);
        bool GetPropertyInfo(const CString& name, uint slotIndex, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* = nullptr);
        bool GetPropertyInfo(const PropertyRecord* propertyRecord, Js::Var data, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* = nullptr);
        bool GetPropertyInfo(const PropertyRecord* propertyRecord, uint slotIndex, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* = nullptr);
        bool GetInternalPropertyInfo(Js::Var data, _Out_ PROPERTY_INFO* prop);
        bool GetInternalPropertyInfo(uint slotIndex, _Out_ PROPERTY_INFO* prop);

    private:
        Js::Var ReadSlot(Js::Var* slots, uint index)
        {
            return ReadVirtual<Js::Var>(slots + index);
        }

        template <bool requireEnumerable, class Descriptor>
        bool HasPropertyCommon(const Descriptor& descriptor)
        {
            Js::PropertyAttributes attribs = descriptor.Attributes;
            return !(attribs & PropertyDeleted) && (!requireEnumerable || (attribs & PropertyEnumerable));
        }
    };

    //
    // RemoteDynamicTypeHandler supports ReadSlot.
    //
    class RemoteDynamicTypeHandler:
        public RemoteTypeHandler<DynamicTypeHandler>
    {
    public:
        RemoteDynamicTypeHandler(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteTypeHandler(context, remoteAddr, obj)
        {
        }

        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) override
        {
            Assert(false);
        }

        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) override
        {
            Assert(false);
            return false;
        }

        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop)
        {
            Assert(false);
            return false;
        }
        bool IsObjectHeaderInlinedTypeHandler();
    };

    //
    // Handles NullTypeHandler
    //
    class RemoteNullTypeHandler:
        public ITypeHandler
    {
    public:
        RemoteNullTypeHandler(InspectionContext* context, const DynamicTypeHandler* typeHandler, const DynamicObject* obj)
        {
        }

        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) override
        {
        }

        virtual void EnumLetConstGlobals(IPropertyListener* listener) override
        {
        }

        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) override
        {
            return false;
        }

        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop)
        {
            return false;
        }
    };

    typedef RemoteNullTypeHandler RemoteNonProtoNullTypeHandler;
    typedef RemoteNullTypeHandler RemoteProtoNullTypeHandler;

    //
    // Handles SimpleTypeHandler
    //
    template<size_t size>
    class RemoteSimpleTypeHandler:
        public RemoteTypeHandler<SimpleTypeHandler<size>>
    {
        typedef SimplePropertyDescriptor DescriptorType;

    public:
        RemoteSimpleTypeHandler(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteTypeHandler(context, remoteAddr, obj)
        {
        }

        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) override;
        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) override;
        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop) override;

    private:
        template <class Fn>
        bool MapUntil(Fn fn);
    };
    typedef RemoteSimpleTypeHandler<1> RemoteSimpleTypeHandlerSize1;
    typedef RemoteSimpleTypeHandler<2> RemoteSimpleTypeHandlerSize2;
    typedef RemoteSimpleTypeHandler<3> RemoteSimpleTypeHandlerSize3;
    //
    // Handles PathTypeHandler variants
    //
    template <class T>
    class RemotePathTypeHandlerBase:
        public RemoteTypeHandler<T>
    {
    private:
        RemoteTypePath m_typePath;

    public:
        RemotePathTypeHandlerBase(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteTypeHandler(context, remoteAddr, obj),
            m_typePath(context->GetReader(), ToTargetPtr()->typePath)
        {
        }

        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) override;
        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) override;
        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop) override;
    };

    typedef RemotePathTypeHandlerBase<PathTypeHandlerBase>  RemoteSimplePathTypeHandler;
    typedef RemotePathTypeHandlerBase<PathTypeHandlerBase>  RemotePathTypeHandler;
    
    //
    // Base class for SimpleDictionaryTypeHandler/DictionaryTypeHandler
    //
    template <class T, class Sub>
    class RemoteDictionaryTypeHandlerCommon:
        public RemoteTypeHandler<T>
    {
    public:
        RemoteDictionaryTypeHandlerCommon(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteTypeHandler(context, remoteAddr, obj),
            m_propertyMap(context->GetReader(), this->ToTargetPtr()->propertyMap)
        {
        }

        virtual void EnumProperties(IPropertyListener* listener, bool requireEnumerable) override;
        virtual void EnumLetConstGlobals(IPropertyListener* listener) override;
        virtual bool GetProperty(Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop) override;
        virtual bool GetInternalProperty(const Js::PropertyRecord* propertyRecord, _Out_ PROPERTY_INFO* prop) override;

    protected:
        typedef typename TargetType::PropertyDescriptorMapType PropertyDescriptorMap;
        typedef typename PropertyDescriptorMap::ValueType DescriptorType;
        Sub* pThis() const { return (Sub*)this; }

        bool HasProperty(const DescriptorType& descriptor) { return descriptor.HasNonLetConstGlobal() && __super::HasProperty(descriptor); }
        bool HasProperty(const DescriptorType& descriptor, bool requireEnumerable) { return descriptor.HasNonLetConstGlobal() && __super::HasProperty(descriptor, requireEnumerable); }

        template <class Descriptor>
        bool HasLetConstGlobal(const Descriptor& descriptor)
        {
            return (descriptor.Attributes & PropertyLetConstGlobal) != 0;
        }

    private:
        RemoteDictionary<PropertyDescriptorMap> m_propertyMap;
    };

    //
    // Handles SimpleDictionaryTypeHandler
    //
    template <class TPropertyIndex, class TMapKey, bool IsNotExtensibleSupported>
    class RemoteSimpleDictionaryTypeHandlerBase:
        public RemoteDictionaryTypeHandlerCommon<SimpleDictionaryTypeHandlerBase<TPropertyIndex, TMapKey, IsNotExtensibleSupported>, RemoteSimpleDictionaryTypeHandlerBase<TPropertyIndex, TMapKey, IsNotExtensibleSupported>>
    {
    private:
        InspectionContext* m_context;

        static CString ReadJavascriptStringKey(InspectionContext* context, JavascriptString* str) { return context->ReadString(str); }
        static const PropertyRecord* ReadJavascriptStringKey(InspectionContext* context, const PropertyRecord* propertyRecord) { return propertyRecord; }

    public:
        RemoteSimpleDictionaryTypeHandlerBase(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteDictionaryTypeHandlerCommon(context, remoteAddr, obj),
            m_context(context)
        {
        }

        template <class PropertyKey>
        bool GetPropertyInfo(PropertyKey propertyKey, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId = nullptr);
        bool GetInternalPropertyInfo(const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop);
        template <class PropertyKey>
        bool GetLetConstGlobalPropertyInfo(PropertyKey propertyKey, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId = nullptr);
    };

    typedef RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, false>      RemoteSimpleDictionaryTypeHandler;
    typedef RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, true>       RemoteSimpleDictionaryTypeHandlerNotExtensible;
    typedef RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, false>   RemoteBigSimpleDictionaryTypeHandler;
    typedef RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, true>    RemoteBigSimpleDictionaryTypeHandlerNotExtensible;

    // SimpleDictionaryUnorderedTypeHandler derives from SimpleDictionaryTypeHandlerBase.  RemoteSimpleDictionaryTypeHandler can handle
    // named properties for SimpleDictionaryUnorderedTypeHandler.
    typedef RemoteSimpleDictionaryTypeHandler                   RemoteSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler;
    typedef RemoteSimpleDictionaryTypeHandlerNotExtensible      RemoteSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible;
    typedef RemoteBigSimpleDictionaryTypeHandler                RemoteBigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler;
    typedef RemoteBigSimpleDictionaryTypeHandlerNotExtensible   RemoteBigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible;

    typedef RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, JavascriptString*, false>      RemoteSimpleDictionaryUnorderedStringKeyedTypeHandler;
    typedef RemoteSimpleDictionaryTypeHandlerBase<PropertyIndex, JavascriptString*, true>       RemoteSimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible;
    typedef RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, JavascriptString*, false>   RemoteBigSimpleDictionaryUnorderedStringKeyedTypeHandler;
    typedef RemoteSimpleDictionaryTypeHandlerBase<BigPropertyIndex, JavascriptString*, true>    RemoteBigSimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible;

    //
    // Handles DictionaryTypeHandler
    //
    template <class T>
    class RemoteDictionaryTypeHandlerBase:
        public RemoteDictionaryTypeHandlerCommon<DictionaryTypeHandlerBase<T>, RemoteDictionaryTypeHandlerBase<T>>
    {
    public:
        RemoteDictionaryTypeHandlerBase(InspectionContext* context, const TargetType* remoteAddr, const DynamicObject* obj):
            RemoteDictionaryTypeHandlerCommon(context, remoteAddr, obj)
        {
        }

        bool GetPropertyInfo(const PropertyRecord* propertyRecord, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId = nullptr);
        bool GetInternalPropertyInfo(const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop);
        bool GetLetConstGlobalPropertyInfo(const PropertyRecord* propertyRecord, const DescriptorType& descriptor, _Out_ PROPERTY_INFO* prop, _Out_opt_ Js::PropertyId* propertyId = nullptr);
    };

    typedef RemoteDictionaryTypeHandlerBase<PropertyIndex>      RemoteDictionaryTypeHandler;
    typedef RemoteDictionaryTypeHandlerBase<BigPropertyIndex>   RemoteBigDictionaryTypeHandler;

    // ES5ArrayTypeHandler derives from DictionaryTypeHandler. RemoteDictionaryTypeHandler can handle named properties for ES5ArrayTypeHandler.
    typedef RemoteDictionaryTypeHandler     RemoteES5ArrayTypeHandler;
    typedef RemoteBigDictionaryTypeHandler  RemoteBigES5ArrayTypeHandler;

} // namespace JsDiag.