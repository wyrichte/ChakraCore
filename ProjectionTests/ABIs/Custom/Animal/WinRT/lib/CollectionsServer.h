namespace Animals
{
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

    class CVectorHSTRING : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVector<HSTRING>, Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IVector<HSTRING> *m_pVector;

    public:
        CVectorHSTRING();
        ~CVectorHSTRING()
        {
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);

        //
        // IVector members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out HSTRING *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt HSTRING value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<HSTRING> **returnValue);
        IFACEMETHOD(SetAt)(__in unsigned index, __in_opt HSTRING value);
        IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt HSTRING value); 
        IFACEMETHOD(RemoveAt)(__in unsigned index);
        IFACEMETHOD(Append)(__in_opt HSTRING value);
        IFACEMETHOD(RemoveAtEnd)();
        IFACEMETHOD(Clear)();
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual) override;
        IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) HSTRING *value);
    };

    class CVectorIAnimal : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVector<IAnimal *>, Windows::Foundation::Collections::IIterable<IAnimal *>>
    {
    private:
        Windows::Foundation::Collections::IVector<IAnimal *> *m_pVector;

    public:
        CVectorIAnimal();
        ~CVectorIAnimal()
        {
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<IAnimal *> **first);

        //
        // IVector members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out IAnimal **item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt IAnimal * value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<IAnimal *> **returnValue);
        IFACEMETHOD(SetAt)(__in unsigned index, __in_opt IAnimal * value);
        IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt IAnimal * value); 
        IFACEMETHOD(RemoveAt)(__in unsigned index);
        IFACEMETHOD(Append)(__in_opt IAnimal * value);
        IFACEMETHOD(RemoveAtEnd)();
        IFACEMETHOD(Clear)();
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IAnimal **items, __RPC__out unsigned int *actual) override;
        IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) IAnimal **value);
    };
    
    class CVectorViewFloat : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVectorView<float>, Windows::Foundation::Collections::IIterable<float>>
    {
    private:
       Windows::Foundation::Collections::IVectorView<float> *m_pVectorView;
       Windows::Foundation::Collections::IVector<float> *m_pVector;

    public:
        CVectorViewFloat();
        ~CVectorViewFloat()
        {
            m_pVectorView->Release();
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<float> **first);

        //
        // IVectorView members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out float *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt float value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) float *items, __RPC__out unsigned int *actual) override;
    };

    class CVectorViewHSTRING : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVectorView<HSTRING>, Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
       Windows::Foundation::Collections::IVectorView<HSTRING> *m_pVectorView;
       Windows::Foundation::Collections::IVector<HSTRING> *m_pVector;

    public:
        CVectorViewHSTRING();
        ~CVectorViewHSTRING()
        {
            m_pVectorView->Release();
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);

        //
        // IVectorView members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out HSTRING *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt HSTRING value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual) override;
    };

    class CVectorViewGUID : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IVectorView<GUID>, Windows::Foundation::Collections::IIterable<GUID>>
    {
    private:
       Windows::Foundation::Collections::IVectorView<GUID> *m_pVectorView;
       Windows::Foundation::Collections::IVector<GUID> *m_pVector;

    public:
        CVectorViewGUID();
        ~CVectorViewGUID()
        {
            m_pVectorView->Release();
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<GUID> **first);

        //
        // IVectorView members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out GUID *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt GUID value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) GUID *items, __RPC__out unsigned int *actual) override;
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

    class CObservableVectorHSTRING : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IObservableVector<HSTRING>, Windows::Foundation::Collections::IVector<HSTRING>, Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IObservableVector<HSTRING> *m_pObservableVector;
        Windows::Foundation::Collections::IVector<HSTRING> *m_pVector;

    public:
        CObservableVectorHSTRING();
        ~CObservableVectorHSTRING()
        {
            m_pVector->Release();
            m_pObservableVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);

        //
        // IVector members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out HSTRING *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt HSTRING value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IVectorView<HSTRING> **returnValue);
        IFACEMETHOD(SetAt)(__in unsigned index, __in_opt HSTRING value);
        IFACEMETHOD(InsertAt)(__in unsigned index, __in_opt HSTRING value); 
        IFACEMETHOD(RemoveAt)(__in unsigned index);
        IFACEMETHOD(Append)(__in_opt HSTRING value);
        IFACEMETHOD(RemoveAtEnd)();
        IFACEMETHOD(Clear)();  
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual) override;
        IFACEMETHOD(ReplaceAll)(unsigned int count, __RPC__in_ecount_full(count) HSTRING *value);

        //
        // IObservableVector members
        //
        IFACEMETHOD(add_VectorChanged)(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<HSTRING> *handler, __RPC__out EventRegistrationToken *token);
        IFACEMETHOD(remove_VectorChanged)(__in EventRegistrationToken token);
    };

    class CMapHStringAndInt : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IMap<HSTRING, int>, Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *>>
    {
    private:
        Windows::Foundation::Collections::IMap<HSTRING, int> *m_pMap;

    public:
        CMapHStringAndInt();
        ~CMapHStringAndInt()
        {
            m_pMap->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> **first);

        //
        // IMap members
        //
        IFACEMETHOD(Lookup)(__in_opt HSTRING key, __out int *value);
        IFACEMETHOD(get_Size)(__out unsigned int *size);
        IFACEMETHOD(HasKey)(__in_opt HSTRING key, __out boolean *found);
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IMapView<HSTRING, int> **view);
        IFACEMETHOD(Insert)(__in_opt HSTRING key, __in_opt int value, __out boolean *replaced);
        IFACEMETHOD(Remove)(__in_opt HSTRING key);
        IFACEMETHOD(Clear)();
    };

    class CObservableMapHStringAndInt : public Microsoft::WRL::Implements<
        Windows::Foundation::Collections::IObservableMap<HSTRING, int>, 
        Windows::Foundation::Collections::IMap<HSTRING, int>, 
        Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *>>
    {
    private:
        Windows::Foundation::Collections::IObservableMap<HSTRING, int> *m_pObservableMap;
        Windows::Foundation::Collections::IMap<HSTRING, int> *m_pMap;

    public:
        CObservableMapHStringAndInt();
        ~CObservableMapHStringAndInt()
        {
            m_pMap->Release();
            m_pObservableMap->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> **first);

        //
        // IMap members
        //
        IFACEMETHOD(Lookup)(__in_opt HSTRING key, __out int *value);
        IFACEMETHOD(get_Size)(__out unsigned int *size);
        IFACEMETHOD(HasKey)(__in_opt HSTRING key, __out boolean *found);
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IMapView<HSTRING, int> **view);
        IFACEMETHOD(Insert)(__in_opt HSTRING key, __in_opt int value, __out boolean *replaced);
        IFACEMETHOD(Remove)(__in_opt HSTRING key);
        IFACEMETHOD(Clear)();

        //
        // IObservableMap members
        //
        IFACEMETHOD(add_MapChanged)(__RPC__in_opt Windows::Foundation::Collections::MapChangedEventHandler<HSTRING, int> *handler, __RPC__out EventRegistrationToken *token);
        IFACEMETHOD(remove_MapChanged)(__in EventRegistrationToken token);
    };

    class CObservableMapGUIDAndInspectable : public Microsoft::WRL::Implements<
        Windows::Foundation::Collections::IObservableMap<GUID, IInspectable*>, 
        Windows::Foundation::Collections::IMap<GUID, IInspectable*>, 
        Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<GUID, IInspectable*> *>>
    {
    private:
        Windows::Foundation::Collections::IObservableMap<GUID, IInspectable*> *m_pObservableMap;
        Windows::Foundation::Collections::IMap<GUID, IInspectable*> *m_pMap;

    public:
        CObservableMapGUIDAndInspectable();
        ~CObservableMapGUIDAndInspectable()
        {
            m_pMap->Release();
            m_pObservableMap->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<GUID, IInspectable*> *> **first);

        //
        // IMap members
        //
        IFACEMETHOD(Lookup)(__in_opt GUID key, __out IInspectable* *value);
        IFACEMETHOD(get_Size)(__out unsigned int *size);
        IFACEMETHOD(HasKey)(__in_opt GUID key, __out boolean *found);
        IFACEMETHOD(GetView)(__deref_out_opt Windows::Foundation::Collections::IMapView<GUID, IInspectable*> **view);
        IFACEMETHOD(Insert)(__in_opt GUID key, __in_opt IInspectable* value, __out boolean *replaced);
        IFACEMETHOD(Remove)(__in_opt GUID key);
        IFACEMETHOD(Clear)();

        //
        // IObservableMap members
        //
        IFACEMETHOD(add_MapChanged)(__RPC__in_opt Windows::Foundation::Collections::MapChangedEventHandler<GUID, IInspectable*> *handler, __RPC__out EventRegistrationToken *token);
        IFACEMETHOD(remove_MapChanged)(__in EventRegistrationToken token);
    };

    class CIDoubleObservableMap :
        public Microsoft::WRL::RuntimeClass<IDoubleIObservableMap, CObservableMapHStringAndInt, CObservableMapGUIDAndInspectable>    
    {
    public:
        CIDoubleObservableMap() { }
        ~CIDoubleObservableMap() { }

        IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName)  
        {  
            *runtimeName = nullptr;  
            HRESULT hr = S_OK;  
            // Return type that does not exist in metadata
            const wchar_t *name = L"Animals.IDoubleObservableMap";  
            hr = WindowsCreateString(name, static_cast<UINT32>(::wcslen(name)), runtimeName);  
            return hr;  
        }  
        IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)  
        {  
            *trustLvl = BaseTrust;  
            return S_OK;  
        }  
        IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **)  
        {  
            iidCount;
            return E_NOTIMPL;
        }
    };

    class CIterableHSTRING : public Microsoft::WRL::Implements<Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IVector<HSTRING> *m_pVector;

    public:
        CIterableHSTRING();
        ~CIterableHSTRING()
        {
            m_pVector->Release();
        }

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);
    };

    class SingleIVectorServer : public Microsoft::WRL::RuntimeClass<CVectorInt>
    {
        InspectableClass(L"Animals.SingleIVector", BaseTrust);

    public:
        SingleIVectorServer() { }
        ~SingleIVectorServer() { }
    };

    class DoubleIVectorServer : public Microsoft::WRL::RuntimeClass<CVectorInt, CVectorHSTRING>
    {
        InspectableClass(L"Animals.DoubleIVector", BaseTrust);

    public:
        DoubleIVectorServer() { }
        ~DoubleIVectorServer() { }
    };

    class MultipleIVectorServer : public Microsoft::WRL::RuntimeClass<CVectorInt, CVectorHSTRING, CVectorIAnimal>
    {
        InspectableClass(L"Animals.MultipleIVector", BaseTrust);

    public:
        MultipleIVectorServer() { }
        ~MultipleIVectorServer() { }
    };

    class InterfaceWithSingleIVectorServer : public Microsoft::WRL::RuntimeClass<ISingleIVector, CVectorInt, CVectorViewFloat>
    {
        InspectableClass(L"Animals.InterfaceWithSingleIVector", BaseTrust);

    public:
        InterfaceWithSingleIVectorServer() { }
        ~InterfaceWithSingleIVectorServer() { }
    };

    class InterfaceWithDoubleIVectorServer : public Microsoft::WRL::RuntimeClass<IDoubleIVector, CVectorInt, CVectorIAnimal, CVectorViewHSTRING>
    {
        InspectableClass(L"Animals.InterfaceWithDoubleIVector", BaseTrust);

    public:
        InterfaceWithDoubleIVectorServer() { }
        ~InterfaceWithDoubleIVectorServer() { }    
    };

    class InterfaceWithMultipleIVectorServer : public Microsoft::WRL::RuntimeClass<IMultipleIVector, CVectorInt, CVectorIAnimal, CVectorViewHSTRING, CVectorViewGUID>
    {
        InspectableClass(L"Animals.InterfaceWithMultipleIVector", BaseTrust);

    public:
        InterfaceWithMultipleIVectorServer() { }
        ~InterfaceWithMultipleIVectorServer() { }
    };

    class RCIObservableServer : public Microsoft::WRL::RuntimeClass<CObservableVectorInt>
    {
        InspectableClass(L"Animals.RCIObservable", BaseTrust);

    public:
        RCIObservableServer() { }
        ~RCIObservableServer() { }
    };

    class RCISingleObservableServer : public Microsoft::WRL::RuntimeClass<ISingleIObservable, CObservableVectorInt>
    {
        InspectableClass(L"Animals.RCISingleObservable", BaseTrust);

    public:
        RCISingleObservableServer() { }
        ~RCISingleObservableServer() { }
    };

    class RCIDoubleObservableServer : public Microsoft::WRL::RuntimeClass<IDoubleIObservable, CObservableVectorInt, CObservableVectorHSTRING>
    {
        InspectableClass(L"Animals.RCIDoubleObservable", BaseTrust);
        
    public:
        RCIDoubleObservableServer() { }
        ~RCIDoubleObservableServer() { }
    };

    class RCIDoubleObservableMapServer : public Microsoft::WRL::RuntimeClass<IDoubleIObservableMap, CObservableMapHStringAndInt, CObservableMapGUIDAndInspectable>
    {
        InspectableClass(L"Animals.RCIDoubleObservableMap", BaseTrust);
        
    public:
        RCIDoubleObservableMapServer() { }
        ~RCIDoubleObservableMapServer() { }
    };

    class RCStringMapServer : public Microsoft::WRL::RuntimeClass<CMapHStringAndInt>
    {
        InspectableClass(L"Animals.RCStringMap", BaseTrust);
        
    public:
        RCStringMapServer() { }
        ~RCStringMapServer() { }
    };

    class RCStringMapWithIterableServer : public Microsoft::WRL::RuntimeClass<CMapHStringAndInt, CIterableHSTRING>
    {
        InspectableClass(L"Animals.RCStringMapWithIterable", BaseTrust);
        
    public:
        RCStringMapWithIterableServer() { }
        ~RCStringMapWithIterableServer() { }
    };

    class RCStringMapWithDefaultIterableServer : public Microsoft::WRL::RuntimeClass<CMapHStringAndInt, CIterableHSTRING>
    {
        InspectableClass(L"Animals.RCStringMapWithDefaultIterable", BaseTrust);
        
    public:
        RCStringMapWithDefaultIterableServer() { }
        ~RCStringMapWithDefaultIterableServer() { }
    };
}
