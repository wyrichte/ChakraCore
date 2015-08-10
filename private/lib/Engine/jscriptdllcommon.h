//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Common header for jscript.dll related definitions, macros, helpers etc. 

#pragma once

// handy macro for checking HRESULTS.
#define IfFailRet(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IFEMPTYSETNULL(p) do { if ((p) && (0 == *(p))) p = NULL; } while(0)
#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define ReleasePointer(p) do {if (p){ (p)->Release(); (p) = NULL; }} while (FALSE)

#define IfFailGoto(expr, label) \
    do                          \
    {                           \
        hr = (expr);            \
        if (FAILED (hr))        \
        {                       \
            goto label;         \
        }                       \
    } while (FALSE)             \
 
#define IfFailGo(expr) IfFailGoto(expr, Error)

template  <typename T>
class AutoLeaveScriptPtr : public BasePtr<T>
#if DBG
    , AutoNestedHandledExceptionType
#endif
{
public:
    AutoLeaveScriptPtr(Js::ScriptContext* scriptContext, T * ptr = null) : 
      BasePtr(null), scriptContext(scriptContext)
#if DBG
          , AutoNestedHandledExceptionType(ExceptionType_HasStackProbe)
#endif
      {
          // Theoretically the check here is not enough. The dtor will be called during unwind high up
          // on the stack IF there were exception thrown in the scope. Generally we won't see exceptions
          // other than OOM so chance of exception is low. the stack probe is defend in depth in the first 
          // place. 
          scriptContext->GetThreadContext()->ProbeStack(Js::Constants::MinStackCallout, scriptContext);
          this->ptr = ptr;
      }    

    ~AutoLeaveScriptPtr()
    {
        Release();
    }

    void Release()
    {
        if (ptr != null)
        {
            BEGIN_LEAVE_SCRIPT_NO_STACK_PROBE(scriptContext)
            {
                ptr->Release();
            }
            END_LEAVE_SCRIPT_NO_STACK_PROBE(scriptContext)
            this->ptr = null;
        }
    }
private:
    Js::ScriptContext* scriptContext;
};

#if _MSC_VER >= 1200
#define STDAPIEXPORT STDAPI
#else // _MSC_VER >= 1200
#define STDAPIEXPORT _declspec(dllexport) STDAPI
#endif // _MSC_VER >= 1200
#define MULTITHREAD 1

FORCEINLINE
VOID
InitializeListHead(
    __out PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    __in PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

__checkReturn
BOOLEAN
FORCEINLINE
IsListEmpty(
    __in const LIST_ENTRY * ListHead
    )
{
    return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    __inout PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}

FORCEINLINE
VOID
InsertHeadList(
    __inout PLIST_ENTRY ListHead,
    __inout PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

FORCEINLINE
PLIST_ENTRY
NextEntryList(
    __in PLIST_ENTRY Entry
    )
{
    return Entry->Flink;
}

FORCEINLINE
BOOLEAN
IsEntryListTail(
    __inout PLIST_ENTRY ListHead,
    __inout PLIST_ENTRY Entry
    )
{
    return Entry->Flink == ListHead;
}

#define MAX_PROGID_LENGTH 39
#define NOBASETHREAD 0xFFFFFFFF


#define QI_IMPL(name, intf)\
    if (IsEqualIID(riid, name))\
{   \
    *ppvObj = static_cast<intf *>(this); \
    AddRef();\
    return NOERROR;\
}\

#define QI_IMPL_I(intf) QI_IMPL(_uuidof(intf), intf)

class ScriptSite;