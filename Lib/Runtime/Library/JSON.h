//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
#include <wchar.h>
const int JSONspaceSize = 10; //ES5 defined limit on the indentation space

#include "JSONScanner.h"
#include "JSONParser.h"

namespace JSON 
{
    class JSONParser;

    class EntryInfo
    {
    public:
        static Js::FunctionInfo Stringify;
        static Js::FunctionInfo Parse;
    };

    Js::Var Stringify(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    Js::Var Parse(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);

    class StringifySession
    {
    public:
        enum ReplacerType
        {
            ReplacerNone,
            ReplacerFunction,
            ReplacerArray
        };
        struct StringTable
        {
            Js::JavascriptString     * propName;            
            Js::PropertyRecord const * propRecord;
        };

        StringifySession(Js::ScriptContext* sc) 
            :   scriptContext(sc), 
                replacerType(ReplacerNone), 
                gap(NULL), 
                indent(0),
                propertySeparator(NULL)
        {
            replacer.propertyList.propertyNames = NULL;
            replacer.propertyList.length = 0;
        };
        
        Js::Var Stringify(){};

        // Init operation is split in three functions
        void InitReplacer(Js::RecyclableObject* f)
        {
            replacerType = ReplacerFunction;
            replacer.ReplacerFunction = f;
        }
        void InitReplacer(StringTable *nameTable, uint len)
        {
            replacerType = ReplacerArray;
            replacer.propertyList.propertyNames = nameTable;
            replacer.propertyList.length = len;
        }
        void CompleteInit(Js::Var space, ArenaAllocator* alloc);

        Js::Var Str(Js::JavascriptString* key, Js::PropertyId keyId, Js::Var holder);
        Js::Var Str(uint32 index, Js::Var holder);
        
    private:
        inline Js::JavascriptString* Quote(Js::JavascriptString* value)
        {
            // By default, optimize for scenario when we don't need to change the inside of the string. That's majority of cases.
            return Js::JSONString::Escape<Js::EscapingOperation_NotEscape>(value);
        }

        Js::Var StringifyObject(Js::Var value);


        Js::Var StringifyArray(Js::Var value);
        Js::JavascriptString* GetArrayElementString(uint32 index, Js::Var arrayVar);
        Js::JavascriptString* GetPropertySeparator();
        Js::JavascriptString* GetIndentString(uint count);
        Js::JavascriptString* GetMemberSeparator(Js::JavascriptString* indentString);
        void StringifyMemberObject( Js::JavascriptString* propertyName, Js::PropertyId id, Js::Var value, Js::ConcatStringBuilder* result, 
            Js::JavascriptString* &indentString, Js::JavascriptString* &memberSeparator, bool &isFirstMember, bool &isEmpty );

        uint32 GetPropertyCount(Js::RecyclableObject* object, Js::JavascriptEnumerator* enumerator);
        uint32 GetPropertyCount(Js::RecyclableObject* object, Js::JavascriptEnumerator* enumerator, bool* isPresise);

        JSONStack *objectStack;

        Js::ScriptContext* scriptContext;
        ReplacerType replacerType;

        union
        {
            Js::RecyclableObject* ReplacerFunction;
            struct PropertyList
            {
                StringTable *propertyNames;
                uint length;
            } propertyList; 
 
        } replacer;

        Js::JavascriptString* gap;
        uint indent; 
        Js::JavascriptString* propertySeparator;     // colon or colon+space
        Js::Var StringifySession::StrHelper(Js::JavascriptString* key, Js::Var value, Js::Var holder);
    };
    typedef JsUtil::GrowingArray<Js::Var, ArenaAllocator> GrowingVarArenaArray;

} // namespace JSON

