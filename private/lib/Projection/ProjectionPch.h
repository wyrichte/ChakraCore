//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>

// =================
// Parser Includes
// =================
#include "idiom.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "AsyncDebug.h"
#include "Debug\DiagProbe.h"
#include "Library\JavascriptErrorDebug.h"

// =================
// Common Includes
// =================
#include "DataStructures\QuickSort.h"
#include "DataStructures\Option.h"
#include "DataStructures\ImmutableList.h"
#include "DataStructures\BufferBuilder.h"

//====================================
// Engine includes
//====================================
#include <objsafe.h>
#include "activscp_private.h"
#include "edgescriptDirect.h"
#include "ChakraInternalInterface.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "..\staticlib\base\scriptenginebase.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "ScriptEngine.h"

#include "jscriptdllcommon.h"
#include "dllfunc.h"
#include "refcountedHostVariant.h"
#include "hostdispatch.h"

#include "DiagnosticsScriptObject.h"
#include "ActiveScriptExternalLibrary.h"
#include "Library\EngineInterfaceObject.h"
#include "WinRtPromiseEngineInterfaceExtensionObject.h"
#include "ProjectionExternalLibrary.h"
#include "scriptsite.h"
#include "AutoCallerPointer.h"

#include "..\staticlib\base\MockExternalObject.h"
#include "ExternalObject.h"
#include "CustomExternalType.h"

#include "ActiveScriptProfilerHeapEnum.h"

//====================================
// Projection includes
//====================================
#ifdef ENABLE_PROJECTION
#include "..\..\Lib\WinRT\WinRTLib.h"

#ifdef _M_ARM
#include "arm\CallingConvention.h"
#endif

#ifdef _M_ARM64
#include "arm64\CallingConvention.h"
#endif
#include "Library\ES5Array.h"

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

#include "ArrayProjectionEnumerator.h"
#include "ArrayProjection.h"
#include "InspectableObjectTypeOperations.h"
#include "namespaceProjection.h"
#include "NamespaceProjectionEnumerator.h"
#endif

