//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//  Implements Iterator that iterate through an forin enumerator. This is 
//  used in Reflect.enumerate
//----------------------------------------------------------------------------
#pragma once

namespace Js
{
    class JavascriptEnumeratorIterator : public DynamicObject
    {
        JavascriptEnumerator* enumerator;
        RecyclableObject* object;
        int objectIndex;

    protected:
        DEFINE_VTABLE_CTOR(JavascriptEnumeratorIterator, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptEnumeratorIterator);
        JavascriptEnumeratorIterator(DynamicType* type, JavascriptEnumerator* enumerator, RecyclableObject* target);
        Var GetNext();

    public:
        static Var Create(JavascriptEnumerator* enumerator, RecyclableObject* target, ScriptContext* scriptContext);

        static bool Is(Var aValue);
        static JavascriptEnumeratorIterator* FromVar(Var aValue);


        class EntryInfo
        {
        public:
            static FunctionInfo Next;
            static FunctionInfo SymbolIterator;
        };

        static Var EntryNext(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...);

    private:
        Var InternalGetNext();
    };
}
