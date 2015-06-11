//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Js
{
    //
    // Base class for implementation of IUnknown and class factory (CreateInstance),
    // similar to CComObjectRootEx and CComCoClass combined together,
    // and taking care of library/module count in jscript9.dll,.
    // - QI supports (only) IUnknown and TInterface.
    // - Example:
    //   class IMyClass { virtual HRESULT Foo() = 0; };
    //   class MyClass : public ComObjectBase<IMyClass, MyClass>
    //   {
    //       // Here you already have IUnknown implementation and CreateInstance. Just add IMyClass methods.
    //       virtual HRESULT Foo() { ... }
    //   }
    //  CComPtr<MyClass> pMyClass;
    //  IFFAILRET(MyClass::CreateInstance(&pMyClass));
    //
    template <typename TInterface, typename TClass>     // Note: TClass must have accessible default ctor.
    class ComObjectBase : public TInterface
    {
    protected:
        ULONG m_refCount;

        ComObjectBase() : m_refCount(1)
        {
            DLLAddRef(); // One DLL reference for each existing instance.
        }

    public:
        virtual ~ComObjectBase() // "virtual" ensures subclass destructors are called when "delete this"
        {
            DLLRelease();
        }

        void FinalDestruct()
        {
        }

        STDMETHODIMP_(ULONG) AddRef()
        {
            Assert(m_refCount > 0);
            return InterlockedIncrement(&m_refCount);
        }

        STDMETHODIMP_(ULONG) Release()
        {
            Assert(m_refCount > 0);
            ULONG ref = InterlockedDecrement(&m_refCount);
            if (ref == 0)
            {
                ((TClass*)this)->FinalDestruct();                
                delete this;
            }
            return ref;
        }

        STDMETHODIMP QueryInterface(REFIID riid, void** ppv) sealed
        {
            IfNullReturnError(ppv, E_POINTER);

            if (IsEqualGUID(riid, IID_IUnknown)
                || IsEqualGUID(riid, __uuidof(TInterface)))
            {
                *ppv = static_cast<TInterface*>(this);
                AddRef();
                return S_OK;
            }

            *ppv = NULL;
            return E_NOINTERFACE;
        }

        static HRESULT CreateInstance(TClass** ppInstance)
        {
            TClass* pInstance = new TClass();
            IfNullReturnError(pInstance, E_OUTOFMEMORY);
            *ppInstance = pInstance;
            return S_OK;
        }
    };
} // namespace Js.
