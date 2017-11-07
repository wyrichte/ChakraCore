

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0581 */
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


#ifndef __samplemetadata_h__
#define __samplemetadata_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "oaidl.h"
#include "winrths.h"
#include "inspectable.h"
#include "EventToken.h"
#include "AsyncInfo.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_samplemetadata_0000_0000 */
/* [local] */ 

typedef struct __x_Microsoft_CClr_CTest_CHStringStruct
    {
    int stringId;
    HSTRING hstring;
    } 	__x_Microsoft_CClr_CTest_CHStringStruct;



/* interface __MIDL_itf_samplemetadata_0000_0000 */
/* [local] */ 

#ifdef __cplusplus
} /* end extern "C" */
namespace Microsoft {
    namespace Clr {
        namespace Test {
            
            typedef struct HStringStruct
                {
                int stringId;
                HSTRING hstring;
                } 	HStringStruct;
            
        } /* end namespace */
    } /* end namespace */
} /* end namespace */

extern "C" { 
#endif



extern RPC_IF_HANDLE __MIDL_itf_samplemetadata_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_samplemetadata_0000_0000_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


