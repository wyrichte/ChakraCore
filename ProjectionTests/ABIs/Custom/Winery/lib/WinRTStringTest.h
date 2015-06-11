

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

#ifndef ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__
#define ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__
typedef interface __x_Microsoft_CClr_CTest_CIHStringStructTest __x_Microsoft_CClr_CTest_CIHStringStructTest;
#endif 	/* ____x_Microsoft_CClr_CTest_CIHStringStructTest_FWD_DEFINED__ */


/* header files for imported files */
#include "Inspectable.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_WinRTStringTest_0000_0000 */
/* [local] */ 

#if !defined(__cplusplus)
typedef struct __x_Microsoft_CClr_CTest_CHStringStruct
    {
    int stringId;
    HSTRING hstring;
    } 	__x_Microsoft_CClr_CTest_CHStringStruct;

#endif


extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WinRTStringTest_0000_0000_v0_0_s_ifspec;

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


