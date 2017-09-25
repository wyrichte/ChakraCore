//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#pragma warning(push)
#pragma warning(disable:4456) // declaration of '' hides previous local declaration
#include <atlbase.h>
#pragma warning(pop)
// atlbase.h 3.0 leak a warning(push) under _DEBUG
#ifdef _DEBUG   
#pragma warning(pop)
#endif

#include <strsafe.h>
#include "Parser.h"
#include "common\ByteSwap.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Base\EtwTrace.h"
#include "Base\VTuneChakraProfile.h"
#include "Base\ThreadContextTLSEntry.h"
#include "Base\ThreadBoundThreadContextManager.h"
#include "AsyncDebug.h"
#include "Library\dataview.h"
#include "Library\JavascriptProxy.h"
#include "Library\HostObjectBase.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "Library\JavascriptWeakMap.h"

#include "Types\PropertyIndexRanges.h"
#include "Types\DictionaryPropertyDescriptor.h"
#include "Types\DictionaryTypeHandler.h"
#include "Types\ES5ArrayTypeHandler.h"
#include "Types\RecyclableObject.h"
#include "Types\DynamicType.h"

#ifdef ENABLE_MUTATION_BREAKPOINT
#include "activdbg_private.h"
#include "Debug\MutationBreakpoint.h"
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
#include "Language\JavascriptStackWalker.h"
// =================

//====================================
// Engine includes
//====================================
#include <objsafe.h>
#include "activscp_private.h"
#include "edgescriptDirect.h"
#include "ChakraInternalInterface.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "jscriptdllcommon.h"
#include "..\StaticLib\base\ScriptEngineBase.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "ScriptEngine.h"


#include <devtbhost.h>
#if !defined(USED_IN_STATIC_LIB)
#include <guids.h>
#endif
#include "jscriptinfo.h"
#include "DynamicSourceHolder.h"
#include "customEnumerator.h"
#include "ErrorTypeHelper.h"

#include "hostvariant.h"
#include "refcountedHostVariant.h"
#include "hostdispatch.h"
#include "dispmemberproxy.h"
#include "javascriptdispatch.h"
#include "..\staticlib\base\MockExternalObject.h"
#include "ExternalObject.h"
#include "CustomExternalType.h"
#include "JavascriptExternalOperators.h"
#include "DefaultScriptOperations.h"
#include "CJavascriptOperations.h"
#include "HostObject.h"
#include "DispatchHelper.h"
#include "NamedEventHandler.h"
#include "EventSink.h"
#include "activdbg.h"
#include "activdbg100.h"
#include "inetpriv.h"
#include "activprof.h"
#include "ad1ex.h"
#include "dbgfmt.h"
#include "codectx.h"
#include "scpnode.h"
#include "scptext.h"
#include "ScriptDebugDocument.h"

//====================================
// Projections includes
//====================================
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
#include "ProjectionMarshaler.h"
#include "ProjectionObjectInstance.h"
#include "ProjectionTypeOperations.h"
#endif

//====================================
// More Engine includes
//====================================
#include "dllfunc.h"
#include "ComObjectBase.h"
#include "classfac.h"
#if !defined(USED_IN_STATIC_LIB)
#include "jsfac.h"
#endif

#include "ScriptDAC.h"
#include "JavascriptThreadService.h"

#include "scrpting.h"
#include "caller.h"
#include "DiagnosticsScriptObject.h"
#include "activescripterror.h"
#include "ActiveScriptExternalLibrary.h"
#include "scriptsite.h"
#include "QueryContinuePoller.h"
#include "dbgprop.h"
#include "dbgpropimpl.h"
#include "ProfileDataObject.h"
#include "DebugObject.h"
#include "DiagnosticsScriptObject.h"
#include "DOMProperties.h"

#include "resource.h"
#include "ChakraVersion.h"
#include "AutoCallerPointer.h"
