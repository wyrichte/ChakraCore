TARGETNAME=JScript.Runtime.Library
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(COM_INC_PATH);$(SDK_LIB_PATH)\clr45;..\..\..\tools\external\inc

#4702: unreachable code
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4702 

SOURCES= \
    ..\ArgumentsObject.cpp \
    ..\ArgumentsObjectType.cpp \
    ..\ArgumentsObjectEnumerator.cpp \
    ..\ArrayBuffer.cpp \
    ..\AsyncDebug.cpp \
    ..\BoundFunction.cpp \
    ..\BoundFunctionType.cpp \
    ..\BufferStringBuilder.cpp \
    ..\CharStringCache.cpp \
    ..\ConcatString.cpp \
    ..\CommonExternalApiImpl.cpp \
    ..\CompoundString.cpp \
    ..\Debug.cpp \
    ..\ES5Array.cpp \
    ..\ES5ArrayEnumerator.cpp \
    ..\ES5ArrayTypeHandler.cpp \
    ..\ExpirableObject.cpp \
    ..\PropertyString.cpp \
    ..\HiResTimer.cpp \
    ..\DateImplementation.cpp \
    ..\DataView.cpp \
    ..\RuntimeFunction.cpp \
    ..\RootObjectBase.cpp \
    ..\GlobalObject.cpp \
    ..\GlobalObjectType.cpp \
    ..\TaggedInt.cpp \
    ..\IteratorObjectEnumerator.cpp \
    ..\JavascriptArray.cpp \
    ..\JavascriptArrayEnumerator.cpp \
    ..\JavascriptArraySnapshotEnumerator.cpp \
    ..\JavascriptArrayEnumeratorBase.cpp \
    ..\JavascriptArrayType.cpp \
    ..\JavascriptArrayIterator.cpp \
    ..\JavascriptBoolean.cpp \
    ..\JavascriptBooleanObject.cpp \
    ..\JavascriptBooleanObjectType.cpp \
    ..\JavascriptBooleanType.cpp \
    ..\JavascriptBuiltInFunctions.cpp \
    ..\JavascriptDate.cpp \
    ..\JavascriptDateType.cpp \
    ..\JavascriptEnumeratorIterator.cpp \
    ..\JavascriptError.cpp \
    ..\JavascriptErrorDebug.cpp \
    ..\JavascriptExternalFunction.cpp \
    ..\JavascriptFunction.cpp \
    ..\JavascriptFunctionType.cpp \
    ..\JavascriptGenerator.cpp \
    ..\JavascriptGeneratorFunction.cpp \
    ..\JavascriptGeneratorFunctionType.cpp \
    ..\JavascriptLibrary.cpp \
    ..\JavascriptMap.cpp \
    ..\JavascriptMapIterator.cpp \
    ..\JavascriptNumber.cpp \
    ..\JavascriptNumberObject.cpp \
    ..\JavascriptNumberObjectType.cpp \
    ..\JavascriptObject.cpp \
    ..\JavascriptPixelArray.cpp \
    ..\JavascriptPixelArrayEnumerator.cpp \
    ..\JavascriptPixelArrayType.cpp \
    ..\JavascriptPromise.cpp \
    ..\JavascriptProxy.cpp \
    ..\JavascriptReflect.cpp \
    ..\JavascriptRegularExpression.cpp \
    ..\JavascriptRegularExpressionType.cpp \
    ..\JavascriptRegularExpressionResult.cpp \
    ..\JavascriptRegExpConstructor.cpp\
    ..\JavascriptRegExpConstructorType.cpp\
	..\JavascriptRegExpEnumerator.cpp\
    ..\JavascriptSet.cpp \
    ..\JavascriptSetIterator.cpp \
    ..\JavascriptString.cpp \
    ..\JavascriptStringEnumerator.cpp \
    ..\JavascriptStringIterator.cpp \
    ..\JavascriptStringType.cpp \
    ..\JavascriptSymbol.cpp \
    ..\JavascriptSymbolObject.cpp \
    ..\JavascriptSymbolType.cpp \
    ..\JavascriptSymbolObjectType.cpp \
    ..\JavascriptTypedNumber.cpp \
    ..\JavascriptTypedObjectSlotAccessorFunction.cpp \
    ..\JavascriptVariantDate.cpp \
    ..\JavascriptVariantDateType.cpp \
    ..\JavascriptWeakMap.cpp \
    ..\JavascriptWeakSet.cpp \
    ..\JavascriptWinRTDate.cpp \
    ..\LiteralString.cpp \
    ..\LiteralStringObject.cpp \
    ..\LiteralStringObjectType.cpp \
    ..\ForInObjectEnumerator.cpp \
    ..\Math.cpp \
    ..\ModuleRoot.cpp \
    ..\ModuleRootType.cpp \
    ..\ObjectPrototypeObject.cpp \
    ..\ProfileString.cpp \
    ..\RegexHelper.cpp \
    ..\RoCausality.Win7.cpp \
    ..\SingleCharString.cpp \
    ..\StringCopyInfo.cpp \
    ..\SubString.cpp \
    ..\ThrowErrorObject.cpp \
    ..\TypedArray.cpp \
    ..\TypedArrayEnumerator.cpp \
    ..\UriHelper.cpp \
    ..\JSONString.cpp \
    ..\JSONStack.cpp\
    ..\JSON.cpp\
    ..\JSONParser.cpp\
    ..\JSONScanner.cpp\
    ..\EngineInterfaceObject.cpp \
    ..\JnDebugHelp.cpp \
    ..\Exception.cpp \
    ..\HostLogger.cpp \
    ..\StackProber.cpp \
    ..\ThreadContext.cpp \
    ..\ThreadContextTLSEntry.cpp \
    ..\ThreadBoundThreadContextManager.cpp \
    ..\ScriptMemoryDumper.cpp \
    ..\ScriptFunction.cpp \
    ..\ScriptFunctionType.cpp \
    ..\StackScriptFunction.cpp \
    ..\EtwTrace.cpp \
    ..\Constants.cpp \
    ..\DelayLoadLibrary.cpp \
    ..\Entropy.cpp \
    ..\TestEtwEventSink.cpp \
    ..\Telemetry.cpp \
    ..\SparseArraySegment.cpp \
    ..\WindowsGlobalizationAdapter.cpp \
    ..\WindowsFoundationAdapter.cpp \
    ..\JavascriptSIMDFloat32x4.cpp \
    ..\SIMDFloat32x4Lib.cpp \
    ..\JavascriptSIMDFloat64x2.cpp \
	..\SIMDFloat64x2Lib.cpp \
    ..\JavascriptSIMDInt32x4.cpp \
    ..\SIMDInt32x4Lib.cpp 


AMD64_SOURCES=\
    ..\amd64\javascriptfunctiona.asm \
    ..\SIMDFloat32x4OperationX86X64.cpp \
    ..\SIMDFloat64x2OperationX86X64.cpp \
    ..\SIMDInt32x4OperationX86X64.cpp 
    
I386_SOURCES=\
    ..\SIMDFloat32x4OperationX86X64.cpp \
    ..\SIMDFloat64x2OperationX86X64.cpp \
    ..\SIMDInt32x4OperationX86X64.cpp 

ARM_SOURCES= \
    ..\arm\arm_CallFunction.asm \
    ..\arm\arm_DeferredParsingThunk.asm \
    ..\arm\arm_DeferredDeserializeThunk.asm \
    ..\SIMDFloat32x4Operation.cpp \
    ..\SIMDFloat64x2Operation.cpp \
    ..\SIMDInt32x4Operation.cpp 

ARM64_SOURCES= \
    ..\arm64\arm64_CallFunction.asm \
    ..\arm64\arm64_DeferredParsingThunk.asm \
    ..\arm64\arm64_DeferredDeserializeThunk.asm \
    ..\SIMDFloat32x4Operation.cpp \
    ..\SIMDFloat64x2Operation.cpp \
    ..\SIMDInt32x4Operation.cpp 

NTTARGETFILE0= \
!IF "$(_BUILDARCH)" == "amd64"
    ..\InJavascript\Intl.js.bc.64b.h \
    ..\InJavascript\Promise.js.bc.64b.h \
!ELSE
    ..\InJavascript\Intl.js.bc.32b.h \
    ..\InJavascript\Promise.js.bc.32b.h \
!ENDIF

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
