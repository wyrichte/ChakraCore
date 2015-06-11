//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef TEST_LOG

namespace Js
{

    class DispIdMap
    {
        // map {IDispatch*,DISPID} to {propertyName} for logging purposes
        typedef JsUtil::BaseDictionary<DISPID, wchar_t*, ArenaAllocator, PowerOf2SizePolicy> DISPIDCache_t;
        typedef JsUtil::BaseDictionary<void *, DISPIDCache_t*, ArenaAllocator, PrimeSizePolicy> propertyNameCache_t;
        propertyNameCache_t *propertyNameCache;
        ThreadContext *threadContext;
    public:
        DispIdMap(ThreadContext *threadContext);

        void AddItem(void* pdex, DISPID id, const wchar_t* str);
        const wchar_t* LookupItem(void* pdex, DISPID id);
    };

    class HostLogger 
    {
    private:    
        DispIdMap *m_pDispIdMap;
        ThreadContext *m_pThreadContext;
        FILE* m_logfile;
        LONG currentInstance;

        static LONG numInstances;

        void DumpVar(Var value);
        void DumpType(Var aValue);
        void DumpValue(Var aValue);
        void OpenLogFile();
        void CloseLogFile();

    public:
        HostLogger(ThreadContext *threadContext);
        ~HostLogger();

        void AddCachedPropertyName(void *disp, DISPID id, const wchar_t* name);
        void LogInvokeCall(void *disp, DISPID id, ulong flags);
        void LogInvokeCall(void *disp, DISPID id, ulong flags, Js::Var value);
        void LogInvokeCall(void *disp, DISPID id, ulong flags, Js::Arguments args);
        void LogInvokeResult(Js::Var result);
    };
}

#endif
