//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    class ATL_NO_VTABLE InspectionContext:
        public CComObjectRoot,
        public IUnknown
    {
    private:
        // Interface to access target memory
        CComPtr<JsDebugProcess> m_debugProcess;

        // vtable maps
        CAtlMap<VTABLE_PTR, ITypeHandlerFactory*> m_typeHandlerMap;
        CAtlMap<VTABLE_PTR, IRemoteStringFactory*> m_remoteStringMap;

        // mshtmldac
        AutoLibrary m_mshtmlDacLib;
        CComPtr<IMshtmlDac> m_mshtmlDac;
        Js::TypeId m_minMshtmlTypeId, m_maxMshtmlTypeId;

        // op stack to handle loops
        CAtlList<Js::Var> m_opStack;

        // locals inspection context
        AutoPtr<RemoteJavascriptLibrary> m_javascriptLibrary;
        enum { NONE, ENABLED, DISABLED } __proto__state;

    public:
        BEGIN_COM_MAP(InspectionContext)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        InspectionContext(): __proto__state(NONE) {}
        void Init(JsDebugProcess* process);

        IVirtualReader* GetReader() const { return m_debugProcess->GetReader(); }
        DebugClient* GetDebugClient() const { return m_debugProcess->GetDebugClient(); }
        const void* ReadVTable(Js::Var var) const;
        bool IsVTable(const void* vtable, Diag_VTableType first, Diag_VTableType last) const;
        bool MatchVTable(Js::Var var, Diag_VTableType vtable) const;

        ITypeHandler* CreateTypeHandler(const DynamicObject* var);
        IRemoteStringFactory* GetRemoteStringFactory(Js::Var var);
        Js::TypeId GetTypeId(Js::Var var) const;
        void CreateDebugProperty(const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent, _Out_ IJsDebugProperty** ppDebugProperty);
        void CreateDebugProperty(const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent, _Out_ IJsDebugPropertyInternal** ppDebugProperty);

        static bool IsStaticType(Js::TypeId typeId);
        static bool IsDynamicType(Js::TypeId typeId);
        static bool IsObjectType(Js::TypeId typeId);
        bool GetProperty(const Js::Var instance, Js::PropertyId propertyId, _Out_ PROPERTY_INFO* prop);

        static CString ReadString(IVirtualReader* reader, LPCWSTR addr, UINT maxLen);
        static void ReadStringLen(IVirtualReader* reader, const char16* addr, _Out_writes_all_(len) char16* buf, charcount_t len);
        static CString ReadStringLen(IVirtualReader* reader, const char16* addr, charcount_t len);
        static CString ReadPropertyName(IVirtualReader* reader, const PropertyRecord* propertyRecord);
        CString ReadPropertyName(const PropertyRecord* propertyRecord) const;

        bool ReadString(JavascriptString* s, _Out_writes_to_(bufLen, *actual) char16* buf, _In_ charcount_t bufLen, _Out_ charcount_t* actual);
        CString ReadString(JavascriptString* s);

        bool IsMshtmlObject(Js::TypeId typeId);
        IMshtmlDac* GetMshtmlDac() const { return m_mshtmlDac; }

        void PushObject(Js::Var var);
        Js::Var PopObject();
        bool CheckObject(Js::Var var);

        void InitLocalsContext(const ScriptContext* scriptContext);
        const RemoteJavascriptLibrary& GetJavascriptLibrary() const;
        void GetUndefinedProperty(_Outptr_ IJsDebugPropertyInternal** ppUndefined);
        bool is__proto__Enabled() const;
        void TryResolve__proto__Value(const RecyclableObject* instance, CAtlArray<PROPERTY_INFO>& items);
        const PCWSTR GetPrototypeDisplay() const;

        const PropertyRecord* GetInternalPropertyRecord(Js::PropertyId propertyId);

        __declspec(noreturn) void ThrowEvaluateNotSupported();

        //
        // Auto object to push var on stack and pop in destructor
        //
        class AutoOpStackObject
        {
        private:
            InspectionContext* m_context;
#if DBG
            Js::Var m_var;
#endif
        public:
            AutoOpStackObject(InspectionContext* context, Js::Var var);
            ~AutoOpStackObject();
        };

    private:
        void InitVTables(_In_reads_(vtablesSize) const VTABLE_PTR* vtables, ULONG vtablesSize);
        void CreateDataDebugProperty(const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent, _Out_ IJsDebugPropertyInternal** ppDebugProperty);
    };

    HRESULT PrivateCoCreate(HMODULE hModule, REFCLSID clsid, REFIID iid, LPVOID* ppUnk);

    // --------------------------------------------------------------------------------------------
    // Base DebugProperty with InspectionContext.
    //  T: Concrete subclass type
    // --------------------------------------------------------------------------------------------
    template <class T, class WalkerPolicy = NoWalkerPolicy>
    class ATL_NO_VTABLE InspectionProperty:
        public JsDebugProperty<T, WalkerPolicy>
    {
    protected:
        CComPtr<InspectionContext> m_context;

    public:
        void Init(InspectionContext* context, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            m_context = context;
            __super::Init(parent);
        }
    };

    // --------------------------------------------------------------------------------------------
    // Base variable property. Associated with a runtime Var or other data.
    // --------------------------------------------------------------------------------------------
    template <class T, class WalkerPolicy = NoWalkerPolicy>
    class ATL_NO_VTABLE VariableProperty:
        public InspectionProperty<T, WalkerPolicy>
    {
    protected:
        PROPERTY_INFO m_info;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            m_info = info;
            __super::Init(context, parent); // After setting m_info so that Name is available.
        }

        const CString& GetName() const
        {
            return m_info.name;
        }

        DWORD GetAttribute()
        {
            return m_info.attr;
        }

        bool_result TryCloneImpl(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const
        {
            m_context->CreateDebugProperty(PROPERTY_INFO(newName, m_info), nullptr, ppDebugProperty);
            return true;
        }
    };

    // --------------------------------------------------------------------------------------------
    // Simple property with known constant type/value string.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE SimpleProperty:
        public VariableProperty<SimpleProperty>
    {
    private:
        LPCWSTR m_type;
        LPCWSTR m_value;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, LPCWSTR type, LPCWSTR value, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            __super::Init(context, info, parent);
            m_type = type;
            m_value = value;
        }

        LPCWSTR GetType() const { return m_type; }
        LPCWSTR GetValue(UINT nRadix) const { return m_value; }

        virtual bool_result TryToIndex(_Out_ UINT* index, _Out_ CString* name) override;
    };

    // --------------------------------------------------------------------------------------------
    // Unknown property, display it, but no walk/evaluate.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE UnknownProperty:
        public SimpleProperty
    {
    public:
        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) override;
        virtual bool_result TryToIndex(_Out_ UINT* index, _Out_ CString* name) override { return false; }
    };

    // --------------------------------------------------------------------------------------------
    // JavascriptBoolean property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE JavascriptBooleanProperty:
        public SimpleProperty
    {
    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, bool value, _In_opt_ IJsDebugPropertyInternal* parent);
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);

        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) override;

        static LPCWSTR GetString(bool b);
    };

    // --------------------------------------------------------------------------------------------
    // JavascriptSymbol property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE JavascriptSymbolProperty:
        public VariableProperty<JavascriptSymbolProperty>
    {
    private:
        CString m_value;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, CString value, _In_opt_ IJsDebugPropertyInternal* parent);
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);
        LPCWSTR GetType() const { return _u("Symbol"); }
        const CString& GetValue(UINT nRadix) const { return m_value; }

        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) override;
        bool IsSymbolProperty() const { return true; }
    };

    // --------------------------------------------------------------------------------------------
    // Base DynamicObject property.
    // --------------------------------------------------------------------------------------------
    template <class T, class RemoteType, class Walker>
    class ATL_NO_VTABLE DynamicObjectProperty:
        public VariableProperty<T, HasWalkerPolicy<T, Walker>>
    {
    public:
        LPCWSTR GetType() const { return _u("Object"); }
        LPCWSTR GetValue(UINT nRadix) const { return _u("{...}"); }

        // Supports WalkerPolicy
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker)
        {
            CreateComObject(m_context, GetInstance(), pThis(), ppWalker);
            return true;
        }

        // ==== IJsDebugPropertyInternal ====
        virtual void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const override
        {
            RemoteDynamicObject obj(m_context->GetReader(), GetInstance());
            if (obj.HasObjectArray(m_context))
            {
                Js::Var arr = obj->GetObjectArrayOrFlagsAsArray();
                CComPtr<IJsDebugPropertyInternal> prop;
                m_context->CreateDebugProperty(PROPERTY_INFO(arr), /*parent*/nullptr, &prop);
                prop->EnumItems(listener, start, end, requireEnumerable);
            }
        }

        virtual void EnumProperties(IPropertyListener* listener, const OriginalObjectInfo& originalObject, bool requireEnumerable) const override
        {
            pThis()->EnumItems(listener, 0, JavascriptArray::MaxArrayLength, requireEnumerable);
            pThis()->EnumNonIndexProperties(listener, originalObject, requireEnumerable);
        }

    protected:
        void EnumNonIndexProperties(IPropertyListener* listener, const OriginalObjectInfo& originalObject, bool requireEnumerable) const
        {
            AutoPtr<ITypeHandler> typeHandler = m_context->CreateTypeHandler(GetInstance());
            if (typeHandler)
            {
                typeHandler->EnumProperties(listener, requireEnumerable);
            }
        }

        const typename RemoteType::TargetType* GetInstance() const
        {
            Assert(m_info.HasData());
            return reinterpret_cast<const RemoteType::TargetType*>(m_info.data);
        }

        void GetUndefinedValue(_Out_ BSTR* pValue) const { ToBSTR(_u("undefined"), pValue); }
        void GetUndefinedType(_Out_ BSTR* pValue) const { ToBSTR(_u("Undefined"), pValue); }
    };

    // --------------------------------------------------------------------------------------------
    // Base Object debug property, caches Type string.
    // --------------------------------------------------------------------------------------------
    template <class T, class RemoteType, class Walker>
    class ATL_NO_VTABLE BaseObjectProperty:
        public DynamicObjectProperty<T, RemoteType, Walker>
    {
    private:
        CString m_typeString; // Cached Type string, which is generally expensive to read.

    public:
        const CString& GetType()
        {
            if (m_typeString.IsEmpty())
            {
                m_typeString = pThis()->GetTypeString();
            }
            return m_typeString;
        }

    protected:
        const CString& GetTypeDirect() const { return m_typeString; }
        CString GetTypeString() const { return __super::GetType(); } // Overload to customize Type display
        void SetTypeString(LPCWSTR type) { m_typeString = type; }
    };

    // --------------------------------------------------------------------------------------------
    // Base Number property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE NumberProperty:
        public VariableProperty<NumberProperty>
    {
    private:
        double m_value;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, double value, _In_opt_ IJsDebugPropertyInternal* parent);

        LPCWSTR GetType() const { return _u("Number"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);

        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) override;

        static void GetValueBSTR(double value, UINT radix, _Out_ BSTR* pValue);

    protected:
        double GetValue() const { return m_value; }
        static BSTR ToStringRadix10(double value);
        static BSTR ToStringRadixHelper(double value, int radix);
        static BSTR ToStringNanOrInfiniteOrZero(double value, int radix);
    };

    // --------------------------------------------------------------------------------------------
    // TaggedInt property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE TaggedIntProperty:
        public NumberProperty
    {
    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, int value, _In_opt_ IJsDebugPropertyInternal* parent);
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);

        virtual bool_result TryToIndex(_Out_ UINT* index, _Out_ CString* name) override;
    };

    // --------------------------------------------------------------------------------------------
    // JavascriptNumber property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE JavascriptNumberProperty:
        public NumberProperty
    {
    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);
    };

    template <class T>
    class ATL_NO_VTABLE JavascriptTypedNumberProperty:
        public VariableProperty<JavascriptTypedNumberProperty<T>>
    {
    public:
        LPCWSTR GetType() const { return _u("Number, (Object)"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    typedef JavascriptTypedNumberProperty<JavascriptInt64Number> JavascriptInt64NumberProperty;
    typedef JavascriptTypedNumberProperty<JavascriptUInt64Number> JavascriptUInt64NumberProperty;

    // --------------------------------------------------------------------------------------------
    // JavascriptString property.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE JavascriptStringProperty:
        public VariableProperty<JavascriptStringProperty>
    {
    private:
        CString m_str;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);

        LPCWSTR GetType() const { return GetTypeString(); }
        const CString& GetValue() const { return m_str; }
        const CString& GetValue(UINT nRadix) const { return GetValue(); }

        virtual bool_result TryToIndex(_Out_ UINT* index, _Out_ CString* name) override;
        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) override;

        // Supports expression evaluation (delegated here after ToObject)
        bool_result TryGetBuiltInProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);
        bool_result TryGetItemDirect(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);

        static LPCWSTR GetTypeString() { return _u("String"); }

    private:
        UINT GetStringLength() const { return static_cast<UINT>(m_str.GetLength() - 2); }  // unquoted
    };

    // --------------------------------------------------------------------------------------------
    // JavascriptPointer property holds a native WinRT pointer.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE JavascriptPointerProperty:
        public VariableProperty<JavascriptPointerProperty>
    {
    public:
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    // --------------------------------------------------------------------------------------------
    // Property collection based property walker.
    // --------------------------------------------------------------------------------------------
    template <class T>
    class ATL_NO_VTABLE PropertyCollectionWalker:
        public PropertyWalker<T>
    {
    protected:
        CComPtr<InspectionContext> m_context;
        CAtlArray<PROPERTY_INFO> m_items;

    public:
        void Init(InspectionContext* context, IJsDebugPropertyInternal* ownerDebugProperty)
        {
            m_context = context;
            __super::Init(ownerDebugProperty);
        }

        uint GetCount() const
        {
            return static_cast<uint>(m_items.GetCount());
        }

        bool_result GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            if (index < GetCount())
            {
                m_context->CreateDebugProperty(m_items[index], GetOwnerDebugProperty(), ppDebugProperty);
                return true;
            }

            return false;
        }

        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            //TODO: binary search if sorted
            return FindProperty(m_items, name, [=](const PROPERTY_INFO& prop)
            {
                m_context->CreateDebugProperty(prop, GetOwnerDebugProperty(), ppDebugProperty);
            });
        }

        void InsertItem(const PROPERTY_INFO& info)
        {
            m_items.Add(info);
        }

        void Sort()
        {
            qsort_s(m_items.GetData(), m_items.GetCount(), sizeof(PROPERTY_INFO), ComparePropertyInfo, NULL);
        }
    };

    int __cdecl ComparePropertyInfo(_In_ void* context, _In_ const void* item1, _In_ const void* item2);

    template <class Func>
    bool FindProperty(const CAtlArray<PROPERTY_INFO>& arr, const CString& name, Func func)
    {
        //TODO: binary search if sorted
        return MapUntil(arr, [=](const PROPERTY_INFO& prop) -> bool
        {
            if (prop.name == name)
            {
                func(prop);
                return true;
            }
            return false;
        });
    }

    // --------------------------------------------------------------------------------------------
    // Simple property collection walker. No subclass customization of property walking.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE SimplePropertyCollectionWalker:
        public PropertyCollectionWalker<SimplePropertyCollectionWalker>
    {
    };

    //
    // The fake [Methods] group.
    //
    class ATL_NO_VTABLE MethodGroupProperty:
        public InspectionProperty<MethodGroupProperty, HasWalkerPolicy<MethodGroupProperty, SimplePropertyCollectionWalker>>
    {
    private:
        static const CString s_name;

    public:
        void Init(InspectionContext* context, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            __super::Init(context, parent);
            TryEnsureWalker(); // pre-create walker
        }

        // Supports WalkerPolicy
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker)
        {
            CreateComObject(m_context, pThis(), ppWalker);
            return true;
        }

        const CString& GetName() const { return s_name; }
        LPCWSTR GetValue(UINT nRadix) { return _u("{...}"); }
        bool HasChildren() { return true; }
        DWORD GetAttribute() { return JS_PROPERTY_FAKE | JS_PROPERTY_METHOD | JS_PROPERTY_READONLY; }

        void InsertItem(const PROPERTY_INFO& info) { GetWalker()->InsertItem(info); }
        void Sort() { GetWalker()->Sort(); }
    };

    //
    // The fake key/value pair walker for use by KeyValueProperty
    //
    class ATL_NO_VTABLE KeyValueWalker:
        public PropertyWalker<KeyValueWalker>
    {
    private:
        CComPtr<IJsDebugPropertyInternal> m_key;
        CComPtr<IJsDebugPropertyInternal> m_value;

    public:
        void Init(InspectionContext* context, Js::Var key, Js::Var value);
        IJsDebugPropertyInternal* GetKeyProperty() { return m_key; }
        IJsDebugPropertyInternal* GetValueProperty() { return m_value; }

        uint GetCount() const { return 2; }
        bool GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty);
    };

    //
    // The fake key/value pair group for use by MapWalker and WeakMapWalker
    //
    class ATL_NO_VTABLE KeyValueProperty:
        public JsDebugProperty<KeyValueProperty>
    {
    private:
        CString m_name;
        CComPtr<KeyValueWalker> m_walker;

    public:
        void Init(InspectionContext* context, const CString& name, Js::Var key, Js::Var value);

        const CString& GetName() const { return m_name; }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
        bool HasChildren() { return true; }
        DWORD GetAttribute() { return JS_PROPERTY_FAKE | JS_PROPERTY_READONLY; }

        bool_result TryGetEnumerator(_Outptr_ IJsEnumDebugProperty **ppEnum)
        {
            m_walker->CreateEnumerator(ppEnum);
            return true;
        }
    };

    //
    // The fake [Map] group's walker
    //
    class ATL_NO_VTABLE MapWalker:
        public SimpleDebugPropertyCollectionWalker
    {
    protected:
        CComPtr<InspectionContext> m_context;

    public:
        void Init(InspectionContext* context, const DynamicObject* map)
        {
            m_context = context;
            InsertProperties(static_cast<const Js::JavascriptMap*>(map));
        }

    protected:
        void InsertProperties(const Js::JavascriptMap* map);
    };

    //
    // The fake [Set] group's walker
    //
    class ATL_NO_VTABLE SetWalker:
        public SimplePropertyCollectionWalker
    {
    public:
        void Init(InspectionContext* context, const DynamicObject* set)
        {
            __super::Init(context, /*ownerDebugProperty*/nullptr);
            InsertProperties(static_cast<const Js::JavascriptSet*>(set));
        }

    protected:
        void InsertProperties(const Js::JavascriptSet* set);
    };

    //
    // The fake [WeakMap] group's walker
    //
    class ATL_NO_VTABLE WeakMapWalker:
        public SimpleDebugPropertyCollectionWalker
    {
    protected:
        CComPtr<InspectionContext> m_context;

    public:
        void Init(InspectionContext* context, const DynamicObject* weakMap)
        {
            m_context = context;
            InsertProperties(static_cast<const Js::JavascriptWeakMap*>(weakMap));
        }

    protected:
        void InsertProperties(const Js::JavascriptWeakMap* weakMap);
        const Js::JavascriptWeakMap::WeakMapKeyMap* GetWeakMapKeyMapFromKey(const DynamicObject* key);
    };

    //
    // The fake [WeakSet] group's walker
    //
    class ATL_NO_VTABLE WeakSetWalker:
        public SimplePropertyCollectionWalker
    {
    public:
        void Init(InspectionContext* context, const DynamicObject* weakSet)
        {
            __super::Init(context, /*ownerDebugProperty*/nullptr);
            InsertProperties(static_cast<const Js::JavascriptWeakSet*>(weakSet));
        }

    protected:
        void InsertProperties(const Js::JavascriptWeakSet* weakSet);
    };

    //
    // The fake [Map], [Set], [WeakMap], or [WeakSet] group.
    //
    template <class TWalker>
    class ATL_NO_VTABLE CollectionGroupProperty:
        public JsDebugProperty<CollectionGroupProperty<TWalker>>
    {
    private:
        static const CString s_name;
        CComPtr<TWalker> m_walker;

    public:
        void Init(InspectionContext* context, const DynamicObject* object)
        {
            CreateComObject(context, object, &m_walker);
        }

        const CString& GetName() const { return s_name; }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
        {
            CString value;
            value.Format(_u("size = %u"), m_walker->GetCount());
            *pValue = value.AllocSysString();
        }
        bool HasChildren() { return m_walker->GetCount() > 0; }
        DWORD GetAttribute() { return JS_PROPERTY_FAKE | JS_PROPERTY_READONLY; }

        bool_result TryGetEnumerator(_Outptr_ IJsEnumDebugProperty **ppEnum)
        {
            m_walker->CreateEnumerator(ppEnum);
            return true;
        }
    };

    template <> const CString CollectionGroupProperty<MapWalker>::s_name(_u("[Map]"));
    template <> const CString CollectionGroupProperty<SetWalker>::s_name(_u("[Set]"));
    template <> const CString CollectionGroupProperty<WeakMapWalker>::s_name(_u("[WeakMap]"));
    template <> const CString CollectionGroupProperty<WeakSetWalker>::s_name(_u("[WeakSet]"));

    typedef CollectionGroupProperty<MapWalker> MapGroupProperty;
    typedef CollectionGroupProperty<SetWalker> SetGroupProperty;
    typedef CollectionGroupProperty<WeakMapWalker> WeakMapGroupProperty;
    typedef CollectionGroupProperty<WeakSetWalker> WeakSetGroupProperty;

    //
    // Base DynamicObject property walker. Retrieves object properties and adds to collection at Init.
    //
    template <class T>
    class ATL_NO_VTABLE DynamicObjectWalker:
        public PropertyCollectionWalker<T>
    {
    public:
        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
        {
            __super::Init(context, ownerDebugProperty);
            pThis()->InsertProperties(var);

            if (DIAG_CONFIG_FLAG(EnumerateSpecialPropertiesInDebugger))
            {
                pThis()->InsertSpecialProperties(var);
            }
        }

        uint GetCount() const
        {
            return __super::GetCount() + pThis()->GetInternalArrayCount();
        }

        bool GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            if (index < __super::GetCount())
            {
                return __super::GetNextProperty(index, ppDebugProperty);
            }
            index -= __super::GetCount();

            return pThis()->TryGetInternalArrayItem(index, ppDebugProperty);
        }

    protected:
        void InsertProperties(const DynamicObject* var)
        {
            auto listener = MakePropertyListener([=](const PROPERTY_INFO& info, const Js::PropertyId propertyId) -> bool
            {
                bool isInDeadZone = false;
                if (pThis()->IsPropertyValid(propertyId, &isInDeadZone))
                {
                    if (isInDeadZone)
                    {
                        Assert(m_context);
                        Assert(m_context->GetJavascriptLibrary().IsUndeclBlockVar(info.data));
                        PROPERTY_INFO deadZoneInfo = info;
                        deadZoneInfo.data = m_context->GetJavascriptLibrary().GetDebuggerDeadZoneBlockVariableString();
                        pThis()->InsertItem(deadZoneInfo);
                    }
                    else
                    {
                        pThis()->InsertItem(info);
                    }
                }

                // Return true to ensure that all properties are enumerated and considered
                // for display.
                return true;
            });

            AutoPtr<ITypeHandler> typeHandler = m_context->CreateTypeHandler(var);
            if (typeHandler)
            {
                typeHandler->EnumProperties(&listener, pThis()->RequirePropertiesEnumerable());
            }
        }

        void InsertSpecialProperties(const DynamicObject* var)
        {
            // By default, no special properties are added.
        }

        template <class TValue>
        void InsertSpecialProperty(
            RemoteThreadContext* remoteThreadContext,
            const Js::RecyclableObject* instance,
            Js::PropertyId propertyId,
            TValue value)
        {
            Assert(remoteThreadContext);
            Assert(instance);
            Assert(propertyId != Js::Constants::NoProperty);

            auto reader = m_context->GetReader();

            const Js::PropertyRecord* propertyRecord = remoteThreadContext->GetPropertyName(propertyId);
            if (propertyRecord)
            {
                auto propertyName = m_context->ReadPropertyName(reader, propertyRecord);
                PROPERTY_INFO info(propertyName, value, JS_PROPERTY_READONLY);
                pThis()->InsertItem(info);
            }
            else
            {
                AssertMsg(false, "Failed to retrieve the special property.");
            }
        }

        bool RequirePropertiesEnumerable() const
        {
            return false; // By default does not require enumerable
        }

        uint GetInternalArrayCount() const
        {
            return 0;
        }

        bool_result TryGetInternalArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return false;
        }

        bool IsPropertyValid(const Js::PropertyId propertyId, bool *isInDeadZone)
        {
            Assert(isInDeadZone);

            // All properties are valid by default.
            *isInDeadZone = false;
            return true;
        }
    };

    //
    // Assembles [Methods] and sort properties by name.
    //
    template <class T>
    class ATL_NO_VTABLE GroupedDynamicObjectWalker:
        public DynamicObjectWalker<T>
    {
    protected:
        CComPtr<MethodGroupProperty> m_methodGroup;
        CAtlArray<CComPtr<IJsDebugPropertyInternal>> m_fakeGroups;

    public:
        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty);
        void InsertItem(const PROPERTY_INFO& info);
        uint GetCount() const;
        bool GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty);
        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty);
    };

    template <class T>
    void GroupedDynamicObjectWalker<T>::Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
    {
        __super::Init(context, var, ownerDebugProperty);

        Sort();
        if (m_methodGroup)
        {
            m_methodGroup->Sort();
        }
    }

    template <class T>
    void GroupedDynamicObjectWalker<T>::InsertItem(const PROPERTY_INFO& info)
    {
        if (info.HasData() && m_context->GetTypeId(info.data) == Js::TypeIds_Function)
        {
            if (!m_methodGroup)
            {
                CreateComObject(m_context, GetOwnerDebugProperty(), &m_methodGroup);

                IJsDebugPropertyInternal* pMethodGroup = m_methodGroup;
                m_fakeGroups.Add(pMethodGroup);
            }

            m_methodGroup->InsertItem(info);
        }
        else
        {
            __super::InsertItem(info);
        }
    }

    template <class T>
    uint GroupedDynamicObjectWalker<T>::GetCount() const
    {
        uint fakeGroupCount = static_cast<uint>(m_fakeGroups.GetCount());
        return fakeGroupCount + __super::GetCount();
    }

    template <class T>
    bool GroupedDynamicObjectWalker<T>::GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        uint fakeGroupCount = static_cast<uint>(m_fakeGroups.GetCount());
        if (index < fakeGroupCount)
        {
            CheckHR(m_fakeGroups[index].CopyTo(ppDebugProperty));
            return true;
        }

        index -= fakeGroupCount;
        return __super::GetNextProperty(index, ppDebugProperty);
    }

    template <class T>
    bool_result GroupedDynamicObjectWalker<T>::TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        if (__super::TryGetProperty(name, ppDebugProperty))
        {
            return true;
        }

        if (m_methodGroup && m_methodGroup->TryGetProperty(name, ppDebugProperty)) // We have some properties in [Methods] group
        {
            return true;
        }
        return false;
    }

    //
    // Base object property walker. Assembles [Methods], [prototype], and sort properties.
    //
    template <class T>
    class ATL_NO_VTABLE BaseObjectWalker:
        public GroupedDynamicObjectWalker<T>
    {
    private:
        CComPtr<IJsDebugPropertyInternal> m_prototypeProperty;

    public:
        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
        {
            __super::Init(context, var, ownerDebugProperty);

            auto prototype = pThis()->GetPrototype(var);
            if (context->GetTypeId(prototype) != Js::TypeIds_Null)
            {
                context->CreateDebugProperty(
                    PROPERTY_INFO(CString(context->GetPrototypeDisplay()), prototype, JS_PROPERTY_READONLY), //REVIEW: If marked fake, fullName cannot distinguish own/proto property
                    GetOwnerDebugProperty(),
                    &m_prototypeProperty);
                m_fakeGroups.Add(m_prototypeProperty);
            }
            else
            {
                context->TryResolve__proto__Value(var, m_items);
            }
        }

        bool_result TryGetPrototype(_Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            if (m_prototypeProperty)
            {
                CheckHR(m_prototypeProperty.CopyTo(ppDebugProperty));
                return true;
            }
            return false;
        }

    protected:
        RecyclableObject* GetPrototype(const DynamicObject* var) const
        {
            return RemoteRecyclableObject(m_context->GetReader(), var).GetPrototype();
        }
    };

    //
    // General object property walker. Inspects internal array.
    //
    template<class T>
    class ATL_NO_VTABLE ObjectWalker:
        public BaseObjectWalker<T>
    {
    private:
        CComPtr<IPropertyWalker> m_internalArrayWalker;

    public:
        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty);
        uint GetInternalArrayCount() const;
        bool_result TryGetInternalArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty);
        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override;
    };

    class ATL_NO_VTABLE JavascriptObjectWalker:
        public ObjectWalker<JavascriptObjectWalker>
    {
    };

    //
    // Represents JavascriptObject property.
    //
    class ATL_NO_VTABLE JavascriptObjectProperty:
        public BaseObjectProperty<JavascriptObjectProperty, RemoteDynamicObject, JavascriptObjectWalker>
    {
    public:
        CString GetTypeString() const;
    };

    template <typename TGroupProperty>
    class ATL_NO_VTABLE JavascriptCollectionWalker :
        public ObjectWalker<JavascriptCollectionWalker<TGroupProperty>>
    {
    public:
        void Init(InspectionContext* context, const DynamicObject* object, IJsDebugPropertyInternal* ownerDebugProperty)
        {
            __super::Init(context, object, ownerDebugProperty);

            CComPtr<TGroupProperty> group;
            CreateComObject(m_context, object, &group);

            IJsDebugPropertyInternal* pGroup = group;
            m_fakeGroups.Add(pGroup);
        }
    };

    typedef JavascriptCollectionWalker<MapGroupProperty> JavascriptMapWalker;
    typedef JavascriptCollectionWalker<SetGroupProperty> JavascriptSetWalker;
    typedef JavascriptCollectionWalker<WeakMapGroupProperty> JavascriptWeakMapWalker;
    typedef JavascriptCollectionWalker<WeakSetGroupProperty> JavascriptWeakSetWalker;

    //
    // Represents JavascriptMap property.
    //
    class ATL_NO_VTABLE JavascriptMapProperty :
        public BaseObjectProperty<JavascriptMapProperty, RemoteDynamicObject, JavascriptMapWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Map"); }
    };

    //
    // Represents JavascriptSet property.
    //
    class ATL_NO_VTABLE JavascriptSetProperty :
        public BaseObjectProperty<JavascriptSetProperty, RemoteDynamicObject, JavascriptSetWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Set"); }
    };

    //
    // Represents JavascriptWeakMap property.
    //
    class ATL_NO_VTABLE JavascriptWeakMapProperty :
        public BaseObjectProperty<JavascriptWeakMapProperty, RemoteDynamicObject, JavascriptWeakMapWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("WeakMap"); }
    };

    //
    // Represents JavascriptWeakSet property.
    //
    class ATL_NO_VTABLE JavascriptWeakSetProperty :
        public BaseObjectProperty<JavascriptWeakSetProperty, RemoteDynamicObject, JavascriptWeakSetWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("WeakSet"); }
    };

    //
    // Inspects remote JavascriptProxy Object
    //
    struct RemoteJavascriptProxyObject :
        public RemoteData<JavascriptProxy>
    {
    public:
        RemoteJavascriptProxyObject(IVirtualReader* reader, const TargetType* addr) :
            RemoteData<TargetType>(reader, addr)
        {
        }

        const DynamicObject* GetTargetObject()
        {
            return reinterpret_cast<DynamicObject*>(ToTargetPtr()->target);
        }
        const DynamicObject* GetHandlerObject()
        {
            return reinterpret_cast<DynamicObject*>(ToTargetPtr()->handler);
        }
    };

    //
    // JavascriptProxy property walker. ForIn-enumerate properties and adds to collection at Init.
    //
    class ATL_NO_VTABLE JavascriptProxyWalker :
        public GroupedDynamicObjectWalker<JavascriptProxyWalker>
    {
    public:
        void InsertProperties(const DynamicObject* var);
    };

    //
    // Represents JavascriptProxy property.
    //
    class ATL_NO_VTABLE JavascriptProxyProperty :
        public BaseObjectProperty<JavascriptProxyProperty, RemoteDynamicObject, JavascriptProxyWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Proxy"); }
    };

    class ATL_NO_VTABLE JavascriptPromiseWalker:
        public ObjectWalker<JavascriptPromiseWalker>
    {
    };

    //
    // Represents JavascriptPromise property.
    //
    class ATL_NO_VTABLE JavascriptPromiseProperty :
        public BaseObjectProperty<JavascriptPromiseProperty, RemoteDynamicObject, JavascriptPromiseWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Promise"); }
    };

    //
    // ExternalObject property walker. ForIn-enumerate properties and adds to collection at Init.
    //
    class ATL_NO_VTABLE ExternalObjectWalker:
        public GroupedDynamicObjectWalker<ExternalObjectWalker>
    {
    public:
        void InsertProperties(const DynamicObject* var);
    };

    //
    // Represents ExternalObject property.
    //
    class ATL_NO_VTABLE ExternalObjectProperty:
        public BaseObjectProperty<ExternalObjectProperty, RemoteExternalObject, ExternalObjectWalker>
    {
    private:
        Js::TypeId m_typeId;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, Js::TypeId typeId, _In_opt_ IJsDebugPropertyInternal* parent);
        CString GetTypeString() const;
        void EnumNonIndexProperties(IPropertyListener* listener, const OriginalObjectInfo& originalObject, bool requireEnumerable) const;
    };

    class ATL_NO_VTABLE JavascriptBooleanObjectWalker:
        public ObjectWalker<JavascriptBooleanObjectWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptBooleanObjectProperty:
        public DynamicObjectProperty<JavascriptBooleanObjectProperty, RemoteJavascriptBooleanObject, JavascriptBooleanObjectWalker>
    {
    public:
        static LPCWSTR GetDisplayType() { return _u("Boolean, (Object)"); }
        static DynamicObject* GetDefaultPrototype(const RemoteJavascriptLibrary& lib) { return lib.GetBooleanPrototype(); }

        LPCWSTR GetType() const { return GetDisplayType(); }
        LPCWSTR GetValue(UINT nRadix);
    };

    class ATL_NO_VTABLE JavascriptSymbolObjectWalker :
        public ObjectWalker<JavascriptSymbolObjectWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptSymbolObjectProperty :
        public DynamicObjectProperty<JavascriptSymbolObjectProperty, RemoteJavascriptSymbolObject, JavascriptSymbolObjectWalker>
    {
    public:
        static LPCWSTR GetDisplayType() { return _u("Symbol, (Object)"); }
        static DynamicObject* GetDefaultPrototype(const RemoteJavascriptLibrary& lib) { return lib.GetSymbolPrototype(); }

        LPCWSTR GetType() const { return GetDisplayType(); }
        LPCWSTR GetValue(UINT nRadix);
    };

    class ATL_NO_VTABLE JavascriptNumberObjectWalker:
        public ObjectWalker<JavascriptNumberObjectWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptNumberObjectProperty:
        public DynamicObjectProperty<JavascriptNumberObjectProperty, RemoteJavascriptNumberObject, JavascriptNumberObjectWalker>
    {
    public:
        static LPCWSTR GetDisplayType() { return _u("Number, (Object)"); }
        static DynamicObject* GetDefaultPrototype(const RemoteJavascriptLibrary& lib) { return lib.GetNumberPrototype(); }

        LPCWSTR GetType() const { return GetDisplayType(); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    class ATL_NO_VTABLE JavascriptStringObjectWalker:
        public ObjectWalker<JavascriptStringObjectWalker>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    class ATL_NO_VTABLE JavascriptStringObjectProperty:
        public DynamicObjectProperty<JavascriptStringObjectProperty, RemoteJavascriptStringObject, JavascriptStringObjectWalker>
    {
    private:
        CComPtr<JavascriptStringProperty> m_value;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);

        static LPCWSTR GetDisplayType() { return JavascriptStringProperty::GetTypeString(); }
        static DynamicObject* GetDefaultPrototype(const RemoteJavascriptLibrary& lib) { return lib.GetStringPrototype(); }

        LPCWSTR GetType() const { return GetDisplayType(); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);

        // Supports expression evaluation
        bool_result TryGetBuiltInProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);
        bool_result TryGetItemDirect(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);

        // ==== IJsDebugPropertyInternal ====
        void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const override;
    };

    class ATL_NO_VTABLE JavascriptFunctionWalker:
        public ObjectWalker<JavascriptFunctionWalker>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    class ATL_NO_VTABLE JavascriptFunctionProperty:
        public DynamicObjectProperty<JavascriptFunctionProperty, RemoteJavascriptFunction, JavascriptFunctionWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Object, (Function)"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);

        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            __super::Init(context, info, parent);
            m_info.attr |= JS_PROPERTY_METHOD;
        }
    };

    class ATL_NO_VTABLE JavascriptRegExpConstructorWalker:
        public ObjectWalker<JavascriptRegExpConstructorWalker>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    class ATL_NO_VTABLE JavascriptRegExpConstructorProperty:
        public DynamicObjectProperty<JavascriptRegExpConstructorProperty, RemoteRuntimeFunction, JavascriptRegExpConstructorWalker>
    {
    public:
        LPCWSTR GetType() const { return JS_DIAG_TYPE_JavascriptRegExpConstructor; }
        LPCWSTR GetValue(UINT nRadix) const { return JS_DIAG_VALUE_JavascriptRegExpConstructor; }

        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            __super::Init(context, info, parent);
            m_info.attr |= JS_PROPERTY_METHOD;
        }
    };

    class ATL_NO_VTABLE JavascriptErrorWalker:
        public ObjectWalker<JavascriptErrorWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptErrorProperty:
        public DynamicObjectProperty<JavascriptErrorProperty, RemoteJavascriptError, JavascriptErrorWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Error"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    class ATL_NO_VTABLE JavascriptDateWalker:
        public ObjectWalker<JavascriptDateWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptDateProperty:
        public DynamicObjectProperty<JavascriptDateProperty, RemoteJavascriptDate, JavascriptDateWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Object, (Date)"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    class ATL_NO_VTABLE JavascriptVariantDateProperty:
        public VariableProperty<JavascriptVariantDateProperty>
    {
    public:
        LPCWSTR GetType() const { return JS_DISPLAY_STRING_DATE; }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    class ATL_NO_VTABLE JavascriptRegExpWalker:
        public ObjectWalker<JavascriptRegExpWalker>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    class ATL_NO_VTABLE JavascriptRegExpProperty:
        public DynamicObjectProperty<JavascriptRegExpProperty, RemoteJavascriptRegExp, JavascriptRegExpWalker>
    {
    public:
        LPCWSTR GetType() const { return JS_DIAG_TYPE_JavascriptRegExp; }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    //
    // Base Array property walker. Displays index properties before normal (non-index) object properties hierarchy.
    //
    template <class T>
    class ATL_NO_VTABLE BaseArrayWalker:
        public BaseObjectWalker<T>
    {
    protected:
        uint m_arrayLength;
        uint m_arrayItemsCount; // subclass is responsible to set this at Init

    public:
        void Init(InspectionContext* context, const DynamicObject* var, _In_ IJsDebugPropertyInternal* ownerDebugProperty)
        {
            __super::Init(context, var, ownerDebugProperty);
            RemoteJavascriptArray arr(context->GetReader(), static_cast<const JavascriptArray*>(var));
            m_arrayLength = arr->length;
        }

        uint GetCount() const
        {
            return pThis()->GetArrayItemsCount() + pThis()->GetNonIndexPropertyCount();
        }

        bool GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            if (index < pThis()->GetNonIndexPropertyCount())
            {
                return pThis()->GetNonIndexProperty(index, ppDebugProperty);
            }
            else
            {
                index -= pThis()->GetNonIndexPropertyCount();
                return pThis()->GetNextArrayItem(index, ppDebugProperty);
            }
        }

        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            if (__super::TryGetProperty(name, ppDebugProperty))
            {
                return true;
            }

            if (name == _u("length")) // Supports evaluation of array.length. It may not be listed by walker.
            {
                m_context->CreateDebugProperty(PROPERTY_INFO(name, m_arrayLength), GetOwnerDebugProperty(), ppDebugProperty);
                return true;
            }
            return false;
        }

        void InsertSpecialProperties(const DynamicObject* var);

    protected:
        uint GetArrayItemsCount() const
        {
            return m_arrayItemsCount;
        }

        template <class Fn>
        bool GetNextArrayItem(uint index, Fn fn)
        {
            if (index < m_arrayItemsCount)
            {
                fn();
                return true;
            }
            return false;
        }

        uint GetNonIndexPropertyCount() const
        {
            return __super::GetCount();
        }

        bool GetNonIndexProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return __super::GetNextProperty(index, ppDebugProperty);
        }
    };

    //
    // Enumerator based Array (JavascriptArray or ES5Array) property walker.
    //
    template <class T, class Array, template <bool requireEnumerable> class Enumerator>
    class ATL_NO_VTABLE EnumeratorArrayWalker:
        public BaseArrayWalker<T>
    {
    public:
        typedef Array ArrayType;

        template <bool requireEnumerable>
        struct ArrayEnumerator
        {
            typedef Enumerator<requireEnumerable> EnumeratorType;
        };

    protected:
        typename ArrayEnumerator</*requireEnumerable*/false>::EnumeratorType m_iter;
        uint m_next;
        bool m_isInternalArray;

    public:
        EnumeratorArrayWalker() :
            m_isInternalArray(false)
        {
        }

        void Init(InspectionContext* context, const DynamicObject* var, _In_ IJsDebugPropertyInternal* ownerDebugProperty);

        uint GetNonIndexPropertyCount() const
        {
            return m_isInternalArray ? 0 : BaseObjectWalker<T>::GetCount();
        }

        bool_result GetNonIndexProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return !m_isInternalArray && BaseObjectWalker<T>::GetNextProperty(index, ppDebugProperty);
        }

        bool_result GetNextArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty);

        // === IPropertyWalker ===
        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override;

        virtual void SetIsInternalArray(_In_ IJsDebugPropertyInternal* outerDebugProperty) override
        {
            __super::SetIsInternalArray(outerDebugProperty);
            m_isInternalArray = true;
        }
    };

    //
    // A small array listener object to help retrieve array items and assemble array display value.
    //
    struct SmallArrayListener: IPropertyListener
    {
        static const uint MAX_SMALL_ARRAY_SIZE = 10;

        PROPERTY_INFO props[MAX_SMALL_ARRAY_SIZE];
        uint count;     // count of retrieved properties (from array and prototype chain)
        uint length;    // array length

        SmallArrayListener(uint length);

        // === IPropertyListener ===
        bool EnumItem(uint index, const PROPERTY_INFO& info) override;

        void FillFromPrototypes(InspectionContext* context, const RecyclableObject* instance);
        CString Join(InspectionContext* context);
    };

    //
    // Base object for Array property.
    //
    template <class T, class RemoteType, class Walker>
    class ATL_NO_VTABLE BaseArrayProperty:
        public DynamicObjectProperty<T, RemoteType, Walker>
    {
    private:
        CString m_valueString;

    public:
        LPCWSTR GetType() const { return _u("Object, (Array)"); }

        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
        {
            if (m_valueString.IsEmpty())
            {
                m_valueString = GetValueString();
            }
            *pValue = m_valueString.AllocSysString();
        }

        // ==== IJsDebugPropertyInternal ====
        void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const override;

    private:
        CString GetValueString() const;

        template <bool requireEnumerable>
        void EnumItems(IPropertyListener* listener, uint start, uint end) const;
    };

    //
    // RemoteArrayElementEnumerator walks JavascriptArray sparse segments.
    //
    template<typename T>
    class RemoteArrayElementEnumerator
    {
    private:
        InspectionContext* m_context;
        const JavascriptArray* m_arr;

        uint32 m_start, m_end;
        AutoPtr<RemoteSparseArraySegment<T>> m_seg;
        uint32 m_left, m_index, m_endIndex;
        T m_item;

    public:
        void Init(InspectionContext* context, const JavascriptArray* arr, uint32 start = 0, uint32 end = JavascriptArray::MaxArrayLength);

        void Reset();
        bool MoveNext();
        uint32 GetIndex() const;
        T GetItem() const;

        bool HasItem() const { return !SparseArraySegment<T>::IsMissingItem(&m_item); }
        PROPERTY_INFO GetPropertyInfo() const { return PROPERTY_INFO(GetIndex(), GetItem()); }
    };

    //
    // Stub template, ignoring requireEnumerable because JavascriptArray items are all enumerable.
    //
    template <bool requireEnumerable>
    class RemoteJavascriptArrayItemEnumerator:
        public RemoteArrayElementEnumerator<Js::Var>
    {
    };

    template <bool requireEnumerable>
    class RemoteJavascriptNativeIntArrayItemEnumerator:
        public RemoteArrayElementEnumerator<int32>
    {
    };

    template <bool requireEnumerable>
    class RemoteJavascriptNativeFloatArrayItemEnumerator:
        public RemoteArrayElementEnumerator<double>
    {
    };

    //
    // JavascriptArray property walker.
    //
    class ATL_NO_VTABLE JavascriptArrayWalker:
        public EnumeratorArrayWalker<JavascriptArrayWalker, JavascriptArray, RemoteJavascriptArrayItemEnumerator>
    {
    };

    class ATL_NO_VTABLE JavascriptNativeIntArrayWalker:
        public EnumeratorArrayWalker<JavascriptNativeIntArrayWalker, JavascriptNativeIntArray, RemoteJavascriptNativeIntArrayItemEnumerator>
    {
    };

    class ATL_NO_VTABLE JavascriptNativeFloatArrayWalker:
        public EnumeratorArrayWalker<JavascriptNativeFloatArrayWalker, JavascriptNativeFloatArray, RemoteJavascriptNativeFloatArrayItemEnumerator>
    {
    };

    //
    // Represents JavascriptArray property.
    //
    class ATL_NO_VTABLE JavascriptArrayProperty:
        public BaseArrayProperty<JavascriptArrayProperty, RemoteJavascriptArray, JavascriptArrayWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptNativeIntArrayProperty:
        public BaseArrayProperty<JavascriptNativeIntArrayProperty, RemoteJavascriptNativeIntArray, JavascriptNativeIntArrayWalker>
    {
    };

    class ATL_NO_VTABLE JavascriptNativeFloatArrayProperty:
        public BaseArrayProperty<JavascriptNativeFloatArrayProperty, RemoteJavascriptNativeFloatArray, JavascriptNativeFloatArrayWalker>
    {
    };

    //
    // ES5Array item enumerator helper. This class collects ES5ArrayTypeHandler index property descriptors
    // and sort them, allowing walking the descriptors by index order.
    //
    class RemoteIndexPropertyMapEnumerator
    {
    private:
        typedef IndexPropertyDescriptorMap::InnerMap::EntryType EntryType;
        CAtlArray<EntryType> m_items;
        uint m_startIndex, m_endIndex;
        uint m_cur;

    public:
        void Init(IVirtualReader* reader, const IndexPropertyDescriptorMap* map, uint start, uint end);

        void Reset();
        bool MoveNext();
        uint32 GetIndex() const;
        const IndexPropertyDescriptor& GetItem() const;
        PROPERTY_INFO GetPropertyInfo() const;

    private:
        static int __cdecl CompareEntry(_In_ void* context, _In_ const void* item1, _In_ const void* item2);
    };

    //
    // Combines sparse Array segments and index descriptor info and enumerates ES5Array items.
    //
    template <bool requireEnumerable>
    class RemoteES5ArrayItemEnumerator
    {
    private:
        RemoteArrayElementEnumerator<Js::Var> m_dataEnumerator;
        RemoteIndexPropertyMapEnumerator m_descriptorEnumerator;
        uint32 m_dataIndex;                       // Current data index
        uint32 m_descriptorIndex;                 // Current descriptor index
        uint32 m_index;

    public:
        void Init(InspectionContext* context, const ES5Array* arr, uint32 start = 0, uint32 end = JavascriptArray::MaxArrayLength);

        void Reset(bool fullReset = true);
        bool MoveNext();
        uint32 GetIndex() const;
        PROPERTY_INFO GetPropertyInfo() const;
    };

    //
    // JavascriptArray property walker.
    //
    class ATL_NO_VTABLE ES5ArrayWalker:
        public EnumeratorArrayWalker<ES5ArrayWalker, ES5Array, RemoteES5ArrayItemEnumerator>
    {
    };

    //
    // Represents ES5Array property.
    //
    class ATL_NO_VTABLE ES5ArrayProperty:
        public BaseArrayProperty<ES5ArrayProperty, RemoteES5Array, ES5ArrayWalker>
    {
    };

    //
    // Specializes TypedArray data type attributes
    //
    template <class T, bool clamped = false>
    struct TypedArrayTrace
    {
        static const LPCWSTR NAME;
        static const LPCWSTR VALUE;

        // This implementation is shared by all Number types
        static PROPERTY_INFO FromData(uint index, T value)
        {
            return PROPERTY_INFO(index, static_cast<double>(value));
        }

        static bool_result TryGetBuiltInProperty(InspectionContext* context, RemoteTypedArray<T, clamped>& arr,
            const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            if (name == _u("length"))
            {
                context->CreateDebugProperty(PROPERTY_INFO(name, arr.ToTargetPtr()->GetLength()), nullptr, ppDebugProperty);
                return true;
            }
            if (name == _u("byteLength"))
            {
                context->CreateDebugProperty(PROPERTY_INFO(name, arr.ToTargetPtr()->GetByteLength()), nullptr, ppDebugProperty);
                return true;
            }
            if (name == _u("byteOffset"))
            {
                context->CreateDebugProperty(PROPERTY_INFO(name, arr.ToTargetPtr()->GetByteOffset()), nullptr, ppDebugProperty);
                return true;
            }
            if (name == _u("BYTES_PER_ELEMENT"))
            {
                context->CreateDebugProperty(PROPERTY_INFO(name, arr.ToTargetPtr()->GetBytesPerElement()), nullptr, ppDebugProperty);
                return true;
            }
            if (name == _u("buffer"))
            {
                context->CreateDebugProperty(PROPERTY_INFO(name, arr.ToTargetPtr()->GetArrayBuffer()), nullptr, ppDebugProperty);
                return true;
            }
            return false;
        }
    };

    PROPERTY_INFO TypedArrayTrace<bool>::FromData(uint index, bool value)
    {
        return PROPERTY_INFO(index, value);
    }

    template <class T, bool clamped>
    const LPCWSTR TypedArrayTrace<T, clamped>::NAME   = _u("Object");
    const LPCWSTR TypedArrayTrace<int8>::NAME         = _u("Object, (Int8Array)");
    const LPCWSTR TypedArrayTrace<uint8, false>::NAME = _u("Object, (Uint8Array)");
    const LPCWSTR TypedArrayTrace<uint8, true>::NAME  = _u("Object, (Uint8ClampedArray)");
    const LPCWSTR TypedArrayTrace<int16>::NAME        = _u("Object, (Int16Array)");
    const LPCWSTR TypedArrayTrace<uint16>::NAME       = _u("Object, (Uint16Array)");
    const LPCWSTR TypedArrayTrace<int32>::NAME        = _u("Object, (Int32Array)");
    const LPCWSTR TypedArrayTrace<uint32>::NAME       = _u("Object, (Uint32Array)");
    const LPCWSTR TypedArrayTrace<float>::NAME        = _u("Object, (Float32Array)");
    const LPCWSTR TypedArrayTrace<double>::NAME       = _u("Object, (Float64Array)");

    template <class T, bool clamped>
    const LPCWSTR TypedArrayTrace<T, clamped>::VALUE = _u("[object]");
    const LPCWSTR TypedArrayTrace<int8>::VALUE = _u("[object Int8Array]");
    const LPCWSTR TypedArrayTrace<uint8, false>::VALUE = _u("[object Uint8Array]");
    const LPCWSTR TypedArrayTrace<uint8, true>::VALUE = _u("[object Uint8ClampedArray]");
    const LPCWSTR TypedArrayTrace<int16>::VALUE = _u("[object Int16Array]");
    const LPCWSTR TypedArrayTrace<uint16>::VALUE = _u("[object Uint16Array]");
    const LPCWSTR TypedArrayTrace<int32>::VALUE = _u("[object Int32Array]");
    const LPCWSTR TypedArrayTrace<uint32>::VALUE = _u("[object Uint32Array]");
    const LPCWSTR TypedArrayTrace<float>::VALUE = _u("[object Float32Array]");
    const LPCWSTR TypedArrayTrace<double>::VALUE = _u("[object Float64Array]");

    // To be consistent with runtime, not type specialized
    struct CharArrayTrace: TypedArrayTrace<char16>
    {
        static PROPERTY_INFO FromData(uint index, char16 value)
        {
            return PROPERTY_INFO(index, CString(value));
        }
    };

    //
    // BaseTypedArrayWalker property walker.
    //
    template <class T, class Data, bool clamped = false, class RemoteArray = RemoteTypedArray<Data, clamped>, class Trace = TypedArrayTrace<Data, clamped>>
    class ATL_NO_VTABLE BaseTypedArrayWalker:
        public BaseArrayWalker<T>
    {
    private:
        AutoPtr<RemoteArray> m_typedArray;

    public:
        typedef Data DataType;
        typedef RemoteArray RemoteArrayType;
        typedef typename RemoteArray::TargetType TargetType;
        typedef Trace TraceType;

        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
        {
            __super::Init(context, var, ownerDebugProperty);

            m_typedArray = new(oomthrow) RemoteArray(context->GetReader(), static_cast<const TargetType*>(var));
            m_arrayItemsCount = m_typedArray->GetLength();
        }

        bool GetNextArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return __super::GetNextArrayItem(index, [=]
            {
                DataType data = m_typedArray->Item(index);
                PROPERTY_INFO info = TraceType::FromData(index, data);
                m_context->CreateDebugProperty(info, GetOwnerDebugProperty(), ppDebugProperty);
            });
        }

        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            return Trace::TryGetBuiltInProperty(m_context, *m_typedArray, name, ppDebugProperty)
                || __super::TryGetProperty(name, ppDebugProperty);
        }

        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            // TypedArray is always dense. Can directly index into array.
            if (pThis()->GetNextArrayItem(index, ppDebugProperty))
            {
                return true;
            }

            m_context->GetUndefinedProperty(ppDebugProperty); // TypedArray returns undefined reading passing its length.
            return true;
        }
    };

    //
    // TypedArray property walker.
    //
    template <class Data, bool clamped = false, class RemoteArray = RemoteTypedArray<Data, clamped>, class Trace = TypedArrayTrace<Data, clamped>>
    class ATL_NO_VTABLE TypedArrayWalker:
        public BaseTypedArrayWalker<TypedArrayWalker<Data, clamped, RemoteArray, Trace>, Data, clamped, RemoteArray, Trace>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    //
    // TypedArray property.
    //
    template <class DataType, bool clamped = false, class Walker = TypedArrayWalker<DataType, clamped>>
    class ATL_NO_VTABLE TypedArrayProperty:
        public DynamicObjectProperty<TypedArrayProperty<DataType, clamped, Walker>, typename Walker::RemoteArrayType, Walker>
    {
    public:
        typedef typename Walker::RemoteArrayType RemoteArrayType;
        typedef typename Walker::TraceType TraceType;

        LPCWSTR GetType() const { return TraceType::NAME; }
        LPCWSTR GetValue(UINT nRadix) const { return TraceType::VALUE; }

        // ==== IJsDebugPropertyInternal ====
        virtual void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const override
        {
            RemoteArrayType typedArray(m_context->GetReader(), GetInstance());

            end = min(end, typedArray.GetLength());
            for (uint i = start; i < end; i++)
            {
                DataType data = typedArray.Item(i);
                PROPERTY_INFO info = TraceType::FromData(i, data);
                if (!listener->EnumItem(i, info))
                {
                    break;
                }
            }
        }
    };

    typedef TypedArrayProperty<int8>         Int8ArrayProperty;
    typedef TypedArrayProperty<uint8, false> Uint8ArrayProperty;
    typedef TypedArrayProperty<uint8, true>  Uint8ClampedArrayProperty;
    typedef TypedArrayProperty<int16>        Int16ArrayProperty;
    typedef TypedArrayProperty<uint16>       Uint16ArrayProperty;
    typedef TypedArrayProperty<int32>        Int32ArrayProperty;
    typedef TypedArrayProperty<uint32>       Uint32ArrayProperty;
    typedef TypedArrayProperty<float>        Float32ArrayProperty;
    typedef TypedArrayProperty<double>       Float64ArrayProperty;
    typedef TypedArrayProperty<int64>        Int64ArrayProperty;
    typedef TypedArrayProperty<uint64>       Uint64ArrayProperty;
    typedef TypedArrayProperty<bool>         BoolArrayProperty;

    typedef TypedArrayProperty<char16, false, TypedArrayWalker<char16, false, RemoteTypedArray<char16, false>, CharArrayTrace>>
        CharArrayProperty;

    class ATL_NO_VTABLE ArrayBufferWalker :
        public ObjectWalker<ArrayBufferWalker>
    {
    public:
        void InsertSpecialProperties(const DynamicObject* var);
    };

    //
    // ArrayBufferProperty
    //
    class ATL_NO_VTABLE ArrayBufferProperty:
        public DynamicObjectProperty<ArrayBufferProperty, RemoteArrayBuffer, ArrayBufferWalker>
    {
    public:
        LPCWSTR GetType() const { return _u("Object, (ArrayBuffer)"); }
        LPCWSTR GetValue(UINT nRadix) const { return _u("[object ArrayBuffer]"); }
    };

    //
    // Inspects remote HeapArgumentsObject
    //
    struct RemoteHeapArgumentsObject:
        public RemoteData<HeapArgumentsObject>
    {
    private:
        AutoPtr<RemoteBVSparse<Recycler>> m_deletedArgs;

    public:
        RemoteHeapArgumentsObject(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr)
        {
            if (ToTargetPtr()->deletedArgs)
            {
                m_deletedArgs = new(oomthrow) RemoteBVSparse<Recycler>(reader, ToTargetPtr()->deletedArgs);
            }
        }

        void GetNamedItems(InspectionContext* context, CAtlArray<PROPERTY_INFO>& arr);

    private:
        const DynamicObject* GetFrameObject()
        {
            return reinterpret_cast<DynamicObject*>(ToTargetPtr()->frameObject);
        }
    };

    //
    // ArgumentsObject property walker. Inspects arguments array.
    //
    class ATL_NO_VTABLE ArgumentsObjectWalker:
        public BaseObjectWalker<ArgumentsObjectWalker>
    {
    private:
        CComPtr<SimplePropertyCollectionWalker> m_args;

    public:
        void Init(InspectionContext* context, const DynamicObject* var, IJsDebugPropertyInternal* prop);
        uint GetInternalArrayCount() const;
        bool TryGetInternalArrayItem(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty);
        void InsertSpecialProperties(const DynamicObject* var);

        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override;
    };

    class ATL_NO_VTABLE ArgumentsObjectProperty:
        public DynamicObjectProperty<ArgumentsObjectProperty, RemoteArgumentsObject, ArgumentsObjectWalker>
    {
    public:
        static LPCWSTR GetDisplayName() { return _u("arguments"); }
        static LPCWSTR GetDisplayType() { return _u("Object, (Arguments)"); }

        LPCWSTR GetType() const { return GetDisplayType(); }

        // Supports WalkerPolicy
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker)
        {
            CreateComObject(m_context, GetInstance(), this, ppWalker);
            return true;
        }

        // ==== IJsDebugPropertyInternal ====
        virtual void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const override;
    };

    class ATL_NO_VTABLE FakeObjectWalker;

    //
    // Fake object property
    //
    class ATL_NO_VTABLE FakeObjectProperty:
        public BaseObjectProperty<FakeObjectProperty, RemoteDynamicObject, FakeObjectWalker>
    {
    private:
        RecyclableObject* m_prototype;
        CAtlArray<PROPERTY_INFO> m_properties;
        CAtlArray<PROPERTY_INFO> m_items;

    public:
        void Init(InspectionContext* context, const CString& displayName, LPCWSTR displayType, RecyclableObject* prototype, _In_opt_ IJsDebugPropertyInternal* parent);
        void Init(const CString& newName, const FakeObjectProperty& other);

        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue) { this->GetDisplayValueBSTR(nRadix, pValue); }
        bool_result TryCreateWalker(_Outptr_  WalkerType** ppWalker);
        bool_result TryCloneImpl(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const;

        void AddProperty(const PROPERTY_INFO& info) { m_properties.Add(info); }
        void AddItem(const PROPERTY_INFO& info) { m_items.Add(info); }

        RecyclableObject* GetPrototype() const { return m_prototype; }
        const CAtlArray<PROPERTY_INFO>& GetProperties() const { return m_properties; }
        const CAtlArray<PROPERTY_INFO>& GetItems() const { return m_items; }

    protected:
        virtual void GetDisplayValueBSTR(UINT nRadix, _Out_ BSTR* pValue) { __super::GetValueBSTR(nRadix, pValue); }
    };

    //
    // Fake object property walker
    //
    class ATL_NO_VTABLE FakeObjectWalker:
        public BaseObjectWalker<FakeObjectWalker>
    {
    private:
        FakeObjectProperty* m_fakeObject; // Walker owned by Property, no back reference

    public:
        FakeObjectWalker() : m_fakeObject(nullptr) {}
        void Init(InspectionContext* context, FakeObjectProperty* fakeObject);

        RecyclableObject* GetPrototype(const DynamicObject* var) const;
        void InsertProperties(const DynamicObject* var);
        uint GetInternalArrayCount() const;
        bool_result TryGetInternalArrayItem(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty);

        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override;
    };

    //
    // A property representing ToObject from a primitive property.
    //
    template <class PrimitiveType, class ObjectType>
    class ATL_NO_VTABLE FakeToObjectProperty:
        public FakeObjectProperty
    {
    private:
        CComPtr<PrimitiveType> m_primitive;

    public:
        void Init(InspectionContext* context, PrimitiveType* primitive, _In_opt_ IJsDebugPropertyInternal* parent)
        {
            __super::Init(context, primitive->GetName(), ObjectType::GetDisplayType(),
                ObjectType::GetDefaultPrototype(context->GetJavascriptLibrary()), parent);
            m_primitive = primitive;
        }

        // Try to get a child property of this object. Used by expression evaluation to do property lookup by string name.
        virtual bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return m_primitive->TryGetProperty(name, ppDebugProperty);
        }

        // Try to get a child item property of this object. Used by expression evaluation to do property lookup by index name.
        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return m_primitive->TryGetItem(index, ppDebugProperty);
        }

    protected:
        virtual void GetDisplayValueBSTR(UINT nRadix, _Out_ BSTR* pValue) override
        {
            m_primitive->GetValueBSTR(nRadix, pValue);
        }
    };

    //
    // SIMD walker
    //
    template <class simdT, const uint elementCount, Js::TypeId tid>
    class ATL_NO_VTABLE SIMDWalker :
        public PropertyWalker<simdT>
    {
    protected:
        CComPtr<IJsDebugPropertyInternal> m_property[elementCount];

    public:
        void Init(InspectionContext* context, Js::Var simd);
        IJsDebugPropertyInternal* GetPropertyByIndex(int index) { return m_property[index]; }

        uint GetCount() const { return elementCount; }
        bool GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            if (index < elementCount)
            {
                *ppDebugProperty = GetPropertyByIndex(index);
                (*ppDebugProperty)->AddRef();
                return true;
            }
            return false;
        }
    };

    //
    // Concrete JavascriptSimdFloat32x4Walker
    //
    class ATL_NO_VTABLE JavascriptSimdFloat32x4Walker : 
        public SIMDWalker<JavascriptSimdFloat32x4Walker, 4, Js::TypeIds_SIMDFloat32x4>
    {
    };

    //
    // Concrete JavascriptSimdInt32x4Walker
    //
    class ATL_NO_VTABLE JavascriptSimdInt32x4Walker :
        public SIMDWalker<JavascriptSimdInt32x4Walker, 4, Js::TypeIds_SIMDInt32x4>
    {
    };

    //
    // Concrete JavascriptSimdInt8x16Walker
    //
    class ATL_NO_VTABLE JavascriptSimdInt8x16Walker :
        public SIMDWalker<JavascriptSimdInt8x16Walker, 16, Js::TypeIds_SIMDInt8x16>
    {
    };

    //
    // Concrete JavascriptSimdInt16x8Walker
    //
    class ATL_NO_VTABLE JavascriptSimdInt16x8Walker :
        public SIMDWalker<JavascriptSimdInt16x8Walker, 8, Js::TypeIds_SIMDInt16x8>
    {
    };

    //
    // Concrete JavascriptSimdBool32x4Walker
    //
    class ATL_NO_VTABLE JavascriptSimdBool32x4Walker :
        public SIMDWalker<JavascriptSimdBool32x4Walker, 4, Js::TypeIds_SIMDBool32x4>
    {
    };

    //
    // Concrete JavascriptSimdBool8x16Walker
    //
    class ATL_NO_VTABLE JavascriptSimdBool8x16Walker :
        public SIMDWalker<JavascriptSimdBool8x16Walker, 16, Js::TypeIds_SIMDBool8x16>
    {
    };

    //
    // Concrete JavascriptSimdBool16x8Walker
    //
    class ATL_NO_VTABLE JavascriptSimdBool16x8Walker :
        public SIMDWalker<JavascriptSimdBool16x8Walker, 8, Js::TypeIds_SIMDBool16x8>
    {
    };

    //
    // Concrete JavascriptSimdUint32x4Walker
    //
    class ATL_NO_VTABLE JavascriptSimdUint32x4Walker :
        public SIMDWalker<JavascriptSimdUint32x4Walker, 4, Js::TypeIds_SIMDUint32x4>
    {
    };

    //
    // Concrete JavascriptSimdUint8x16Walker
    //
    class ATL_NO_VTABLE JavascriptSimdUint8x16Walker :
        public SIMDWalker<JavascriptSimdUint8x16Walker, 16, Js::TypeIds_SIMDUint8x16>
    {
    };

    //
    // Concrete JavascriptSimdUint16x8Walker
    //
    class ATL_NO_VTABLE JavascriptSimdUint16x8Walker :
        public SIMDWalker<JavascriptSimdUint16x8Walker, 8, Js::TypeIds_SIMDUint16x8>
    {
    };

    //
    // SIMD property
    //
    template <class T, typename Walker, typename simdType, typename remoteSimd>
    class ATL_NO_VTABLE SIMDProperty :
        public VariableProperty<T>
    {
    protected:
        SIMDValue m_value;
        CComPtr<Walker> m_walker;

    public:
        void Init(InspectionContext* context, const PROPERTY_INFO& info, _In_opt_ IJsDebugPropertyInternal* parent);

        bool HasChildren() { return true; }
        DWORD GetAttribute() { return JS_PROPERTY_FAKE | JS_PROPERTY_READONLY; }

        bool_result TryGetEnumerator(_Outptr_ IJsEnumDebugProperty **ppEnum)
        {
            m_walker->CreateEnumerator(ppEnum);
            return true;
        }

        SIMDValue GetValue() const { return m_value; }

        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    //
    // Concrete JavascriptSimdInt32x4Property
    //
    class ATL_NO_VTABLE JavascriptSimdInt32x4Property :
        public SIMDProperty<JavascriptSimdInt32x4Property, JavascriptSimdInt32x4Walker, Js::JavascriptSIMDInt32x4, RemoteJavascriptSimdInt32x4>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Int32x4"); }
    };

    //
    // Concrete JavascriptSimdFloat32x4Property
    //
    class ATL_NO_VTABLE JavascriptSimdFloat32x4Property :
        public SIMDProperty<JavascriptSimdFloat32x4Property, JavascriptSimdFloat32x4Walker, Js::JavascriptSIMDFloat32x4, RemoteJavascriptSimdFloat32x4>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Float32x4"); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);
    };

    //
    // Concrete JavascriptSimdInt8x16Property
    //
    class ATL_NO_VTABLE JavascriptSimdInt8x16Property :
        public SIMDProperty<JavascriptSimdInt8x16Property, JavascriptSimdInt8x16Walker, Js::JavascriptSIMDInt8x16, RemoteJavascriptSimdInt8x16>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Int8x16"); }
    };

    //
    // Concrete JavascriptSimdInt16x8Property
    //
    class ATL_NO_VTABLE JavascriptSimdInt16x8Property :
        public SIMDProperty<JavascriptSimdInt16x8Property, JavascriptSimdInt16x8Walker, Js::JavascriptSIMDInt16x8, RemoteJavascriptSimdInt16x8>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Int16x8"); }
    };

    //
    // Concrete JavascriptSimdBool32x4Property
    //
    class ATL_NO_VTABLE JavascriptSimdBool32x4Property :
        public SIMDProperty<JavascriptSimdBool32x4Property, JavascriptSimdBool32x4Walker, Js::JavascriptSIMDBool32x4, RemoteJavascriptSimdBool32x4>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Bool32x4"); }
    };

    //
    // Concrete JavascriptSimdBool8x16Property
    //
    class ATL_NO_VTABLE JavascriptSimdBool8x16Property :
        public SIMDProperty<JavascriptSimdBool8x16Property, JavascriptSimdBool8x16Walker, Js::JavascriptSIMDBool8x16, RemoteJavascriptSimdBool8x16>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Bool8x16"); }
    };

    //
    // Concrete JavascriptSimdBool16x8Property
    //
    class ATL_NO_VTABLE JavascriptSimdBool16x8Property :
        public SIMDProperty<JavascriptSimdBool16x8Property, JavascriptSimdBool16x8Walker, Js::JavascriptSIMDBool16x8, RemoteJavascriptSimdBool16x8>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Bool16x8"); }
    };

    //
    // Concrete JavascriptSimdUint32x4Property
    //
    class ATL_NO_VTABLE JavascriptSimdUint32x4Property :
        public SIMDProperty<JavascriptSimdUint32x4Property, JavascriptSimdUint32x4Walker, Js::JavascriptSIMDUint32x4, RemoteJavascriptSimdUint32x4>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Uint32x4"); }
    };

    //
    // Concrete JavascriptSimdUint8x16Property
    //
    class ATL_NO_VTABLE JavascriptSimdUint8x16Property :
        public SIMDProperty<JavascriptSimdUint8x16Property, JavascriptSimdUint8x16Walker, Js::JavascriptSIMDUint8x16, RemoteJavascriptSimdUint8x16>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Uint8x16"); }
    };

    //
    // Concrete JavascriptSimdUint16x8Property
    //
    class ATL_NO_VTABLE JavascriptSimdUint16x8Property :
        public SIMDProperty<JavascriptSimdUint16x8Property, JavascriptSimdUint16x8Walker, Js::JavascriptSIMDUint16x8, RemoteJavascriptSimdUint16x8>
    {
    public:
        LPCWSTR GetType() const { return _u("SIMD.Uint16x8"); }
    };

} // namespace JsDiag.
