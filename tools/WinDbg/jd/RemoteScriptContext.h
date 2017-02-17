//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteThreadContext;
class RemoteScriptContext
{
public:
    RemoteScriptContext();
    RemoteScriptContext(ExtRemoteTyped const& scriptContext);
    
    JDRemoteTyped GetJavascriptLibrary()
    {
        return scriptContext.Field("javascriptLibrary");
    }

    RemoteThreadContext GetThreadContext();
    ExtRemoteTyped GetHostScriptContext();
    void PrintReferencedPids();

    template <class Fn>
    void ForEachPageAllocator(Fn fn)
    {
        const bool useCodePageAllocators = GetThreadContext().UseCodePageAllocator();

        // Page allocators for code no longer in the custom heap after we switch to use the per thread code page allocators in RS1
        if (!useCodePageAllocators)
        {
            ExtRemoteTyped thunkCustomHeap = scriptContext.Field("interpreterThunkEmitter.emitBufferManager.allocationHeap");
            if (thunkCustomHeap.HasField("preReservedHeapPageAllocator"))
            {
                fn("CodeThunkPreRes", RemotePageAllocator(thunkCustomHeap.Field("preReservedHeapPageAllocator")));
            }
            fn("CodeThunk", RemotePageAllocator(thunkCustomHeap.Field("pageAllocator")));

            if (scriptContext.HasField("asmJsInterpreterThunkEmitter"))
            {
                ExtRemoteTyped asmJsThunkCustomHeap = scriptContext.Field("asmJsInterpreterThunkEmitter.emitBufferManager.allocationHeap");
                if (asmJsThunkCustomHeap.HasField("preReservedHeapPageAllocator"))
                {
                    fn("CodeAsmJSThunkPreRes", RemotePageAllocator(asmJsThunkCustomHeap.Field("preReservedHeapPageAllocator")));
                }
                fn("CodeAsmJSThunk", RemotePageAllocator(asmJsThunkCustomHeap.Field("pageAllocator")));
            }
        }

        ExtRemoteTyped nativeCodeGen = scriptContext.Field("nativeCodeGen");

        ForEachCodeGenAllocatorPageAllocator(nativeCodeGen.Field("foregroundAllocators"), true, useCodePageAllocators, fn);
        ForEachCodeGenAllocatorPageAllocator(nativeCodeGen.Field("backgroundAllocators"), false, useCodePageAllocators, fn);
    }

    template <class Fn>
    void ForEachArenaAllocator(Fn fn)
    {
        fn("SC", scriptContext.Field("generalAllocator"));
        fn("SC-DynamicProfile", scriptContext.Field("dynamicProfileInfoAllocator"));
        fn("SC-InlineCache", scriptContext.Field("inlineCacheAllocator"));
        if (scriptContext.HasField("isInstInlineCacheAllocator"))
        {
            // IE11 don't have this arena allocator
            fn("SC-IsInstIC", scriptContext.Field("isInstInlineCacheAllocator"));
        }

        ExtRemoteTyped interpreterArena = scriptContext.Field("interpreterArena");
        if (interpreterArena.GetPtr() != 0)
        {
            fn("SC-Interpreter", interpreterArena);
        }
        ExtRemoteTyped guestArena = scriptContext.Field("guestArena");
        if (guestArena.GetPtr() != 0)
        {
            fn("SC-Guest", guestArena);
        }
        ExtRemoteTyped diagArena = scriptContext.Field("diagnosticArena");
        if (diagArena.GetPtr() != 0)
        {
            fn("SC-Diag", diagArena);
        }

        if (scriptContext.HasField("sourceCodeAllocator"))
        {
            fn("SC-SourceCode", scriptContext.Field("sourceCodeAllocator"));
        }
        if (scriptContext.HasField("regexAllocator"))
        {
            fn("SC-Regex", scriptContext.Field("regexAllocator"));
        }
        if (scriptContext.HasField("miscAllocator"))
        {
            fn("SC-Misc", scriptContext.Field("miscAllocator"));
        }

        ExtRemoteTyped nativeCodeGen = scriptContext.Field("nativeCodeGen");

        auto forEachCodeGenAllocatorArenaAllocator = [fn](ExtRemoteTyped codeGenAllocators)
        {
            if (codeGenAllocators.GetPtr() == 0) { return; }

            fn("SC-BGJIT", codeGenAllocators.Field("allocator"));
        };
        forEachCodeGenAllocatorArenaAllocator(nativeCodeGen.Field("foregroundAllocators"));
        forEachCodeGenAllocatorArenaAllocator(nativeCodeGen.Field("backgroundAllocators"));
    }
private:
    template <class Fn>
    void ForEachCodeGenAllocatorPageAllocator(ExtRemoteTyped codeGenAllocators, bool foreground, bool useCodePageAllocators, Fn fn)
    {
        if (codeGenAllocators.GetPtr() == 0) { return; }

        // IE11 don't have parallel JIT thread added by CL# 1364258
        // So it doesn't have separate page allocator
        if (codeGenAllocators.HasField("pageAllocator"))
        {
            fn(foreground ? "FG-CodeGen" : "BG-CodeGen", RemotePageAllocator(codeGenAllocators.Field("pageAllocator")));
        }

        // Page allocators for code no longer in the custom heap after we switch to use the per thread code page allocators in RS1
        if (!useCodePageAllocators)
        {
            ExtRemoteTyped customHeap = codeGenAllocators.Field("emitBufferManager.allocationHeap");
            if (customHeap.HasField("preReservedHeapPageAllocator"))
            {
                fn(foreground ? "FG-CodePreRes" : "BG-CodePreRes", RemotePageAllocator(customHeap.Field("preReservedHeapPageAllocator")));
            }
            fn(foreground ? "FG-Code" : "BG-Code", RemotePageAllocator(customHeap.Field("pageAllocator")));
        }
    };
    ExtRemoteTyped scriptContext;
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------