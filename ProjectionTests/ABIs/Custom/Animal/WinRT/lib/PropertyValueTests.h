namespace Animals
{
    class CPropertyValueTests : public Microsoft::WRL::RuntimeClass<Animals::ITypeReceiver, Animals::IInspectableTests, Animals::IIPropertyValueTests, Animals::IPropertyValueTests, Animals::IReferenceTests>
    {
        InspectableClass(L"Animals.PropertyValueTests", BaseTrust);

    private:
        Microsoft::WRL::ComPtr<Windows::Foundation::IPropertyValueStatics> spPropertyValueFactory;
        IInspectable *m_empty;

        Windows::Foundation::IPropertyValue *m_propertyValue;
        Windows::Foundation::IReference<Dimensions> *m_dimensions;

    public:
        CPropertyValueTests();
        ~CPropertyValueTests();

        IFACEMETHOD(ReceiveGuidArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue) override;
        IFACEMETHOD(ReceiveAnimalArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IAnimal ***outValue) override;
        IFACEMETHOD(ReceiveFishArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IFish ***outValue) override;
        IFACEMETHOD(ReceiveVectorArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Collections::IVector<int> ***outValue) override;
        IFACEMETHOD(ReceiveDateArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue) override;
        IFACEMETHOD(ReceiveTimeSpanArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue) override;
        IFACEMETHOD(ReceivePointArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue) override;
        IFACEMETHOD(ReceiveSizeArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue) override;
        IFACEMETHOD(ReceiveRectArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue) override;
        IFACEMETHOD(ReceiveBooleanArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue) override;
        IFACEMETHOD(ReceiveStringArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue) override;
        IFACEMETHOD(ReceiveInspectableArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue) override;
        IFACEMETHOD(ReceiveChar16Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue) override;
        IFACEMETHOD(ReceiveUInt8Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue) override;
        IFACEMETHOD(ReceiveInt16Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue) override;
        IFACEMETHOD(ReceiveUInt16Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue) override;
        IFACEMETHOD(ReceiveInt32Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue) override;
        IFACEMETHOD(ReceiveUInt32Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue) override;
        IFACEMETHOD(ReceiveInt64Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue) override;
        IFACEMETHOD(ReceiveUInt64Array)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue) override;
        IFACEMETHOD(ReceiveFloatArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue) override;
        IFACEMETHOD(ReceiveDoubleArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue) override;
        IFACEMETHOD(ReceiveStructArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue) override;
        IFACEMETHOD(ReceiveEnumArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue) override;
        IFACEMETHOD(ReceiveWinrtDelegateArray)(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue) override;
        IFACEMETHOD(ReceiveJSDelegateArray)(__in IDelegateWithOutParam_HSTRING *delegate1, __in IDelegateWithOutParam_HSTRING *delegate2, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue) override;

        IFACEMETHOD(ReceiveVectorOfVector)(__out Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IVector<int> *> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfStruct)(__out Windows::Foundation::Collections::IVector<Dimensions> **outValue) override;
        IFACEMETHOD(ReceiveMapOfStructAndVector)(__out Windows::Foundation::Collections::IMap<Dimensions, Windows::Foundation::Collections::IVector<HSTRING> *> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfRCObservableVector)(__out Windows::Foundation::Collections::IVector<RCIObservable *> **outValue) override;
        //IFACEMETHOD(ReceiveVectorOfHRESULT)(__out Windows::Foundation::Collections::IVector<HRESULT> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfGuid)(__out Windows::Foundation::Collections::IVector<GUID> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfDate)(__out Windows::Foundation::Collections::IVector<Windows::Foundation::DateTime> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfTimeSpan)(__out Windows::Foundation::Collections::IVector<Windows::Foundation::TimeSpan> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfEnum)(__out Windows::Foundation::Collections::IVector<Phylum> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfDelegate)(__out Windows::Foundation::Collections::IVector<IDelegateWithOutParam_HSTRING *> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfAsyncInfo)(__out Windows::Foundation::Collections::IVector<Windows::Foundation::IAsyncInfo *> **outValue) override;
        IFACEMETHOD(ReceiveVectorOfEventRegistration)(__out Windows::Foundation::Collections::IVector<EventRegistrationToken> **outValue) override;
        IFACEMETHOD(ReceiveMapOfStringAndInspectable)(__out Windows::Foundation::Collections::IMap<HSTRING, IInspectable *> **outValue) override;

        IFACEMETHOD(ReceiveVectorOfVector_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfStruct_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveMapOfStructAndVector_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfRCObservableVector_InspectableOut)(__out IInspectable **outValue) override;
        //IFACEMETHOD(ReceiveVectorOfHRESULT_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfGuid_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfDate_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfTimeSpan_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfEnum_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfDelegate_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfAsyncInfo_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveVectorOfEventRegistration_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(ReceiveMapOfStringAndInspectable_InspectableOut)(__out IInspectable **outValue) override;

        IFACEMETHOD(IsSameDelegate)(__in IDelegateWithOutParam_HSTRING *inValue1, __in IDelegateWithOutParam_HSTRING *inValue2, __out boolean *isSame) override;

        // Inspectable In
        IFACEMETHOD(TestNull_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoolean_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out boolean *outValue) override;
        IFACEMETHOD(TestString_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out HSTRING *outValue) override;
        IFACEMETHOD(TestNumber_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out double *outValue) override;
        IFACEMETHOD(TestDate_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out Windows::Foundation::DateTime *outValue) override;
        IFACEMETHOD(TestInspectable_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out IInspectable **outValue) override;
        IFACEMETHOD(TestArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue) override;
        IFACEMETHOD(TestGuidArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue) override;
        IFACEMETHOD(TestDateArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue) override;
        IFACEMETHOD(TestDimensionsArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue) override;
        IFACEMETHOD(TestTimeSpanArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue) override;
        IFACEMETHOD(TestPointArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue) override;
        IFACEMETHOD(TestSizeArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue) override;
        IFACEMETHOD(TestRectArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue) override;
        IFACEMETHOD(TestEnumArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue) override;
        IFACEMETHOD(TestBooleanArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue) override;
        IFACEMETHOD(TestStringArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue) override;
        IFACEMETHOD(TestChar16Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue) override;
        IFACEMETHOD(TestUInt8Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue) override;
        IFACEMETHOD(TestInt16Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue) override;
        IFACEMETHOD(TestUInt16Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue) override;
        IFACEMETHOD(TestInt32Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue) override;
        IFACEMETHOD(TestUInt32Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue) override;
        IFACEMETHOD(TestInt64Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue) override;
        IFACEMETHOD(TestUInt64Array_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue) override;
        IFACEMETHOD(TestFloatArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue) override;
        IFACEMETHOD(TestDoubleArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue) override;
        IFACEMETHOD(TestDelegateArray_InspectableIn)(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue) override;

         // Inspectable Out
        IFACEMETHOD(TestNull_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestBoolean_InspectableOut)(__in boolean inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestString_InspectableOut)(__in HSTRING inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestChar16_InspectableOut)(__in wchar_t inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt8_InspectableOut)(__in byte inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt16_InspectableOut)(__in short inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt16_InspectableOut)(__in unsigned short inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt32_InspectableOut)(__in INT32 inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt32_InspectableOut)(__in UINT32 inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt64_InspectableOut)(__in INT64 inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt64_InspectableOut)(__in UINT64 inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestFloat_InspectableOut)(__in float inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestDouble_InspectableOut)(__in double inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestGuid_InspectableOut)(__in GUID inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestDate_InspectableOut)(__in Windows::Foundation::DateTime inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestDimensions_InspectableOut)(__in Windows::Foundation::IReference<Dimensions> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestTimeSpan_InspectableOut)(__in Windows::Foundation::TimeSpan inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestPoint_InspectableOut)(__in Windows::Foundation::Point inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestSize_InspectableOut)(__in Windows::Foundation::Size inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestRect_InspectableOut)(__in Windows::Foundation::Rect inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestEnum_InspectableOut)(__in Windows::Foundation::IReference<Phylum> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV1_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV2_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV3_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV4_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV5_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestRCPV6_InspectableOut)(__out IInspectable **outValue) override;
        IFACEMETHOD(TestGuidArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) GUID *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestDateArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::DateTime *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestTimeSpanArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::TimeSpan *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestPointArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Point *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestSizeArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Size *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestRectArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Rect *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBooleanArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) boolean *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestStringArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) HSTRING *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestChar16Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) wchar_t *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt8Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) byte *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt16Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) short *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt16Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) unsigned short *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt32Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) INT32 *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt32Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) UINT32 *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInt64Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) INT64 *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestUInt64Array_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) UINT64 *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestFloatArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) float *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestDoubleArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) double *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) IInspectable **inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestAnimalArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) IAnimal **inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestFishArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) IFish **inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestVectorArray_InspectableOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Collections::IVector<int> **inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoxInspectable_InspectableOut)(__in IInspectable *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestInspectable_InspectableOut)(__in IInspectable *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoxedNull_InspectableOut)(__out IInspectable **outValue) override;

        IFACEMETHOD(TestIterable_InspectableOut)(__in Windows::Foundation::Collections::IIterable<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestIterator_InspectableOut)(__in Windows::Foundation::Collections::IIterator<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestIVector_InspectableOut)(__in Windows::Foundation::Collections::IVector<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestIVectorView_InspectableOut)(__in Windows::Foundation::Collections::IVectorView<int> *inValue, __out IInspectable **outValue) override;

        IFACEMETHOD(TestBoxIterable_InspectableOut)(__in Windows::Foundation::Collections::IIterable<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoxIterator_InspectableOut)(__in Windows::Foundation::Collections::IIterator<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoxIVector_InspectableOut)(__in Windows::Foundation::Collections::IVector<int> *inValue, __out IInspectable **outValue) override;
        IFACEMETHOD(TestBoxIVectorView_InspectableOut)(__in Windows::Foundation::Collections::IVectorView<int> *inValue, __out IInspectable **outValue) override;

        IFACEMETHOD(GetRuntimeClassWithEmptyString)(__out IInspectable **inspectable) override;
        IFACEMETHOD(VerifyRuntimeClassWithEmptyString)(__in IInspectable *inspectable, __out boolean *isSame) override;

        IFACEMETHOD(GetRuntimeClassWithFailingGRCN)(__out IInspectable **inspectable) override;
        IFACEMETHOD(GetRuntimeClassWithEmptyStringAsInterface)(__out IEmptyGRCN **outValue) override;

        IFACEMETHOD(TestFailingRuntimeClassNameWithAnotherInterface)(__in IInspectable *inValue, __out IInspectable **outFailingValue, __out IInspectable **outValue) override;

        // IPropertyValue In
        IFACEMETHOD(TestNull_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestBoolean_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out boolean *outValue) override;
        IFACEMETHOD(TestString_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out HSTRING *outValue) override;
        IFACEMETHOD(TestNumber_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out double *outValue) override;
        IFACEMETHOD(TestDate_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out Windows::Foundation::DateTime *outValue) override;
        IFACEMETHOD(TestInspectable_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out IInspectable **outValue) override;
        IFACEMETHOD(TestArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue) override;
        IFACEMETHOD(TestGuidArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue) override;
        IFACEMETHOD(TestDateArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue) override;
        IFACEMETHOD(TestDimensionsArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue) override;
        IFACEMETHOD(TestTimeSpanArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue) override;
        IFACEMETHOD(TestPointArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue) override;
        IFACEMETHOD(TestSizeArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue) override;
        IFACEMETHOD(TestRectArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue) override;
        IFACEMETHOD(TestEnumArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue) override;
        IFACEMETHOD(TestBooleanArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue) override;
        IFACEMETHOD(TestStringArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue) override;
        IFACEMETHOD(TestChar16Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue) override;
        IFACEMETHOD(TestUInt8Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue) override;
        IFACEMETHOD(TestInt16Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue) override;
        IFACEMETHOD(TestUInt16Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue) override;
        IFACEMETHOD(TestInt32Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue) override;
        IFACEMETHOD(TestUInt32Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue) override;
        IFACEMETHOD(TestInt64Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue) override;
        IFACEMETHOD(TestUInt64Array_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue) override;
        IFACEMETHOD(TestFloatArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue) override;
        IFACEMETHOD(TestDoubleArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue) override;
        IFACEMETHOD(TestDelegateArray_IPropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue) override;

         // IPropertyValue Out
        IFACEMETHOD(TestNull_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestBoolean_IPropertyValueOut)(__in boolean inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestString_IPropertyValueOut)(__in HSTRING inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestChar16_IPropertyValueOut)(__in wchar_t inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt8_IPropertyValueOut)(__in byte inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt16_IPropertyValueOut)(__in short inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt16_IPropertyValueOut)(__in unsigned short inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt32_IPropertyValueOut)(__in INT32 inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt32_IPropertyValueOut)(__in UINT32 inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt64_IPropertyValueOut)(__in INT64 inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt64_IPropertyValueOut)(__in UINT64 inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestFloat_IPropertyValueOut)(__in float inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestDouble_IPropertyValueOut)(__in double inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestGuid_IPropertyValueOut)(__in GUID inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestDate_IPropertyValueOut)(__in Windows::Foundation::DateTime inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestDimensions_IPropertyValueOut)(__in Windows::Foundation::IReference<Dimensions> *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestTimeSpan_IPropertyValueOut)(__in Windows::Foundation::TimeSpan inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestPoint_IPropertyValueOut)(__in Windows::Foundation::Point inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestSize_IPropertyValueOut)(__in Windows::Foundation::Size inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRect_IPropertyValueOut)(__in Windows::Foundation::Rect inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestEnum_IPropertyValueOut)(__in Windows::Foundation::IReference<Phylum> *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV1_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV2_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV3_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV4_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV5_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV6_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestGuidArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) GUID *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestDateArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::DateTime *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestTimeSpanArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::TimeSpan *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestPointArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Point *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestSizeArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Size *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRectArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Rect *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestBooleanArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) boolean *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestStringArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) HSTRING *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestChar16Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) wchar_t *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt8Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) byte *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt16Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) short *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt16Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) unsigned short *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt32Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) INT32 *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt32Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) UINT32 *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInt64Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) INT64 *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestUInt64Array_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) UINT64 *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestFloatArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) float *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestDoubleArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) double *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) IInspectable **inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestAnimalArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) IAnimal **inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestFishArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) IFish **inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestVectorArray_IPropertyValueOut)(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Collections::IVector<int> **inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestBoxInspectable_IPropertyValueOut)(__in IInspectable *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestInspectable_IPropertyValueOut)(__in IInspectable *inValue, __out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestBoxedNull_IPropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;

        // PropertyValue In
        IFACEMETHOD(TestRCPV1_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;
        IFACEMETHOD(TestRCPV2_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;
        IFACEMETHOD(TestRCPV3_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;
        IFACEMETHOD(TestRCPV4_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;
        IFACEMETHOD(TestRCPV5_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;
        IFACEMETHOD(TestRCPV6_PropertyValueIn)(__in Windows::Foundation::IPropertyValue *inValue) override;

         // PropertyValue Out
        IFACEMETHOD(TestRCPV1_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV2_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV3_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV4_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV5_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;
        IFACEMETHOD(TestRCPV6_PropertyValueOut)(__out Windows::Foundation::IPropertyValue **outValue) override;

        // IReference<T> in
        IFACEMETHOD(TestBoolean_ReferenceIn)(__in Windows::Foundation::IReference<bool> *inValue, __out boolean *isNull, __out boolean *isValidType, __out boolean *outValue) override;
        IFACEMETHOD(TestChar16_ReferenceIn)(__in Windows::Foundation::IReference<wchar_t> *inValue, __out boolean *isNull, __out boolean *isValidType, __out wchar_t *outValue) override;
        IFACEMETHOD(TestUInt8_ReferenceIn)(__in Windows::Foundation::IReference<byte> *inValue, __out boolean *isNull, __out boolean *isValidType, __out byte *outValue) override;
        IFACEMETHOD(TestInt16_ReferenceIn)(__in Windows::Foundation::IReference<short> *inValue, __out boolean *isNull, __out boolean *isValidType, __out short *outValue) override;
        IFACEMETHOD(TestUInt16_ReferenceIn)(__in Windows::Foundation::IReference<unsigned short> *inValue, __out boolean *isNull, __out boolean *isValidType, __out unsigned short *outValue) override;
        IFACEMETHOD(TestInt32_ReferenceIn)(__in Windows::Foundation::IReference<INT32> *inValue, __out boolean *isNull, __out boolean *isValidType, __out INT32 *outValue) override;
        IFACEMETHOD(TestUInt32_ReferenceIn)(__in Windows::Foundation::IReference<UINT32> *inValue, __out boolean *isNull, __out boolean *isValidType, __out UINT32 *outValue) override;
        IFACEMETHOD(TestInt64_ReferenceIn)(__in Windows::Foundation::IReference<INT64> *inValue, __out boolean *isNull, __out boolean *isValidType, __out INT64 *outValue) override;
        IFACEMETHOD(TestUInt64_ReferenceIn)(__in Windows::Foundation::IReference<UINT64> *inValue, __out boolean *isNull, __out boolean *isValidType, __out UINT64 *outValue) override;
        IFACEMETHOD(TestFloat_ReferenceIn)(__in Windows::Foundation::IReference<float> *inValue, __out boolean *isNull, __out boolean *isValidType, __out float *outValue) override;
        IFACEMETHOD(TestDouble_ReferenceIn)(__in Windows::Foundation::IReference<double> *inValue, __out boolean *isNull, __out boolean *isValidType, __out double *outValue) override;    
        IFACEMETHOD(TestGuid_ReferenceIn)(__in Windows::Foundation::IReference<GUID> *inValue, __out boolean *isNull, __out boolean *isValidType, __out GUID *outValue) override;
        IFACEMETHOD(TestDate_ReferenceIn)(__in Windows::Foundation::IReference<Windows::Foundation::DateTime> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::DateTime *outValue) override;
        IFACEMETHOD(TestDimensions_ReferenceIn)(__in Windows::Foundation::IReference<Dimensions> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Dimensions *outValue) override;
        IFACEMETHOD(TestTimeSpan_ReferenceIn)(__in Windows::Foundation::IReference<Windows::Foundation::TimeSpan> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::TimeSpan *outValue) override;
        IFACEMETHOD(TestPoint_ReferenceIn)(__in Windows::Foundation::IReference<Windows::Foundation::Point> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Point *outValue) override;
        IFACEMETHOD(TestSize_ReferenceIn)(__in Windows::Foundation::IReference<Windows::Foundation::Size> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Size *outValue) override;
        IFACEMETHOD(TestRect_ReferenceIn)(__in Windows::Foundation::IReference<Windows::Foundation::Rect> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Rect *outValue) override;
        IFACEMETHOD(TestEnum_ReferenceIn)(__in Windows::Foundation::IReference<Phylum> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Phylum *outValue) override;

        // IReference<T> out
        IFACEMETHOD(TestBoolean_ReferenceOut)(__out Windows::Foundation::IReference<bool> **outValue, __in boolean inValue) override;
        IFACEMETHOD(TestChar16_ReferenceOut)(__out Windows::Foundation::IReference<wchar_t> **outValue, __in wchar_t inValue) override;
        IFACEMETHOD(TestUInt8_ReferenceOut)(__out Windows::Foundation::IReference<byte> **outValue, __in byte inValue) override;
        IFACEMETHOD(TestInt16_ReferenceOut)(__out Windows::Foundation::IReference<short> **outValue, __in short inValue) override;
        IFACEMETHOD(TestUInt16_ReferenceOut)(__out Windows::Foundation::IReference<unsigned short> **outValue, __in unsigned short inValue) override;
        IFACEMETHOD(TestInt32_ReferenceOut)(__out Windows::Foundation::IReference<INT32> **outValue, __in INT32 inValue) override;
        IFACEMETHOD(TestUInt32_ReferenceOut)(__out Windows::Foundation::IReference<UINT32> **outValue, __in UINT32 inValue) override;
        IFACEMETHOD(TestInt64_ReferenceOut)(__out Windows::Foundation::IReference<INT64> **outValue, __in INT64 inValue) override;
        IFACEMETHOD(TestUInt64_ReferenceOut)(__out Windows::Foundation::IReference<UINT64> **outValue, __in UINT64 inValue) override;
        IFACEMETHOD(TestFloat_ReferenceOut)(__out Windows::Foundation::IReference<float> **outValue, __in float inValue) override;
        IFACEMETHOD(TestDouble_ReferenceOut)(__out Windows::Foundation::IReference<double> **outValue, __in double inValue) override;    
        IFACEMETHOD(TestGuid_ReferenceOut)(__out Windows::Foundation::IReference<GUID> **outValue, __in GUID inValue) override;
        IFACEMETHOD(TestDate_ReferenceOut)(__out Windows::Foundation::IReference<Windows::Foundation::DateTime> **outValue, __in Windows::Foundation::DateTime inValue) override;
        IFACEMETHOD(TestDimensions_ReferenceOut)(__out Windows::Foundation::IReference<Dimensions> **outValue, __in Windows::Foundation::IReference<Dimensions> *inValue) override;
        IFACEMETHOD(TestTimeSpan_ReferenceOut)(__out Windows::Foundation::IReference<Windows::Foundation::TimeSpan> **outValue, __in Windows::Foundation::TimeSpan inValue) override;
        IFACEMETHOD(TestPoint_ReferenceOut)(__out Windows::Foundation::IReference<Windows::Foundation::Point> **outValue, __in Windows::Foundation::Point inValue) override;
        IFACEMETHOD(TestSize_ReferenceOut)(__out Windows::Foundation::IReference<Windows::Foundation::Size> **outValue, __in Windows::Foundation::Size inValue) override;
        IFACEMETHOD(TestRect_ReferenceOut)(__out Windows::Foundation::IReference<Windows::Foundation::Rect> **outValue, __in Windows::Foundation::Rect inValue) override;
        IFACEMETHOD(TestEnum_ReferenceOut)(__out Windows::Foundation::IReference<Phylum> **outValue, __in Windows::Foundation::IReference<Phylum> *inValue) override;

        IFACEMETHOD(get_MyPropertyValue)(__out Windows::Foundation::IPropertyValue **value) override;
        IFACEMETHOD(put_MyPropertyValue)(__in Windows::Foundation::IPropertyValue *value) override;

        IFACEMETHOD(get_MyDimensionsReference)(__out Windows::Foundation::IReference<Dimensions> **value) override;
        IFACEMETHOD(put_MyDimensionsReference)(__in Windows::Foundation::IReference<Dimensions> *value) override;
   };

    class CRCPropertyValue1 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<Dimensions>>
    {
        InspectableClass(L"Windows.Foundation.IReference`1<Animals.Dimensions>", BaseTrust);

    public:
        CRCPropertyValue1() {} ;
        ~CRCPropertyValue1() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out Dimensions *value) override;
    };

    class CRCPropertyValue2 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<Dimensions>>
    {
        InspectableClass(L"Animals.RCPropertyValue2", BaseTrust);

    public:
        CRCPropertyValue2() {} ;
        ~CRCPropertyValue2() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out Dimensions *value) override;
    };

    class CRCPropertyValue3 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<Dimensions>>
    {
        InspectableClass(L"Windows.Foundation.IPropertyValue", BaseTrust);

    public:
        CRCPropertyValue3() {} ;
        ~CRCPropertyValue3() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out Dimensions *value) override;
    };

    class CRCPropertyValue4 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<WCHAR>>
    {
        InspectableClass(L"Windows.Foundation.IReference`1<Char16>", BaseTrust);

    public:
        CRCPropertyValue4() {} ;
        ~CRCPropertyValue4() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out WCHAR *value) override;
    };

    class CRCPropertyValue5 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<WCHAR>>
    {
        InspectableClass(L"Animals.RCPropertyValue5", BaseTrust);

    public:
        CRCPropertyValue5() {} ;
        ~CRCPropertyValue5() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out WCHAR *value) override;
    };

    class CRCPropertyValue6 : public Microsoft::WRL::RuntimeClass<Windows::Foundation::IPropertyValue, Windows::Foundation::IReference<WCHAR>>
    {
        InspectableClass(L"Windows.Foundation.IPropertyValue", BaseTrust);

    public:
        CRCPropertyValue6() {} ;
        ~CRCPropertyValue6() { }

        IFACEMETHOD(get_Type)(__RPC__out enum Windows::Foundation::PropertyType *value) override;
        IFACEMETHOD(get_IsNumericScalar)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetUInt8)(__RPC__out BYTE *value) override;
        IFACEMETHOD(GetInt16)(__RPC__out INT16 *value) override;
        IFACEMETHOD(GetUInt16)(__RPC__out UINT16 *value) override;
        IFACEMETHOD(GetInt32)(__RPC__out INT32 *value) override;
        IFACEMETHOD(GetUInt32)(__RPC__out UINT32 *value) override;
        IFACEMETHOD(GetInt64)(__RPC__out INT64 *value) override;
        IFACEMETHOD(GetUInt64)(__RPC__out UINT64 *value) override;
        IFACEMETHOD(GetSingle)(__RPC__out FLOAT *value) override;
        IFACEMETHOD(GetDouble)(__RPC__out DOUBLE *value) override;
        IFACEMETHOD(GetChar16)(__RPC__out WCHAR *value) override;
        IFACEMETHOD(GetBoolean)(__RPC__out boolean *value) override;
        IFACEMETHOD(GetString)(__RPC__deref_out_opt HSTRING *value) override;
        IFACEMETHOD(GetGuid)(__RPC__out GUID *value) override;
        IFACEMETHOD(GetDateTime)(__RPC__out Windows::Foundation::DateTime *value) override;
        IFACEMETHOD(GetTimeSpan)(__RPC__out Windows::Foundation::TimeSpan *value) override;
        IFACEMETHOD(GetPoint)(__RPC__out Windows::Foundation::Point *value) override;
        IFACEMETHOD(GetSize)(__RPC__out Windows::Foundation::Size *value) override;
        IFACEMETHOD(GetRect)(__RPC__out Windows::Foundation::Rect *value) override;
        IFACEMETHOD(GetUInt8Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value) override;
        IFACEMETHOD(GetInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value) override;
        IFACEMETHOD(GetUInt16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value) override;
        IFACEMETHOD(GetInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value) override;
        IFACEMETHOD(GetUInt32Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value) override;
        IFACEMETHOD(GetInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value) override;
        IFACEMETHOD(GetUInt64Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value) override;
        IFACEMETHOD(GetSingleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value) override;
        IFACEMETHOD(GetDoubleArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value) override;
        IFACEMETHOD(GetChar16Array)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value) override;
        IFACEMETHOD(GetBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value) override;
        IFACEMETHOD(GetStringArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;
        IFACEMETHOD(GetInspectableArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value) override;
        IFACEMETHOD(GetGuidArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value) override;
        IFACEMETHOD(GetDateTimeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value) override;
        IFACEMETHOD(GetTimeSpanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value) override;
        IFACEMETHOD(GetPointArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value) override;
        IFACEMETHOD(GetSizeArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value) override;
        IFACEMETHOD(GetRectArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value) override;

        IFACEMETHOD(get_Value)(__RPC__out WCHAR *value) override;
    };

    class CEmptyGRCNString : public Microsoft::WRL::RuntimeClass<IInspectable>
    {
        InspectableClass(L"", BaseTrust);

    public:
        CEmptyGRCNString() { }
        ~CEmptyGRCNString() { }
    };

    class CFailingGRCNString : public Microsoft::WRL::RuntimeClass<
            Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
            IInspectable>
    {
    public:
        CFailingGRCNString() { }
        ~CFailingGRCNString() { }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids) override;
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className) override;
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel) override;
    };

    class CEmptyGRCN : public Microsoft::WRL::RuntimeClass<Animals::IEmptyGRCN>
    {
        InspectableClass(L"", BaseTrust);

    public:
        CEmptyGRCN() { }
        ~CEmptyGRCN() { }

        IFACEMETHOD(GetMyClassName)(__out HSTRING *outValue) override;
    };

    class CEmptyGRCNInterface : public Microsoft::WRL::RuntimeClass<Animals::IEmptyGRCN>
    {
        InspectableClass(L"", BaseTrust);

    public:
        CEmptyGRCNInterface() { }
        ~CEmptyGRCNInterface() { }

        IFACEMETHOD(GetMyClassName)(__out HSTRING *outValue) override;
    };

    class CEmptyFailingGRCNString : public Microsoft::WRL::RuntimeClass<
            Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
            Animals::IEmptyGRCN>
    {
    public:
        CEmptyFailingGRCNString() { }
        ~CEmptyFailingGRCNString() { }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids) override;
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className) override;
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel) override;

        IFACEMETHOD(GetMyClassName)(__out HSTRING *outValue) override;
    };
}
