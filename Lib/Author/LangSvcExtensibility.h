//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class JsContext;

    class LangSvcExtensibility 
    {
        FileAuthoring*        _fileAuthoring;
        Js::ScriptContext*    _scriptContext;
        ArenaAllocator*       _alloc;
        Js::RecyclableObject* _intellisenseObj;
    public:
        LangSvcExtensibility(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring); 

        void FireOnCompletion(Completions* completions, charcount_t offset, Js::Var target, LPCWSTR targetName);
        void FireOnParameterHelp(PageAllocator* pageAlloc, IAuthorFunctionHelp*& funcHelp, charcount_t offset, Js::JavascriptFunction* target,  Js::RecyclableObject* parentObject);
        void FireOnCompletionHint(PageAllocator* pageAlloc, Completions* completions, int itemIndex, IAuthorSymbolHelp*& symbolHelp);

    private:

        bool HasExtensions(LPCWSTR eventName);
        void ReportMessages();
        void ReportMessage(LPCWSTR msg);

        template<typename TArg>
        void InvokeExtensions(LPCWSTR eventName, TArg arg);
    };
}