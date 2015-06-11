//
//    Copyright (C) Microsoft.  All rights reserved.
//
// use these lines to enable/disable CodeMarkers support
#if defined(_M_IX86)
#define CODEMARKERSENABLED
#endif

#ifdef CODEMARKERSENABLED
#define Codemarkers_IncludeBrowserToolsMarkers
#include "CodeMarkers\CodeMarkers.h"
#define CODEMARKER(m) CodeMarker(m)
#else
#define CODEMARKER(m)
#endif

#ifndef IfFailGo
#define IfFailGo(expr) IfFailGoto(expr, Error)

#define IfFailGoto(expr, label) \
 do {                           \
  hr = (expr);                  \
  if (FAILED (hr)) {            \
   goto label;                  \
  }                             \
 } while (0)
#endif

#define IfFailThrow(hr) do { if (FAILED(hr)) throw HrException(hr); } while (0)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)

#define ValidatePtr(expr, err) \
    do { \
      if (!(expr)) { \
        hr = err; \
        goto Error; \
      } \
    } while (0)

#define ValidateAlloc(expr) ValidatePtr(expr, E_OUTOFMEMORY);
#define ValidateArg(expr) ValidatePtr(expr, E_INVALIDARG);
#define Validate(expr) ValidatePtr(expr, E_FAIL)

class HrException : public Js::ExceptionBase
{
public:
    HRESULT hr;
    HrException(HRESULT hr) : hr(hr) { }
};

namespace Authoring 
{
    // The following functions ensure an idempotent releaseing or disposing of memory
    // by first getting the value of the pointer, setting the original to null then
    // releasing or freeing the memory. This guarentees idempotentcy for single
    // threaded execution. Multi-threaded would required an interlocked-exchange
    // of null and ptr.
    template<class T>
    inline void ReleasePtr(T*& ptr) 
    {
        T* t = ptr;
        ptr = null;
        if (t) t->Release();
    }

    template<class T>
    inline void FreePtr(T*& ptr)
    {
        T* t = ptr;
        ptr = null;
        if (t) free(t);
    }

    template<class T>
    inline void ADeletePtr(ArenaAllocator *alloc, T*& ptr)
    {
        T* t = ptr;
        ptr = null;
        if (t) Adelete(alloc, t);
    }

    template <class T>
    inline void DeletePtr(T*& ptr)
    {
        T* t = ptr;
        ptr = null;
        if (t) delete t;
    }

    template <class T>
    inline void HeapDeletePtr(T*& ptr)
    {
        T* t = ptr;
        ptr = null;
        if (t) HeapDelete(t);
    }

    template< size_t N >
    IdentPtr CreatePidFromLiteral( Parser *parser, const wchar_t (&w)[N] )
    {
        return parser->CreatePid(w, N - 1); // -1 because the size contains the null terminator
    }

    template< size_t N >
    Js::PropertyId GetOrAddPropertyIdFromLiteral( Js::ScriptContext *scriptContext, const wchar_t (&w)[N] )
    {
        return scriptContext->GetOrAddPropertyIdTracked(w, N - 1); // -1 because the size contains the null terminator
    }

    template< size_t N, typename E >
    inline size_t AllocSize(const E (&w)[N]) { return N; }

    template< size_t N >
    inline size_t LengthOfLiteral(const wchar_t (&w)[N]) { return N - 1; }

// Macros to use in STDMETHOD() implementations
#define STDMETHOD_PREFIX HRESULT hr = S_OK; try { AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
#define STDMETHOD_POSTFIX STDMETHOD_POSTFIX_CLEAN_START STDMETHOD_POSTFIX_CLEAN_END
#define STDMETHOD_POSTFIX_CLEAN_START } catch(Js::OutOfMemoryException) { hr = E_OUTOFMEMORY; } catch(Js::StackOverflowException) { hr = VBSERR_OutOfStack; } catch(HrException ex) { hr = ex.hr; } Error:
#define STDMETHOD_POSTFIX_CLEAN_END return hr == E_ABORT ? S_FALSE : hr

// Macros to use in internal method implementations
#define METHOD_PREFIX HRESULT hr = S_OK
#define METHOD_POSTFIX Error: return hr
#define METHOD_POSTFIX_CLEAN_START Error:
#define METHOD_POSTFIX_CLEAN_END return hr

// Macros to use in lambda method implementations
#define INTERNALMETHOD_PREFIX HRESULT hr = S_OK
#define INTERNALMETHOD_POSTFIX InternalError: return hr
#define INTERNALMETHOD_POSTFIX_CLEAN_START InternalError:
#define INTERNALMETHOD_POSTFIX_CLEAN_END return hr

#define IfFailGoInternal(expr) IfFailGoto(expr, InternalError)

    inline bool InRange(charcount_t offset, charcount_t min, charcount_t lim, bool excludeEndOffset = false)
    {
        return offset >= min && ((!excludeEndOffset && offset <= lim) || (excludeEndOffset && offset < lim));
    }

    inline bool ContainsOffset(ParseNode* node, charcount_t offset)
    {
        Assert(node);
        return InRange(offset, node->ichMin, node->ichLim);
    }

    inline bool IsZeroExtent(ParseNode* pnode)
    {
        Assert(pnode);
        return pnode->ichMin == pnode->ichLim;
    }

    long ActualMin(ParseNode *pnode);
    long ActualLim(ParseNode *pnode);

    bool IsInCallParenthesis(charcount_t offset, ParseNode* callNode, bool includeLParen, LanguageServiceExtension* extensions);
    wchar_t ExtensionsObjectName[];
    bool IsExtensionObjectName(LPCWSTR name, charcount_t length);
    void ApplyLocation(ParseNodePtr pnode, charcount_t location);
    ParseNodePtr *FindEndCodeRef(ParseNodePtr pnodeFnc);
    int CountNestedFunctions(ParseNodePtr scope);
    ParseNodePtr *FindNodeReference(ParseNodePtr parent, ParseNodePtr node, bool fullScope = false);
    ParseNode* GetPreviousNode(ParseNode* parent, ParseNode* node);

    bool InternalName(LPCWSTR name);
    bool InternalName(IdentPtr name);

#if DEBUG
    int InstructionsExecuted(Js::ScriptContext *scriptContext);
#endif

    struct NullType { };

    const Js::RegSlot NoRegister = (Js::RegSlot)-1;

    typedef void (*ScriptContextFreeCallback)(void *data, Js::ScriptContext *scriptContext);

    class RefCountedScriptContext 
    {
    private:
#if TRACK_TYPE_STATS
        static TypeStats _typeStats;
#endif
        struct CallbackEntry
        {
            ScriptContextFreeCallback callback;
            void *data;

            CallbackEntry() : callback(nullptr), data(nullptr) { }
            CallbackEntry(ScriptContextFreeCallback callback, void *data): callback(callback), data(data) { }
        };

        template<typename T>
        struct CallbackComparer
        {
            static bool Equals(CallbackEntry x, CallbackEntry y)
            {
                return x.callback == y.callback && x.data == y.data;
            }
        };
        
        static JsUtil::List<CallbackEntry, HeapAllocator, true, Js::CopyRemovePolicy, CallbackComparer> callbackEntries;

        uint _refCount;
        Js::ScriptContext* _scriptContext;

        RefCountedScriptContext (Js::ScriptContext* scriptContext) 
            : _refCount(0)
        {
#if TRACK_TYPE_STATS
                _typeStats.IncrementInstances();
#endif

            _scriptContext = scriptContext;
            if (_scriptContext)
            {
                AddRef();
            }
        }

    public:
        static RefCountedScriptContext* Create(Js::ScriptContext* scriptContext)
        {
            return HeapNew(RefCountedScriptContext, scriptContext);
        }

        ~RefCountedScriptContext()
        {
#if TRACK_TYPE_STATS
            _typeStats.DecrementInstances();
#endif
            _scriptContext->MarkForClose();
        }

        Js::ScriptContext* Ptr()
        {
            return _scriptContext;
        }

        void AddRef() 
        {
            ++_refCount;
        }

        void Release()
        {
            if (--_refCount == 0)
            {
                callbackEntries.Map([this](int index, CallbackEntry callbackEntry) {
                    callbackEntry.callback(callbackEntry.data, this->_scriptContext);
                });
                HeapDelete(this);
            }
        }

        static void ObserveFree(ScriptContextFreeCallback callback, void *data)
        {
            CallbackEntry entry(callback, data);
            callbackEntries.Add(entry);
        }

        static void UnobserveFree(ScriptContextFreeCallback callback, void *data)
        {
            CallbackEntry entry(callback, data);
            callbackEntries.Remove(entry);
        }
    };

    struct DeletePolicy
    {
        struct OperatorDelete
        {
            typedef void Allocator;
            template<typename T>
            static void Delete(T* obj) { delete obj; }
        };

        struct ArenaDelete
        {
            typedef ArenaAllocator Allocator;
            template<typename T>
            static void Delete(T* obj) { Adelete(obj->Alloc(), obj); }
        };
    };

    // TDeletePolicy values: 
    //  DeletePolicy::OperatorDelete, 
    //  DeletePolicy::ArenaDelete etc.
    template<typename TDeletePolicy>
    class RefCounted
    {
        typedef typename TDeletePolicy::Allocator Allocator;
        uint _refCount;

#if TRACK_TYPE_STATS
        static TypeStats _typeStats;
#endif

    public:
        RefCounted() : _refCount(0) 
        {
#if TRACK_TYPE_STATS
            _typeStats.IncrementInstances();
#endif
        }

        virtual ~RefCounted() 
        {
#if TRACK_TYPE_STATS
            _typeStats.DecrementInstances();
#endif
        }

        void AddRef() 
        { 
            ++_refCount; 
        }
        
        void Release() 
        { 
            if(--_refCount == 0) 
            {
                TDeletePolicy::Delete(this);
            }
        }

        virtual Allocator* Alloc() 
        { 
            Assert(false);
            return nullptr;
        } 
    };

    template<typename T>
    class RefCountedPtr : public AutoReleasePtr<T>
    {
    public:
        RefCountedPtr(): AutoReleasePtr<T>(nullptr) { }

        RefCountedPtr(T* p) 
            : AutoReleasePtr<T>(p)
        {
            if (ptr)
            {
                ptr->AddRef();
            }
        }

        operator bool ()
        {
            return ptr ? true : false;
        }

        RefCountedPtr& Assign(T* p)
        {
            return Assign(p, false);
        }

        RefCountedPtr& TakeOwnership(T* p)
        {
            return Assign(p, true);
        }

        void ReleaseAndNull()
        {
            Assign(nullptr, false);
        }

        T* Detach()
        {
            T* p = ptr;
            ptr = nullptr;
            return p;
        }

    private:
        RefCountedPtr& operator = (T* p) 
        {
            return Assign(p, false);
        }

        RefCountedPtr& Assign(T* p, bool takeOwnership)
        {
            if (ptr)
            {
                Release();
            }

            ptr = p;

            if (ptr)
            {
                ptr->AddRef();

                if (takeOwnership)
                {
                    p->Release();
                }
            }

            return *this;
        }
    };

    class ScriptContextAutoPtr : public AutoReleasePtr<RefCountedScriptContext>
    {
    public:
        ScriptContextAutoPtr(): AutoReleasePtr<RefCountedScriptContext>(nullptr) { }

        ScriptContextAutoPtr(RefCountedScriptContext* contextPtr) 
            : AutoReleasePtr<RefCountedScriptContext>(contextPtr)
        {
            if (ptr)
            {
                ptr->AddRef();
            }
        }

        Js::ScriptContext* operator -> () 
        { 
            Assert(ptr != null); 
            return ptr->Ptr(); 
        }	

        operator Js::ScriptContext* ()
        {
            return ptr ? ptr->Ptr() : nullptr;
        }

        operator RefCountedScriptContext* ()
        {
            return ptr;
        }

        operator bool ()
        {
            return (ptr && ptr->Ptr()) ? true : false;
        }

        ScriptContextAutoPtr& Assign(RefCountedScriptContext* contextPtr)
        {
            return Assign(contextPtr, false);
        }

        ScriptContextAutoPtr& TakeOwnership(RefCountedScriptContext* contextPtr)
        {
            return Assign(contextPtr, true);
        }

        void ReleaseAndNull()
        {
            Assign(nullptr);
        }

    private:

        ScriptContextAutoPtr& Assign(RefCountedScriptContext* contextPtr, bool takeOwnership)
        {
            if (ptr)
            {
                Release();
            }

            ptr = contextPtr;

            if (ptr)
            {
                ptr->AddRef();
                
                if (takeOwnership)
                {
                    contextPtr->Release();
                }
            }

            return *this;
        }
    };

    class ListHelpers 
    {
    public:
        template<typename TElement, typename TCompare>
        static int BinarySearch(JsUtil::List<TElement, ArenaAllocator>& list, TCompare Compare, int from, int to)
        {
            if(to < from)
                return -1;

            int mid = from + (to - from) / 2;
            auto item = list.Item(mid);
            int compareResult = Compare(item);
            if(compareResult > 0)
                return BinarySearch<TElement, TCompare>(list, Compare, from, mid - 1);
            else if(compareResult < 0)
                return BinarySearch<TElement, TCompare>(list, Compare, mid + 1, to);
            else 
                return mid;
        }

        template<typename TElement, typename TCompare>
        static int BinarySearch(JsUtil::List<TElement, ArenaAllocator>& list, TCompare Compare)
        {
            return BinarySearch<TElement, TCompare>(list, Compare, 0, list.Count() - 1);
        }
    };

    template<typename T>
    struct TemporaryAssignment
    {
    private:
        T *locationRef;
        T original;
    public:
        TemporaryAssignment(T& location, T value) 
        {
            locationRef = &location;
            original = location;
            location = value;
        }
        ~TemporaryAssignment()
        {
            *locationRef = original;
        }
    };

#if DEBUG
    void DumpIdOfName(Js::ScriptContext *scriptContext, LPCWSTR name);
    Js::Var DebugGetPropertyOf(Js::DynamicObject *obj, LPCWSTR propertyName);
#endif
}
