/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/


/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* Compiler settings for ad1ex.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
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
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __ad1ex_h__
#define __ad1ex_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDebugApplicationEx_FWD_DEFINED__
#define __IDebugApplicationEx_FWD_DEFINED__
typedef interface IDebugApplicationEx IDebugApplicationEx;
#endif 	/* __IDebugApplicationEx_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx_FWD_DEFINED__
#define __IRemoteDebugApplicationEx_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEx IRemoteDebugApplicationEx;
#endif 	/* __IRemoteDebugApplicationEx_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx7_FWD_DEFINED__
#define __IRemoteDebugApplicationEx7_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEx7 IRemoteDebugApplicationEx7;
#endif 	/* __IRemoteDebugApplicationEx7_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThreadEx_FWD_DEFINED__
#define __IRemoteDebugApplicationThreadEx_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThreadEx IRemoteDebugApplicationThreadEx;
#endif 	/* __IRemoteDebugApplicationThreadEx_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelperEx_FWD_DEFINED__
#define __IDebugDocumentHelperEx_FWD_DEFINED__
typedef interface IDebugDocumentHelperEx IDebugDocumentHelperEx;
#endif 	/* __IDebugDocumentHelperEx_FWD_DEFINED__ */


#ifndef __IDebugHelperEx_FWD_DEFINED__
#define __IDebugHelperEx_FWD_DEFINED__
typedef interface IDebugHelperEx IDebugHelperEx;
#endif 	/* __IDebugHelperEx_FWD_DEFINED__ */


#ifndef __IDebugSetValueCallback_FWD_DEFINED__
#define __IDebugSetValueCallback_FWD_DEFINED__
typedef interface IDebugSetValueCallback IDebugSetValueCallback;
#endif 	/* __IDebugSetValueCallback_FWD_DEFINED__ */


#ifndef __ISetNextStatement_FWD_DEFINED__
#define __ISetNextStatement_FWD_DEFINED__
typedef interface ISetNextStatement ISetNextStatement;
#endif 	/* __ISetNextStatement_FWD_DEFINED__ */


#ifndef __IDebugSessionProviderEx_FWD_DEFINED__
#define __IDebugSessionProviderEx_FWD_DEFINED__
typedef interface IDebugSessionProviderEx IDebugSessionProviderEx;
#endif 	/* __IDebugSessionProviderEx_FWD_DEFINED__ */

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_ad1ex_0000_0000 */
/* [local] */ 








DEFINE_GUID(IID_IDebugHelperExOld, 0xE0284F00, 0xEDA1, 0x11d0, 0xB4, 0x52, 0x00, 0xA0, 0x24, 0x4A, 0x1D, 0xD2);


extern RPC_IF_HANDLE __MIDL_itf_ad1ex_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ad1ex_0000_0000_v0_0_s_ifspec;

#ifndef __IDebugApplicationEx_INTERFACE_DEFINED__
#define __IDebugApplicationEx_INTERFACE_DEFINED__

/* interface IDebugApplicationEx */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplicationEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C00-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onCallEnter( 
            /* [in] */ DWORD dwLim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallOut( 
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallReturn( 
            /* [in] */ DWORD dwLim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onCallExit( 
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IDebugApplicationEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IDebugApplicationEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IDebugApplicationEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *onCallEnter )( 
            __RPC__in IDebugApplicationEx * This,
            /* [in] */ DWORD dwLim);
        
        HRESULT ( STDMETHODCALLTYPE *onCallOut )( 
            __RPC__in IDebugApplicationEx * This,
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest);
        
        HRESULT ( STDMETHODCALLTYPE *onCallReturn )( 
            __RPC__in IDebugApplicationEx * This,
            /* [in] */ DWORD dwLim);
        
        HRESULT ( STDMETHODCALLTYPE *onCallExit )( 
            __RPC__in IDebugApplicationEx * This,
            /* [in] */ DWORD dwLim,
            /* [in] */ DWORD dwAddrDest);
        
        END_INTERFACE
    } IDebugApplicationExVtbl;

    interface IDebugApplicationEx
    {
        CONST_VTBL struct IDebugApplicationExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDebugApplicationEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDebugApplicationEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDebugApplicationEx_onCallEnter(This,dwLim)	\
    ( (This)->lpVtbl -> onCallEnter(This,dwLim) ) 

#define IDebugApplicationEx_onCallOut(This,dwLim,dwAddrDest)	\
    ( (This)->lpVtbl -> onCallOut(This,dwLim,dwAddrDest) ) 

#define IDebugApplicationEx_onCallReturn(This,dwLim)	\
    ( (This)->lpVtbl -> onCallReturn(This,dwLim) ) 

#define IDebugApplicationEx_onCallExit(This,dwLim,dwAddrDest)	\
    ( (This)->lpVtbl -> onCallExit(This,dwLim,dwAddrDest) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugApplicationEx_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationEx_INTERFACE_DEFINED__

/* interface IRemoteDebugApplicationEx */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRemoteDebugApplicationEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C01-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHostPid( 
            /* [out] */ __RPC__out DWORD *dwHostPid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHostMachineName( 
            /* [out] */ __RPC__deref_out_opt BSTR *pbstrHostMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLocale( 
            /* [in] */ DWORD dwLangID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeBreak( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProxyBlanketAndAddRef( 
            /* [in] */ __RPC__in_opt IUnknown *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseFromSetProxyBlanket( 
            /* [in] */ __RPC__in_opt IUnknown *pUnk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IRemoteDebugApplicationEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IRemoteDebugApplicationEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHostPid )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [out] */ __RPC__out DWORD *dwHostPid);
        
        HRESULT ( STDMETHODCALLTYPE *GetHostMachineName )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [out] */ __RPC__deref_out_opt BSTR *pbstrHostMachineName);
        
        HRESULT ( STDMETHODCALLTYPE *SetLocale )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [in] */ DWORD dwLangID);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeBreak )( 
            __RPC__in IRemoteDebugApplicationEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProxyBlanketAndAddRef )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [in] */ __RPC__in_opt IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseFromSetProxyBlanket )( 
            __RPC__in IRemoteDebugApplicationEx * This,
            /* [in] */ __RPC__in_opt IUnknown *pUnk);
        
        END_INTERFACE
    } IRemoteDebugApplicationExVtbl;

    interface IRemoteDebugApplicationEx
    {
        CONST_VTBL struct IRemoteDebugApplicationExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRemoteDebugApplicationEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRemoteDebugApplicationEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRemoteDebugApplicationEx_GetHostPid(This,dwHostPid)	\
    ( (This)->lpVtbl -> GetHostPid(This,dwHostPid) ) 

#define IRemoteDebugApplicationEx_GetHostMachineName(This,pbstrHostMachineName)	\
    ( (This)->lpVtbl -> GetHostMachineName(This,pbstrHostMachineName) ) 

#define IRemoteDebugApplicationEx_SetLocale(This,dwLangID)	\
    ( (This)->lpVtbl -> SetLocale(This,dwLangID) ) 

#define IRemoteDebugApplicationEx_RevokeBreak(This)	\
    ( (This)->lpVtbl -> RevokeBreak(This) ) 

#define IRemoteDebugApplicationEx_SetProxyBlanketAndAddRef(This,pUnk)	\
    ( (This)->lpVtbl -> SetProxyBlanketAndAddRef(This,pUnk) ) 

#define IRemoteDebugApplicationEx_ReleaseFromSetProxyBlanket(This,pUnk)	\
    ( (This)->lpVtbl -> ReleaseFromSetProxyBlanket(This,pUnk) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRemoteDebugApplicationEx_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationEx7_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationEx7_INTERFACE_DEFINED__

/* interface IRemoteDebugApplicationEx7 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRemoteDebugApplicationEx7;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF718CD4-3738-498d-8E15-8029A11BDFA8")
    IRemoteDebugApplicationEx7 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetApartmentThreadId( 
            /* [out] */ __RPC__out DWORD *pdwAptThreadId) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationEx7Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IRemoteDebugApplicationEx7 * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IRemoteDebugApplicationEx7 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IRemoteDebugApplicationEx7 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetApartmentThreadId )( 
            __RPC__in IRemoteDebugApplicationEx7 * This,
            /* [out] */ __RPC__out DWORD *pdwAptThreadId);
        
        END_INTERFACE
    } IRemoteDebugApplicationEx7Vtbl;

    interface IRemoteDebugApplicationEx7
    {
        CONST_VTBL struct IRemoteDebugApplicationEx7Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationEx7_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRemoteDebugApplicationEx7_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRemoteDebugApplicationEx7_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRemoteDebugApplicationEx7_GetApartmentThreadId(This,pdwAptThreadId)	\
    ( (This)->lpVtbl -> GetApartmentThreadId(This,pdwAptThreadId) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRemoteDebugApplicationEx7_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__

/* interface IRemoteDebugApplicationThreadEx */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IRemoteDebugApplicationThreadEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B9B32B0C-9147-11d1-94EA-00C04FA302A1")
    IRemoteDebugApplicationThreadEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumGlobalExpressionContexts( 
            /* [out] */ __RPC__deref_out_opt IEnumDebugExpressionContexts **ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationThreadExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IRemoteDebugApplicationThreadEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IRemoteDebugApplicationThreadEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IRemoteDebugApplicationThreadEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumGlobalExpressionContexts )( 
            __RPC__in IRemoteDebugApplicationThreadEx * This,
            /* [out] */ __RPC__deref_out_opt IEnumDebugExpressionContexts **ppEnum);
        
        END_INTERFACE
    } IRemoteDebugApplicationThreadExVtbl;

    interface IRemoteDebugApplicationThreadEx
    {
        CONST_VTBL struct IRemoteDebugApplicationThreadExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationThreadEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRemoteDebugApplicationThreadEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRemoteDebugApplicationThreadEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRemoteDebugApplicationThreadEx_EnumGlobalExpressionContexts(This,ppEnum)	\
    ( (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRemoteDebugApplicationThreadEx_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHelperEx_INTERFACE_DEFINED__
#define __IDebugDocumentHelperEx_INTERFACE_DEFINED__

/* interface IDebugDocumentHelperEx */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentHelperEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C02-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHelperEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDocumentClassId( 
            /* [in] */ CLSID clsidDocument) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHelperExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IDebugDocumentHelperEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IDebugDocumentHelperEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IDebugDocumentHelperEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentClassId )( 
            __RPC__in IDebugDocumentHelperEx * This,
            /* [in] */ CLSID clsidDocument);
        
        END_INTERFACE
    } IDebugDocumentHelperExVtbl;

    interface IDebugDocumentHelperEx
    {
        CONST_VTBL struct IDebugDocumentHelperExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHelperEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDebugDocumentHelperEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDebugDocumentHelperEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDebugDocumentHelperEx_SetDocumentClassId(This,clsidDocument)	\
    ( (This)->lpVtbl -> SetDocumentClassId(This,clsidDocument) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugDocumentHelperEx_INTERFACE_DEFINED__ */


#ifndef __IDebugHelperEx_INTERFACE_DEFINED__
#define __IDebugHelperEx_INTERFACE_DEFINED__

/* interface IDebugHelperEx */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugHelperEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C08-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugHelperEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserFromError( 
            /* [in] */ IActiveScriptError *pase,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [out] */ IDebugProperty **ppdp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateWriteablePropertyBrowser( 
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [in] */ IDebugSetValueCallback *pdsvcb,
            /* [out] */ IDebugProperty **ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserFromCodeContext( 
            /* [in] */ IDebugCodeContext *pdcc,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [out] */ IDebugProperty **ppdp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugHelperExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugHelperEx * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugHelperEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugHelperEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePropertyBrowserFromError )( 
            IDebugHelperEx * This,
            /* [in] */ IActiveScriptError *pase,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [out] */ IDebugProperty **ppdp);
        
        HRESULT ( STDMETHODCALLTYPE *CreateWriteablePropertyBrowser )( 
            IDebugHelperEx * This,
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [in] */ IDebugSetValueCallback *pdsvcb,
            /* [out] */ IDebugProperty **ppdob);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePropertyBrowserFromCodeContext )( 
            IDebugHelperEx * This,
            /* [in] */ IDebugCodeContext *pdcc,
            /* [in] */ LPCOLESTR pszName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [out] */ IDebugProperty **ppdp);
        
        END_INTERFACE
    } IDebugHelperExVtbl;

    interface IDebugHelperEx
    {
        CONST_VTBL struct IDebugHelperExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugHelperEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDebugHelperEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDebugHelperEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDebugHelperEx_CreatePropertyBrowserFromError(This,pase,pszName,pdat,pdf,ppdp)	\
    ( (This)->lpVtbl -> CreatePropertyBrowserFromError(This,pase,pszName,pdat,pdf,ppdp) ) 

#define IDebugHelperEx_CreateWriteablePropertyBrowser(This,pvar,bstrName,pdat,pdf,pdsvcb,ppdob)	\
    ( (This)->lpVtbl -> CreateWriteablePropertyBrowser(This,pvar,bstrName,pdat,pdf,pdsvcb,ppdob) ) 

#define IDebugHelperEx_CreatePropertyBrowserFromCodeContext(This,pdcc,pszName,pdat,ppdp)	\
    ( (This)->lpVtbl -> CreatePropertyBrowserFromCodeContext(This,pdcc,pszName,pdat,ppdp) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugHelperEx_INTERFACE_DEFINED__ */


#ifndef __IDebugSetValueCallback_INTERFACE_DEFINED__
#define __IDebugSetValueCallback_INTERFACE_DEFINED__

/* interface IDebugSetValueCallback */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugSetValueCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C06-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSetValueCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
            /* [in] */ VARIANT *pvarNode,
            /* [in] */ DISPID dispid,
            /* [in] */ ULONG cIndices,
            /* [size_is][in] */ LONG *rgIndices,
            /* [in] */ LPCOLESTR pszValue,
            /* [in] */ UINT nRadix,
            /* [out] */ BSTR *pbstrError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSetValueCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugSetValueCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugSetValueCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugSetValueCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            IDebugSetValueCallback * This,
            /* [in] */ VARIANT *pvarNode,
            /* [in] */ DISPID dispid,
            /* [in] */ ULONG cIndices,
            /* [size_is][in] */ LONG *rgIndices,
            /* [in] */ LPCOLESTR pszValue,
            /* [in] */ UINT nRadix,
            /* [out] */ BSTR *pbstrError);
        
        END_INTERFACE
    } IDebugSetValueCallbackVtbl;

    interface IDebugSetValueCallback
    {
        CONST_VTBL struct IDebugSetValueCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSetValueCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDebugSetValueCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDebugSetValueCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDebugSetValueCallback_SetValue(This,pvarNode,dispid,cIndices,rgIndices,pszValue,nRadix,pbstrError)	\
    ( (This)->lpVtbl -> SetValue(This,pvarNode,dispid,cIndices,rgIndices,pszValue,nRadix,pbstrError) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugSetValueCallback_INTERFACE_DEFINED__ */


#ifndef __ISetNextStatement_INTERFACE_DEFINED__
#define __ISetNextStatement_INTERFACE_DEFINED__

/* interface ISetNextStatement */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ISetNextStatement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C03-CB0C-11d0-B5C9-00A0244A0E7A")
    ISetNextStatement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CanSetNextStatement( 
            /* [in] */ __RPC__in_opt IDebugStackFrame *pStackFrame,
            /* [in] */ __RPC__in_opt IDebugCodeContext *pCodeContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNextStatement( 
            /* [in] */ __RPC__in_opt IDebugStackFrame *pStackFrame,
            /* [in] */ __RPC__in_opt IDebugCodeContext *pCodeContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISetNextStatementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in ISetNextStatement * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in ISetNextStatement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in ISetNextStatement * This);
        
        HRESULT ( STDMETHODCALLTYPE *CanSetNextStatement )( 
            __RPC__in ISetNextStatement * This,
            /* [in] */ __RPC__in_opt IDebugStackFrame *pStackFrame,
            /* [in] */ __RPC__in_opt IDebugCodeContext *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetNextStatement )( 
            __RPC__in ISetNextStatement * This,
            /* [in] */ __RPC__in_opt IDebugStackFrame *pStackFrame,
            /* [in] */ __RPC__in_opt IDebugCodeContext *pCodeContext);
        
        END_INTERFACE
    } ISetNextStatementVtbl;

    interface ISetNextStatement
    {
        CONST_VTBL struct ISetNextStatementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISetNextStatement_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISetNextStatement_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISetNextStatement_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISetNextStatement_CanSetNextStatement(This,pStackFrame,pCodeContext)	\
    ( (This)->lpVtbl -> CanSetNextStatement(This,pStackFrame,pCodeContext) ) 

#define ISetNextStatement_SetNextStatement(This,pStackFrame,pCodeContext)	\
    ( (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISetNextStatement_INTERFACE_DEFINED__ */


#ifndef __IDebugSessionProviderEx_INTERFACE_DEFINED__
#define __IDebugSessionProviderEx_INTERFACE_DEFINED__

/* interface IDebugSessionProviderEx */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugSessionProviderEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C09-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSessionProviderEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( 
            /* [in] */ __RPC__in_opt IRemoteDebugApplication *pda,
            /* [in] */ BOOL fQuery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanJITDebug( 
            /* [in] */ DWORD pid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSessionProviderExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IDebugSessionProviderEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IDebugSessionProviderEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IDebugSessionProviderEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartDebugSession )( 
            __RPC__in IDebugSessionProviderEx * This,
            /* [in] */ __RPC__in_opt IRemoteDebugApplication *pda,
            /* [in] */ BOOL fQuery);
        
        HRESULT ( STDMETHODCALLTYPE *CanJITDebug )( 
            __RPC__in IDebugSessionProviderEx * This,
            /* [in] */ DWORD pid);
        
        END_INTERFACE
    } IDebugSessionProviderExVtbl;

    interface IDebugSessionProviderEx
    {
        CONST_VTBL struct IDebugSessionProviderExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSessionProviderEx_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDebugSessionProviderEx_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDebugSessionProviderEx_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDebugSessionProviderEx_StartDebugSession(This,pda,fQuery)	\
    ( (This)->lpVtbl -> StartDebugSession(This,pda,fQuery) ) 

#define IDebugSessionProviderEx_CanJITDebug(This,pid)	\
    ( (This)->lpVtbl -> CanJITDebug(This,pid) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugSessionProviderEx_INTERFACE_DEFINED__ */

#ifdef __cplusplus
}
#endif

#endif


