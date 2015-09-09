//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>
#include "edgescriptDirect.h"


// =================
// Parser Includes
// =================
#include "idiom.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Library\AsyncDebug.h"
#include "Debug\DiagProbe.h"

// =================
// Common Includes
// =================
#include "DataStructures\QuickSort.h"
#include "DataStructures\Option.h"
#include "DataStructures\ImmutableList.h"
#include "DataStructures\BufferBuilder.h"
// =================

#include <objsafe.h>

#include "ChakraInternalInterface.h"

#include "jscriptdllcommon.h"
#include "customEnumerator.h"

#include "classfac.h"
#include "siteserv.h"
#include "hostvariant.h"
#include "refcountedHostVariant.h"
#include "hostdispatch.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "hostdispatchenumerator.h"
#include "dispmemberproxy.h"
#include "..\staticlib\base\MockExternalObject.h"
#include "CustomExternalType.h"
#include "JavascriptExternalOperators.h"
#include "DispatchHelper.h"
#include "EventSink.h"

#include "activscp_private.h"
#include "ad1ex.h"
#include "scpnode.h"

#ifdef ENABLE_PROJECTION
#include "..\..\Lib\WinRT\WinRTLib.h"

#ifdef _M_ARM
#include "arm\CallingConvention.h"
#endif

#ifdef _M_ARM64
#include "arm64\CallingConvention.h"
#endif

#include "ProjectionMemoryInformation.h"
#include "JavascriptWinRTFunction.h"
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
#include "..\staticlib\base\scriptenginebase.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "ScriptEngine.h"

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
#include "DOMProperties.h"
#if !defined(USED_IN_STATIC_LIB)
#include "ActiveScriptProfilerHeapEnum.h"
#endif

#include "AutoCallerPointer.h"
