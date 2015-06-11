//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    enum class JavascriptSetIteratorKind
    {
        Value,
        KeyAndValue,
    };

    class JavascriptSetIterator : public DynamicObject
    {
    private:
        JavascriptSet*                          m_set;
        JavascriptSet::SetDataList::Iterator    m_setIterator;
        JavascriptSetIteratorKind               m_kind;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptSetIterator, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptSetIterator);

    public:
        JavascriptSetIterator(DynamicType* type, JavascriptSet* set, JavascriptSetIteratorKind kind);

        static bool Is(Var aValue);
        static JavascriptSetIterator* FromVar(Var aValue);

        class EntryInfo
        {
        public:
            static FunctionInfo Next;
            static FunctionInfo SymbolIterator;
        };

        static Var EntryNext(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...);

    public:
        JavascriptSet* GetSetForHeapEnum() { return m_set; }
    };
} // namespace Js