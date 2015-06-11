//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    enum class JavascriptMapIteratorKind
    {
        Key,
        Value,
        KeyAndValue,
    };

    class JavascriptMapIterator : public DynamicObject
    {
    private:
        JavascriptMap*                          m_map;
        JavascriptMap::MapDataList::Iterator    m_mapIterator;
        JavascriptMapIteratorKind               m_kind;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptMapIterator, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptMapIterator);

    public:
        JavascriptMapIterator(DynamicType* type, JavascriptMap* map, JavascriptMapIteratorKind kind);

        static bool Is(Var aValue);
        static JavascriptMapIterator* FromVar(Var aValue);

        class EntryInfo
        {
        public:
            static FunctionInfo Next;
            static FunctionInfo SymbolIterator;
        };

        static Var EntryNext(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...);

    public:
        JavascriptMap* GetMapForHeapEnum() { return m_map; }
    };
} // namespace Js