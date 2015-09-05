//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>
#include "dllfunc.h"
#include <atlcom.h>
#include "edgescriptDirect.h"

#include "Parser.h"
#include "common\ByteSwap.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Library\EtwTrace.h"
#include "Library\AsyncDebug.h"
#include "Library\dataview.h"
#include "Library\JavascriptProxy.h"
#include "Library\ThreadBoundThreadContextManager.h"
#include "Library\HostObjectBase.h"
#ifdef ENABLE_MUTATION_BREAKPOINT
// REVIEW: ChakraCore Dependency
#include "activdbg_private.h"
#endif
#include "Debug\DebuggingFlags.h"
#include "Debug\DiagProbe.h"
#include "Debug\BreakpointProbe.h"
#include "Debug\DebugDocument.h"
#include "Debug\DebugManager.h"
#include "Debug\ProbeContainer.h"
#include "Debug\DebugContext.h"
#include "Debug\DiagObjectModel.h"
#include "Debug\DiagStackFrame.h"
#ifdef ENABLE_MUTATION_BREAKPOINT
#include "Debug\MutationBreakpoint.h"
#endif
// =================


#include <objsafe.h>
#include <comcat.h>
#include <strsafe.h>
#include <intsafe.h>
#include <io.h>
#include <xmllite.h>

#include <mshtmhst.h>
#include <devtbhost.h>
#if !defined(USED_IN_STATIC_LIB)
#include <guids.h>
#endif
#include "ChakraInternalInterface.h"
#include "jscriptinfo.h"
#include "DynamicSourceHolder.h"
#include "jscriptdllcommon.h"
#include "customEnumerator.h"
#include "ErrorTypeHelper.h"
#include "EnumVariantEnumerator.h"
#include "classfac.h"
#include "siteserv.h"
#include "hostvariant.h"
#include "refcountedHostVariant.h"
#include "hostdispatch.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "hostdispatchenumerator.h"
#include "dispmemberproxy.h"
#include "javascriptdispatch.h"
#include "..\staticlib\base\MockExternalObject.h"
#include "CustomExternalType.h"
#include "JavascriptExternalOperators.h"
#include "DefaultScriptOperations.h"
#include "CJavascriptOperations.h"
#include "HostObject.h"
#include "DispatchHelper.h"
#include "EventSink.h"
#include "NamedEventHandler.h"
#include "activdbg.h"
#include "activdbg100.h"
#include "inetpriv.h"
#include "activscp_private.h"
#include "activprof.h"
#include "ad1ex.h"
#include "typeinfobuilder.h"
#include "dbgfmt.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "codectx.h"
#include "scpnode.h"
#include "scptext.h"
#include "ScriptDebugDocument.h"

#ifdef ENABLE_PROJECTION
#include "..\..\Lib\WinRT\WinRTLib.h"

#ifdef _M_ARM
#include "arm\CallingConvention.h"
#endif

#ifdef _M_ARM64
#include "arm64\CallingConvention.h"
#endif

#include "ProjectionMemoryInformation.h"
#include "ProjectionContext.h"
#include "SpecialProjection.h"
#include "VariableArgMethodHelper.h"
#include "ExternalWeakReferenceImpl.h"
#include "ExternalWeakReferenceSourceImpl.h"
#include "UnknownImpl.h"
#include "ABIEvents.h"
#include "abidelegates.h"
#include "ProjectionMarshaler.h"
#include "ProjectionFastPath.h"
#include "ProjectionMethodInvoker.h"
#include "ProjectionAsyncDebug.h"
#include "FinalizableTypedArrayContents.h"
#include "ProjectionObjectInstance.h"
#include "ProjectionWriter.h"
#include "ArrayAsCollection.h"
#include "ArrayAsIterable.h"
#include "ArrayAsIterator.h"
#include "ArrayAsVector.h"
#include "VectorArrayEnumerator.h"
#include "VectorArray.h"
#include "PropertySet.h"
#include "PropertySetEnumerator.h"
#include "ProjectionTypeOperations.h"
#include "ObjectAsIPropertyValue.h"
#include "ObjectAsIReference.h"
#endif

#include "ComObjectBase.h"
#include "ScriptDAC.h"
#include "DiagHook.h"
#include "..\staticlib\base\scriptenginebase.h"
#include "ScriptEngine.h"
#include "JavascriptThreadService.h"
#if !defined(USED_IN_STATIC_LIB)
#include "jsfac.h"
#endif

#include "JsrtContext.h"

#ifdef ENABLE_PROJECTION
#include "ArrayProjectionEnumerator.h"
#include "ArrayProjection.h"
#include "InspectableObjectTypeOperations.h"
#include "namespaceProjection.h"
#include "NamespaceProjectionEnumerator.h"
#endif

#include "scrpting.h"
#include "caller.h"
#include "activescripterror.h"
#include "scriptsite.h"
#include "QueryContinuePoller.h"
#include "dbgprop.h"
#include "dbgpropimpl.h"
#include "ProfileDataObject.h"
#include "DebugObject.h"
#include "DiagnosticsScriptObject.h"
#include "SCAEngine.h"
#include "StreamHelper.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "SCAPropBag.h"
#include "SCASerialization.h"
#include "SCADeserialization.h"
#include "TestHooks.h"
#include "DOMProperties.h"
#if !defined(USED_IN_STATIC_LIB)
#include "ActiveScriptProfilerHeapEnum.h"
#endif

#include "resource.h"
#include "ChakraVersion.h"
#include "AutoCallerPointer.h"