//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct PROPERTY_INFO;
    struct IPropertyWalker;
    struct OriginalObjectInfo;

    typedef _Success_(return) bool bool_result;

    //
    // Allows a listener object to examine properties.
    //
    struct IPropertyListener
    {
        // Enum an index property. Return false to stop further enumeration.
        virtual bool EnumItem(uint index, const PROPERTY_INFO& info) { return false; }

        // Enum a non-index property. Return false to stop further enumeration.
        virtual bool EnumProperty(const Js::PropertyId propertyId, const PROPERTY_INFO& info) { return false; }
    };

    //
    // A property listener that examines index properties.
    //
    template <class Fn>
    struct ItemListener : IPropertyListener
    {
        Fn fn;

        ItemListener(Fn fn) : fn(fn) {}

        virtual bool EnumItem(uint index, const PROPERTY_INFO& info) override
        {
            return fn(index, info);
        }
    };

    template <class Fn>
    ItemListener<Fn> MakeItemListener(Fn fn)
    {
        return ItemListener<Fn>(fn);
    }

    //
    // A property listener that examines all index or non-index properties.
    //
    template <class Fn>
    struct PropertyListener : IPropertyListener
    {
        Fn fn;

        PropertyListener(Fn fn) : fn(fn) {}

        virtual bool EnumItem(uint index, const PROPERTY_INFO& info) override
        {
            return fn(info, Js::Constants::NoProperty);
        }

        virtual bool EnumProperty(const Js::PropertyId propertyId, const PROPERTY_INFO& info) override
        {
            return fn(info, propertyId);
        }
    };

    template <class Fn>
    PropertyListener<Fn> MakePropertyListener(Fn fn)
    {
        return PropertyListener<Fn>(fn);
    }

    // --------------------------------------------------------------------------------------------
    // Interface to allow internal access to object properties.
    // --------------------------------------------------------------------------------------------
    struct IJsDebugPropertyInternal : IJsDebugProperty
    {
        // Get Name of this object. Used for variable lookup by name.
        virtual const CString& GetName() const = 0;

        // Get FullName of this object. Used to compose full name for child nodes.
        virtual const CString& GetFullName() const = 0;

        // Get display value for this object
        virtual void GetDisplayValue(UINT nRadix, _Out_ BSTR* pValue) = 0;

        // Set parent node. This affects full name. Used when parent is determined later than creation time.
        virtual void SetParent(_In_opt_ IJsDebugPropertyInternal* parent) = 0;

        // Get a property representing ToObject from this object. Only supported by primitive Number, Boolean and String.
        // Used by strict mode "this" inspection, and also expression evaluation.
        virtual bool_result TryToObject(_Out_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Get internal walker for this object. Used by DynamicObjects to walk internal arrays.
        virtual bool_result TryGetWalker(_Out_ IPropertyWalker **ppEnum) { return false; }

        // Try to convert this property to an index, used by expression evaluation for [] syntax. Only implemented by TaggedInt and String.
        virtual bool_result TryToIndex(_Out_ UINT* index, _Out_ CString* name) { return false; }

        // Try to get a child property of this object. Used by expression evaluation to do property lookup by string name.
        virtual bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Try to get a child item property of this object. Used by expression evaluation to do property lookup by index name.
        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Try to get prototype property of this object. Used by expression evaluation to walk prototype chain.
        virtual bool_result TryGetPrototype(_Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Try to clone this property with a new name. Used to create expression evaluation result.
        virtual bool_result TryClone(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Enumerte array item properties of this object
        virtual void EnumItems(IPropertyListener* listener, uint start, uint end, bool requireEnumerable) const {} // Do nothing by default

        // Enumerte all properties (index or non-index) of this object, in the same way as the runtime object's GetEnumerator.
        virtual void EnumProperties(IPropertyListener* listener, const OriginalObjectInfo& originalObject, bool requireEnumerable) const {} // Do nothing by default
    };

    // --------------------------------------------------------------------------------------------
    // No walker policy. Some objects (mostly primitives) don't have child nodes.
    // --------------------------------------------------------------------------------------------
    class NoWalkerPolicy
    {
    public:
        typedef void WalkerType;

        template <class T>
        bool HasChildren(T* ownerDebugProperty) { return false; }

        template <class T>
        bool_result TryGetEnumerator(T* ownerDebugProperty, _Outptr_ IJsEnumDebugProperty **ppEnum) { return false; }

        template <class T>
        bool_result TryGetWalker(T* ownerDebugProperty, _Outptr_ IPropertyWalker **ppWalker) { return false; }

        template <class T>
        bool_result TryGetProperty(T* ownerDebugProperty, const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        template <class T>
        bool_result TryGetItem(T* ownerDebugProperty, UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        template <class T>
        bool_result TryGetPrototype(T* ownerDebugProperty, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const { return false; }

        template <class T>
        bool_result TryToObject(T* ownerDebugProperty, _Out_ IJsDebugPropertyInternal** ppDebugProperty) const { return false; }
    };

    // --------------------------------------------------------------------------------------------
    // Has walker policy. Creates a walker on demand.
    //  T: The concrete owner debug property type, which owns this walker policy and m_walker member.
    //  Walker: The walker type used by the owner debug property type T.
    // --------------------------------------------------------------------------------------------
    template <class T, class Walker>
    class HasWalkerPolicy
    {
    private:
        CComPtr<Walker> m_walker;

    public:
        typedef Walker WalkerType;

        // PERF Note: This implementation requires walker creation. Some debug properties, e.g. [Scope]s, know
        // it has child nodes. In such case, consider overloading HasChildren for the debug property to postpone
        // walker creation.
        bool HasChildren(T* ownerDebugProperty)
        {
            return TryEnsureWalker(ownerDebugProperty)
                && (GetWalker()->GetCount() > 0);
        }

        bool_result TryGetEnumerator(T* ownerDebugProperty, _Outptr_ IJsEnumDebugProperty **ppEnum)
        {
            if (TryEnsureWalker(ownerDebugProperty))
            {
                GetWalker()->CreateEnumerator(ppEnum);
                return true;
            }
            return false;
        }

        bool_result TryGetWalker(T* ownerDebugProperty, _Outptr_ IPropertyWalker **ppWalker)
        {
            if (TryEnsureWalker(ownerDebugProperty))
            {
                *ppWalker = GetWalker();
                (*ppWalker)->AddRef();
                return true;
            }
            return false;
        }

        bool_result TryGetProperty(T* ownerDebugProperty, const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            return TryEnsureWalker(ownerDebugProperty)
                && GetWalker()->TryGetProperty(name, ppDebugProperty);
        }

        bool_result TryGetItem(T* ownerDebugProperty, UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            return TryEnsureWalker(ownerDebugProperty)
                && GetWalker()->TryGetItem(index, ppDebugProperty);
        }

        bool_result TryGetPrototype(T* ownerDebugProperty, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            return TryEnsureWalker(ownerDebugProperty)
                && GetWalker()->TryGetPrototype(ppDebugProperty);
        }

        bool_result TryToObject(T* ownerDebugProperty, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            if (TryEnsureWalker(ownerDebugProperty)) // Generally, if an object has a walker, it behaves like a DynamicObject
            {
                *ppDebugProperty = ownerDebugProperty;
                (*ppDebugProperty)->AddRef();
                return true;
            }
            return false;
        }

        bool TryEnsureWalker(T* ownerDebugProperty)
        {
            if (m_walker)
            {
                return true; // yes, walker exists
            }
            return ownerDebugProperty->TryCreateWalker(&m_walker); // Owner debug property must provide
        }

        Walker* GetWalker() const
        {
            Assert(m_walker);
            return m_walker;
        }
    };

    // --------------------------------------------------------------------------------------------
    // Base IJsDebugProperty (IJsDebugPropertyInternal) implementation.
    //  T: Concrete subclass type
    // --------------------------------------------------------------------------------------------
    template <class T, class WalkerPolicy = NoWalkerPolicy>
    class ATL_NO_VTABLE JsDebugProperty :
        public CComObjectRoot,
        public IJsDebugPropertyInternal
    {
    private:
        CString m_fullName;
        WalkerPolicy m_walkerPolicy;

    public:
        BEGIN_COM_MAP(JsDebugProperty)
            COM_INTERFACE_ENTRY(IJsDebugProperty)
        END_COM_MAP()

        void Init(_In_opt_ IJsDebugPropertyInternal* parent = nullptr);

        // === IJsDebugProperty =====
        virtual STDMETHODIMP GetPropertyInfo(
            /* [in] */ UINT nRadix,
            /* [out] */ __RPC__out JsDebugPropertyInfo *pPropertyInfo);

        virtual STDMETHODIMP GetMembers(
            /* [in] */ JS_PROPERTY_MEMBERS members,
            /* [out] */ __RPC__deref_out_opt IJsEnumDebugProperty **ppEnum);

        // ==== IJsDebugPropertyInternal ====
        virtual const CString& GetName() const override
        {
            return pThis()->GetName(); // subclass must implement
        }

        virtual const CString& GetFullName() const override
        {
            return m_fullName;
        }

        virtual void GetDisplayValue(UINT nRadix, _Out_ BSTR* pValue) override
        {
            return pThis()->GetValueBSTR(nRadix, pValue);
        }

        virtual void SetParent(_In_opt_ IJsDebugPropertyInternal* parent) override
        {
            InitParent(parent);
        }

        virtual bool_result TryToObject(_Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return m_walkerPolicy.TryToObject(pThis(), ppDebugProperty);
        }

        virtual bool_result TryGetWalker(_Outptr_ IPropertyWalker **ppWalker) override
        {
            return m_walkerPolicy.TryGetWalker(pThis(), ppWalker);
        }

        virtual bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return pThis()->TryGetBuiltInProperty(name, ppDebugProperty)
                || m_walkerPolicy.TryGetProperty(pThis(), name, ppDebugProperty);
        }

        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return pThis()->TryGetItemDirect(index, ppDebugProperty)
                || m_walkerPolicy.TryGetItem(pThis(), index, ppDebugProperty);
        }

        virtual bool_result TryGetPrototype(_Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            return m_walkerPolicy.TryGetPrototype(pThis(), ppDebugProperty);
        }

        virtual bool_result TryClone(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            // If both name and fullName are the same as newName, return this object.
            if (newName == GetName() && newName == GetFullName())
            {
                *ppDebugProperty = this;
                (*ppDebugProperty)->AddRef();
                return true;
            }
            return pThis()->TryCloneImpl(newName, ppDebugProperty);
        }

        // Whether it is ES6 symbol property.
        bool IsSymbolProperty() const { return false; }

        // Overload to customize diagnostics display of Name, Type, Value
        void GetNameBSTR(_Out_ BSTR* pValue) { ToBSTR(pThis()->GetName(), pValue); }
        void GetTypeBSTR(_Out_ BSTR* pValue) { ToBSTR(pThis()->GetType(), pValue); }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue) { ToBSTR(pThis()->GetValue(nRadix), pValue); }
        void GetFullNameBSTR(_Out_ BSTR* pValue) { ToBSTR(m_fullName, pValue); }

    protected:
        typedef typename WalkerPolicy::WalkerType WalkerType;

        T* pThis() { return static_cast<T*>(this); }
        const T* pThis() const { return static_cast<const T*>(this); }

        void InitParent(_In_opt_ IJsDebugPropertyInternal* parent);
        bool TryEnsureWalker() { return m_walkerPolicy.TryEnsureWalker(pThis()); } // NOTE: Only available for HasWalkerPolicy
        WalkerType* GetWalker() const { return m_walkerPolicy.GetWalker(); } // NOTE: Only available for HasWalkerPolicy

        // Overload to customize diagnostics display of Name, Type, Value
        //LPCWSTR GetName() { return _u(""); }
        LPCWSTR GetType() { return _u(""); }
        LPCWSTR GetValue(UINT nRadix) { return _u(""); }

        // Overload to customize diagnostics display attributes
        bool HasChildren() { return m_walkerPolicy.HasChildren(pThis()); }
        DWORD GetAttribute() { return JS_PROPERTY_ATTRIBUTE_NONE; }

        // Return the JS_PROPERTY_ATTRIBUTES::JS_PROPERTY_FRAME_* for JsDebugPropertyInfo::attr (frame contribution)
        DWORD GetFrameAttribute()
        {
            // this value is OR-ed with GetAttribute()
            C_ASSERT(JS_PROPERTY_ATTRIBUTE_NONE == 0);
            return JS_PROPERTY_ATTRIBUTE_NONE;
        }

        // Overload to customize how to clone (used to return expression evaluation result)
        bool_result TryCloneImpl(const CString& newName, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const { return false; }

        // Overload to return builtin property, such as String length (used by expression evaluation)
        bool_result TryGetBuiltInProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Overload to return direct index property, such as String item (used by expression evaluation)
        bool_result TryGetItemDirect(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) { return false; }

        // Overload to customize child node enumerator
        bool_result TryGetEnumerator(_Outptr_ IJsEnumDebugProperty **ppEnum) { return m_walkerPolicy.TryGetEnumerator(pThis(), ppEnum); }
        bool_result TryGetEnumeratorEx(JS_PROPERTY_MEMBERS members, _Outptr_ IJsEnumDebugProperty **ppEnum) { return pThis()->TryGetEnumerator(ppEnum); }
    };

    template <class T, class WalkerPolicy>
    inline STDMETHODIMP JsDebugProperty<T, WalkerPolicy>::GetPropertyInfo(
        /* [in] */ UINT nRadix,
        /* [out] */ __RPC__out JsDebugPropertyInfo *pPropertyInfo)
    {
        return JsDebugApiWrapper([=]
        {
            memset(pPropertyInfo, 0, sizeof(JsDebugPropertyInfo));

            // Keep BSTRs in local variables first, so they won't leak if an exception occurs in between.
            CComBSTR name, type, value, fullName;
            pThis()->GetNameBSTR(&name);
            pThis()->GetTypeBSTR(&type);
            pThis()->GetValueBSTR(nRadix, &value);

            DWORD attr = pThis()->GetAttribute();
            if ((attr & JS_PROPERTY_FAKE) != 0 || pThis()->IsSymbolProperty())
            {
                // Fake/Symbol property, returns same as 'name'
                pThis()->GetNameBSTR(&fullName);
            }
            else
            {
                pThis()->GetFullNameBSTR(&fullName);
            }

            if (pThis()->HasChildren())
            {
                attr |= JS_PROPERTY_HAS_CHILDREN;
            }
            attr |= JS_PROPERTY_READONLY; // Currently all readonly
            attr |= pThis()->GetFrameAttribute();

            pPropertyInfo->attr = (JS_PROPERTY_ATTRIBUTES)attr;

            // Success, detach BSTRs
            pPropertyInfo->name = name.Detach();
            pPropertyInfo->type = type.Detach();
            pPropertyInfo->value = value.Detach();
            pPropertyInfo->fullName = fullName.Detach();
            return S_OK;
        });
    }

    template <class T, class WalkerPolicy>
    inline STDMETHODIMP JsDebugProperty<T, WalkerPolicy>::GetMembers(
        /* [in] */ JS_PROPERTY_MEMBERS members,
        /* [out] */ __RPC__deref_out_opt IJsEnumDebugProperty **ppEnum)
    {
        return JsDebugApiWrapper([=]
        {
            *ppEnum = NULL;
            return pThis()->TryGetEnumeratorEx(members, ppEnum) ? S_OK : S_FALSE;
        });
    }

    template <class T, class WalkerPolicy>
    void JsDebugProperty<T, WalkerPolicy>::Init(_In_opt_ IJsDebugPropertyInternal* parent /*= nullptr*/)
    {
        InitParent(parent);
    }

    template <class T, class WalkerPolicy>
    void JsDebugProperty<T, WalkerPolicy>::InitParent(_In_opt_ IJsDebugPropertyInternal* parent)
    {
        // NOTE: Subclass must be ready for GetName() before this Init() call.
        const CString& name = pThis()->GetName();

        // Following code constructs FullName of this node. Note that FullName doesn't include any JS_PROPERTY_FAKE node on the path.
        const bool skipThisName = name.IsEmpty() || (pThis()->GetAttribute() & JS_PROPERTY_FAKE);

        if (parent != nullptr)
        {
            const CString& parentFullName = parent->GetFullName();
            if (!parentFullName.IsEmpty())
            {
                m_fullName = parentFullName;
                if (!skipThisName && name[0] != _u('['))
                {
                    m_fullName += _u('.');
                }
            }
        }

        if (!skipThisName)
        {
            m_fullName += name;
        }
    }

    // --------------------------------------------------------------------------------------------
    // Wraps an internal property walker into an IJsEnumDebugProperty object.
    //  Walker: the internal Walker type
    // --------------------------------------------------------------------------------------------
    template <class Walker>
    class ATL_NO_VTABLE JsEnumDebugProperty:
        public CComObjectRoot,
        public IJsEnumDebugProperty
    {
    private:
        CComPtr<Walker> m_walker;
        uint m_index;

    public:
        BEGIN_COM_MAP(JsEnumDebugProperty)
            COM_INTERFACE_ENTRY(IJsEnumDebugProperty)
        END_COM_MAP()

        void Init(_In_ Walker* walker)
        {
            m_walker = walker;
            m_index = 0;
        }

        // === IJsEnumDebugProperty =====
        virtual STDMETHODIMP Next(
            /* [in] */ ULONG count,
            /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty,
            /* [out] */ __RPC__out ULONG *pActualCount);

        virtual STDMETHODIMP GetCount(
            /* [out] */ ULONG *pCount);

    private:
        uint GetCount() const
        {
            return m_walker->GetCount();
        }

        bool_result GetNext(_Outptr_ IJsDebugProperty **ppDebugProperty)
        {
            return m_walker->GetNextProperty(m_index++, (IJsDebugPropertyInternal**)ppDebugProperty);
        }
    };

    template <class Walker>
    STDMETHODIMP JsEnumDebugProperty<Walker>::Next(
        /* [in] */ ULONG count,
        /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty,
        /* [out] */ __RPC__out ULONG *pActualCount)
    {
        return JsDebugApiWrapper([=]
        {
            ULONG i = 0;
            try
            {
                while (i < count
                    && this->GetNext(&ppDebugProperty[i]))
                {
                    i++;
                }

                if (pActualCount)
                {
                    *pActualCount = i;
                }

                return i == count ? S_OK : S_FALSE;
            }
            catch(...)
            {
                while (i-- > 0)
                {
                    ppDebugProperty[i]->Release();
                    ppDebugProperty[i] = NULL;
                }

                if (pActualCount)
                {
                    *pActualCount = 0;
                }

                throw; // rethrow
            }
        });
    }

    template <class Walker>
    STDMETHODIMP JsEnumDebugProperty<Walker>::GetCount(
            /* [out] */ ULONG *pCount)
    {
        return JsDebugApiWrapper([=]
        {
            *pCount = static_cast<ULONG>(this->GetCount());
            return S_OK;
        });
    }

    //
    // Allows internal access to walkers
    //
    struct IPropertyWalker: IUnknown
    {
        // Get count of debug properties
        virtual uint GetCount() = 0;

        // Get next debug property of this object by index
        virtual bool_result TryGetProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty) = 0;

        // Get a property by name (for expression evaluation)
        virtual bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty) = 0;

        // Get an index named property
        virtual bool_result TryGetItem(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty) = 0;

        // Indicate the object being walked is the internal array of another object.
        virtual void SetIsInternalArray(_In_ IJsDebugPropertyInternal* outerDebugProperty) = 0;
    };

    // --------------------------------------------------------------------------------------------
    // Base debug property walker implementation
    //  T: concrete walker subclass type
    // --------------------------------------------------------------------------------------------
    template <class T>
    class ATL_NO_VTABLE PropertyWalker:
        public CComObjectRoot,
        public IPropertyWalker
    {
    private:
        IJsDebugPropertyInternal* m_ownerDebugProperty; // Walker owned by ownerDebugProperty, do not AddRef()!

    protected:
        T* pThis() { return static_cast<T*>(this); }
        const T* pThis() const { return static_cast<const T*>(this); }

        PropertyWalker() : m_ownerDebugProperty(nullptr) {}

        IJsDebugPropertyInternal* GetOwnerDebugProperty() const
        {
            return m_ownerDebugProperty;
        }

    public:
        BEGIN_COM_MAP(PropertyWalker)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        void Init(_In_ IJsDebugPropertyInternal* ownerDebugProperty)
        {
            m_ownerDebugProperty = ownerDebugProperty;
        }

        //
        // Create a new IJsEnumDebugProperty wrapper for this walker
        //
        void CreateEnumerator(_Outptr_ IJsEnumDebugProperty** ppEnum)
        {
            CreateComObject<JsEnumDebugProperty<T>>(pThis(), ppEnum);
        }

        bool_result TryGetPrototype(_Outptr_ IJsDebugPropertyInternal** ppDebugProperty) const
        {
            return false; // by default don't know about prototype
        }

        // === IPropertyWalker ===
        virtual uint GetCount() override
        {
            return pThis()->GetCount(); // walker subclass must implement
        }

        virtual bool_result TryGetProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty) override
        {
            return pThis()->GetNextProperty(index, ppDebugProperty); // walker subclass must implement
        }

        virtual bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty) override
        {
            return pThis()->TryGetProperty(name, ppDebugProperty); // walker subclass must implement
        }

        virtual bool_result TryGetItem(UINT index, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty) override
        {
            // NOTE: subclass usually stores index properties separately from non-index properties. Needs to provide individual implementation.
            return false; // By default don't know how to GetItem.
        }

        virtual void SetIsInternalArray(_In_ IJsDebugPropertyInternal* outerDebugProperty) override
        {
            // If this walker if for an internal array of an object, use the outer object debug property.
            m_ownerDebugProperty = outerDebugProperty;
        };
    };

    // --------------------------------------------------------------------------------------------
    // ATL container helper
    // --------------------------------------------------------------------------------------------
    template <class T, class Func>
    bool MapUntil(const CAtlArray<T>& arr, Func func)
    {
        for (size_t i = 0; i < arr.GetCount(); i++)
        {
            if (func(arr[i]))
            {
                return true;
            }
        }
        return false;
    }

    template <class T, class Func>
    void Map(const CAtlArray<T>& arr, Func func)
    {
        MapUntil(arr, [=](const T& item)
        {
            func(item);
            return false; // Map all items
        });
    }

    // --------------------------------------------------------------------------------------------
    // Debug property collection property walker.
    // --------------------------------------------------------------------------------------------
    template <class T>
    class ATL_NO_VTABLE DebugPropertyCollectionWalker:
        public PropertyWalker<T>
    {
    protected:
        CAtlArray<CComPtr<IJsDebugPropertyInternal>> m_items;

    public:
        uint GetCount() const
        {
            return static_cast<uint>(m_items.GetCount());
        }

        bool_result GetNextProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            if (index < GetCount())
            {
                CheckHR(m_items[index].CopyTo(ppDebugProperty));
                return true;
            }

            return false;
        }

        // Supports WalkerPolicy
        bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return MapUntil(m_items, [=](const CComPtr<IJsDebugPropertyInternal>& item) -> bool
            {
                if (item->GetName() == name)
                {
                    CheckHR(const_cast<CComPtr<IJsDebugPropertyInternal>&>(item).CopyTo(ppDebugProperty));
                    return true;
                }
                return false;
            });
        }

        void InsertItem(IJsDebugPropertyInternal* prop)
        {
            m_items.Add(prop);
        }
    };

    // --------------------------------------------------------------------------------------------
    // Simple debug property collection walker. No subclass customization of property walking.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE SimpleDebugPropertyCollectionWalker:
        public DebugPropertyCollectionWalker<SimpleDebugPropertyCollectionWalker>
    {
    };

    // --------------------------------------------------------------------------------------------
    // Merged property walker combines properties from multiple child property walkers.
    // --------------------------------------------------------------------------------------------
    class ATL_NO_VTABLE MergedPropertyWalker:
        public PropertyWalker<MergedPropertyWalker>
    {
    private:
        CAtlArray<CComPtr<IPropertyWalker>> m_walkers;

    public:
        uint GetCount() const
        {
            uint count = 0;

            Map(m_walkers, [&](IPropertyWalker* walker)
            {
                count += walker->GetCount();
            });

            return count;
        }

        bool_result GetNextProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return MapUntil(m_walkers, [&](IPropertyWalker* walker) -> bool
            {
                uint count = walker->GetCount();
                if (index < count)
                {
                    return walker->TryGetProperty(index, ppDebugProperty);
                }

                index -= count;
                return false;
            });
        }

        // Supports WalkerPolicy
        bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return MapUntil(m_walkers, [&](IPropertyWalker* walker) -> bool
            {
                return walker->TryGetProperty(name, ppDebugProperty);
            });
        }

        void InsertWalker(IPropertyWalker* walker)
        {
            m_walkers.Add(walker);
        }
    };

    inline void ToBSTR(LPCOLESTR str, _Outptr_result_maybenull_z_ BSTR* pValue)
    {
        *pValue = ::SysAllocString(str);
    }

    inline void ToBSTR(const CString& str, _Outptr_result_maybenull_z_ BSTR* pValue)
    {
        *pValue = str.AllocSysString();
    }

    // --------------------------------------------------------------------------------------------
    // A dummy Lib simulating methods of JavascriptLibrary to help share runtime logic.
    // --------------------------------------------------------------------------------------------
    class DiagBSTRLib
    {
    public:
        typedef BSTR LibStringType;

        template <size_t N>
        BSTR CreateStringFromCppLiteral(const char16 (&value)[N]) const
        {
            BSTR bstr = ::SysAllocStringLen(value, N - 1); // "N - 1" excludes terminating NULL
            if (bstr == NULL)
            {
                DiagException::ThrowOOM();
            }
            return bstr;
        }
    };

    // --------------------------------------------------------------------------------------------
    // A dummy Lib simulating methods of JavascriptLibrary to help share runtime logic.
    // --------------------------------------------------------------------------------------------
    class DiagLibrary
    {
    public:
        // Simulate JavascriptLibrary method
        template <size_t N>
        CString CreateStringFromCppLiteral(const char16 (&value)[N]) const
        {
            return CString(value, N - 1); // "N - 1" excludes terminating NULL
        }

        CString GetFunctionDisplayString() const
        {
            return JS_DISPLAY_STRING_FUNCTION_ANONYMOUS;
        }
    };

    // --------------------------------------------------------------------------------------------
    // A dummy ScriptContext simulating methods of Js::ScriptContext to help share runtime logic.
    // --------------------------------------------------------------------------------------------
    class DiagScriptContext
    {
        DiagLibrary m_lib;

    public:
        // Simulate scriptContext->GetLibrary
        const DiagLibrary* GetLibrary() const { return &m_lib; }
    };

    // --------------------------------------------------------------------------------------------
    // A dummy string builder to simulate runtime StringBuilder/CompountString, used to share runtime logic.
    // --------------------------------------------------------------------------------------------
    class DiagStringBuilder
    {
    private:
        CString m_str;

    public:
        void Append(WCHAR ch)
        {
            m_str += ch;
        }

        void Append(LPCWSTR s)
        {
            m_str += s;
        }

        void Append(_In_reads_(count) LPCWSTR s, const CharCount count)
        {
            m_str.Append(s, count);
        }

        void AppendChars(WCHAR ch) { Append(ch); }
        void AppendChars(_In_reads_(count) LPCWSTR s, const CharCount count) { Append(s, count); }

        template<CharCount AppendCharLengthPlusOne>
        void AppendChars(const char16 (&s)[AppendCharLengthPlusOne])
        {
            Append(s, AppendCharLengthPlusOne - 1);
        }

        template<class TValue, class FConvertToString>
        void AppendChars(const TValue &value, CharCount maximumAppendCharLength, const FConvertToString ConvertToString)
        {
            const CharCount AbsoluteMaximumAppendCharLength = 20; // maximum length of uint64 converted to base-10 string
            Assert(maximumAppendCharLength != 0);
            Assert(maximumAppendCharLength <= AbsoluteMaximumAppendCharLength);
            
            ++maximumAppendCharLength; // + 1 for null terminator
            char16 convertBuffer[AbsoluteMaximumAppendCharLength + 1]; // + 1 for null terminator

            ConvertToString(value, convertBuffer, maximumAppendCharLength);

            Append(convertBuffer);
        }

        const CString& GetString() const
        {
            return m_str;
        }

        // Simulate JavascriptString methods
        static CString Concat(const CString& first, const CString& second)
        {
            return first + second;
        }
        template <class T>
        static CString NewCopySz(PCWSTR s, const T& ignored)
        {
            return s;
        }
    };

} // namespace JsDiag.
