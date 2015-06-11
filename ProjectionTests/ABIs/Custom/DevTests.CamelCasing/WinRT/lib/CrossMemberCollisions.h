//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

namespace DevTests 
{
    namespace CamelCasing
    {
        namespace CrossMemberCollisions
        {
            inline HSTRING hs(LPCWSTR sz)
            {
                HSTRING hs;
                WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
                return hs;
            }

            class BuiltInConflictsStaticFactory :
                public Microsoft::WRL::ActivationFactory<
                    DevTests::CamelCasing::CrossMemberCollisions::IBuiltInConflicts
                >
            {
            private:
                static int m_hasOwnProperty;
                static int m_ToString;

            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

                // IBuiltInConflicts
                HRESULT STDMETHODCALLTYPE get_Apply(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Apply"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_hasOwnProperty(__out int * value)
                { *value = m_hasOwnProperty; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_hasOwnProperty(__in int value)
                { m_hasOwnProperty = value; return S_OK; }
                HRESULT STDMETHODCALLTYPE get_isPrototypeOf(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.isPrototypeOf"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_Prototype(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Prototype"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_ToString(__out int * value)
                { *value = m_ToString; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_ToString(__in int value)
                { m_ToString = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE Call(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Call(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE constructor(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE PropertyIsEnumerable(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.PropertyIsEnumerable(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE toLocalString(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.toLocalString(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE ValueOf(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.ValueOf(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE Length(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called"); return S_OK; }
            };

            class BuiltInConflictsStaticServer :
                public Microsoft::WRL::RuntimeClass<IInspectable>
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.BuiltInConflictsStatic", BaseTrust);
            };

            class BuiltInConflictsServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IBuiltInConflicts,
                    DevTests::CamelCasing::CrossMemberCollisions::IStructBuiltInConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.BuiltInConflicts", BaseTrust);

            private:
                StructBuiltInConflict m_StructConflict;
                int m_hasOwnProperty;
                int m_ToString;

            public:
                BuiltInConflictsServer() : m_hasOwnProperty(0), m_ToString(0)
                {
                    m_StructConflict.apply = 0;
                    m_StructConflict.Call = 0;
                    m_StructConflict.constructor = 0;
                    m_StructConflict.HasOwnProperty = 0;
                    m_StructConflict.isPrototypeOf = 0;
                    m_StructConflict.PropertyIsEnumerable = 0;
                    m_StructConflict.Prototype = 0;
                    m_StructConflict.toLocalString = 0;
                    // m_StructConflict.toString = 0;
                    m_StructConflict.ValueOf = 0;
                }
                ~BuiltInConflictsServer() { }

                // IBuiltInConflicts
                HRESULT STDMETHODCALLTYPE get_Apply(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Apply"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_hasOwnProperty(__out int * value)
                { *value = m_hasOwnProperty; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_hasOwnProperty(__in int value)
                { m_hasOwnProperty = value; return S_OK; }
                HRESULT STDMETHODCALLTYPE get_isPrototypeOf(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.isPrototypeOf"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_Prototype(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Prototype"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_ToString(__out int * value)
                { *value = m_ToString; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_ToString(__in int value)
                { m_ToString = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE Call(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Call(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE constructor(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE PropertyIsEnumerable(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.PropertyIsEnumerable(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE toLocalString(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.toLocalString(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE ValueOf(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.ValueOf(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE Length(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called"); return S_OK; }

                // IStructBuiltInConflict
                HRESULT STDMETHODCALLTYPE get_StructBuiltInConflict(__out StructBuiltInConflict * value) { *value = m_StructConflict; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_StructBuiltInConflict(__in StructBuiltInConflict value) { m_StructConflict = value; return S_OK; }
            };

            class CamelLengthConflictFactory :
                public Microsoft::WRL::ActivationFactory<
                    DevTests::CamelCasing::CrossMemberCollisions::ICamelLengthConflict
                >
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

                // ICamelLengthConflict
                HRESULT STDMETHODCALLTYPE length(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.ICamelLengthConflict.length(int) Called"); return S_OK; }
            };

            class CamelLengthConflictServer :
                public Microsoft::WRL::RuntimeClass<DevTests::CamelCasing::CrossMemberCollisions::IDummyInterface>
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.CamelLengthConflict", BaseTrust);

            public:
                // IDummyInterface
                HRESULT STDMETHODCALLTYPE GetName(__out HSTRING * name)
                { *name = hs(L"CamelLengthConflict"); return S_OK; }
            };


            class PascalLengthConflictFactory :
                public Microsoft::WRL::ActivationFactory<
                    DevTests::CamelCasing::CrossMemberCollisions::IPascalLengthConflict
                >
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

                // IPascalLengthConflict
                HRESULT STDMETHODCALLTYPE get_Length(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IPascalLengthConflict.Length"); return S_OK; }
            };

            class PascalLengthConflictServer :
                public Microsoft::WRL::RuntimeClass<DevTests::CamelCasing::CrossMemberCollisions::IDummyInterface>
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.PascalLengthConflict", BaseTrust);

            public:
                // IDummyInterface
                HRESULT STDMETHODCALLTYPE GetName(__out HSTRING * name)
                { *name = hs(L"PascalLengthConflict"); return S_OK; }
            };

            class CVectorInt : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVector<int>, Windows::Foundation::Collections::IIterable<int>>
            {
            private:
                Windows::Foundation::Collections::IVector<int> *m_pVector;

            public:
                CVectorInt();
                ~CVectorInt()
                {
                    m_pVector->Release();
                }

                //
                // IIterable members
                //
                IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<int> **first);

                //
                // IVector members
                //
                IFACEMETHOD(GetAt)(__in unsigned index, __out int *item);
                IFACEMETHOD(get_Size)(__out unsigned *size);
                IFACEMETHOD(IndexOf)(__in_opt int value, __out unsigned *index, __out boolean *found);    
                IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<int> **returnValue);
                IFACEMETHOD(SetAt)(__in unsigned index, __in_opt int value);
                IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt int value); 
                IFACEMETHOD(RemoveAt)(__in unsigned index);
                IFACEMETHOD(Append)(__in_opt int value);
                IFACEMETHOD(RemoveAtEnd)();
                IFACEMETHOD(Clear)();
            };

            class VectorLengthConflictServer :
                public Microsoft::WRL::RuntimeClass<DevTests::CamelCasing::CrossMemberCollisions::IBuiltInConflicts, CVectorInt>
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.VectorLengthConflict", BaseTrust);

            private:
                int m_hasOwnProperty;
                int m_ToString;

            public:
                VectorLengthConflictServer() : m_hasOwnProperty(0), m_ToString(0) { }
                ~VectorLengthConflictServer() { }

                // IBuiltInConflicts
                HRESULT STDMETHODCALLTYPE get_Apply(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Apply"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_hasOwnProperty(__out int * value)
                { *value = m_hasOwnProperty; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_hasOwnProperty(__in int value)
                { m_hasOwnProperty = value; return S_OK; }
                HRESULT STDMETHODCALLTYPE get_isPrototypeOf(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.isPrototypeOf"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_Prototype(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Prototype"); return S_OK; }
                HRESULT STDMETHODCALLTYPE get_ToString(__out int * value)
                { *value = m_ToString; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_ToString(__in int value)
                { m_ToString = value; return S_OK; }

                HRESULT STDMETHODCALLTYPE Call(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Call(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE constructor(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.constructor(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE PropertyIsEnumerable(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.PropertyIsEnumerable(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE toLocalString(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.toLocalString(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE ValueOf(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.ValueOf(int) Called"); return S_OK; }
                HRESULT STDMETHODCALLTYPE Length(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IBuiltInConflicts.Length(int) Called"); return S_OK; }
            };

            class InternalCrossMemberConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.InternalCrossMemberConflict", BaseTrust);

            public:
                // IInternalCrossMemberConflict
                HRESULT STDMETHODCALLTYPE get_conflicting(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called"); return S_OK; }
            };

            class ExternalPropPropConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict,
                    DevTests::CamelCasing::CrossMemberCollisions::IExternalPropertyConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.ExternalPropPropConflict", BaseTrust);

            private:
                int m_ConflictingInt;

            public:
                ExternalPropPropConflictServer() : m_ConflictingInt(0) { }
                ~ExternalPropPropConflictServer() { }

                // IInternalCrossMemberConflict
                HRESULT STDMETHODCALLTYPE get_conflicting(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IExternalPropertyConflict
                HRESULT STDMETHODCALLTYPE get_Conflicting(__out int * value)
                { *value = m_ConflictingInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_Conflicting(__in int value)
                { m_ConflictingInt = value; return S_OK; }
            };

            class ExternalPropMethodConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict,
                    DevTests::CamelCasing::CrossMemberCollisions::IExternalMethodConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.ExternalPropMethodConflict", BaseTrust);

            public:
                // IInternalCrossMemberConflict
                HRESULT STDMETHODCALLTYPE get_conflicting(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IExternalMethodConflict
                HRESULT STDMETHODCALLTYPE Conflicting(__in HSTRING, __in HSTRING, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.Conflicting(HSTRING,HSTRING) Called"); return S_OK; }
            };

            class ExternalMethodMethodConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict2,
                    DevTests::CamelCasing::CrossMemberCollisions::IExternalMethodConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.ExternalMethodMethodConflict", BaseTrust);

            public:
                // IInternalCrossMemberConflict2
                HRESULT STDMETHODCALLTYPE get_ConflictingProp(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp"); return S_OK; }

                HRESULT STDMETHODCALLTYPE conflicting(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called"); return S_OK; }

                // IExternalMethodConflict
                HRESULT STDMETHODCALLTYPE Conflicting(__in HSTRING, __in HSTRING, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IExternalMethodConflict.Conflicting(HSTRING,HSTRING) Called"); return S_OK; }
            };

            class ExternalMethodPropConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict2,
                    DevTests::CamelCasing::CrossMemberCollisions::IExternalPropertyConflict
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.ExternalMethodPropConflict", BaseTrust);

            private:
                int m_ConflictingInt;

            public:
                ExternalMethodPropConflictServer() : m_ConflictingInt(0) { }
                ~ExternalMethodPropConflictServer() { }

                // IInternalCrossMemberConflict2
                HRESULT STDMETHODCALLTYPE get_ConflictingProp(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp"); return S_OK; }

                HRESULT STDMETHODCALLTYPE conflicting(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called"); return S_OK; }

                // IExternalPropertyConflict
                HRESULT STDMETHODCALLTYPE get_Conflicting(__out int * value)
                { *value = m_ConflictingInt; return S_OK; }
                HRESULT STDMETHODCALLTYPE put_Conflicting(__in int value)
                { m_ConflictingInt = value; return S_OK; }
            };

            class DoubleCrossMemberConflictServer :
                public Microsoft::WRL::RuntimeClass<
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict,
                    DevTests::CamelCasing::CrossMemberCollisions::IInternalCrossMemberConflict2
                >
            {
                InspectableClass(L"DevTests.CamelCasing.CrossMemberCollisions.DoubleCrossMemberConflict", BaseTrust);

            public:
                // IInternalCrossMemberConflict
                HRESULT STDMETHODCALLTYPE get_conflicting(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.conflicting"); return S_OK; }

                HRESULT STDMETHODCALLTYPE ConflictingMethod(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict.ConflictingMethod(int) Called"); return S_OK; }

                // IInternalCrossMemberConflict2
                HRESULT STDMETHODCALLTYPE get_ConflictingProp(__out HSTRING * value)
                { *value = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.ConflictingProp"); return S_OK; }

                HRESULT STDMETHODCALLTYPE conflicting(__in int, __out HSTRING * result)
                { *result = hs(L"DevTests.CamelCasing.CrossMemberCollisions.IInternalCrossMemberConflict2.conflicting(int) Called"); return S_OK; }
            };
        }
    }
}