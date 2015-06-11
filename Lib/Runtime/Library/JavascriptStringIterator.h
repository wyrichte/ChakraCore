//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptStringIterator : public DynamicObject
    {
    private:
        JavascriptString*   m_string;
        charcount_t         m_nextIndex;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptStringIterator, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptStringIterator);

    public:
        JavascriptStringIterator(DynamicType* type, JavascriptString* string);

        static bool Is(Var aValue);
        static JavascriptStringIterator* FromVar(Var aValue);

        class EntryInfo
        {
        public:
            static FunctionInfo Next;
            static FunctionInfo SymbolIterator;
        };

        static Var EntryNext(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...);

    public:
        JavascriptString* GetStringForHeapEnum() { return m_string; }
    };
} // namespace Js