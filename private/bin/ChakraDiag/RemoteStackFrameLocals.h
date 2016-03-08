//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Base class for stack frame property objects
    //
    template <class T, class Walker>
    class ATL_NO_VTABLE StackFrameProperty:
        public JsDebugProperty<T, HasWalkerPolicy<T, Walker>>
    {
    protected:
        CComPtr<RemoteStackFrame> m_frame;

    protected:
        InspectionContext* GetInspectionContext() const { return m_frame->GetInspectionContext(); }
        RemoteFunctionBody* GetRemoteFunctionBody() const { return m_frame->GetRemoteFunctionBody(); }

    private:
        bool IfFlagSet(DWORD value, DWORD flag)
        {
            bool retValue = (value & flag) == flag;
            return retValue;
        }

    public:
        void Init(RemoteStackFrame* frame)
        {
            m_frame = frame;
            __super::Init();
        }

        // Return the JS_PROPERTY_ATTRIBUTES::JS_PROPERTY_FRAME_* for JsDebugPropertyInfo::attr (frame contribution)
        DWORD GetFrameAttribute() 
        {
            UINT16 frameFlags = m_frame->GetTempDiagFrame()->GetFlags();

            DWORD framePropertyAttributes = 0;
            framePropertyAttributes |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinTryBlock) ? JS_PROPERTY_FRAME_INTRYBLOCK : 0;
            framePropertyAttributes |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinCatchBlock) ? JS_PROPERTY_FRAME_INCATCHBLOCK : 0;
            framePropertyAttributes |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinFinallyBlock) ? JS_PROPERTY_FRAME_INFINALLYBLOCK : 0;

            return framePropertyAttributes;
        }
    };

    //
    // Base class for stack frame walker objects
    //
    template <class T>
    class ATL_NO_VTABLE StackFrameWalker:
        public PropertyCollectionWalker<T>
    {
    protected:
        void Init(RemoteStackFrame* frame);
        static ThreadContext* GetThreadContext(RemoteStackFrame* frame);
    };

    class ATL_NO_VTABLE LocalsWalker;

    //
    // Hidden root object for Locals
    //
    class ATL_NO_VTABLE RemoteStackFrameLocals:
        public StackFrameProperty<RemoteStackFrameLocals, LocalsWalker>
    {
    public:
        static const CString s_localsName;
        static const CString s_scopeName;
        static const CString s_globalsScopeName;

    public:
        void Init(RemoteStackFrame* frame);
        const CString& GetName() const { return s_localsName; }
        LPCWSTR GetValue(UINT nRadix) { return _u("Locals"); }
        bool HasChildren() { return true; }
        bool_result TryGetEnumeratorEx(JS_PROPERTY_MEMBERS members, _Outptr_ IJsEnumDebugProperty **ppEnum);
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker);
    };

    class RegSlotLocalsWalker;
    class SlotArrayLocalsWalker;
    class ActivationObjectWalker;
    class GlobalObjectWalker;
    class DiagScopeLocalsWalker;
    class LocalScopeWalker;
    class BlockScopeWalker;

    //
    // Root locals walker for a stack frame
    //
    class ATL_NO_VTABLE LocalsWalker:
        public DebugPropertyCollectionWalker<LocalsWalker>
    {
    private:
        CComPtr<GlobalObjectWalker> m_globalsWalker;
        CComPtr<RegSlotLocalsWalker> m_regSlotLocalsWalker;
        CComPtr<SlotArrayLocalsWalker> m_slotArrayLocalsWalker;
        CComPtr<ActivationObjectWalker> m_objectLocalsWalker;
        CComPtr<DiagScopeLocalsWalker> m_extraScopeLocalsWalker;

        CComPtr<LocalScopeWalker> m_scopeWalker;

    public:
        void Init(RemoteStackFrame* frame);
        uint GetCount();
        bool_result GetNextProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty);
        bool_result TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);

        // Static helpers.
        static Js::ScopeObjectChain::ScopeObjectChainList* GetScopeChain(RemoteStackFrame* frame);
        static bool HasScopeChain(RemoteStackFrame* frame);
        static bool IsPropertyValid(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, bool *isLetConst, bool* isInDeadZone);
        static void GetArgumentsObjectProperty(RemoteStackFrame* frame, bool isStrictMode, _Out_opt_ Js::Var* argumentsObject, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);

    private:
        template <class Walker>
        uint GetCount(Walker& walker);

        template <class Walker>
        bool GetNextLocal(Walker& walker, uint& index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty);

        template <class Walker>
        bool_result TryGetProperty(Walker& walker, const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty);

        void InsertException(RemoteStackFrame* frame);
        void InsertThis(RemoteStackFrame* frame, bool isStrictMode);
        void InsertReturnValue(RemoteStackFrame* frame);
        void InsertFakeArgumentsObject(RemoteStackFrame* frame, bool isStrictMode);
    };

    //
    // Reg slot local variable walker
    //
    class ATL_NO_VTABLE RegSlotLocalsWalker:
        public StackFrameWalker<RegSlotLocalsWalker>
    {
    public:
        // debuggerScope, will be passed when the regslotlocalswalker is created due to inner block scope (blockscopedirect)
        void Init(RemoteStackFrame* frame, const RemotePropertyIdOnRegSlotsContainer& propIdContainer, DebuggerScope * debuggerScope);
    };

    //
    // Slot array local variable walker
    //
    class ATL_NO_VTABLE SlotArrayLocalsWalker:
        public StackFrameWalker<SlotArrayLocalsWalker>
    {
    public:
       void Init(RemoteStackFrame* frame, void* slotArrayAddr);
    };

    //
    // ActivationObject local variable walker
    //
    class ATL_NO_VTABLE ActivationObjectWalker:
        public DynamicObjectWalker<ActivationObjectWalker>
    {
    protected:
        CComPtr<RemoteStackFrame> m_frame;

    public:
        void Init(RemoteStackFrame* frame, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty);

        bool RequirePropertiesEnumerable() const
        {
            return true; // Only list enumerable properties for scopes
        }

        bool IsPropertyValid(const Js::PropertyId propertyId, bool *isInDeadZone)
        {
            Assert(m_frame);
            Assert(propertyId != Js::Constants::NoProperty);
            Assert(isInDeadZone);
            bool isLetConst;
            return LocalsWalker::IsPropertyValid(m_frame, propertyId, Js::Constants::NoRegister, &isLetConst, isInDeadZone);
        }
    };

    //
    // Global object variable walker. Extends ActivationObjectWalker to support lookups other than enumerable children.
    //
    class ATL_NO_VTABLE GlobalObjectWalker:
        public ActivationObjectWalker
    {
    public:
        void Init(RemoteStackFrame* frame, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty);

        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty);
    };

    //
    // Walks block/catch/with scopes etc. for local variables
    //
    class ATL_NO_VTABLE DiagScopeLocalsWalker:
        public MergedPropertyWalker
    {
    public:
        void Init(RemoteStackFrame* frame, ScopeObjectChain* scopeObjectChain);

    private:
        void InsertWalkerForProperty(InspectionContext* context, const PROPERTY_INFO& prop);
    };

    //
    // A "with" object doesn't display locals, but participate in expression evaluation.
    //
    class ATL_NO_VTABLE WithObjectLocalsWalker:
        public PropertyCollectionWalker<WithObjectLocalsWalker>
    {
    public:
        void Init(InspectionContext* context, const Js::DynamicObject* object);

        uint GetCount() const
        {
            return 0; // claims we have no properties to display
        }

        bool_result GetNextProperty(uint index, _Out_ IJsDebugPropertyInternal **ppDebugProperty)
        {
            return false;
        }

        bool_result TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
        {
            return __super::TryGetProperty(name, ppDebugProperty); // supports lookup by name
        }
    };

    //
    // Base class for Scope objects
    //
    template <class T, class Walker>
    class ATL_NO_VTABLE ScopeProperty:
        public StackFrameProperty<T, Walker>
    {
    protected:
        Js::Var m_instance;
        CComBSTR m_value;

    public:
        void Init(RemoteStackFrame* frame, Js::Var instance);

        const CString& GetName() const { return RemoteStackFrameLocals::s_scopeName; }
        void GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue);

        bool HasChildren() { return true; }

    protected:
        void ReadValue() { m_value = _u(""); }
    };

    template <class T, class Walker>
    void ScopeProperty<T, Walker>::Init(RemoteStackFrame* frame, Js::Var instance)
    {
        __super::Init(frame);
        m_instance = instance;
    }

    template <class T, class Walker>
    void ScopeProperty<T, Walker>::GetValueBSTR(UINT nRadix, _Out_ BSTR* pValue)
    {
        if (!m_value)
        {
            pThis()->ReadValue();
        }
        CheckHR(m_value.CopyTo(pValue));
    }

    //
    // ScopeSlots scope object property
    //
    class ATL_NO_VTABLE ScopeSlotsProperty:
        public ScopeProperty<ScopeSlotsProperty, SlotArrayLocalsWalker>
    {
    public:
        void Init(RemoteStackFrame* frame, Js::Var instance);
        void ReadValue();
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker);
    };

    //
    // ScopeObject scope object property
    //
    class ATL_NO_VTABLE ScopeObjectProperty:
        public ScopeProperty<ScopeObjectProperty, ActivationObjectWalker>
    {
    public:
        void ReadValue();
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker);
    };

    //
    // "with" object scope
    //
    class ATL_NO_VTABLE WithObjectScopeProperty:
        public ScopeProperty<WithObjectScopeProperty, WithObjectLocalsWalker>
    {
    public:
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker);
    };

    //
    // [Globals] scope object property
    //
    class ATL_NO_VTABLE GlobalsScopeProperty:
        public StackFrameProperty<GlobalsScopeProperty, GlobalObjectWalker>
    {
    public:
        const CString& GetName() const { return RemoteStackFrameLocals::s_globalsScopeName; }
        bool_result TryCreateWalker(_Outptr_ WalkerType** ppWalker);
    };

    //
    // Locals scope walker
    //
    class ATL_NO_VTABLE LocalScopeWalker:
        public SimpleDebugPropertyCollectionWalker
    {
    private:
        struct HiddenScope
        {
            size_t index;
            CComPtr<IJsDebugPropertyInternal> scope;

            HiddenScope(): index(MAXSIZE_T) {}
            HiddenScope(size_t index, IJsDebugPropertyInternal* scope): index(index), scope(scope) {}

            bool IsEmpty() const { return index == MAXSIZE_T; }
        };

        CAtlArray<HiddenScope> m_hiddenScopes;

    public:
        void Init(RemoteStackFrame* frame, const RemoteFrameDisplay& display, uint startIndex);
        bool_result TryGetScopedProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty);

    private:
        static bool IsActivationObject(InspectionContext* context, Js::Var instance);
        static ScopeType GetScopeType(InspectionContext* context, Js::Var instance);

        void InsertHiddenScope(IJsDebugPropertyInternal* scope);
        HiddenScope GetHiddenScope(size_t index) const;
    };
} // namespace JsDiag.
