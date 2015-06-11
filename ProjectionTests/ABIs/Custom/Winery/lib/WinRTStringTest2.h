

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0582 */
/* Compiler settings for WinRTStringTest.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0582 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __WinRTStringTest_h__
#define __WinRTStringTest_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__StringIn __x_Microsoft_CClr_CTest_CIDelegate__StringIn;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__StringInOut __x_Microsoft_CClr_CTest_CIDelegate__StringInOut;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__StringOut __x_Microsoft_CClr_CTest_CIDelegate__StringOut;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__StringReturn __x_Microsoft_CClr_CTest_CIDelegate__StringReturn;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn;
#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringTest_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringTest_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIHStringTest __x_Microsoft_CClr_CTest_CIHStringTest;
#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringTest_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIHStringDelegateTest __x_Microsoft_CClr_CTest_CIHStringDelegateTest;
#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIHStringStructTest __x_Microsoft_CClr_CTest_CIHStringStructTest;
#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIProfile_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIProfile_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIProfile __x_Microsoft_CClr_CTest_CIProfile;
#endif 	/* ____x_Microsoft_CClr_CTest_CIProfile_FWD_DEFINED__ */


/* header files for imported files */
#include "Inspectable.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_WinRTStringTest_0000_0000 */
/* [local] */ 

#ifndef ACTIVATABLECLASSID_Microsoft_Clr_Test_HStringTest_DEFINED
#define ACTIVATABLECLASSID_Microsoft_Clr_Test_HStringTest_DEFINED
extern const __declspec(selectany) PCWSTR RuntimeClass_Microsoft_Clr_Test_HStringTest = L"Microsoft.Clr.Test.HStringTest";
#endif
#if !defined(__cplusplus)
typedef struct __x_Microsoft_CClr_CTest_CHStringStruct
    {
    int stringId;
    HSTRING hstring;
    } 	__x_Microsoft_CClr_CTest_CHStringStruct;

#endif


extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0000_v0_0_s_ifspec;

#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__StringIn */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__StringIn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C2969913-53E8-478B-B765-AEC39B0C8872")
    __x_Microsoft_CClr_CTest_CIDelegate__StringIn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__StringInVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringIn * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringIn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringIn * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringIn * This,
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__StringInVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__StringIn
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__StringInVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__StringIn_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringIn_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringIn_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__StringIn_Invoke(This,stringId,hstring)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,hstring) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringIn_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__StringInOut */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__StringInOut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("822092BD-26C2-4CA3-B8B3-749D06EE84A7")
    __x_Microsoft_CClr_CTest_CIDelegate__StringInOut : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__StringInOutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringInOut * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringInOut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringInOut * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringInOut * This,
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__StringInOutVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__StringInOut
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__StringInOutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__StringInOut_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringInOut_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringInOut_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__StringInOut_Invoke(This,pStringId,pHstring)	\
    ( (This)->lpVtbl -> Invoke(This,pStringId,pHstring) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringInOut_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__StringOut */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__StringOut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2C7145FC-3AAA-4ABA-A959-7FDD5F7E4CB9")
    __x_Microsoft_CClr_CTest_CIDelegate__StringOut : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__StringOutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringOut * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringOut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringOut * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringOut * This,
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__StringOutVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__StringOut
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__StringOutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__StringOut_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringOut_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringOut_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__StringOut_Invoke(This,stringId,pHstring)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,pHstring) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringOut_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__StringReturn */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__StringReturn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("903F6B0B-4097-4393-88C6-81E5AFC8A3AA")
    __x_Microsoft_CClr_CTest_CIDelegate__StringReturn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHstring) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__StringReturnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringReturn * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringReturn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringReturn * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__StringReturn * This,
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHstring);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__StringReturnVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__StringReturn
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__StringReturnVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__StringReturn_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringReturn_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__StringReturn_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__StringReturn_Invoke(This,stringId,pHstring)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,pHstring) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__StringReturn_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2EF307B4-304F-40D4-96D0-5240FBFBEF1E")
    __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn * This,
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_Invoke(This,stringId,hstring,pass)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,hstring,pass) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("17227F57-3343-4621-98A2-3B7162978191")
    __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut * This,
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOutVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_Invoke(This,pStringId,pHstring,pass)	\
    ( (This)->lpVtbl -> Invoke(This,pStringId,pHstring,pass) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BD4499A4-E95C-4D81-9191-2667EC7C205C")
    __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut * This,
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOutVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_Invoke(This,stringId,pHstring,pass)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,pHstring,pass) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn */
/* [object][uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADC38F2A-EFBC-43AA-AAA3-222EE3E0981C")
    __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHstring) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn * This,
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHstring);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturnVtbl;

    interface __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturnVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_Invoke(This,stringId,pHstring)	\
    ( (This)->lpVtbl -> Invoke(This,stringId,pHstring) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringTest_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringTest_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIHStringTest */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIHStringTest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5f72096e-db7f-41c4-b7d2-707c6757401f")
    __x_Microsoft_CClr_CTest_CIHStringTest : public IInspectable
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CheckIn( 
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckInOut( 
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckOut( 
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckReturn( 
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHString) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIHStringTestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            /* [out] */ __RPC__out ULONG *iidCount,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            /* [out] */ __RPC__deref_out_opt HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            /* [out] */ __RPC__out TrustLevel *trustLevel);
        
        HRESULT ( STDMETHODCALLTYPE *CheckIn )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            int stringId,
            /* [in] */ __RPC__in HSTRING hstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInOut )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in][out] */ __RPC__deref_inout_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckOut )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            int stringId,
            /* [out] */ __RPC__deref_out_opt HSTRING *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckReturn )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringTest * This,
            int stringId,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *pHString);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIHStringTestVtbl;

    interface __x_Microsoft_CClr_CTest_CIHStringTest
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIHStringTestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIHStringTest_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIHStringTest_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_Microsoft_CClr_CTest_CIHStringTest_CheckIn(This,stringId,hstring,pass)	\
    ( (This)->lpVtbl -> CheckIn(This,stringId,hstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_CheckInOut(This,pStringId,pHstring,pass)	\
    ( (This)->lpVtbl -> CheckInOut(This,pStringId,pHstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_CheckOut(This,stringId,pHstring,pass)	\
    ( (This)->lpVtbl -> CheckOut(This,stringId,pHstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringTest_CheckReturn(This,stringId,pHString)	\
    ( (This)->lpVtbl -> CheckReturn(This,stringId,pHString) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringTest_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIHStringDelegateTest */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIHStringDelegateTest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("39B51105-44EF-4D0B-BBB0-CB49F778E1EA")
    __x_Microsoft_CClr_CTest_CIHStringDelegateTest : public IInspectable
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CheckInDelegateParam( 
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringIn *StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckInOutDelegateParam( 
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringInOut *StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckOutDelegateParam( 
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringOut *StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckReturnDelegateParam( 
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringReturn *StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckCallBackDelegateInParam( 
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn **StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckCallBackDelegateInOutParam( 
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut **StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckCallBackDelegateOutParam( 
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut **StringChecker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckCallBackDelegateReturnParam( 
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn **StringChecker) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIHStringDelegateTestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out] */ __RPC__out ULONG *iidCount,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out] */ __RPC__deref_out_opt HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out] */ __RPC__out TrustLevel *trustLevel);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInDelegateParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringIn *StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInOutDelegateParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [in][out] */ __RPC__inout int *pStringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringInOut *StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckOutDelegateParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringOut *StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckReturnDelegateParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            int stringId,
            /* [in] */ __RPC__in_opt __x_Microsoft_CClr_CTest_CIDelegate__StringReturn *StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckCallBackDelegateInParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringIn **StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckCallBackDelegateInOutParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringInOut **StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckCallBackDelegateOutParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringOut **StringChecker);
        
        HRESULT ( STDMETHODCALLTYPE *CheckCallBackDelegateReturnParam )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringDelegateTest * This,
            /* [out][retval] */ __RPC__deref_out_opt __x_Microsoft_CClr_CTest_CIDelegate__CallBackStringReturn **StringChecker);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIHStringDelegateTestVtbl;

    interface __x_Microsoft_CClr_CTest_CIHStringDelegateTest
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIHStringDelegateTestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckInDelegateParam(This,stringId,StringChecker)	\
    ( (This)->lpVtbl -> CheckInDelegateParam(This,stringId,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckInOutDelegateParam(This,pStringId,StringChecker)	\
    ( (This)->lpVtbl -> CheckInOutDelegateParam(This,pStringId,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckOutDelegateParam(This,stringId,StringChecker)	\
    ( (This)->lpVtbl -> CheckOutDelegateParam(This,stringId,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckReturnDelegateParam(This,stringId,StringChecker)	\
    ( (This)->lpVtbl -> CheckReturnDelegateParam(This,stringId,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckCallBackDelegateInParam(This,StringChecker)	\
    ( (This)->lpVtbl -> CheckCallBackDelegateInParam(This,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckCallBackDelegateInOutParam(This,StringChecker)	\
    ( (This)->lpVtbl -> CheckCallBackDelegateInOutParam(This,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckCallBackDelegateOutParam(This,StringChecker)	\
    ( (This)->lpVtbl -> CheckCallBackDelegateOutParam(This,StringChecker) ) 

#define __x_Microsoft_CClr_CTest_CIHStringDelegateTest_CheckCallBackDelegateReturnParam(This,StringChecker)	\
    ( (This)->lpVtbl -> CheckCallBackDelegateReturnParam(This,StringChecker) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringDelegateTest_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIHStringStructTest_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringStructTest_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIHStringStructTest */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIHStringStructTest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("88905C1F-33F5-4B30-AA9E-CE1427C5AEC2")
    __x_Microsoft_CClr_CTest_CIHStringStructTest : public IInspectable
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CheckInStructField( 
            /* [in] */ __x_Microsoft_CClr_CTest_CHStringStruct hstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckInOutStructField( 
            /* [in][out] */ __RPC__inout __x_Microsoft_CClr_CTest_CHStringStruct *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckOutStructField( 
            int stringId,
            /* [out] */ __RPC__out __x_Microsoft_CClr_CTest_CHStringStruct *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckReturnStructField( 
            int stringId,
            /* [out][retval] */ __RPC__out __x_Microsoft_CClr_CTest_CHStringStruct *pHString) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIHStringStructTestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [out] */ __RPC__out ULONG *iidCount,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [out] */ __RPC__deref_out_opt HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [out] */ __RPC__out TrustLevel *trustLevel);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInStructField )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [in] */ __x_Microsoft_CClr_CTest_CHStringStruct hstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInOutStructField )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            /* [in][out] */ __RPC__inout __x_Microsoft_CClr_CTest_CHStringStruct *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckOutStructField )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            int stringId,
            /* [out] */ __RPC__out __x_Microsoft_CClr_CTest_CHStringStruct *pHstring,
            /* [out][retval] */ __RPC__out boolean *pass);
        
        HRESULT ( STDMETHODCALLTYPE *CheckReturnStructField )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIHStringStructTest * This,
            int stringId,
            /* [out][retval] */ __RPC__out __x_Microsoft_CClr_CTest_CHStringStruct *pHString);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIHStringStructTestVtbl;

    interface __x_Microsoft_CClr_CTest_CIHStringStructTest
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIHStringStructTestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIHStringStructTest_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIHStringStructTest_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_Microsoft_CClr_CTest_CIHStringStructTest_CheckInStructField(This,hstring,pass)	\
    ( (This)->lpVtbl -> CheckInStructField(This,hstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_CheckInOutStructField(This,pHstring,pass)	\
    ( (This)->lpVtbl -> CheckInOutStructField(This,pHstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_CheckOutStructField(This,stringId,pHstring,pass)	\
    ( (This)->lpVtbl -> CheckOutStructField(This,stringId,pHstring,pass) ) 

#define __x_Microsoft_CClr_CTest_CIHStringStructTest_CheckReturnStructField(This,stringId,pHString)	\
    ( (This)->lpVtbl -> CheckReturnStructField(This,stringId,pHString) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringStructTest_INTERFACE_DEFINED__ */


#ifndef ____x_Microsoft_CClr_CTest_CIProfile_INTERFACE_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIProfile_INTERFACE_DEFINED__

/* interface __x_Microsoft_CClr_CTest_CIProfile */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CClr_CTest_CIProfile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C69A2C50-9426-4B85-B0B8-3A163DA59DB4")
    __x_Microsoft_CClr_CTest_CIProfile : public IInspectable
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ __RPC__in HSTRING value) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CClr_CTest_CIProfileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [out] */ __RPC__out ULONG *iidCount,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [out] */ __RPC__deref_out_opt HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [out] */ __RPC__out TrustLevel *trustLevel);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [out][retval] */ __RPC__deref_out_opt HSTRING *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            __RPC__in __x_Microsoft_CClr_CTest_CIProfile * This,
            /* [in] */ __RPC__in HSTRING value);
        
        END_INTERFACE
    } __x_Microsoft_CClr_CTest_CIProfileVtbl;

    interface __x_Microsoft_CClr_CTest_CIProfile
    {
        CONST_VTBL struct __x_Microsoft_CClr_CTest_CIProfileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CClr_CTest_CIProfile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CClr_CTest_CIProfile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CClr_CTest_CIProfile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CClr_CTest_CIProfile_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_Microsoft_CClr_CTest_CIProfile_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_Microsoft_CClr_CTest_CIProfile_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_Microsoft_CClr_CTest_CIProfile_get_Name(This,value)	\
    ( (This)->lpVtbl -> get_Name(This,value) ) 

#define __x_Microsoft_CClr_CTest_CIProfile_put_Name(This,value)	\
    ( (This)->lpVtbl -> put_Name(This,value) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CClr_CTest_CIProfile_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_WinRTStringTest_0000_0012 */
/* [local] */ 

#ifndef ACTIVATABLECLASSID_Microsoft_Clr_Test_HStringTest_DEFINED
#define ACTIVATABLECLASSID_Microsoft_Clr_Test_HStringTest_DEFINED
extern const __declspec(selectany) PCWSTR RuntimeClass_Microsoft_Clr_Test_HStringTest = L"Microsoft.Clr.Test.HStringTest";
#endif


extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0012_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HSTRING_UserSize(     __RPC__in unsigned long *, unsigned long            , __RPC__in HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserMarshal(  __RPC__in unsigned long *, __RPC__inout_xcount(0) unsigned char *, __RPC__in HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserUnmarshal(__RPC__in unsigned long *, __RPC__in_xcount(0) unsigned char *, __RPC__out HSTRING * ); 
void                      __RPC_USER  HSTRING_UserFree(     __RPC__in unsigned long *, __RPC__in HSTRING * ); 

unsigned long             __RPC_USER  HSTRING_UserSize64(     __RPC__in unsigned long *, unsigned long            , __RPC__in HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserMarshal64(  __RPC__in unsigned long *, __RPC__inout_xcount(0) unsigned char *, __RPC__in HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserUnmarshal64(__RPC__in unsigned long *, __RPC__in_xcount(0) unsigned char *, __RPC__out HSTRING * ); 
void                      __RPC_USER  HSTRING_UserFree64(     __RPC__in unsigned long *, __RPC__in HSTRING * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


