//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
    template<class Fn>
    static STDMETHODIMP JsDebugApiWrapper(Fn fn)
    {
        // We only use C++ exception in jscirpt9diag and don't expect SEH exceptions here. However if an asseriton is hit,
        // it triggers a SEH exception. SEH exceptions escaped here will be eaten by dbgeng in jdtest. To prevent assertions
        // from getting unnoticed, we install a SEH exception filter and crash the process.
#if DBG
        __try
        {
#endif
            return _JsDebugApiInternalWrapper(fn);
#if DBG
        }
        __except(JsDebugApiUnhandledExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
        {
        }
        return E_FAIL;
#endif
    }

#if DBG
    inline int JsDebugApiUnhandledExceptionFilter(int /* exceptionCode */, PEXCEPTION_POINTERS exceptionInfo)
    {
        LONG rc = UnhandledExceptionFilter(exceptionInfo);

        // re == EXCEPTION_EXECUTE_HANDLER means there is no debugger attached, let's terminate
        // the process. Otherwise give control to the debugger.
        // Note: in case when post-mortem debugger is registered but no actual debugger attached, 
        //       rc will be 0 (and EXCEPTION_EXECUTE_HANDLER is 1), so it acts as if there is debugger attached.
        if (rc == EXCEPTION_EXECUTE_HANDLER)
        {
            TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
        }
        else
        {
            Assert(IsDebuggerPresent());
            DebugBreak();
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
#endif

    template<class Fn>
    static STDMETHODIMP _JsDebugApiInternalWrapper(Fn fn)
    {
        HRESULT hr = NOERROR;
        try
        {
            hr = fn();
        }
        catch (const DiagException& ex)
        {
            hr = ex.hr;
        }
        catch (const CAtlException& atlEx)
        {
            hr = atlEx;
        }
        catch (const std::bad_alloc&)
        {
            hr = E_OUTOFMEMORY;
        }
        catch(...)
        {
            Assert(false);
            hr = E_UNEXPECTED;
        }
        return hr;
    }

    // Create a com object
    template <class T>
    void CreateComObject(T** pp)
    {
        CComObject<T>* p;
        CheckHR(CComObject<T>::CreateInstance(&p));
        p->AddRef();
        *pp = p;
    }

    // Create a com object, Init(arg0)
    template <class T, class Arg0>
    void CreateComObject(const Arg0& arg0, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0);
        *pp = p.Detach();
    }

    // Create a com object, Init(arg0, arg1)
    template <class T, class Arg0, class Arg1>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0, arg1);
        *pp = p.Detach();
    }

    // Create a com object, Init(arg0, arg1, arg2)
    template <class T, class Arg0, class Arg1, class Arg2>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0, arg1, arg2);
        *pp = p.Detach();
    }

    // Create a com object, Init(arg0, arg1, arg2, arg3)
    template <class T, class Arg0, class Arg1, class Arg2, class Arg3>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0, arg1, arg2, arg3);
        *pp = p.Detach();
    }

    // Create a com object, Init(arg0, arg1, arg2, arg3, arg4)
    template <class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0, arg1, arg2, arg3, arg4);
        *pp = p.Detach();
    }

    // Create a com object, Init(arg0, arg1, arg2, arg3, arg4, arg5)
    template <class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, T** pp)
    {
        CComPtr<T> p;
        CreateComObject(&p);
        p->Init(arg0, arg1, arg2, arg3, arg4, arg5);
        *pp = p.Detach();
    }


    template <class T, class I>
    void CreateComObject(I** pp)
    {
        T* p;
        CreateComObject(arg, &p);
        *pp = p;
    }

    template <class T, class Arg0, class I>
    void CreateComObject(const Arg0& arg0, I** pp)
    {
        T* p;
        CreateComObject(arg0, &p);
        *pp = p;
    }

    template <class T, class Arg0, class Arg1, class I>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, I** pp)
    {
        T* p;
        CreateComObject(arg0, arg1, &p);
        *pp = p;
    }

    template <class T, class Arg0, class Arg1, class Arg2, class I>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, I** pp)
    {
        T* p;
        CreateComObject(arg0, arg1, arg2, &p);
        *pp = p;
    }

    template <class T, class Arg0, class Arg1, class Arg2, class Arg3, class I>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, I** pp)
    {
        T* p;
        CreateComObject(arg0, arg1, arg2, arg3, &p);
        *pp = p;
    }

    template <class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4, class I>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, I** pp)
    {
        T* p;
        CreateComObject(arg0, arg1, arg2, arg3, arg4, &p);
        *pp = p;
    }

    template <class T, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class I>
    void CreateComObject(const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, I** pp)
    {
        T* p;
        CreateComObject(arg0, arg1, arg2, arg3, arg4, arg5, &p);
        *pp = p;
    }

    //
    // A COM object ref-count wrapper on a pointer. Delete the pointer in destructor.
    //
    template <class T>
    class ATL_NO_VTABLE RefCountedObject:
        public CComObjectRoot,
        public IUnknown
    {
    private:
        T* m_ptr;

    public:
        BEGIN_COM_MAP(RefCountedObject<T>)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        RefCountedObject() :
            m_ptr(nullptr)
        {
        }

        ~RefCountedObject()
        {
            if (m_ptr)
            {
                delete m_ptr;
                m_ptr = nullptr;
            }
        }

        void Init(T* ptr)
        {
            Assert(m_ptr == nullptr);
            m_ptr = ptr;
        }

        T* Get() const
        {
            return m_ptr;
        }
    };

    template <class T>
    class RefCounted: public CComPtr<RefCountedObject<T>>
    {
    };
}
