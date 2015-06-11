//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
#if ENABLE_SCRIPT_DEBUG
    class ScriptDebug
    {
    private:
        ScriptContext* scriptContext;
        ArenaAllocator alloc;
        static int maxDepth;
        static int maxLines;
        static int lineCount;
        regex::Stack<void*>* cursor;
        JsUtil::BaseDictionary<wchar_t*, void*, ArenaAllocator, PrimeSizePolicy>* namedObjects;
        JsUtil::BaseDictionary<void*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* getFieldBreakpoints;
        JsUtil::BaseDictionary<void*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* setFieldBreakpoints;
        JsUtil::BaseDictionary<void*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* deleteFieldBreakpoints;
        JsUtil::BaseDictionary<wchar_t*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* getFieldGlobalBreakpoints;
        JsUtil::BaseDictionary<wchar_t*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* setFieldGlobalBreakpoints;
        JsUtil::BaseDictionary<wchar_t*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* deleteFieldGlobalBreakpoints;
        JsUtil::BaseDictionary<void*, JavascriptMethod, ArenaAllocator, PrimeSizePolicy>* invokeBreakpoints;
        JsUtil::BaseDictionary<wchar_t*, wchar_t*, ArenaAllocator, PrimeSizePolicy>* strings;
        JsUtil::BaseDictionary<wchar_t*, void*, ArenaAllocator, PrimeSizePolicy>* namedFunctions;
        JsUtil::BaseDictionary<void*, void*, ArenaAllocator, PrimeSizePolicy>* functions;

    public:
        ScriptDebug(PageAllocator * pageAllocator);
        void SetScriptContext(__in ScriptContext* scriptContext);

        static void RecyclerPrintFn(void* addr,int size);
        int GetMaxDepth();
        void SetMaxDepth(__in int value);
        int GetMaxLines();
        void SetMaxLines(__in int value);

        void SetNamedObject(__in LPCWSTR name, __in void* object);
        void* GetNamedObject(__in LPCWSTR name);

        void SetObject(__in void* object);
        void* GetObject();
        BOOL MoveBack();
        BOOL MoveToPrototype();
        BOOL MoveToField(__in LPCWSTR name);
        BOOL MoveToFieldId(__in int fieldId);

        void* GetFunction(__in LPCWSTR name);
        void* CreateNumber(__in int number);
        void* CreateString(__in LPCWSTR string);
        void* CreateBoolean(__in BOOL value);
        void* CreateDate();
        void* CreateArray(__in int size);
        void* CreateObject();
        void* GetUndefined();

        int   ShowType(__in void* object);
        static void  ShowObject(__in void* object);
        void  ShowArguments(__in Arguments* args);

        BOOL  HasOwnField(__in void* object, __in LPCWSTR name);
        void* GetOwnField(__in void* object, __in LPCWSTR name);
        BOOL  HasField(__in void* object, __in LPCWSTR name);
        void* GetField(__in void* object, __in LPCWSTR name);
        BOOL  SetField(__in void* object, __in LPCWSTR name, __in void* value);
        BOOL  DeleteField(__in void* object, __in LPCWSTR name);

        BOOL ListInvokeBreakpoints();
        BOOL ListSetFieldBreakpoints();
        BOOL ListGetFieldBreakpoints();
        BOOL ListDeleteFieldBreakpoints();
        
        BOOL ClearInvokeBreakpoints(__in void* object);
        BOOL ClearSetFieldBreakpoints(__in void* object, __in LPCWSTR name);
        BOOL ClearGetFieldBreakpoints(__in void* object, __in LPCWSTR name);
        BOOL ClearDeleteFieldBreakpoints(__in void* object, __in LPCWSTR name);

        BOOL BreakOnInvoke(__in void* object);
        BOOL BreakOnGetField(__in void* object, __in LPCWSTR name);
        BOOL BreakOnSetField(__in void* object, __in LPCWSTR name);
        BOOL BreakOnDeleteField(__in void* object, __in LPCWSTR name);

    public:
        void OnGetField(__in Var instance, __in PropertyId propertyId);
        void OnGetField(__in Var instance, __in JavascriptString* propertyNameString);
        void OnSetField(__in Var instance, __in PropertyId propertyId, __in Var value);
        void OnSetField(__in Var instance, __in JavascriptString* propertyNameString, __in Var value);
        void OnDeleteField(__in Var instance, __in PropertyId propertyId);
        void OnGetItem(__in Var instance, __in uint32 index);
        void OnSetItem(__in Var instance, __in uint32 index, __in Var value);
        void OnDeleteItem(__in Var instance, __in uint32 index);
        void OnCreateFunction(__in Var function);

    private:
        static Var __cdecl ScriptDebug::DebugBreakThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static void Indent(__in int indent);
        static void ShowObjectInternal(__in Var object, __in int depth, __in regex::Stack<void*>* stack);
        LPCWSTR MakeString(__in LPCWSTR str);

        void OnGetField(__in Var instance, __in LPCWSTR propertyName);
        void OnSetField(__in Var instance, __in LPCWSTR propertyName, __in Var value);
    };
#endif
} 
