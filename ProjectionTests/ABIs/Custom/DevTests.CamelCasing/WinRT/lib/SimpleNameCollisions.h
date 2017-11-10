//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

namespace DevTests 
{
    namespace CamelCasing
    {
        namespace SimpleNameCollisions
        {
            inline HSTRING hs(LPCWSTR sz)
            {
                HSTRING hs;
                WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
                return hs;
            }

            class InternalConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IStructConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IFireConflictingEvent
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.InternalConflict", BaseTrust);

            private:
                int m_ConflictingPropertyInt;
                int m_conflictingPropertyInt;
                StructInternalConflict m_StructConflict;
                Microsoft::WRL::EventSource<IInternalConflictHandler> _evtInternalPascal;

            public:
                InternalConflictServer() : m_ConflictingPropertyInt(42), m_conflictingPropertyInt(0)
                {
                    m_StructConflict.ConflictingField = 256;
                }
                ~InternalConflictServer()
                {
                }

                // IInternalEventConflict members
                HRESULT STDMETHODCALLTYPE add_ConflictingEvent(__in IInternalConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtInternalPascal.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_ConflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtInternalPascal.Remove(iCookie); }   

                HRESULT STDMETHODCALLTYPE addEventListener(__out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener() called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE AddEventListener(__out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.AddEventListener() called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE RemoveEventListener(__out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.RemoveEventListener() called"); return S_OK; }

                // IInternalConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value){ *value = m_conflictingPropertyInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in int value){ m_conflictingPropertyInt = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IStructConflict members
                HRESULT STDMETHODCALLTYPE get_StructConflict(__out StructInternalConflict * value) { *value = m_StructConflict; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_StructConflict(__in StructInternalConflict value) { m_StructConflict = value; return S_OK; }
            
                // IFireConflictingEvent members
                IFACEMETHOD(FireEvent)(__in ConflictingEvents evt);
            };

            class ExternalConflictSameCaseServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalPascalConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.ExternalConflictSameCase", BaseTrust);

            private:
                int m_ConflictingProperty;

            public:
                ExternalConflictSameCaseServer() : m_ConflictingProperty(1024)
                {
                }
                ~ExternalConflictSameCaseServer()
                {
                }

                // IExternalConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingProperty"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            
                // IExternalPascalConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out int * value){ *value = m_ConflictingProperty; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_ConflictingProperty(__in int value){ m_ConflictingProperty = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalPascalConflict.ConflictingMethod(int) Called"); return S_OK; }
            };

            class ExternalConflictDifferentCaseServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalCamelConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalCamelEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IEventMethodConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IFireConflictingEvent
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.ExternalConflictDifferentCase", BaseTrust);

            private:
                Microsoft::WRL::EventSource<IExternalConflictHandler> _evtExternal;
                Microsoft::WRL::EventSource<IExternalCamelConflictHandler> _evtExternalCamel;

            public:
                ExternalConflictDifferentCaseServer()
                {
                }
                ~ExternalConflictDifferentCaseServer()
                {
                }

                // IExternalEventConflict members
                HRESULT STDMETHODCALLTYPE add_ConflictingEvent(__in IExternalConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtExternal.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_ConflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtExternal.Remove(iCookie); }                       

                // IExternalConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingProperty"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            
                // IEventMethodConflict members
                HRESULT STDMETHODCALLTYPE AddEventListener(__out HSTRING * result){ *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.AddEventListener() called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_removeEventListener(__deref_out HSTRING * result){ *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener"); return S_OK; }
  
                // IExternalCamelEventConflict members
                HRESULT STDMETHODCALLTYPE add_conflictingEvent(__in IExternalCamelConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtExternalCamel.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_conflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtExternalCamel.Remove(iCookie); }                       

                // IExternalCamelConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value)
                { *value = 3; return S_OK; }

                HRESULT STDMETHODCALLTYPE conflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.ConflictingMethod(int) Called"); return S_OK; }
            
                // IFireConflictingEvent members
                IFACEMETHOD(FireEvent)(__in ConflictingEvents evt);
            };

            class InternalConflictWithExternalConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.InternalConflictWithExternalConflict", BaseTrust);

            private:
                int m_ConflictingPropertyInt;
                int m_conflictingPropertyInt;

            public:
                InternalConflictWithExternalConflictServer() : m_ConflictingPropertyInt(42), m_conflictingPropertyInt(0)
                {
                }
                ~InternalConflictWithExternalConflictServer()
                {
                }

                // IInternalConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value){ *value = m_conflictingPropertyInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in int value){ m_conflictingPropertyInt = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IExternalConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingProperty"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            };

            class InternalConflictWithExternalConflict2Server :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalConflict2
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.InternalConflictWithExternalConflict2", BaseTrust);

            private:
                int m_ConflictingPropertyInt;
                int m_conflictingPropertyInt;
                HSTRING m_conflictingPropertyString;

            public:
                InternalConflictWithExternalConflict2Server() : m_ConflictingPropertyInt(42), m_conflictingPropertyInt(0)
                {
                    m_conflictingPropertyString = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict2.conflictingProperty");
                }
                ~InternalConflictWithExternalConflict2Server()
                {
                    WindowsDeleteString(m_conflictingPropertyString);
                }

                // IInternalConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value){ *value = m_conflictingPropertyInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in int value){ m_conflictingPropertyInt = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IExternalConflict2 members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out HSTRING * value){ *value = m_conflictingPropertyString; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in HSTRING value){ m_conflictingPropertyString = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE conflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict2.conflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            };

            class DoubleConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IDoubleConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.DoubleConflict", BaseTrust);

            private:
                int m_ConflictingPropertyInt;
                int m_conflictingPropertyInt;
                HSTRING m_ConflictingPropertyString;
                HSTRING m_conflictingPropertyString;

            public:
                DoubleConflictServer() : m_ConflictingPropertyInt(42), m_conflictingPropertyInt(0)
                {
                    m_ConflictingPropertyString = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IDoubleConflict.ConflictingProperty");
                    m_conflictingPropertyString = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IDoubleConflict.conflictingProperty");
                }
                ~DoubleConflictServer()
                {
                    WindowsDeleteString(m_conflictingPropertyString);
                }

                // IInternalConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value){ *value = m_conflictingPropertyInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in int value){ m_conflictingPropertyInt = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IDoubleConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out HSTRING * value){ *value = m_ConflictingPropertyString; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_ConflictingProperty(__in HSTRING value){ m_ConflictingPropertyString = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE conflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IDoubleConflict.conflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            };

            class StaticInternalConflictFactory : 
                public Microsoft::WRL::ActivationFactory<
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IInternalEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IFireConflictingEvent
                >
            {
            private:
                int m_ConflictingPropertyInt;
                int m_conflictingPropertyInt;
                Microsoft::WRL::EventSource<IInternalConflictHandler> _evtInternalPascal;

            public:
                StaticInternalConflictFactory() : m_ConflictingPropertyInt(42), m_conflictingPropertyInt(0)
                {
                }

                // IActivationFactory
                HRESULT STDMETHODCALLTYPE ActivateInstance(__deref_out IInspectable **ppInspectable)
                { *ppInspectable = nullptr; return E_NOTIMPL; }

                // IInternalEventConflict
                HRESULT STDMETHODCALLTYPE add_ConflictingEvent(__in IInternalConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtInternalPascal.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_ConflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtInternalPascal.Remove(iCookie); }                       

                HRESULT STDMETHODCALLTYPE addEventListener(__out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.addEventListener() called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE RemoveEventListener(__out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.RemoveEventListener() called"); return S_OK; }

                // IInternalConflict
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value){ *value = m_conflictingPropertyInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_conflictingProperty(__in int value){ m_conflictingPropertyInt = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IInternalConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IFireConflictingEvent members
                IFACEMETHOD(FireEvent)(__in ConflictingEvents evt);
            };

            class StaticInternalConflictServer :
                public Microsoft::WRL::RuntimeClass<DevTests::CamelCasing::SimpleNameCollisions::IEmpty>
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.StaticInternalConflict", BaseTrust);
            };

            class StaticExternalConflictDifferentCaseFactory :
                public Microsoft::WRL::ActivationFactory<Microsoft::WRL::Implements<
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalCamelConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IExternalCamelEventConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IEventMethodConflict,
                    DevTests::CamelCasing::SimpleNameCollisions::IFireConflictingEvent>
                >
            {
            private:
                Microsoft::WRL::EventSource<IExternalConflictHandler> _evtExternal;
                Microsoft::WRL::EventSource<IExternalCamelConflictHandler> _evtExternalCamel;

            public:
                // IActivationFactory
                HRESULT STDMETHODCALLTYPE ActivateInstance(__deref_out IInspectable **ppInspectable)
                { *ppInspectable = nullptr; return E_NOTIMPL; }

                // IExternalEventConflict members
                HRESULT STDMETHODCALLTYPE add_ConflictingEvent(__in IExternalConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtExternal.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_ConflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtExternal.Remove(iCookie); }                       

                // IExternalConflict members
                HRESULT STDMETHODCALLTYPE get_ConflictingProperty(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingProperty"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(HSTRING, HSTRING, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalConflict.ConflictingMethod(HSTRING, HSTRING) Called"); return S_OK; }
            
                // IEventMethodConflict members
                HRESULT STDMETHODCALLTYPE AddEventListener(__out HSTRING * result){ *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.AddEventListener() called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_removeEventListener(__deref_out HSTRING * result){ *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IEventMethodConflict.removeEventListener"); return S_OK; }

                // IExternalCamelEventConflict members
                HRESULT STDMETHODCALLTYPE add_conflictingEvent(__in IExternalCamelConflictHandler * clickHandler, __out EventRegistrationToken * pCookie)
                { return _evtExternalCamel.Add(clickHandler, pCookie); }                        
                HRESULT STDMETHODCALLTYPE remove_conflictingEvent(__in EventRegistrationToken iCookie)
                { return _evtExternalCamel.Remove(iCookie); }                       

                // IExternalCamelConflict members
                HRESULT STDMETHODCALLTYPE get_conflictingProperty(__out int * value)
                { *value = 3; return S_OK; }

                HRESULT STDMETHODCALLTYPE conflictingMethod(int, HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelConflict.ConflictingMethod(int) Called"); return S_OK; }
            
                // IFireConflictingEvent members
                IFACEMETHOD(FireEvent)(__in ConflictingEvents evt);
            };

            class StaticExternalConflictDifferentCaseServer : 
                public Microsoft::WRL::RuntimeClass<DevTests::CamelCasing::SimpleNameCollisions::IEmpty>
            {
                InspectableClass(L"DevTests.CamelCasing.SimpleNameCollisions.StaticExternalConflictDifferentCase", BaseTrust);
            };

        }
    }
}