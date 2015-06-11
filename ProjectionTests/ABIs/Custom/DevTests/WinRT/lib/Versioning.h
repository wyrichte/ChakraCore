//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>

#define NTDDI_MOCK_WIN8SP1  0x06020100
#define NTDDI_MOCK_WIN9     0x06030000
#define NTDDI_MAX           0xFFFFFFFE

namespace DevTests
{
    namespace Versioning
    {
        /*                       *
         * -- Factory Classes -- *
         *                       */
        class MinVersionFactory :
            public Microsoft::WRL::ActivationFactory<IMinFactory, IWin8SP1Interface>
        {
        public:
            MinVersionFactory(){ }
            ~MinVersionFactory(){ }

            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable ** ppInspectable);
            // IMinFactory
            IFACEMETHOD(CreateClass)(__in int value, __out IMinVersionInterface** instance);
            // IWin8SP1Interface
            IFACEMETHOD(get_Win8SP1Version)(__out unsigned int * value){ *value = NTDDI_MOCK_WIN8SP1; return S_OK; }
        };

        class Win8Factory :
            public Microsoft::WRL::ActivationFactory<IWin8Factory, IWin8Interface>
        {
        public:
            Win8Factory(){ }
            ~Win8Factory(){ }

            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable ** ppInspectable);
            // IWin8Factory
            IFACEMETHOD(CreateClass)(__in int value, __out IMinVersionInterface** instance);
            // IWin8Interface
            IFACEMETHOD(get_Win8Version)(__out unsigned int * value){ *value = NTDDI_WIN8; return S_OK; }
        };

        class Win8SP1Factory :
            public Microsoft::WRL::ActivationFactory<IWin8SP1Factory, IWin8Interface>
        {
        public:
            Win8SP1Factory(){ }
            ~Win8SP1Factory(){ }

            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable ** ppInspectable);
            // IWin8SP1Factory
            IFACEMETHOD(CreateClass)(__in int value, __out IMinVersionInterface** instance);
            // IWin8Interface
            IFACEMETHOD(get_Win8Version)(__out unsigned int * value){ *value = NTDDI_WIN8; return S_OK; }
        };

        class Win9Factory :
            public Microsoft::WRL::ActivationFactory<IWin9Factory, IMinVersionInterface>
        {
        public:
            Win9Factory(){ }
            ~Win9Factory(){ }

            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable ** ppInspectable);
            // IWin9Factory
            IFACEMETHOD(CreateClass)(__in int value, __out IWin8Interface** instance);
            // IMinVersionInterface
            IFACEMETHOD(get_MinVersion)(__out unsigned int * value){ *value = 0; return S_OK; }
        };

        /*                        *
         * -- Instance Classes -- *
         *                        */
        class MinVersionClass :
            public Microsoft::WRL::RuntimeClass<IMinVersionInterface, IWin9Interface, IMaxVersionInterface, IActivationValue>
        {
            InspectableClass(L"DevTests.Versioning.MinVersionClass", BaseTrust);

            int m_activationValue;

        public:
            MinVersionClass() : m_activationValue(0) { }
            MinVersionClass(int value) : m_activationValue(value) { }
            ~MinVersionClass(){ }

            // IMinVersionInterface
            IFACEMETHOD(get_MinVersion)(__out unsigned int * value){ *value = 0; return S_OK; }
            // IWin9Interface
            IFACEMETHOD(get_Win9Version)(__out unsigned int * value){ *value = NTDDI_MOCK_WIN9; return S_OK; }
            // IMaxVersionInterface
            IFACEMETHOD(get_MaxVersion)(__out unsigned int * value){ *value = NTDDI_MAX; return S_OK; }
            // IActivationValue
            IFACEMETHOD(get_ActivationValue)(__out int * value){ *value = m_activationValue; return S_OK; }
        };

        class Win8Class :
            public Microsoft::WRL::RuntimeClass<IMinVersionInterface, IWin8SP1Interface, IActivationValue>
        {
            InspectableClass(L"DevTests.Versioning.Win8Class", BaseTrust);

            int m_activationValue;

        public:
            Win8Class() : m_activationValue(0) { }
            Win8Class(int value) : m_activationValue(value) { }
            ~Win8Class(){ }

            // IMinVersionInterface
            IFACEMETHOD(get_MinVersion)(__out unsigned int * value){ *value = 0; return S_OK; }
            // IWin8SP1Interface
            IFACEMETHOD(get_Win8SP1Version)(__out unsigned int * value){ *value = NTDDI_MOCK_WIN8SP1; return S_OK; }
            // IActivationValue
            IFACEMETHOD(get_ActivationValue)(__out int * value){ *value = m_activationValue; return S_OK; }
        };

        class Win8SP1Class :
            public Microsoft::WRL::RuntimeClass<IMinVersionInterface, IWin9Interface, IActivationValue>
        {
            InspectableClass(L"DevTests.Versioning.Win8SP1Class", BaseTrust);

            int m_activationValue;

        public:
            Win8SP1Class() : m_activationValue(0) { }
            Win8SP1Class(int value) : m_activationValue(value) { }
            ~Win8SP1Class(){ }

            // IMinVersionInterface
            IFACEMETHOD(get_MinVersion)(__out unsigned int * value){ *value = 0; return S_OK; }
            // IWin9Interface
            IFACEMETHOD(get_Win9Version)(__out unsigned int * value){ *value = NTDDI_MOCK_WIN9; return S_OK; }
            // IActivationValue
            IFACEMETHOD(get_ActivationValue)(__out int * value){ *value = m_activationValue; return S_OK; }
        };

        class Win9Class :
            public Microsoft::WRL::RuntimeClass<IWin8Interface, IWin9Interface, IActivationValue>
        {
            InspectableClass(L"DevTests.Versioning.Win9Class", BaseTrust);

            int m_activationValue;

        public:
            Win9Class() : m_activationValue(0) { }
            Win9Class(int value) : m_activationValue(value) { }
            ~Win9Class(){ }

            // IWin8Interface
            IFACEMETHOD(get_Win8Version)(__out unsigned int * value){ *value = NTDDI_WIN8; return S_OK; }
            // IWin9Interface
            IFACEMETHOD(get_Win9Version)(__out unsigned int * value){ *value = NTDDI_MOCK_WIN9; return S_OK; }
            // IActivationValue
            IFACEMETHOD(get_ActivationValue)(__out int * value){ *value = m_activationValue; return S_OK; }
        };

        class MaxVersionClass :
            public Microsoft::WRL::RuntimeClass<IMaxVersionInterface>
        {
            InspectableClass(L"DevTests.Versioning.MaxVersionClass", BaseTrust);

        public:
            MaxVersionClass(){ }
            ~MaxVersionClass(){ }

            // IMaxVersionInterface
            IFACEMETHOD(get_MaxVersion)(__out unsigned int * value){ *value = NTDDI_MAX; return S_OK; }
        };

        class MarshalVersionedTypes :
            public Microsoft::WRL::RuntimeClass<IMarshalVersionedTypes>
        {
            InspectableClass(L"DevTests.Versioning.MarshalVersionedTypes", BaseTrust);

            MinVersionStruct m_minStruct;
            Win8Struct m_win8Struct;
            Win8SP1Struct m_win8sp1Struct;
            Win9Struct m_win9Struct;
            MaxVersionStruct m_maxStruct;

        public:
            MarshalVersionedTypes(){ }
            ~MarshalVersionedTypes(){ }

            // Version 0 types
            IFACEMETHOD(MinVersionInterfaceIn)(__in IMinVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(MinVersionClassIn)(__in IMinVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(MinVersionStructIn)(__in MinVersionStruct value){ m_minStruct = value; return S_OK; }
            IFACEMETHOD(MinVersionStructOut)(__out MinVersionStruct * value){ *value = m_minStruct; return S_OK; }
            IFACEMETHOD(MinVersionEnumIn)(__in MinVersionEnum value){ value; return S_OK; }
            IFACEMETHOD(MinVersionEnumOut)(__out MinVersionEnum * value){ *value = MinVersionEnum_Min; return S_OK; }
            IFACEMETHOD(CallMinVersionDelegate)(__in int inputVersion, __in IMinVersionDelegate * value);
            IFACEMETHOD(MinVersionInterfaceVectorIn)(__in Windows::Foundation::Collections::IVector<IMinVersionInterface *> * value)
            { 
                if (!value) { return E_POINTER; }
                return S_OK; 
            }

            // Version NTDDI_WIN8 types
            IFACEMETHOD(Win8InterfaceIn)(__in IWin8Interface * value){ value; return S_OK; }
            IFACEMETHOD(Win8ClassIn)(__in IMinVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(Win8StructIn)(__in Win8Struct value){ m_win8Struct = value; return S_OK; }
            IFACEMETHOD(Win8StructOut)(__out Win8Struct * value){ *value = m_win8Struct; return S_OK; }
            IFACEMETHOD(Win8EnumIn)(__in Win8Enum value){ value; return S_OK; }
            IFACEMETHOD(Win8EnumOut)(__out Win8Enum * value){ *value = Win8Enum_Win8; return S_OK; }
            IFACEMETHOD(CallWin8Delegate)(__in IWin8Delegate * value);
            IFACEMETHOD(Win8InterfaceVectorIn)(__in Windows::Foundation::Collections::IVector<IWin8Interface *> * value)
            { 
                if (!value) { return E_POINTER; }
                return S_OK; 
            }

            // Version NTDDI_MOCK_WIN8SP1 types
            IFACEMETHOD(Win8SP1InterfaceIn)(__in IWin8SP1Interface * value){ value; return S_OK; }
            IFACEMETHOD(Win8SP1ClassIn)(__in IMinVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(Win8SP1StructIn)(__in Win8SP1Struct value){ m_win8sp1Struct = value; return S_OK; }
            IFACEMETHOD(Win8SP1StructOut)(__out Win8SP1Struct * value){ *value = m_win8sp1Struct; return S_OK; }
            IFACEMETHOD(Win8SP1EnumIn)(__in Win8SP1Enum value){ value; return S_OK; }
            IFACEMETHOD(Win8SP1EnumOut)(__out Win8SP1Enum * value){ *value = Win8SP1Enum_Win8SP1; return S_OK; }
            IFACEMETHOD(CallWin8SP1Delegate)(__in IWin8SP1Delegate * value);
            IFACEMETHOD(Win8SP1InterfaceVectorIn)(__in Windows::Foundation::Collections::IVector<IWin8SP1Interface *> * value)
            { 
                if (!value) { return E_POINTER; }
                return S_OK; 
            }

            // Version NTDDI_MOCK_WIN9 types
            IFACEMETHOD(Win9InterfaceIn)(__in IWin9Interface * value){ value; return S_OK; }
            IFACEMETHOD(Win9ClassIn)(__in IWin8Interface * value){ value; return S_OK; }
            IFACEMETHOD(Win9StructIn)(__in Win9Struct value){ m_win9Struct = value; return S_OK; }
            IFACEMETHOD(Win9StructOut)(__out Win9Struct * value){ *value = m_win9Struct; return S_OK; }
            IFACEMETHOD(Win9EnumIn)(__in Win9Enum value){ value; return S_OK; }
            IFACEMETHOD(Win9EnumOut)(__out Win9Enum * value){ *value = Win9Enum_Win9; return S_OK; }
            IFACEMETHOD(CallWin9Delegate)(__in IWin9Delegate * value);
            IFACEMETHOD(Win9InterfaceVectorIn)(__in Windows::Foundation::Collections::IVector<IWin9Interface *> * value)
            { 
                if (!value) { return E_POINTER; }
                return S_OK; 
            }

            // Version NTDDI_MAX types
            IFACEMETHOD(MaxVersionInterfaceIn)(__in IMaxVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(MaxVersionClassIn)(__in IMaxVersionInterface * value){ value; return S_OK; }
            IFACEMETHOD(MaxVersionStructIn)(__in MaxVersionStruct value){ m_maxStruct = value; return S_OK; }
            IFACEMETHOD(MaxVersionStructOut)(__out MaxVersionStruct * value){ *value = m_maxStruct; return S_OK; }
            IFACEMETHOD(MaxVersionEnumIn)(__in MaxVersionEnum value){ value; return S_OK; }
            IFACEMETHOD(MaxVersionEnumOut)(__out MaxVersionEnum * value){ *value = MaxVersionEnum_Max; return S_OK; }
            IFACEMETHOD(CallMaxVersionDelegate)(__in IMaxVersionDelegate * value);
            IFACEMETHOD(MaxVersionInterfaceVectorIn)(__in Windows::Foundation::Collections::IVector<IMaxVersionInterface *> * value)
            { 
                if (!value) { return E_POINTER; }
                return S_OK; 
            }
        };

        /*                           *
        * -- Collections Classes -- *
        *                           */
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
            IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual) override;
            IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) int *value);
        };

        class CVectorIWin9Interface : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVector<IWin9Interface *>, Windows::Foundation::Collections::IIterable<IWin9Interface *>>
        {
        private:
            Windows::Foundation::Collections::IVector<IWin9Interface *> *m_pVector;

        public:
            CVectorIWin9Interface();
            ~CVectorIWin9Interface()
            {
                m_pVector->Release();
            }

            //
            // IIterable members
            //
            IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<IWin9Interface *> **first);

            //
            // IVector members
            //
            IFACEMETHOD(GetAt)(__in unsigned index, __out IWin9Interface **item);
            IFACEMETHOD(get_Size)(__out unsigned *size);
            IFACEMETHOD(IndexOf)(__in_opt IWin9Interface * value, __out unsigned *index, __out boolean *found);    
            IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<IWin9Interface *> **returnValue);
            IFACEMETHOD(SetAt)(__in unsigned index, __in_opt IWin9Interface * value);
            IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt IWin9Interface * value); 
            IFACEMETHOD(RemoveAt)(__in unsigned index);
            IFACEMETHOD(Append)(__in_opt IWin9Interface * value);
            IFACEMETHOD(RemoveAtEnd)();
            IFACEMETHOD(Clear)();
            IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IWin9Interface **items, __RPC__out unsigned int *actual) override;
            IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) IWin9Interface **value);
        };

        class CObservableVectorInt : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IObservableVector<int>, Windows::Foundation::Collections::IVector<int>, Windows::Foundation::Collections::IIterable<int>>
        {
        private:
            Windows::Foundation::Collections::IObservableVector<int> *m_pObservableVector;
            Windows::Foundation::Collections::IVector<int> *m_pVector;

        public:
            CObservableVectorInt();
            ~CObservableVectorInt()
            {
                m_pVector->Release();
                m_pObservableVector->Release();
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
            IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual) override;
            IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) int *value);

            //
            // IObservableVector members
            //
            IFACEMETHOD(add_VectorChanged)(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<int> *handler, __RPC__out EventRegistrationToken *token);
            IFACEMETHOD(remove_VectorChanged)(__in EventRegistrationToken token);
        };

        class CObservableVectorIWin9Interface : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IObservableVector<IWin9Interface *>, Windows::Foundation::Collections::IVector<IWin9Interface *>, Windows::Foundation::Collections::IIterable<IWin9Interface *>>
        {
        private:
            Windows::Foundation::Collections::IObservableVector<IWin9Interface *> *m_pObservableVector;
            Windows::Foundation::Collections::IVector<IWin9Interface *> *m_pVector;

        public:
            CObservableVectorIWin9Interface();
            ~CObservableVectorIWin9Interface()
            {
                m_pVector->Release();
                m_pObservableVector->Release();
            }

            //
            // IIterable members
            //
            IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<IWin9Interface *> **first);

            //
            // IVector members
            //
            IFACEMETHOD(GetAt)(__in unsigned index, __out IWin9Interface **item);
            IFACEMETHOD(get_Size)(__out unsigned *size);
            IFACEMETHOD(IndexOf)(__in_opt IWin9Interface * value, __out unsigned *index, __out boolean *found);    
            IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<IWin9Interface *> **returnValue);
            IFACEMETHOD(SetAt)(__in unsigned index, __in_opt IWin9Interface * value);
            IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt IWin9Interface * value); 
            IFACEMETHOD(RemoveAt)(__in unsigned index);
            IFACEMETHOD(Append)(__in_opt IWin9Interface * value);
            IFACEMETHOD(RemoveAtEnd)();
            IFACEMETHOD(Clear)();  
            IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IWin9Interface **items, __RPC__out unsigned int *actual) override;
            IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) IWin9Interface **value);

            //
            // IObservableVector members
            //
            IFACEMETHOD(add_VectorChanged)(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<IWin9Interface *> *handler, __RPC__out EventRegistrationToken *token);
            IFACEMETHOD(remove_VectorChanged)(__in EventRegistrationToken token);
        };

        class VectorInt :
            public Microsoft::WRL::RuntimeClass<IDefault, CVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.VectorInt", BaseTrust);

        public:
            VectorInt() { }
            ~VectorInt(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };

        class ObservableVectorInt :
            public Microsoft::WRL::RuntimeClass<IDefault, CObservableVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.ObservableVectorInt", BaseTrust);

        public:
            ObservableVectorInt() { }
            ~ObservableVectorInt(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };

        class RequiresVectorInt :
            public Microsoft::WRL::RuntimeClass<IRequiresVector, CVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.RequiresVectorInt", BaseTrust);

        public:
            RequiresVectorInt() { }
            ~RequiresVectorInt(){ }

            // IRequiresVector
            IFACEMETHOD(HasIRequiresVector)(__out boolean * result){ *result = true; return S_OK; }
        };

        class RequiresObservableVectorInt :
            public Microsoft::WRL::RuntimeClass<IRequiresObservableVector, CObservableVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.RequiresObservableVectorInt", BaseTrust);

        public:
            RequiresObservableVectorInt() { }
            ~RequiresObservableVectorInt(){ }

            // IRequiresObservableVector
            IFACEMETHOD(HasIRequiresObservableVector)(__out boolean * result){ *result = true; return S_OK; }
        };

        class VersionedVectorInt :
            public Microsoft::WRL::RuntimeClass<IDefault, CVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.VersionedVectorInt", BaseTrust);

        public:
            VersionedVectorInt() { }
            ~VersionedVectorInt(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };

        class VersionedObservableVectorInt :
            public Microsoft::WRL::RuntimeClass<IDefault, CObservableVectorInt>
        {
            InspectableClass(L"DevTests.Versioning.VersionedObservableVectorInt", BaseTrust);

        public:
            VersionedObservableVectorInt() { }
            ~VersionedObservableVectorInt(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };

        class VectorVersionedT :
            public Microsoft::WRL::RuntimeClass<IDefault, CVectorIWin9Interface>
        {
            InspectableClass(L"DevTests.Versioning.VectorVersionedT", BaseTrust);

        public:
            VectorVersionedT() { }
            ~VectorVersionedT(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };

        class ObservableVectorVersionedT :
            public Microsoft::WRL::RuntimeClass<IDefault, CObservableVectorIWin9Interface>
        {
            InspectableClass(L"DevTests.Versioning.ObservableVectorVersionedT", BaseTrust);

        public:
            ObservableVectorVersionedT() { }
            ~ObservableVectorVersionedT(){ }

            // IDefault
            IFACEMETHOD(HasDefault)(__out boolean * result){ *result = true; return S_OK; }
        };
    }
}