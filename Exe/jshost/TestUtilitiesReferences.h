/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
MIDL_INTERFACE("8d720cdf-3934-5d3f-9a55-40e8063b086a")
__FIVectorView_1_int : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetAt( 
        /* [in] */ unsigned int index,
        /* [retval][out] */ __RPC__out int *item) = 0;

    virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Size( 
        /* [retval][out] */ __RPC__out unsigned int *size) = 0;

    virtual HRESULT STDMETHODCALLTYPE IndexOf( 
        /* [in] */ int item,
        /* [out] */ __RPC__out unsigned int *index,
        /* [retval][out] */ __RPC__out boolean *found) = 0;

};

MIDL_INTERFACE("b939af5b-b45d-5489-9149-61442c1905fe")
__FIVector_1_int : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetAt( 
        /* [in] */ unsigned int index,
        /* [retval][out] */ __RPC__out int *item) = 0;

    virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Size( 
        /* [retval][out] */ __RPC__out unsigned int *size) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetView( 
        /* [retval][out] */ __RPC__deref_out_opt __FIVectorView_1_int **view) = 0;

    virtual HRESULT STDMETHODCALLTYPE IndexOf( 
        /* [in] */ int item,
        /* [out] */ __RPC__out unsigned int *index,
        /* [retval][out] */ __RPC__out boolean *found) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetAt( 
        /* [in] */ unsigned int index,
        /* [in] */ int item) = 0;

    virtual HRESULT STDMETHODCALLTYPE InsertAt( 
        /* [in] */ unsigned int index,
        /* [in] */ int item) = 0;

    virtual HRESULT STDMETHODCALLTYPE RemoveAt( 
        /* [in] */ unsigned int index) = 0;

    virtual HRESULT STDMETHODCALLTYPE Append( 
        /* [in] */ int item) = 0;

    virtual HRESULT STDMETHODCALLTYPE RemoveAtEnd( void) = 0;

    virtual HRESULT STDMETHODCALLTYPE Clear( void) = 0;

};

namespace Animals {
    MIDL_INTERFACE("57D5CC77-A6A3-46E3-B754-CDD6E2D6EF0E")
IGetVector : public IInspectable
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetVector( 
            /* [out][retval] */ __RPC__deref_out_opt __FIVector_1_int **uniqueNumbersVector) = 0;

    };

    extern const __declspec(selectany) IID & IID_IGetVector = __uuidof(IGetVector);
}  /* end namespace */

namespace DevTests {
    namespace SimpleTestNamespace {

        MIDL_INTERFACE("BDD1D741-CC42-4CA6-8A19-76F7C7EC5B40")
        IEmptyInterface : public IInspectable
        {
        public:
        };

        extern const __declspec(selectany) IID & IID_IEmptyInterface = __uuidof(IEmptyInterface);

        MIDL_INTERFACE("93BDBF34-FFAA-4B40-80AB-9F6DF5439D99")
        ISimpleInterface : public IInspectable
        {
        public:
            virtual HRESULT STDMETHODCALLTYPE SetMessage( 
                /* [in] */ __RPC__in HSTRING message) = 0;

            virtual HRESULT STDMETHODCALLTYPE GetMessage( 
                /* [out][retval] */ __RPC__deref_out_opt HSTRING *message) = 0;

            virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
                /* [out][retval] */ __RPC__out int *value) = 0;

            virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Value( 
                /* [in] */ int value) = 0;

        };

        extern const __declspec(selectany) IID & IID_ISimpleInterface = __uuidof(ISimpleInterface);


    }  /* end namespace */
}  /* end namespace */
