//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#define MarshalMethod(type) IFACEMETHOD(Marshal##type)(t_##type _in, __out t_##type* _out) override;

#define SendBackSameInterface(inValue, outValue)        \
    if (outValue == NULL)                               \
    {                                                   \
    return E_POINTER;                               \
    }                                                   \
    \
    *outValue = inValue;                                \
    AddRefPtr(inValue);                                 \
    return S_OK;

#define CallDelegateWithInterface(m_interface)          \
    if (inValue == NULL || outValue == NULL)            \
    {                                                   \
    return E_POINTER;                               \
    }                                                   \
    \
    return inValue->Invoke(m_interface, outValue);

#define GetInterface(m_interface)                       \
    if (value == NULL)                                  \
    {                                                   \
    return E_POINTER;                               \
    }                                                   \
    \
    *value = m_interface;                               \
    AddRefPtr(m_interface);                             \
    return S_OK;

#define PutInterface(m_interface)                       \
    AddRefPtr(m_interface);                             \
    ReleasePtr(m_interface);                            \
    \
    ReleasePtr(m_interface);                            \
    m_interface = value;                                \
    AddRefPtr(m_interface);                             \
    return S_OK;


namespace Animals
{
    class CFastSigInterfaceStatic : public Microsoft::WRL::Implements<Animals::IFastSigInterface>
    {
    public:
        CFastSigInterfaceStatic() { }
        ~CFastSigInterfaceStatic() { }

        IFACEMETHOD(GetOneVector)(__out Windows::Foundation::Collections::IVector<int> **outVal) override;
        IFACEMETHOD(GetNullAsVector)(__out Windows::Foundation::Collections::IVector<int> **outVal) override;
        IFACEMETHOD(GetOneObservableVector)(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) override;
        IFACEMETHOD(GetNullAsObservableVector)(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) override;
        IFACEMETHOD(GetOneAnimal)(__out IAnimal **outVal) override;
        IFACEMETHOD(GetNullAsAnimal)(__out IAnimal **outVal) override;
        IFACEMETHOD(GetOneMap)(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) override;
        IFACEMETHOD(GetNullAsMap)(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) override;
        IFACEMETHOD(GetOnePropertyValue)(__out Windows::Foundation::IPropertyValue **outVal) override;
        IFACEMETHOD(GetNullAsPropertyValue)(__out Windows::Foundation::IPropertyValue **outVal) override;
        IFACEMETHOD(GetOneEmptyGRCNInterface)(__out IEmptyGRCN **outValue) override;
        IFACEMETHOD(GetOneEmptyGRCNNull)(__out IEmptyGRCN **outValue) override;
        IFACEMETHOD(GetOneEmptyGRCNFail)(__out IEmptyGRCN **outValue) override;
    };

    class AnimalServer :
        public Microsoft::WRL::RuntimeClass<Animals::IAnimal, Animals::IGetVector, Animals::IArrayMethods>
    {
        InspectableClass(L"Animals.Animal", BaseTrust);

    public:

        AnimalServer();
        ~AnimalServer();

        // non-ABI method for setting up private state of Toast
        //HRESULT PrivateInitialize(HSTRING hstrMessage);

        // DevX & UEX Coding standards require Hungarian notation. WinRT
        // API guidelines prohibit it. This is because the hungarian type
        // prefixes may be misleading when mapped into different languages. 
        // The way to resolve this is to specify non-prefixed names in the 
        // ReXML or IDL files, but use hungarian parameter names in your 
        // server implementation files.

        // IToast::Message property (read)
        //IFACEMETHOD(get_Message)(__out HSTRING *phstrMessage) override;


        IFACEMETHOD(GetNames)(__out Names* numberOfLegs) override;
        IFACEMETHOD(SetNumLegs)(int numberOfLegs) override;
        IFACEMETHOD(GetNumLegs)(__out int* numberOfLegs) override;
        IFACEMETHOD(SetGreeting)(HSTRING greeting);
        IFACEMETHOD(GetGreeting)(__out HSTRING* greeting);
        IFACEMETHOD(get_Weight)(__out int* weight) override;
        IFACEMETHOD(put_Weight)(int weight) override;

        IFACEMETHOD(IsHungry)(__out boolean* hungry) override;
        IFACEMETHOD(isSleepy)(__out boolean* sleepy) override;

        IFACEMETHOD(get_Mother)(__out IAnimal ** value) override {
            if (*value != nullptr)
            {
                return E_INVALIDARG;
            }

            *value = mother;
            if (*value) (*value)->AddRef();
            return S_OK;
        }

        IFACEMETHOD(put_Mother)(IAnimal * value) override {
            if (mother) mother->Release();
            mother = value;
            if (mother) mother->AddRef();
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_ID(__out GUID* value)
        {
            *value = m_ID;
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE put_ID(GUID value)
        {
            m_ID = value;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_MyDimensions(__out Dimensions *value)
        {
            *value = m_Dimensions;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE put_MyDimensions(Dimensions value)
        {
            m_Dimensions = value;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_MyPhylum(__out Phylum *value)
        {
            *value = m_Phylum;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE put_MyPhylum(Phylum value)
        {
            m_Phylum = value;
            return S_OK;
        }

        IFACEMETHOD(GetDimensions)(__out Dimensions* dimensions) override;

        IFACEMETHOD(FillDimensions)(__in const Dimensions* dimensions) override; 
        IFACEMETHOD(PassIDByRef)(__in const GUID* value) override;
        IFACEMETHOD(CheckMother)(__in const IAnimal* value) override;
        IFACEMETHOD(CheckMotherConcrete)(__in const IAnimal* animal) override;
        IFACEMETHOD(DelegateByRef_Struct)(__in const Animals::IDelegateWithByRefParameter_Struct* value, __in const Dimensions* dimensions) override;
        IFACEMETHOD(DelegateByRef_GUID)(__in const Animals::IDelegateWithByRefParameter_GUID* value, __in const GUID* guid) override;
        IFACEMETHOD(DelegateByRef_Interface)(__in const Animals::IDelegateWithByRefParameter_Interface* value, __in const IAnimal* animal) override;
        IFACEMETHOD(DelegateByRef_Class)(__in const Animals::IDelegateWithByRefParameter_Class* value, __in const IAnimal* animal) override;
        IFACEMETHOD(DelegateByRef_Delegate)(__in const Animals::IDelegateWithByRefParameter_Delegate* value, __in const Animals::IDelegateWithByRefParameter_Struct* del, __in const Dimensions* dimensions) override;
        IFACEMETHOD(IsStructModified)(__in const IDelegateWithByRefParameter_Struct* value, __in const Dimensions* dimensions, __out boolean *wasModified) override;
        IFACEMETHOD(AreDimensionPointersEqual)(__in const Dimensions* one, __in const Dimensions* two, __out boolean *areEqual) override;
        IFACEMETHOD(AcceptKiloStruct)(__in const KiloStruct* kiloStruct) override;
        IFACEMETHOD(CheckByRefStruct)(__in Windows::Foundation::IReference<MixedStruct>* one, __in const MixedStruct* two, __out MixedStructResult* result) override;

        IFACEMETHOD(AddInts)(int val1, int val2, __out int* result) override;
        IFACEMETHOD(GetOuterStruct)(__out OuterStruct* strct) override;

        IFACEMETHOD(MarshalPhylum)(Phylum _in, __out Phylum* _out) override;
        IFACEMETHOD(MarshalPhylumChange)(PhylumChange _in, __out PhylumChange* _out) override;
        IFACEMETHOD(MarshalHSTRING)(HSTRING _in, __out HSTRING* _out) override;
        IFACEMETHOD(MarshalNames)(Names _in, __out Names* _out) override;

        IFACEMETHOD(GetVector)(
            __out Windows::Foundation::Collections::IVector<int> **uniqueNumbersVector) override;
        IFACEMETHOD(CopyVector)(
            Windows::Foundation::Collections::IVector<int> * inVector,__out Windows::Foundation::Collections::IVector<int> ** outVector) override;

        IFACEMETHOD(GetStringVector)(
            __out Windows::Foundation::Collections::IVector<HSTRING> **outVector) override;
        IFACEMETHOD(CopyStringVector)(
            Windows::Foundation::Collections::IVector<HSTRING> * inVector,__out Windows::Foundation::Collections::IVector<HSTRING> ** outVector) override;

        IFACEMETHOD(GetMap)(
            Windows::Foundation::Collections::IVector<int> *uniqueNumbersVector,
            __out Windows::Foundation::Collections::IMap<int, HSTRING> **uniqueNumbersMap) override;


        IFACEMETHOD(LikesChef)(
            __out Fabrikam::Kitchen::IChef **chef) override;

        HRESULT GetArray(__out Windows::Foundation::Collections::IVector<int> ** outVector, __in int from, __in int to);
        HRESULT PassArrayCore(__in UINT32 length, __RPC__in_ecount_part(length, lengthValue) int *value, __in UINT32 lengthValue);

        IFACEMETHOD(PurePassArray)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) int *value) override;

        IFACEMETHOD(PureFillArray)( 
            __in UINT32 length,
            __RPC__out_ecount_full(length) int *value) override;

        IFACEMETHOD(PureReceiveArray)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) int **value) override;

        IFACEMETHOD(put_MyArrayProp)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) int *value) override;

        IFACEMETHOD(get_MyArrayProp)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) int **value) override;

        IFACEMETHOD(put_MyArrayPropHSTRING)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) HSTRING *value) override;

        IFACEMETHOD(get_MyArrayPropHSTRING)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;

        IFACEMETHOD(PassArray)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) int *value,
            __out Windows::Foundation::Collections::IVector<int> ** outVector) override;

        IFACEMETHOD(FillArray)( 
            __in UINT32 length,
            __RPC__out_ecount_full(length) int *value,
            __out Windows::Foundation::Collections::IVector<int> ** outVector) override;

        IFACEMETHOD(ReceiveArray)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) int **value,
            __out Windows::Foundation::Collections::IVector<int> ** outVector) override;

        IFACEMETHOD(CallDelegatePassArray)(__in IDelegateWithInParam_Array* delegatePassArray) override;
        IFACEMETHOD(CallDelegateFillArray)(__in IDelegateWithInOutParam_Array* delegateFillArray) override;
        IFACEMETHOD(CallDelegateReceiveArray)(__in IDelegateWithOutParam_Array* delegateReceiveArray) override;

        IFACEMETHOD(PassArrayWithInLength)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) int *value, 
            __in UINT32 lengthValue) override;

        IFACEMETHOD(PassArrayWithOutLength)( 
            __in UINT32 length,
            __RPC__in_ecount_part(length, *lengthValue) int *value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(FillArrayWithInLength)( 
            __in UINT32 length,
            __RPC__out_ecount_part(length, lengthValue) int *value,
            __in UINT32 lengthValue) override;

        IFACEMETHOD(FillArrayWithOutLength)( 
            __in UINT32 length,
            __RPC__out_ecount_part(length, *lengthValue) int *value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(ReceiveArrayWithInLength)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, lengthValue) int **value,
            __in UINT32 lengthValue) override;

        IFACEMETHOD(ReceiveArrayWithOutLength)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) int **value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(PassArrayWithOutLengthWithRetValLength)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) int* value, 
            __RPC__out UINT32 *lengthValue) override
        {
            return PassArrayWithOutLength(length, value, lengthValue);
        }

        IFACEMETHOD(PassArrayWithOutLengthWithRetValRandomParam)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) int* value, 
            __RPC__out UINT32 *lengthValue, 
            __out int *randomRetVal) override
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = PassArrayWithOutLength(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(FillArrayWithOutLengthWithRetValLength)( 
            __in UINT32 length, 
            __RPC__out_ecount_part(length, *lengthValue) int* value, 
            __RPC__out UINT32 *lengthValue) override
        {
            return FillArrayWithOutLength(length, value, lengthValue);
        }

        IFACEMETHOD(FillArrayWithOutLengthWithRetValRandomParam)( 
            __in UINT32 length, 
            __RPC__out_ecount_part(length, *lengthValue) int* value, 
            __RPC__out UINT32 *lengthValue, 
            __out int *randomRetVal)
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = FillArrayWithOutLength(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(ReceiveArrayWithOutLengthWithRetValLength)(
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) int **value,
            __RPC__out UINT32 *lengthValue) override
        {
            return ReceiveArrayWithOutLength(length, value, lengthValue);
        }

        IFACEMETHOD(ReceiveArrayWithOutLengthWithRetValRandomParam)(
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) int **value,
            __RPC__out UINT32 *lengthValue,
            __out int *randomRetVal) override
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = ReceiveArrayWithOutLength(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(CallDelegatePassArrayWithInLength)(__in IDelegatePassArrayWithInLength *delegateIn) override;
        IFACEMETHOD(CallDelegatePassArrayWithOutLength)(__in IDelegatePassArrayWithOutLength *delegateIn) override; 
        IFACEMETHOD(CallDelegateFillArrayWithInLength)(__in IDelegateFillArrayWithInLength *delegateIn) override; 
        IFACEMETHOD(CallDelegateFillArrayWithOutLength)(__in IDelegateFillArrayWithOutLength *delegateIn) override; 
        IFACEMETHOD(CallDelegateReceiveArrayWithInLength)(__in IDelegateReceiveArrayWithInLength *delegateIn) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLength)(__in IDelegateReceiveArrayWithOutLength *delegateIn) override; 

        IFACEMETHOD(CallDelegatePassArrayWithOutLengthWithRetValLength)(__in IDelegatePassArrayWithOutLengthWithRetValLength *delegateIn) override;
        IFACEMETHOD(CallDelegatePassArrayWithOutLengthWithRetValRandomParam)(__in IDelegatePassArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal) override;
        IFACEMETHOD(CallDelegateFillArrayWithOutLengthWithRetValLength)(__in IDelegateFillArrayWithOutLengthWithRetValLength *delegateIn) override;
        IFACEMETHOD(CallDelegateFillArrayWithOutLengthWithRetValRandomParam)(__in IDelegateFillArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLengthWithRetValLength)(__in IDelegateReceiveArrayWithOutLengthWithRetValLength *delegateIn) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLengthWithRetValRandomParam)(__in IDelegateReceiveArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal) override;

        HRESULT GetArrayHSTRING(__out Windows::Foundation::Collections::IVector<HSTRING> ** outVector, __in int from, __in int to);
        HRESULT PassArrayHSTRINGCore(__in UINT32 length, __RPC__in_ecount_part(length, lengthValue) HSTRING *value, __in UINT32 lengthValue);

        IFACEMETHOD(PassArrayHSTRING)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) HSTRING *value,
            __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector) override;

        IFACEMETHOD(FillArrayHSTRING)( 
            __in UINT32 length,
            __RPC__out_ecount_full(length) HSTRING *value,
            __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector) override;

        IFACEMETHOD(ReceiveArrayHSTRING)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) HSTRING **value,
            __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector) override;

        IFACEMETHOD(CallDelegatePassArrayHSTRING)(__in IDelegateWithInParam_ArrayHSTRING* delegatePassArrayHSTRING) override;
        IFACEMETHOD(CallDelegateFillArrayHSTRING)(__in IDelegateWithInOutParam_ArrayHSTRING* delegateFillArrayHSTRING) override;
        IFACEMETHOD(CallDelegateReceiveArrayHSTRING)(__in IDelegateWithOutParam_ArrayHSTRING* delegateReceiveArrayHSTRING) override;

        IFACEMETHOD(PassArrayWithInLengthHSTRING)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) HSTRING *value, 
            __in UINT32 lengthValue) override;

        IFACEMETHOD(PassArrayWithOutLengthHSTRING)( 
            __in UINT32 length,
            __RPC__in_ecount_part(length, *lengthValue) HSTRING *value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(FillArrayWithInLengthHSTRING)( 
            __in UINT32 length,
            __RPC__out_ecount_part(length, lengthValue) HSTRING *value,
            __in UINT32 lengthValue) override;

        IFACEMETHOD(FillArrayWithOutLengthHSTRING)( 
            __in UINT32 length,
            __RPC__out_ecount_part(length, *lengthValue) HSTRING *value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(ReceiveArrayWithInLengthHSTRING)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, lengthValue) HSTRING **value,
            __in UINT32 lengthValue) override;

        IFACEMETHOD(ReceiveArrayWithOutLengthHSTRING)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) HSTRING **value,
            __RPC__out UINT32 *lengthValue) override;

        IFACEMETHOD(PassArrayWithOutLengthWithRetValLengthHSTRING)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) HSTRING* value, 
            __RPC__out UINT32 *lengthValue) override
        {
            return PassArrayWithOutLengthHSTRING(length, value, lengthValue);
        }

        IFACEMETHOD(PassArrayWithOutLengthWithRetValRandomParamHSTRING)(
            __in UINT32 length, 
            __RPC__in_ecount_part(length, lengthValue) HSTRING* value, 
            __RPC__out UINT32 *lengthValue, 
            __out int *randomRetVal) override
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = PassArrayWithOutLengthHSTRING(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(FillArrayWithOutLengthWithRetValLengthHSTRING)( 
            __in UINT32 length, 
            __RPC__out_ecount_part(length, *lengthValue) HSTRING* value, 
            __RPC__out UINT32 *lengthValue) override
        {
            return FillArrayWithOutLengthHSTRING(length, value, lengthValue);
        }

        IFACEMETHOD(FillArrayWithOutLengthWithRetValRandomParamHSTRING)( 
            __in UINT32 length, 
            __RPC__out_ecount_part(length, *lengthValue) HSTRING* value, 
            __RPC__out UINT32 *lengthValue, 
            __out int *randomRetVal)
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = FillArrayWithOutLengthHSTRING(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(ReceiveArrayWithOutLengthWithRetValLengthHSTRING)(
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) HSTRING **value,
            __RPC__out UINT32 *lengthValue) override
        {
            return ReceiveArrayWithOutLengthHSTRING(length, value, lengthValue);
        }

        IFACEMETHOD(ReceiveArrayWithOutLengthWithRetValRandomParamHSTRING)(
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_part_opt(*length, *lengthValue) HSTRING **value,
            __RPC__out UINT32 *lengthValue,
            __out int *randomRetVal) override
        {
            if (randomRetVal == nullptr)
            {
                return E_POINTER;
            }

            HRESULT hr = ReceiveArrayWithOutLengthHSTRING(length, value, lengthValue);
            if (SUCCEEDED(hr))
            {
                return *randomRetVal = 100;
            }
            return hr;
        }

        IFACEMETHOD(CallDelegatePassArrayWithInLengthHSTRING)(__in IDelegatePassArrayWithInLengthHSTRING *delegateIn) override;
        IFACEMETHOD(CallDelegatePassArrayWithOutLengthHSTRING)(__in IDelegatePassArrayWithOutLengthHSTRING *delegateIn) override; 
        IFACEMETHOD(CallDelegateFillArrayWithInLengthHSTRING)(__in IDelegateFillArrayWithInLengthHSTRING *delegateIn) override; 
        IFACEMETHOD(CallDelegateFillArrayWithOutLengthHSTRING)(__in IDelegateFillArrayWithOutLengthHSTRING *delegateIn) override; 
        IFACEMETHOD(CallDelegateReceiveArrayWithInLengthHSTRING)(__in IDelegateReceiveArrayWithInLengthHSTRING *delegateIn) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLengthHSTRING)(__in IDelegateReceiveArrayWithOutLengthHSTRING *delegateIn) override; 

        IFACEMETHOD(CallDelegatePassArrayWithOutLengthWithRetValLengthHSTRING)(__in IDelegatePassArrayWithOutLengthWithRetValLengthHSTRING *delegateIn) override;
        IFACEMETHOD(CallDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING)(__in IDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal) override;
        IFACEMETHOD(CallDelegateFillArrayWithOutLengthWithRetValLengthHSTRING)(__in IDelegateFillArrayWithOutLengthWithRetValLengthHSTRING *delegateIn) override;
        IFACEMETHOD(CallDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING)(__in IDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING)(__in IDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING *delegateIn) override;
        IFACEMETHOD(CallDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING)(__in IDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal) override;

        MarshalMethod(Bool);
        MarshalMethod(UInt8);
        MarshalMethod(Int32);
        MarshalMethod(UInt32);
        MarshalMethod(Int64);
        MarshalMethod(UInt64);
        MarshalMethod(Single);
        MarshalMethod(Double);
        MarshalMethod(Char16);
        IFACEMETHOD(MarshalDimensions)(Dimensions _in, __out Dimensions* _out) override;
        IFACEMETHOD(MarshalOuterStruct)(OuterStruct _in, __out OuterStruct* _out) override;
        IFACEMETHOD(MarshalStudyInfo)(StudyInfo _in, __out StudyInfo* _out);

        HRESULT STDMETHODCALLTYPE MarshalInt16(short _in, __out short* _out)
        {
            *_out =_in;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE MarshalUInt16(unsigned short _in, unsigned short* _out)
        {
            *_out =_in;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE MarshalGUID(GUID _in, __out GUID* _out)
        {
            *_out =_in;
            return S_OK;
        }

        IFACEMETHOD(get_ErrorCode)(HRESULT * hr) override {
            *hr = 192;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE MarshalHRESULT(HRESULT hrIn, HRESULT * hrOut)
        {
            *hrOut = hrIn;
            return S_OK;
        }

        IFACEMETHOD(VerifyMarshalGUID)(HSTRING expected, GUID _in, __out GUID* _out);

        //MIDLRT bug: Win8 112810
        //MarshalMethod(Dimensions);
        //MarshalMethod(OuterStruct);

        IFACEMETHOD(GetNULLHSTRING)(__out HSTRING* _out) override;

        //Methods with Multiple [out] parameters (all basic types)
        IFACEMETHOD(MultipleOutBool)(t_Bool a, t_Bool b, __out t_Bool* reta, __out t_Bool* retb);
        IFACEMETHOD(MultipleOutUInt8)(t_UInt8 a, t_UInt8 b, __out t_UInt8* reta, __out t_UInt8* retb);
        IFACEMETHOD(MultipleOutInt32)(t_Int32 a, t_Int32 b, __out t_Int32* reta, __out t_Int32* retb);
        IFACEMETHOD(MultipleOutUInt32)(t_UInt32 a, t_UInt32 b, __out t_UInt32* reta, __out t_UInt32* retb);
        IFACEMETHOD(MultipleOutInt64)(t_Int64 a, t_Int64 b, __out t_Int64* reta, __out t_Int64* retb);
        IFACEMETHOD(MultipleOutUInt64)(t_UInt64 a, t_UInt64 b, __out t_UInt64* reta, __out t_UInt64* retb);
        IFACEMETHOD(MultipleOutSingle)(t_Single a, t_Single b, __out t_Single* reta, __out t_Single* retb);
        IFACEMETHOD(MultipleOutDouble)(t_Double a, t_Double b, __out t_Double* reta, __out t_Double* retb);
        IFACEMETHOD(MultipleOutChar16)(t_Char16 a, t_Char16 b, __out t_Char16* reta, __out t_Char16* retb);
        IFACEMETHOD(MultipleOutHSTRING)(HSTRING a, HSTRING b, __out HSTRING* reta, __out HSTRING* retb);
        IFACEMETHOD(MultipleOutPhylum)(Phylum a, Phylum b, __out Phylum* reta, __out Phylum* retb);
        IFACEMETHOD(MultipleOutDimensions)(Dimensions a, Dimensions b, __out Dimensions* reta, __out Dimensions* retb);
        IFACEMETHOD(MultipleOutIFish)(IFish* a, IFish* b, __out IFish** reta, __out IFish** retb);
        IFACEMETHOD(MultipleOutFish)(IFish* a, IFish* b, __out IFish** reta, __out IFish** retb);    

        //Methods with interspersed [in] and [out] parameters (all basic types)
        IFACEMETHOD(InterspersedInOutBool)(t_Bool a, __out t_Bool* reta, t_Bool b, __out t_Bool* retb);	 
        IFACEMETHOD(InterspersedInOutUInt8)(t_UInt8 a, __out t_UInt8* reta, t_UInt8 b, __out t_UInt8* retb);
        IFACEMETHOD(InterspersedInOutInt32)(t_Int32 a, __out t_Int32* reta, t_Int32 b, __out t_Int32* retb);
        IFACEMETHOD(InterspersedInOutUInt32)(t_UInt32 a, __out t_UInt32* reta, t_UInt32 b, __out t_UInt32* retb);
        IFACEMETHOD(InterspersedInOutInt64)(t_Int64 a, __out t_Int64* reta, t_Int64 b, __out t_Int64* retb);
        IFACEMETHOD(InterspersedInOutUInt64)(t_UInt64 a, __out t_UInt64* reta, t_UInt64 b, __out t_UInt64* retb);
        IFACEMETHOD(InterspersedInOutSingle)(t_Single a, __out t_Single* reta, t_Single b, __out t_Single* retb);
        IFACEMETHOD(InterspersedInOutDouble)(t_Double a, __out t_Double* reta, t_Double b, __out t_Double* retb);
        IFACEMETHOD(InterspersedInOutChar16)(t_Char16 a, __out t_Char16* reta, t_Char16 b, __out t_Char16* retb);
        IFACEMETHOD(InterspersedInOutHSTRING)(HSTRING a, HSTRING b, __out HSTRING* reta, __out HSTRING* retb);
        IFACEMETHOD(InterspersedInOutPhylum)(Phylum a, __out Phylum* reta, Phylum b, __out Phylum* retb);
        IFACEMETHOD(InterspersedInOutDimensions)(Dimensions a, __out Dimensions* reta, Dimensions b, __out Dimensions* retb);
        IFACEMETHOD(InterspersedInOutIFish)(IFish* a, __out IFish** reta, IFish* b, __out IFish** retb);
        IFACEMETHOD(InterspersedInOutFish)(IFish* a, __out IFish** reta, IFish* b, __out IFish** retb);    

        //Method to ensure layout is correct for with multiple or different alignment members
        IFACEMETHOD(LayoutOfManyMembers)(t_UInt8 a, t_Int32 b, t_UInt8 c, t_Double d, t_UInt8 e, t_UInt8 f, t_Double g, t_Int32 h,  t_Double i, 
            __out t_UInt8* reta, __out t_Int32* retb, __out t_UInt8* retc, __out t_Double* retd, __out t_UInt8* rete, __out t_UInt8* retf, __out t_Double* retg, __out t_Int32* reth,  __out t_Double* reti);

        IFACEMETHOD(LayoutStructs)(InnerStruct a, Dimensions b, OuterStruct c, Names d, PhylumChange e, __out InnerStruct* reta, __out Dimensions* retb, __out OuterStruct* retc, __out Names* retd, __out PhylumChange* rete);

        IFACEMETHOD(LayoutBasicWithStructs)(t_UInt8 a, InnerStruct b, t_Int32 c, t_Double d, Names e, t_UInt8 f, t_UInt8 g, Dimensions h, t_Int32 i, 
            __out t_UInt8* reta, __out InnerStruct* retb, __out t_Int32* retc, __out t_Double* retd, __out Names* rete, __out t_UInt8* retf, __out t_UInt8* retg, __out Dimensions* reth, __out t_Int32* reti);

        //Methods with multiple float/double parameters
        IFACEMETHOD(MultiFloat3)(t_Single a, t_Single b, t_Single c, __out t_Single* reta, __out t_Single* retb, __out t_Single* retc);	 
        IFACEMETHOD(MultiFloat4)(t_Single a, t_Single b, t_Single c, t_Single d, __out t_Single* reta, __out t_Single* retb, __out t_Single* retc, __out t_Single* retd);
        IFACEMETHOD(MultiDouble3)(t_Double a, t_Double b, t_Double c, __out t_Double* reta, __out t_Double* retb, __out t_Double* retc);	 	 
        IFACEMETHOD(MultiDouble4)(t_Double a, t_Double b, t_Double c, t_Double d, __out t_Double* reta, __out t_Double* retb, __out t_Double* retc, __out t_Double* retd);


        //Methods with t_Single/t_Double parameters at different offsets
        IFACEMETHOD(FloatOffsetChar)(t_Char16 a, t_Single b, __out t_Char16* reta, __out t_Single* retb);
        IFACEMETHOD(FloatOffsetByte)(t_UInt8 a, t_Single b, __out t_UInt8* reta, __out t_Single* retb);
        IFACEMETHOD(FloatOffsetInt)(t_Int32 a, t_Single b, __out t_Int32* reta, __out t_Single* retb);
        IFACEMETHOD(FloatOffsetInt64)(t_Int64 a, t_Single b, __out t_Int64* reta, __out t_Single* retb);
        IFACEMETHOD(FloatOffset2Int)(t_Int32 a, t_Int32 b, t_Single c, __out t_Int32* reta, __out t_Int32* retb, __out t_Single* retc);
        IFACEMETHOD(FloatOffsetStruct)(Names a, t_Single b, __out Names* reta, __out t_Single* retb);

        IFACEMETHOD(DoubleOffsetChar)(t_Char16 a, t_Double b, __out t_Char16* reta, __out t_Double* retb);
        IFACEMETHOD(DoubleOffsetByte)(t_UInt8 a, t_Double b, __out t_UInt8* reta, __out t_Double* retb);
        IFACEMETHOD(DoubleOffsetInt)(t_Int32 a, t_Double b, __out t_Int32* reta, __out t_Double* retb);
        IFACEMETHOD(DoubleOffsetInt64)(t_Int64 a, t_Double b, __out t_Int64* reta, __out t_Double* retb);
        IFACEMETHOD(DoubleOffset2Int)(t_Int32 a, t_Int32 b, t_Double c, __out t_Int32* reta, __out t_Int32* retb, __out t_Double* retc);
        IFACEMETHOD(DoubleOffsetStruct)(Names a, t_Double b, __out Names* reta, __out t_Double* retb);

        HRESULT STDMETHODCALLTYPE TestInSimpleIRefStruct( SimpleIRefStruct simpleIRefStruct);
        HRESULT STDMETHODCALLTYPE TestOutSimpleIRefStruct(int seedValue, __out SimpleIRefStruct* simpleIRefStruct);

        HRESULT STDMETHODCALLTYPE TestInMixIRefStruct(__in SimpleMixIRefStruct mixedIRefStruct);
        HRESULT STDMETHODCALLTYPE TestOutMixIRefStruct(int seedValue, __out SimpleMixIRefStruct* mixedIRefStruct);

        HRESULT STDMETHODCALLTYPE TestInNestedIRefStruct(__in NestedIRefStruct nestedIRefStruct);
        HRESULT STDMETHODCALLTYPE TestOutNestedIRefStruct(int seedValue, __out NestedIRefStruct* nestedIRefStruct);

        HRESULT STDMETHODCALLTYPE TestInNestedIRefNestedStruct(__in NestedIRefNestedStruct nestedIRefStruct);
        HRESULT STDMETHODCALLTYPE TestOutNestedIRefNestedStruct(int seedValue, __out NestedIRefNestedStruct* nestedIRefStruct);

        HRESULT STDMETHODCALLTYPE TestInAllIRefStruct(__in AllIRefStruct allIRefStruct);
        HRESULT STDMETHODCALLTYPE TestOutAllIRefStruct(int seedValue, __out AllIRefStruct* allIRefStruct);

        // Bug 258665
        HRESULT STDMETHODCALLTYPE TestOutBug258665_HttpProgress(__in HSTRING url, __out Bug258665_HttpProgress* structHttpProgress);
        HRESULT STDMETHODCALLTYPE TestOutBug258665_HttpProgressAsOptEmpty(__out_opt Windows::Foundation::IReference<Bug258665_HttpProgress>** optionalStructHttpProgress);
        HRESULT STDMETHODCALLTYPE TestOutBug258665_HttpProgressAsOptIntEmpty(__out_opt Windows::Foundation::IReference<UINT64>** optionalUInt64);
        // /Bug 258665
    
        //Method to return given int as HRESULT (for error testing)
        IFACEMETHOD(TestError)(HRESULT hr);
        IFACEMETHOD(TestPackedByte12)(PackedByte value)
        {
            if(value.Field0 != 188) 
                return E_FAIL;
            return S_OK;
        }

        IFACEMETHOD(DelIn_BooleanOut2)(IBooleanOut2* p0)
        {
            boolean outparam0;
            boolean outparam1;
            HRESULT hr = p0->Invoke(&outparam0, &outparam1);
            if(!SUCCEEDED(hr)) return hr;
            if(!(outparam0 == TRUE)) return E_FAIL;
            if(!(outparam1 == FALSE)) return E_FAIL;
            return S_OK;
        }

        IFACEMETHOD(TestPackedBoolean1)(PackedBoolean4 value)
        {
            if(value.Field0 && !value.Field1 && value.Field2 && !value.Field3) 
                return S_OK;
            return E_FAIL;
        }

        IFACEMETHOD(SendAndGetIVectorStructs)(Windows::Foundation::Collections::IVector<InnerStruct >* inVector, Windows::Foundation::Collections::IVector<InnerStruct >** outVector)
        {
            HRESULT hr=S_OK;
            InnerStruct myStruct;
            UINT32 ccount;
            hr=inVector->GetAt(0,&myStruct);
            inVector->get_Size(&ccount);
            *outVector=inVector;
            AddRefPtr(inVector)
                return hr;
        }

        HRESULT STDMETHODCALLTYPE FastPath()
        {
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE FastPathIn(int _in)
        {
            UNREFERENCED_PARAMETER(_in);

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE FastPathOut(int* _out)
        {
            *_out = 16;

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE FastPathInOut(int _in, int* _out)
        {
            *_out = _in;

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE FastPathInIn(int _in1, int _in2)
        {
            UNREFERENCED_PARAMETER(_in1);
            UNREFERENCED_PARAMETER(_in2);

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE SlowPath(int _in1, int _in2, int _in3, int _in4, int _in5, int _in6)
        {
            UNREFERENCED_PARAMETER(_in1);
            UNREFERENCED_PARAMETER(_in2);
            UNREFERENCED_PARAMETER(_in3);
            UNREFERENCED_PARAMETER(_in4);
            UNREFERENCED_PARAMETER(_in5);
            UNREFERENCED_PARAMETER(_in6);

            return S_OK;
        }

        IFACEMETHOD(add_EventHandler)(Animals::ISimpleDelegateForEventHandler *handler, EventRegistrationToken *eventCookie)
        {
            return _evtSimpleEventHandler.Add(handler, eventCookie);
        }

        IFACEMETHOD(remove_EventHandler)(EventRegistrationToken eventCookie)
        {
            return _evtSimpleEventHandler.Remove(eventCookie);
        }

        Microsoft::WRL::EventSource<Animals::ISimpleDelegateForEventHandler> _evtSimpleEventHandler;

        IFACEMETHOD(TestBug202724_GetInt64)(long long * _out)
        {
            *_out = 4294967296;

            return S_OK;
        }

        IFACEMETHOD(TestBug202724_GetUInt64)(unsigned long long * _out)
        {
            *_out = 4294967296;

            return S_OK;
        }

        IFACEMETHOD(TestBug8327782_StackArguments)(HSTRING str1, HSTRING str2, HSTRING str3, HSTRING str4, HSTRING str5, HSTRING str6, HSTRING str7, HSTRING str8, HSTRING str9, HSTRING* result)
        {
            // Do non-trivial work to ensure that the stack/registers are touched
            WindowsConcatString(str1, str2, result);
            WindowsConcatString(*result, str3, result);
            WindowsConcatString(*result, str4, result);
            WindowsConcatString(*result, str5, result);
            WindowsConcatString(*result, str6, result);
            WindowsConcatString(*result, str7, result);
            WindowsConcatString(*result, str8, result);
            WindowsConcatString(*result, str9, result);

            return S_OK;
        }

#define CallDelegateWithOutParamDeclare(typeName) \
    IFACEMETHOD(CallDelegateWithOutParam_##typeName)( \
    IDelegateWithOutParam_##typeName* onDelegateWithOut##typeName, \
##typeName *outParam); \

        CallDelegateWithOutParamDeclare(HSTRING);
        CallDelegateWithOutParamDeclare(int);

        IFACEMETHOD(CallDelegateWithOutParam_Interface)(
            IDelegateWithOutParam_Interface* onDelegateWithOutInterface, 
            IAnimal **outParam); 

        IFACEMETHOD(CallDelegateWithOutParam_Struct)(
            IDelegateWithOutParam_Struct* onDelegateWithOutStruct, 
            Dimensions *outParam); 

        IFACEMETHOD(CallDelegateWithOutParam_InOutMixed)(
            IDelegateWithOutParam_InOutMixed* onDelegateWithInOutMixed, 
            Dimensions *outParam,
            int weight); 

        IFACEMETHOD(CallDelegateWithMultipleOutParams)(
            IDelegateWithOutParam_MultipleOutParams* onDelegateWithMultipleOutParams, 
            Names * names, 
            int *newWeight, 
            int weight, 
            IAnimal **outAnimal);

        IFACEMETHOD(MarshalNullAsDelegate)(
            IDelegateWithOutParam_HSTRING* inDelegate, 
            HSTRING *outMessage);

        IFACEMETHOD(MethodDelegateAsOutParam)(
            IDelegateWithOutParam_HSTRING* inDelegate, 
            IDelegateWithOutParam_HSTRING** outDelegate);

        IFACEMETHOD(GetNativeDelegateAsOutParam)(
            IDelegateWithOutParam_HSTRING **outDelegate);


        IFACEMETHOD(DuplicateIterable)(
            __in Windows::Foundation::Collections::IIterable<int> *inIterable,
            __out Windows::Foundation::Collections::IIterable<int> **outIterable) override;
        IFACEMETHOD(DuplicateStringIterable)(
            __in Windows::Foundation::Collections::IIterable<HSTRING> *inIterable,
            __out Windows::Foundation::Collections::IIterable<HSTRING> **outIterable) override;

        IFACEMETHOD(DuplicateIterator)(
            __in Windows::Foundation::Collections::IIterator<int> *inIterator,
            __out Windows::Foundation::Collections::IIterator<int> **outIterator) override;
        IFACEMETHOD(DuplicateStringIterator)(
            __in Windows::Foundation::Collections::IIterator<HSTRING> *inIterator,
            __out Windows::Foundation::Collections::IIterator<HSTRING> **outIterator) override;

        IFACEMETHOD(DuplicateVectorView)(
            __in Windows::Foundation::Collections::IVectorView<int> *inVectorView,
            __out Windows::Foundation::Collections::IVectorView<int> **outVectorView) override;
        IFACEMETHOD(DuplicateStringVectorView)(
            __in Windows::Foundation::Collections::IVectorView<HSTRING> *inVectorView,
            __out Windows::Foundation::Collections::IVectorView<HSTRING> **outVectorView) override;

        IFACEMETHOD(DuplicateVector)(
            __in Windows::Foundation::Collections::IVector<int> *inVector,
            __out Windows::Foundation::Collections::IVector<int> **outVector) override;
        IFACEMETHOD(DuplicateStringVector)(
            __in Windows::Foundation::Collections::IVector<HSTRING> *inVector,
            __out Windows::Foundation::Collections::IVector<HSTRING> **outVector) override;

        IFACEMETHOD(SendBackSameIterable)(
            __in Windows::Foundation::Collections::IIterable<int> *inIterable,
            __out Windows::Foundation::Collections::IIterable<int> **outIterable) override;
        IFACEMETHOD(SendBackSameStringIterable)(
            __in Windows::Foundation::Collections::IIterable<HSTRING> *inIterable,
            __out Windows::Foundation::Collections::IIterable<HSTRING> **outIterable) override;

        IFACEMETHOD(SendBackSameIterator)(
            __in Windows::Foundation::Collections::IIterator<int> *inIterator,
            __out Windows::Foundation::Collections::IIterator<int> **outIterator) override;
        IFACEMETHOD(SendBackSameStringIterator)(
            __in Windows::Foundation::Collections::IIterator<HSTRING> *inIterator,
            __out Windows::Foundation::Collections::IIterator<HSTRING> **outIterator) override;

        IFACEMETHOD(SendBackSameVectorView)(
            __in Windows::Foundation::Collections::IVectorView<int> *inVectorView,
            __out Windows::Foundation::Collections::IVectorView<int> **outVectorView) override;
        IFACEMETHOD(SendBackSameStringVectorView)(
            __in Windows::Foundation::Collections::IVectorView<HSTRING> *inVectorView,
            __out Windows::Foundation::Collections::IVectorView<HSTRING> **outVectorView) override;

        IFACEMETHOD(SendBackSameVector)(
            __in Windows::Foundation::Collections::IVector<int> *inVector,
            __out Windows::Foundation::Collections::IVector<int> **outVector) override;
        IFACEMETHOD(SendBackSameStringVector)(
            __in Windows::Foundation::Collections::IVector<HSTRING> *inVector,
            __out Windows::Foundation::Collections::IVector<HSTRING> **outVector) override;

        IFACEMETHOD(GetObservableVector)(
            __out Windows::Foundation::Collections::IObservableVector<int> **outObservableVector) override;

        IFACEMETHOD(GetObservableStringVector)(
            __out Windows::Foundation::Collections::IObservableVector<HSTRING> **outObservableVector) override;

        IFACEMETHOD(CallDelegateWithVector)(__in IDelegateWithVector *inValue, __out Windows::Foundation::Collections::IVector<int> **outValue) override
        {
            CallDelegateWithInterface(m_Vector);
        }

        IFACEMETHOD(CallDelegateWithIterable)(__in IDelegateWithIterable *inValue, __out Windows::Foundation::Collections::IIterable<int> **outValue) override
        {
            CallDelegateWithInterface(m_Iterable);
        }

        IFACEMETHOD(get_MyVector)(__out Windows::Foundation::Collections::IVector<int> **value) override
        {
            GetInterface(m_Vector);
        }

        IFACEMETHOD(put_MyVector)(__in Windows::Foundation::Collections::IVector<int> *value) override
        {
            PutInterface(m_Vector);
        }

        IFACEMETHOD(get_MyIterable)(__out Windows::Foundation::Collections::IIterable<int> **value) override
        {
            GetInterface(m_Iterable);
        }

        IFACEMETHOD(put_MyIterable)(__in Windows::Foundation::Collections::IIterable<int> *value) override
        {
            PutInterface(m_Iterable);
        }

        IFACEMETHOD(GetReadOnlyVector)(
            __in Windows::Foundation::Collections::IVector<int> *inVector,
            __out Windows::Foundation::Collections::IVector<int> **outVector) override;

    private:
        int m_Weight;
        int m_NumLegs;
        HSTRING m_Greeting;
        Dimensions m_Dimensions;
        OuterStruct m_OuterStruct;
        Names m_Names;
        IAnimal * mother;
        GUID m_ID;
        int* m_array;
        UINT32 m_arrayLength; 
        UINT32 m_arraySize;
        HSTRING* m_arrayHSTRING;
        UINT32 m_arrayLengthHSTRING;
        UINT32 m_arraySizeHSTRING;
        Phylum m_Phylum;
        Windows::Foundation::Collections::IVector<int> *m_Vector;
        Windows::Foundation::Collections::IIterable<int> *m_Iterable;
        Windows::Foundation::IReference<Dimensions> *m_dimensionsRef;

        template <class typeName>
        HRESULT GetIReferenceValue(Windows::Foundation::IReference<typeName>* inReference,  Windows::Foundation::PropertyType inPropertyType, __out typeName* value);

        HRESULT GetBooleanIReferenceValue(Windows::Foundation::IReference<bool>* inReference,  __out boolean* value);

        Microsoft::WRL::ComPtr<Windows::Foundation::IPropertyValueStatics> spPropertyValueFactory;

        //Windows::Internal::String _hstrMessage;
    };

    // This class doesn't use the ActivatableClass macro because the only way to 
    // acquire one is via a method call on other objects. Only define the macro if
    // the class should be directly instantiable via ActivateInstance.
    // ActivatableClass(Toaster)

    class AnimalFactory :
        public Microsoft::WRL::ActivationFactory<IAnimalFactory, IAnimalFactory2, Microsoft::WRL::Implements<IStaticAnimal,IStaticAnimal2, CFastSigInterfaceStatic>>
    {
    private:
        boolean m_isLovable;

        Animals::IFish *m_Fish;
        Animals::ILikeToSwim *m_LikeToSwim;

        Animals::IDino *m_Dino;
        Animals::IExtinct *m_Extinct;

        Fabrikam::Kitchen::IToaster *m_Toaster;

        int* m_array;
        UINT32 m_arraySize;
        HSTRING* m_arrayHSTRING;
        UINT32 m_arraySizeHSTRING;

    public:
        AnimalFactory() 
        { 
            m_Fish = NULL; 
            m_LikeToSwim = NULL;
            m_Dino = NULL;
            m_Extinct = NULL;
            m_Toaster = nullptr;

            m_array = nullptr;
            m_arraySize = 0;

            m_arrayHSTRING = nullptr;
            m_arraySizeHSTRING = 0;
        }

        ~AnimalFactory() 
        { 
            ReleasePtr(m_Fish);
            ReleasePtr(m_LikeToSwim);
            ReleasePtr(m_Dino);
            ReleasePtr(m_Extinct);
            ReleasePtr(m_Toaster);

            if (m_array != nullptr)
            {
                // Clear the array
                CoTaskMemFree(m_array);
                m_array = nullptr;
                m_arraySize = 0;
            }

            if (m_arrayHSTRING != nullptr)
            {
                for (int iIndex = 0; iIndex < (int)m_arraySizeHSTRING; iIndex++)
                {
                    WindowsDeleteString(m_arrayHSTRING[iIndex]);
                }

                // Clear the array
                CoTaskMemFree(m_arrayHSTRING);
                m_arrayHSTRING = nullptr;
                m_arraySizeHSTRING = 0;
            }
        }

        IFACEMETHOD(AnimalBornWithWeight)(__in IAnimal* mother, __in int weight, __deref_out IAnimal** ppAnimal);

        IFACEMETHOD(AnimalBornWithStats)(__in IAnimal* mother, __in int weight, __in int legs1, __in int legs2, __in int legs3, __deref_out IAnimal** ppAnimal);

        IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

        IFACEMETHOD(CreateWithLegs)(__in int numberOfLegs, __deref_out IAnimal ** ppAnimal);

        IFACEMETHOD(CreateWithMoreLegs)(__in int legs1, __in int legs2, __in int legs3, __deref_out IAnimal** ppAnimal);

        IFACEMETHOD(CreateWithEvenMoreLegs)(__in int legs1, __in int legs2, __in int legs3, __in int legs4, __in int legs5, __in int legs6, __deref_out IAnimal** ppAnimal);

        IFACEMETHOD(CreateWithMostLegs)(__in int legs1, __in int legs2, __in int legs3, __in int legs4, __in int legs5, __in int legs6, __in int legs7, __deref_out IAnimal** ppAnimal);

        // IStaticAnimal
        IFACEMETHOD(get_IsLovable)(__out boolean* value);
        IFACEMETHOD(put_IsLovable)(__in boolean value);
        IFACEMETHOD(GetAnswer)(__out int* answer);
        IFACEMETHOD(TakeANap)(__in int numberOfMinutes, __out boolean* isDreaming);
        IFACEMETHOD(DinoMarshalAs)(__deref_out IDino** dino);
        IFACEMETHOD(DinoDefault)(__deref_out IDino** dino);
        IFACEMETHOD(DinoDefaultVector)(__deref_out Windows::Foundation::Collections::IVector<Dino*>** dino);

        IFACEMETHOD(SendBackSameIDino)(__in Animals::IDino *inValue, __out Animals::IDino **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }
        IFACEMETHOD(SendBackSameDino)(__in Animals::IDino *inValue, __out Animals::IDino **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }
        IFACEMETHOD(SendBackSameExtinct)(__in Animals::IExtinct *inValue, __out Animals::IExtinct **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }

        IFACEMETHOD(CallDelegateWithExtinct)(__in IDelegateWithExtinct *inValue, __out Animals::IExtinct **outValue) override
        {
            CallDelegateWithInterface(m_Extinct);
        }

        IFACEMETHOD(get_MyDino)(__out Animals::IDino **value) override
        {
            GetInterface(m_Dino);
        }
        IFACEMETHOD(put_MyDino)(__in Animals::IDino *value) override
        {
            PutInterface(m_Dino);
        }
        IFACEMETHOD(get_MyExtinct)(__out Animals::IExtinct **value) override
        {
            GetInterface(m_Extinct);
        }
        IFACEMETHOD(put_MyExtinct)(__in Animals::IExtinct *value) override
        {
            PutInterface(m_Extinct);
        }

        // IStaticAnimal2
        IFACEMETHOD(GetCLSID)(__out GUID* clsid);
        IFACEMETHOD(MultiplyNumbers)(__in int value1, __in int value2, __out int* result);

        IFACEMETHOD(SendBackSameIFish)(__in Animals::IFish *inValue, __out Animals::IFish **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }
        IFACEMETHOD(SendBackSameFish)(__in Animals::IFish *inValue, __out Animals::IFish **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }
        IFACEMETHOD(SendBackSameLikeToSwim)(__in Animals::ILikeToSwim *inValue, __out Animals::ILikeToSwim **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }

        IFACEMETHOD(CallDelegateWithIFish)(__in IDelegateWithIFish *inValue, __out Animals::IFish **outValue) override
        {
            CallDelegateWithInterface(m_Fish);
        }
        IFACEMETHOD(CallDelegateWithFish)(__in IDelegateWithFish *inValue, __out Animals::IFish **outValue) override
        {
            CallDelegateWithInterface(m_Fish);
        }
        IFACEMETHOD(CallDelegateWithLikeToSwim)(__in IDelegateWithLikeToSwim *inValue, __out Animals::ILikeToSwim **outValue) override
        {
            CallDelegateWithInterface(m_LikeToSwim);
        }

        IFACEMETHOD(get_AnimalObjectSize)(int * size) override
        {
            *size = sizeof(AnimalServer);
            return S_OK;
        }

        IFACEMETHOD(get_MyFish)(__out Animals::IFish **value) override
        {
            GetInterface(m_Fish);
        }
        IFACEMETHOD(put_MyFish)(__in Animals::IFish *value) override
        {
            PutInterface(m_Fish);
        }
        IFACEMETHOD(get_MyIFish)(__out Animals::IFish **value) override
        {
            GetInterface(m_Fish);
        }
        IFACEMETHOD(put_MyIFish)(__in Animals::IFish *value) override
        {
            PutInterface(m_Fish);
        }
        IFACEMETHOD(CallMyFishMethod)(__in int expected, __out boolean * result) override
        {
            int actual;
            m_Fish->GetNumFins(&actual);
            *result = (actual == expected);
            return S_OK;
        }

        IFACEMETHOD(get_MyLikeToSwim)(__out Animals::ILikeToSwim **value) override
        {
            GetInterface(m_LikeToSwim);
        }
        IFACEMETHOD(put_MyLikeToSwim)(__in Animals::ILikeToSwim *value) override
        {
            PutInterface(m_LikeToSwim);
        }

        IFACEMETHOD(GetRefCount)(__in IInspectable *inValue, __out t_UInt64 *refCount) override;
        IFACEMETHOD(get_MyFishRefCount)(__out t_UInt64 *refCount) override;

        IFACEMETHOD(get_MyToaster)(__out Fabrikam::Kitchen::IToaster **value) override
        {
            GetInterface(m_Toaster);
        }
        IFACEMETHOD(put_MyToaster)(__in Fabrikam::Kitchen::IToaster *value) override
        {
            PutInterface(m_Toaster);
        }
        IFACEMETHOD(get_MyToasterRefCount)(__out t_UInt64 *refCount) override;

        IFACEMETHOD(SendBackSameInspectableVector)(__in Windows::Foundation::Collections::IVector<IInspectable *> *inValue, __out Windows::Foundation::Collections::IVector<IInspectable *> **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }

        IFACEMETHOD(MethodWithInParam_BigStruct)(__in CollectionChangedEventArgs inParam, __out HSTRING *objectId, __out CollectionChangeType *eType, __out UINT32 *index, __out UINT32 *previousIndex) override;
        IFACEMETHOD(MethodWithOutParam_BigStruct)(__in HSTRING objectId, __in CollectionChangeType eType, __in UINT32 index, __in UINT32 previousIndex, __out CollectionChangedEventArgs *outParam) override;
        IFACEMETHOD(CallDelegateWithInParam_BigStruct)(__in IDelegateWithInParam_BigStruct* delegateStruct, __in HSTRING objectId, __in CollectionChangeType eType, __in UINT32 index, __in UINT32 previousIndex) override;
        IFACEMETHOD(CallDelegateWithOutParam_BigStruct)(    
            __in IDelegateWithOutParam_BigStruct* delegateStruct, 
            __out HSTRING *objectId, 
            __out CollectionChangeType *eType, 
            __out UINT32 *index, 
            __out UINT32 *previousIndex, 
            __out HSTRING *objectIdFromStruct, 
            __out CollectionChangeType *eTypeFromStruct, 
            __out UINT32 *indexFromStruct, 
            __out UINT32 *previousIndexFromStruct) override;

        IFACEMETHOD(MarshalInAndOutPackedByte)(__in PackedByte inParam, __out PackedByte *outParam) override;
        IFACEMETHOD(GetPackedByteArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) PackedByte** value) override;
        IFACEMETHOD(CallDelegateWithInOutPackedByte)(__in PackedByte inParam, __out PackedByte *outParam, __in IDelegatePackedByte *delegateIn) override;

        IFACEMETHOD(MarshalInAndOutPackedBoolean)(__in PackedBoolean4 inParam, __out PackedBoolean4 *outParam) override;
        IFACEMETHOD(GetPackedBooleanArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) PackedBoolean4** value) override;
        IFACEMETHOD(CallDelegateWithInOutPackedBoolean)(__in PackedBoolean4 inParam, __out PackedBoolean4 *outParam, __in IDelegatePackedBoolean *delegateIn) override;

        IFACEMETHOD(MarshalInAndOutOddSizedStruct)(__in OddSizedStruct inParam, __out OddSizedStruct *outParam) override;
        IFACEMETHOD(GetOddSizedStructArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) OddSizedStruct** value) override;
        IFACEMETHOD(CallDelegateWithInOutOddSizedStruct)(__in OddSizedStruct inParam, __out OddSizedStruct *outParam, __in IDelegateOddSizedStruct *delegateIn) override;

        IFACEMETHOD(MarshalInAndOutSmallComplexStruct)(__in SmallComplexStruct inParam, __out SmallComplexStruct *outParam) override;
        IFACEMETHOD(GetSmallComplexStructArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) SmallComplexStruct** value) override;
        IFACEMETHOD(CallDelegateWithInOutSmallComplexStruct)(__in SmallComplexStruct inParam, __out SmallComplexStruct *outParam, __in IDelegateSmallComplexStruct *delegateIn) override;

        IFACEMETHOD(MarshalInAndOutBigComplexStruct)(__in BigComplexStruct inParam, __out BigComplexStruct *outParam) override;
        IFACEMETHOD(GetBigComplexStructArray)(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BigComplexStruct** value) override;
        IFACEMETHOD(CallDelegateWithInOutBigComplexStruct)(__in BigComplexStruct inParam, __out BigComplexStruct *outParam, __in IDelegateBigComplexStruct *delegateIn) override;

        IFACEMETHOD(CallDelegateWithInFloat)(__in IDelegateWithInParam_Float *inDelegate, __in float inValue) override;
        IFACEMETHOD(CallDelegateWithOutFloat)(__in IDelegateWithOutParam_Float *inDelegate, __out float *outValue) override;
        IFACEMETHOD(CallDelegateWithInOutFloat)(__in IDelegateWithInOut_Float *inDelegate, __in int inValue1, __out float *outValue1, __in float inValue2, __in int inValue3, __in int inValue4, __in float inValue5, __out float *outValue2) override;

        IFACEMETHOD(SendBackSamePropertySet)(__in Windows::Foundation::Collections::IPropertySet *inValue, __out Windows::Foundation::Collections::IPropertySet **outValue) override
        {
            SendBackSameInterface(inValue, outValue);
        }

        IFACEMETHOD(GetStringIntegerMap)(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outValue);
        IFACEMETHOD(GetObservableStringIntegerMap)(__out Windows::Foundation::Collections::IObservableMap<HSTRING, int> **outValue);
        IFACEMETHOD(GetDoubleObservableMap)(__out IDoubleIObservableMap **outValue);
        IFACEMETHOD(GetStringHiddenTypeMap)(__out Windows::Foundation::Collections::IMap<HSTRING, IHiddenInterface *> **outValue, __out boolean *wasMethodCalled);

        IFACEMETHOD(PassUInt8Array)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) t_UInt8 *value,
            __out Windows::Foundation::Collections::IVector<t_UInt8> ** passedValuesVector) override;

        IFACEMETHOD(FillUInt8Array)( 
            __in UINT32 length,
            __RPC__out_ecount_full(length) t_UInt8 *value,
            __in Windows::Foundation::Collections::IVector<t_UInt8> * fillFromVector) override;

        IFACEMETHOD(GetStaticAnimalAsInspectable)(__out IInspectable **staticInspectableAnimal) override;
        IFACEMETHOD(GetStaticAnimalAsStaticInterface)(__out IStaticAnimal2 **staticAnimal) override;

        IFACEMETHOD(TestDefaultDino)(__in IDino *inValue, __out boolean *isSame) override;
        IFACEMETHOD(TestDefaultFish)(__in IFish *inValue, __out boolean *isSame) override;
        IFACEMETHOD(TestDefaultAnimal)(__in IAnimal *inValue, __out boolean *isSame) override;
        IFACEMETHOD(TestDefaultMultipleIVector)(__in __in Windows::Foundation::Collections::IVector<int> *inValue, __out boolean *isSame) override;

        IFACEMETHOD(put_MyStaticArrayProp)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) int *value) override;

        IFACEMETHOD(get_MyStaticArrayProp)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) int **value) override;

        IFACEMETHOD(put_MyStaticArrayPropHSTRING)( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) HSTRING *value) override;

        IFACEMETHOD(get_MyStaticArrayPropHSTRING)( 
            __RPC__out UINT32 *length,
            __RPC__deref_out_ecount_full_opt(*length) HSTRING **value) override;

        IFACEMETHOD(StaticFastPath)()
        {
            return S_OK;
        }

        IFACEMETHOD(StaticFastPathIn)(int _in)
        {
            UNREFERENCED_PARAMETER(_in);

            return S_OK;
        }

        IFACEMETHOD(StaticFastPathOut)(int* _out)
        {
            *_out = 16;

            return S_OK;
        }

        IFACEMETHOD(StaticFastPathInOut)(int _in, int* _out)
        {
            *_out = _in;

            return S_OK;
        }

        IFACEMETHOD(StaticFastPathInIn)(int _in1, int _in2)
        {
            UNREFERENCED_PARAMETER(_in1);
            UNREFERENCED_PARAMETER(_in2);

            return S_OK;
        }

        IFACEMETHOD(StaticSlowPath)(int _in1, int _in2, int _in3, int _in4, int _in5, int _in6)
        {
            UNREFERENCED_PARAMETER(_in1);
            UNREFERENCED_PARAMETER(_in2);
            UNREFERENCED_PARAMETER(_in3);
            UNREFERENCED_PARAMETER(_in4);
            UNREFERENCED_PARAMETER(_in5);
            UNREFERENCED_PARAMETER(_in6);

            return S_OK;
        }
    };

    class AnimalDelegateWithOutParam_HSTRING :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
        Animals::IDelegateWithOutParam_HSTRING>
    {
    public:
        AnimalDelegateWithOutParam_HSTRING() 
        {
        }

        ~AnimalDelegateWithOutParam_HSTRING()
        {
        }

        IFACEMETHOD(Invoke)(IAnimal* sender, __out HSTRING *outParam);
    };

    class AnimalIIterable_int :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IIterable<int>>
    {
    private:
        Windows::Foundation::Collections::IIterable<int> *m_pIterable;
        AnimalServer *m_pAnimal;

    public:
        AnimalIIterable_int(Windows::Foundation::Collections::IIterable<int> *inIterable, AnimalServer *pAnimal) 
        {
            m_pIterable = inIterable;
            m_pIterable->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIIterable_int()
        {
            m_pIterable->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<int> **first);
    };

    class AnimalIIterable_HSTRING :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IIterable<HSTRING> *m_pIterable;
        AnimalServer *m_pAnimal;

    public:
        AnimalIIterable_HSTRING(Windows::Foundation::Collections::IIterable<HSTRING> *inIterable, AnimalServer *pAnimal) 
        {
            m_pIterable = inIterable;
            m_pIterable->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIIterable_HSTRING()
        {
            m_pIterable->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);
    };

    class AnimalIIterator_int :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IIterator<int>>
    {
    private:
        Windows::Foundation::Collections::IIterator<int> *m_pIterator;

    public:
        AnimalIIterator_int(Windows::Foundation::Collections::IIterator<int> *inIterator) 
        {
            m_pIterator = inIterator;
            m_pIterator->AddRef();
        }

        ~AnimalIIterator_int()
        {
            m_pIterator->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterator members
        //
        IFACEMETHOD(get_Current)(__out int * current);
        IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent);
        IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent);
        IFACEMETHOD(GetMany)(__in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual) override;
    };

    class AnimalIIterator_HSTRING :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IIterator<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IIterator<HSTRING> *m_pIterator;

    public:
        AnimalIIterator_HSTRING(Windows::Foundation::Collections::IIterator<HSTRING> *inIterator) 
        {
            m_pIterator = inIterator;
            m_pIterator->AddRef();
        }

        ~AnimalIIterator_HSTRING()
        {
            m_pIterator->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterator members
        //
        IFACEMETHOD(get_Current)(__out HSTRING * current);
        IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent);
        IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent);
        IFACEMETHOD(GetMany)(__in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual) override;
    };

    class AnimalIVectorView_int :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IVectorView<int>, Windows::Foundation::Collections::IIterable<int>>
    {
    private:
        Windows::Foundation::Collections::IVectorView<int> *m_pVectorView;
        AnimalServer *m_pAnimal;

    public:
        AnimalIVectorView_int(Windows::Foundation::Collections::IVectorView<int> *inVectorView, AnimalServer *pAnimal) 
        {
            m_pVectorView = inVectorView;
            m_pVectorView->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIVectorView_int()
        {
            m_pVectorView->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<int> **first);

        //
        // IVector View members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out int *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt int value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual) override;
    };

    class AnimalIVectorView_HSTRING :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IVectorView<HSTRING>, Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IVectorView<HSTRING> *m_pVectorView;
        AnimalServer *m_pAnimal;

    public:
        AnimalIVectorView_HSTRING(Windows::Foundation::Collections::IVectorView<HSTRING> *inVectorView, AnimalServer *pAnimal) 
        {
            m_pVectorView = inVectorView;
            m_pVectorView->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIVectorView_HSTRING()
        {
            m_pVectorView->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

        //
        // IIterable members
        //
        IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<HSTRING> **first);

        //
        // IVector View members
        //
        IFACEMETHOD(GetAt)(__in unsigned index, __out HSTRING *item);
        IFACEMETHOD(get_Size)(__out unsigned *size);
        IFACEMETHOD(IndexOf)(__in_opt HSTRING value, __out unsigned *index, __out boolean *found);    
        IFACEMETHOD(GetMany)(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual) override;
    };

    class AnimalIVector_int :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IVector<int>, Windows::Foundation::Collections::IIterable<int>>
    {
    private:
        Windows::Foundation::Collections::IVector<int> *m_pVector;
        AnimalServer *m_pAnimal;

    public:
        AnimalIVector_int(Windows::Foundation::Collections::IVector<int> *inVector, AnimalServer *pAnimal) 
        {
            m_pVector = inVector;
            m_pVector->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIVector_int()
        {
            m_pVector->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

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

    class AnimalIVector_HSTRING :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IVector<HSTRING>, Windows::Foundation::Collections::IIterable<HSTRING>>
    {
    private:
        Windows::Foundation::Collections::IVector<HSTRING> *m_pVector;
        AnimalServer *m_pAnimal;

    public:
        AnimalIVector_HSTRING(Windows::Foundation::Collections::IVector<HSTRING> *inVector, AnimalServer *pAnimal) 
        {
            m_pVector = inVector;
            m_pVector->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalIVector_HSTRING()
        {
            m_pVector->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

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

    class AnimalReadOnlyVector_int :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
        Windows::Foundation::Collections::IVector<int>, Windows::Foundation::Collections::IIterable<int>>
    {
    private:
        Windows::Foundation::Collections::IVector<int> *m_pVector;
        AnimalServer *m_pAnimal;

    public:
        AnimalReadOnlyVector_int(Windows::Foundation::Collections::IVector<int> *inVector, AnimalServer *pAnimal) 
        {
            m_pVector = inVector;
            m_pVector->AddRef();

            m_pAnimal = pAnimal;
            m_pAnimal->AddRef();
        }

        ~AnimalReadOnlyVector_int()
        {
            m_pVector->Release();
            m_pAnimal->Release();
        }

        // IInspectable Methods
        IFACEMETHOD(GetIids)(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        IFACEMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className);
        IFACEMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel);

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

    class PomapoodleServer :
        public Microsoft::WRL::RuntimeClass<Animals::IPuppy>
    {
        InspectableClass(L"Animals.Pomapoodle", BaseTrust);

    public:
        // IPuppy
        IFACEMETHOD(WagTail)(__in int numberOfHeadPats, __out int* numberOfWags){
            *numberOfWags = (2*numberOfHeadPats - 1);
            return S_OK;
        };
    };

    class PomapoodleFactory :
        public Microsoft::WRL::ActivationFactory<IStaticPuppy>
    {
    public:
        // IActivationFactory
        IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);

        // IStaticPuppy
        IFACEMETHOD(EatCookies)(__in int numberOfCookies, __out int* cookiesEaten);

        IFACEMETHOD(add_CookiesEatenEvent)( 
            __in Animals::ICookiesEatenHandler *clickHandler,
            __out EventRegistrationToken *pCookie) override;

        IFACEMETHOD(remove_CookiesEatenEvent)( 
            __in EventRegistrationToken iCookie) override;
    private:
        Microsoft::WRL::EventSource<ICookiesEatenHandler> _evtCookiesEaten;
    };

    class SimplestClassServer :
        public Microsoft::WRL::RuntimeClass<Animals::IEmpty>
    {
        InspectableClass(L"Animals.SimplestClass", BaseTrust);
    };


    class EmptyClassServer :
        public Microsoft::WRL::RuntimeClass<Animals::IEmpty>
    {
        InspectableClass(L"Animals.EmptyClass", BaseTrust);
    };

    class EmptyFactory :
        public Microsoft::WRL::ActivationFactory<IEmptyFactory>
    {
    public:
        // IActivationFactory
        IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable) {
            *ppInspectable = nullptr;
            return E_NOTIMPL;
        };
    };



}
