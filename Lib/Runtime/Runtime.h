//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "Banned.h"
#include "Common.h"
#include "Parser.h"
#include "RuntimeCommon.h"

#if !defined(UNREFERENCED_PARAMETER)
#define UNREFERENCED_PARAMETER(x) (x)
#endif

//#define BODLOG

class Lowerer;
class LowererMD;
class LowererMDArch;
class ByteCodeGenerator;

enum RegexOp {
  RegexOp_None,
  RegexOp_Replace,
  RegexOp_Exec,
  RegexOp_Test,
  RegexOp_Split,
  RegexOp_Match
};

namespace Js
{
    //
    // Forward declarations
    //
    typedef int32 MessageId;
    /* enum */ struct PropertyIds;
    struct Utf8SourceInfo;
    struct CallInfo;
    struct InlineeCallInfo;
    struct InlineCache;
    struct PolymorphicInlineCache;
    struct Arguments;
    class StringDictionaryWrapper;
    struct ByteCodeDumper;
    struct ByteCodeReader;
    struct ByteCodeWriter;
    class JavascriptConversion;
    class JavascriptDate;
    class JavascriptVariantDate;
    class DateImplementation;
    class BufferString;
    class BufferStringBuilder;
    class ConcatString;
    class GcConcatString;
    class JavascriptBoolean;
    class JavascriptBooleanObject;
    class JavascriptSymbol;
    class JavascriptSymbolObject;
    class JavascriptProxy;
    class JavascriptReflect;
    class JavascriptEnumeratorIterator;
    class JavascriptArrayIterator;
    enum class JavascriptArrayIteratorKind;
    class JavascriptMapIterator;
    enum class JavascriptMapIteratorKind;
    class JavascriptSetIterator;
    enum class JavascriptSetIteratorKind;
    class JavascriptStringIterator;
    class JavascriptPromise;
    class JavascriptPromiseCapability;
    class JavascriptPromiseReaction;
    class JavascriptPromiseCapabilitiesExecutorFunction;
    class JavascriptPromiseResolveOrRejectFunction;
    class JavascriptPromiseReactionTaskFunction;
    class JavascriptPromiseResolveThenableTaskFunction;
    class JavascriptPromiseAllResolveElementFunction;
    struct JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper;
    class JavascriptGenerator;
    class LiteralString;
    class ArenaLiteralString;
    class JavascriptStringObject;
    struct PropertyDescriptor;
    class Type;
    class DynamicType;
    class ScriptFunctionType;
    class DynamicTypeHandler;
    class DeferredTypeHandlerBase;
    template <bool IsPrototype> class NullTypeHandler;
    template<size_t size> class SimpleTypeHandler;
    class PathTypeHandler;
    class IndexPropertyDescriptor;
    class DynamicObject;
    class ArrayObject;
    class WithScopeObject;
    class SpreadArgument;
    class JavascriptString;
    class StringCopyInfo;
    class StringCopyInfoStack;
    class ObjectPrototypeObject;
    class PropertyString;
    class ArgumentsObject;
    class HeapArgumentsObject;
    class ActivationObject;
    class JavascriptNumber;
    class JavascriptNumberObject;

#ifdef SIMD_JS_ENABLED
    // SIMD
    class SIMDFloat32x4Lib;
    class JavascriptSIMDFloat32x4;
    class SIMDFloat64x2Lib;
    class JavascriptSIMDFloat64x2;
    class SIMDInt32x4Lib;
    class JavascriptSIMDInt32x4;
    
#endif


    class RecyclableObject;
    class JavascriptRegExp;
    class JavascriptRegularExpressionResult;
    template<typename T> class SparseArraySegment;
    enum class DynamicObjectFlags : uint16;
    class JavascriptArray;
    class JavascriptNativeIntArray;
    class JavascriptCopyOnAccessNativeIntArray;
    class JavascriptNativeFloatArray;
    class JavascriptArrayType;
    class ES5Array;
    class JavascriptFunction;
    class ScriptFunction;
    class StackScriptFunction;
    class GeneratorVirtualScriptFunction;
    class JavascriptRegExpConstructor;
    class JavascriptRegExpEnumerator;
    class BoundFunction;
    class JavascriptMap;
    class JavascriptSet;
    class JavascriptWeakMap;
    class JavascriptWeakSet;
    class DynamicObject;
    class RootObjectBase;
    class ModuleRoot;
    class GlobalObject;
    class GcValueHolder;
    class Math;
    class JavascriptOperators;
    class JavascriptLibrary;
    class JavascriptEval;
    class JavascriptParseInt;
    class JavascriptParseFloat;
    class JavascriptIsNaN;
    class JavascriptIsFinite;
    class JavascriptEncodeURI;
    class JavascriptEncodeURIComponent;
    class JavascriptDecodeURI;
    class JavascriptDecodeURIComponent;
    struct ConstructorCache;
    enum class OpCode : ushort;
    enum class OpCodeAsmJs : ushort;    
    /* enum */ struct OpLayoutType;
    /* enum */ struct OpLayoutTypeAsmJs;
    class RcBlock;
    class ExceptionBase;
    class NotImplementedException;
    class RcObject;
    class OutOfMemoryException;
    class ScriptDebug;
    class ScriptContext;
    struct NativeModule;
    template <class T> class RcRef;
    class RcString;
    class TaggedInt;
    class TaggedNumber;
    struct InterpreterStackFrame;
    struct ScriptEntryExitRecord;
    class JavascriptStackWalker;
    struct AsmJsCallStackLayout;
    class JavascriptCallStackLayout;
    class Throw;
    struct Tick;
    struct TickDelta;
    class ByteBlock;
    class FunctionInfo;
    class FunctionBody;
    class ParseableFunctionInfo;
    struct StatementLocation;
    class EntryPointInfo;
    struct LoopHeader;
    class InternalString;
    /* enum */ struct JavascriptHint;
    /* enum */ struct BuiltinFunction;
    class EnterScriptObject;
    class PropertyRecord;
    struct IsInstInlineCache;
    class EntryPointInfo;
    class PolymorphicInlineCacheInfo;
    class PropertyGuard;


    // asm.js
    namespace ArrayBufferView
    {
        enum ViewType;
    }
    struct EmitExpressionInfo;
    struct AsmJsModuleMemory;
    namespace AsmJsLookupSource
    {
        enum Source;
    }
    struct AsmJsByteCodeWriter;
    class AsmJsArrayView;
    class AsmJsType;
    class AsmJsRetType;
    class AsmJsVarType;
    class AsmJsSymbol;
    class AsmJsVarBase;
    class AsmJsVar;
    class AsmJsConstantImport;
    class AsmJsArgument;
    class AsmJsFunc;
    class AsmJsFunctionDeclaration;
    class AsmJsFunctionInfo;
    class AsmJsModuleInfo;
    class AsmJsGlobals;
    class AsmJsImportFunction;
    class AsmJsTypedArrayFunction;
    class AsmJsMathFunction;
    class AsmJsMathConst;
#ifdef ASMJS_PLAT
    class AsmJsCodeGenerator;
    class AsmJsEncoder;
#endif
    struct MathBuiltin;
    struct ExclusiveContext;
    class AsmJsModuleCompiler;
    class AsmJSCompiler;
    class AsmJSByteCodeGenerator;
    enum AsmJSMathBuiltinFunction;
    //////////////////////////////////////////////////////////////////////////
    typedef JsUtil::WeakReferenceDictionary<PropertyId, PropertyString, PowerOf2SizePolicy> PropertyStringCacheMap;

    extern const FrameDisplay NullFrameDisplay;
    extern const FrameDisplay StrictNullFrameDisplay;

    struct FuncInfoEntry
    {
        uint nestedIndex;
        uint scopeSlot;
    };

    struct FuncCacheEntry
    {
        ScriptFunction *func;
        DynamicType *type;
    };

    typedef uint16 DirectCode;

    inline bool IsMathLibraryId(PropertyId propertyId) {
        return (propertyId>=PropertyIds::abs)&&(propertyId<=PropertyIds::fround);
    }

    struct PropertyIdArray
    {
        uint32 count;
        bool   hadDuplicates;
        bool   has__proto__; // Only used for object literal
        PropertyId elements[];
        PropertyIdArray(uint32 count, bool hadDuplicates = false, bool has__proto__ = false) : count(count), hadDuplicates(hadDuplicates), has__proto__(has__proto__)
        {
        }

        uint32 GetDataSize(uint32 extraSlots) const { return sizeof(PropertyIdArray) + sizeof(PropertyId) * (count + extraSlots); }
    };

    template<typename T>
    struct AuxArray
    {
        uint32 count;
        T elements[];

        AuxArray(uint32 count) : count(count)
        {
        }

        void SetCount(uint count) { this->count = count; }
        uint32 GetDataSize() const { return sizeof(AuxArray) + sizeof(T) * count; }
    };
    typedef AuxArray<Var> VarArray;

    typedef AuxArray<FuncInfoEntry> FuncInfoArray;

    struct VarArrayVarCount
    {
        Var count;
        Var elements[];

        VarArrayVarCount(Var count) : count(count)
        {
        }

        void SetCount(uint count);
        uint32 GetDataSize() const;
    };


    // Inline cache flags, when property if from prototype object and is not writable
    #define InlineCacheProtoFlags (PropertyPrototypeObject)

}

class SourceContextInfo;
class AsyncDebug;
struct LazyBailOutRecord;

// Forward declaration to avoid including scriptdirect.h
typedef HRESULT (__cdecl *InitializeMethod)(Js::Var instance);
#ifndef SCRIPT_DIRECT_TYPE
typedef enum JsNativeValueType
{
    JsInt8Type,
    JsUint8Type,
    JsInt16Type,
    JsUint16Type,
    JsInt32Type,
    JsUint32Type,
    JsInt64Type,
    JsUint64Type,
    JsFloatType,
    JsDoubleType,
    JsNativeStringType
} JsNativeValueType;

typedef struct JsNativeString
{
    unsigned int length;
    LPCWSTR str;
} JsNativeString;
#else
enum JsNativeValueType;
#endif

#include "Language\SourceHolder.h"
#include "Language\Utf8SourceInfo.h"
#include "Language\PropertyRecord.h"
#include "Library\DelayLoadLibrary.h"
#include "Language\CallInfo.h"
#include "Language\ExecutionMode.h"
#include "BackEndAPI.h"
#include "DetachedStateBase.h"

#include "Library\Constants.h"
#include "ByteCode\OpLayoutsCommon.h"
#include "ByteCode\OpLayouts.h"
#include "ByteCode\OpLayoutsAsmJs.h"
#include "ByteCode\OpCodeUtil.h"
#include "ByteCode\OpCodeUtilAsmJs.h"
#include "Language\Arguments.h"

#include "Types\TypeId.h"
#include "Types\RecyclableObject.h"
#include "Library\ExpirableObject.h"
#include "Types\TypePropertyCache.h"
#include "Types\Type.h"
#include "Types\StaticType.h"
#include "Types\IWalkPropertyCallback.h"
#include "Types\CrossSite.h"
#include "Types\CrossSiteObject.h"
#include "Types\CrossSiteEnumerator.h"
#include "Types\JavascriptEnumerator.h"
#include "Types\DynamicObject.h"
#include "Types\ArrayObject.h"
#include "Types\DynamicObjectEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumeratorWPCache.h"
#include "Types\WithScopeObject.h"
#include "Types\TypePath.h"
#include "Types\SimplePropertyDescriptor.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\DictionaryPropertyDescriptor.h"
#include "Types\TypeHandler.h"
#include "Types\NullTypeHandler.h"
#include "Types\DeferredTypeHandler.h"
#include "Types\SimpleTypeHandler.h"
#include "Types\PathTypeHandler.h"
#include "Types\SimpleDictionaryTypeHandler.h"
#include "Types\SimpleDictionaryUnorderedTypeHandler.h"
#include "Types\DictionaryTypeHandler.h"
#include "Types\DynamicType.h"
#include "Types\CopyOnWrite.h"
#include "Types\ExternalObject.h"
#include "Types\DOMFastPath.h"
#include "Types\SpreadArgument.h"
#include "Language\StackTraceArguments.h"
#include "Types\MissingPropertyTypeHandler.h"
#include "Types\PropertyDescriptor.h"
#include "Types\ActivationObjectType.h"
#include "Types\TempArenaAllocatorObject.h"
#include "activscp_private.h"
#include "Language\ValueType.h"
#include "Language\DynamicProfileInfo.h"
#include "Language\ReadOnlyDynamicProfileInfo.h"
#include "Language\SourceDynamicProfileManager.h"
#include "Language\SourceContextInfo.h"
#include "Language\DynamicProfileMutator.h"
#include "Language\InlineCache.h"
#include "Language\InlineCachePointerArray.h"
#include "Language\CacheOperators.h"
#include "Types\FunctionInfo.h"
#include "Language\FunctionCodeGenRuntimeData.h"
#include "Types\FunctionBody.h"
#include "Language\FunctionCodeGenJitTimeData.h"
#include "Language\CodeGenRecyclableData.h"
#include "Types\JavascriptExceptionContext.h"
#include "Types\JavascriptExceptionObject.h"
#include "Types\PerfHint.h"

#include "ByteCode\ByteBlock.h"

#include "Library\JavascriptBuiltInFunctions.h"
#include "Library\JavascriptString.h"
#include "Library\JavascriptStringIterator.h"
#include "Library\StringCopyInfo.h"
#include "Library\JSONString.h"
#include "Library\JavascriptArrayEnumeratorBase.h"
#include "Library\JavascriptArrayEnumerator.h"
#include "Library\JavascriptArraySnapshotEnumerator.h"
#include "Library\JavascriptArrayNonIndexSnapshotEnumerator.h"
#include "Library\JavascriptStringEnumerator.h"
#include "Library\ForInObjectEnumerator.h"
#include "Library\HostObjectBase.h"
#include "Library\RootObjectBase.h"
#include "Library\ModuleRoot.h"
#include "Library\ArgumentsObjectEnumerator.h"
#include "Library\ArgumentsObject.h"
#include "Library\LiteralString.h"
#include "Library\BufferStringBuilder.h"
#include "Library\ConcatString.h"
#include "Library\CompoundString.h"
#include "Library\PropertyString.h"
#include "Library\LiteralStringObject.h"
#include "Library\JavascriptNumber.h"
#include "Library\JavascriptNumberObject.h"

#ifdef SIMD_JS_ENABLED
// SIMD types
#include "Library\JavascriptSIMDFloat32x4.h"
#include "Library\JavascriptSIMDFloat64x2.h"
#include "Library\JavascriptSIMDInt32x4.h"
// SIMD operations
#include "Library\SIMDFloat32x4Operation.h"
#include "Library\SIMDFloat64x2Operation.h"
#include "Library\SIMDInt32x4Operation.h"
#endif

#include "Library\JavascriptTypedNumber.h"
#include "Library\SparseArraySegment.h"
#include "Library\JavascriptError.h"
#include "Library\JavascriptErrorDebug.h"
#include "Library\JavascriptArray.h"
#include "Library\JavascriptArrayIndexEnumerator.h"
#include "Library\JavascriptArrayIterator.h"
#include "Library\ES5ArrayTypeHandler.h"
#include "Library\ES5Array.h"
#include "Library\ArrayBuffer.h"
#include "Library\TypedArray.h"
#include "Library\DataView.h"
#include "Library\TypedArrayEnumerator.h"
#include "Library\JavascriptPixelArray.h"
#include "Library\JavascriptPixelArrayEnumerator.h"
#include "Library\ES5ArrayEnumerator.h"
#include "Library\ES5ArrayNonIndexEnumerator.h"
#include "Library\ES5ArrayIndexEnumerator.h"
#include "Library\JavascriptRegularExpression.h"
#include "Library\JavascriptRegularExpressionResult.h"
#include "Library\JavascriptBoolean.h"
#include "Library\JavascriptBooleanObject.h"
#include "Library\JavascriptFunction.h"
#include "Library\ScriptFunctionType.h"
#include "Library\ScriptFunction.h"
#include "Library\StackScriptFunction.h"
#include "Library\RuntimeFunction.h"
#include "Library\JavascriptGeneratorFunction.h"
#include "Library\JavascriptTypedObjectSlotAccessorFunction.h"
#include "Library\JavascriptExternalFunction.h"
#include "Library\JavascriptWinRTFunction.h"
#include "Library\JavascriptRegExpConstructor.h"
#include "Library\JavascriptRegExpEnumerator.h"
#include "Library\BoundFunction.h"
#include "Library\JavascriptObject.h"
#include "Library\BuiltInFlags.h"
#include "Library\CharStringCache.h"
#include "..\static\base\JavascriptLibraryBase.h"
#include "Library\JavascriptLibrary.h"

#ifdef SIMD_JS_ENABLED
// SIMD libs
#include "Library\SIMDFloat32x4Lib.h"
#include "Library\SIMDFloat64x2Lib.h"
#include "Library\SIMDInt32x4Lib.h"
// SIMD operations
#include "Library\SIMDFloat32x4Operation.h"
#include "Library\SIMDInt32x4Operation.h"
#include "Library\SIMDFloat64x2Operation.h"
#endif

#include "Library\GlobalObject.h"
#include "Library\JavascriptGenerator.h"
#include "Language\DiagHelperMethodWrapper.h"
#include "Language\JavascriptMathOperators.h"
#include "Math\JavascriptSSE2MathOperators.h"
#include "Language\JavascriptExceptionOperators.h"
#include "Language\EHBailoutData.h"
#include "Language\JavascriptOperators.h"
#include "Library\TaggedInt.h"
#include "Library\SubString.h"
#include "Library\UriHelper.h"
#include "Library\HiResTimer.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "Library\JavascriptSymbol.h"
#include "Library\JavascriptSymbolObject.h"
#include "Library\JavascriptProxy.h"
#include "Library\IteratorObjectEnumerator.h"
#include "Library\JavascriptReflect.h"
#include "Library\JavascriptEnumeratorIterator.h"
#include "Library\JavascriptWinRTDate.h"
#include "Library\JavascriptVariantDate.h"
#include "Library\JavascriptPromise.h"
#include "Library\MathLibrary.h"
#include "Library\RegexHelper.h"
#include "Library\JSONStack.h"
#include "Library\JSON.h"
#include "Library\ObjectPrototypeObject.h"
#include "Library\ProfileString.h"
#include "Library\SingleCharString.h"
#include "Library\ThrowErrorObject.h"
#include "Library\WindowsGlobalizationAdapter.h"
#include "Library\WindowsFoundationAdapter.h"
#include "Library\EngineInterfaceObject.h"
#include "Library\Debug.h"
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#include "Library\ScriptMemoryDumper.h"
#endif
#include "Library\DOMFastPathInfo.h"

#ifdef TEST_LOG
#include "Library\HostLogger.h"
#endif

#ifdef MUTATORS
#include "Language\SourceMutator.h"
#endif

#ifdef _M_X64
#include "Language\amd64\stackframe.h"
#endif
#include "Language\DiagProbe.h"
#include "Language\ProbeContainer.h"
#include "Library\Entropy.h"
#include "Language\PropertyRecord.h"
#include "Library\threadservicewrapper.h"
#include "Library\StackProber.h"
#include "Library\Telemetry.h"
#include "Telemetry\ScriptContextTelemetry.h"
#include "Telemetry\ScriptEngineTelemetry.h"
#include "Library\ThreadContext.h"
#include "Library\ThreadContextTLSEntry.h"
#include "Library\ThreadBoundThreadContextManager.h"
#include "Library\EtwTrace.h"
#include "Library\AsyncDebug.h"

#ifdef VTUNE_PROFILING
#ifdef CDECL
#define ORIGINAL_CDECL CDECL
#undef CDECL
#endif
#include "..\..\tools\external\inc\jitProfiling.h"
#ifdef ORIGINAL_CDECL
#undef CDECL
#endif
#define CDECL ORIGINAL_CDECL
#endif

#include "Language\EvalMapRecord.h"
#include "Language\JavascriptConversion.h"
#include "Language\diagobjectmodel.h"
#include "Language\ScriptContextProfiler.h"
#include "Language\ScriptContextOptimizationOverrideInfo.h"
#include "..\static\base\scriptContextbase.h"
#include "Language\ScriptContext.h"

#include "Language\JavascriptFunctionArgIndex.h"
#include "Language\JavascriptStackWalker.h"
#include "ByteCode\ByteCodeDumper.h"
#include "ByteCode\ByteCodeReader.h"
#include "ByteCode\ByteCodeWriter.h"
#include "ByteCode\AsmJsByteCodeWriter.h"
#include "ByteCode\Symbol.h"
#include "ByteCode\Scope.h"
#include "ByteCode\FuncInfo.h"
#include "ByteCode\ByteCodeGenerator.h"
#include "ByteCode\AsmJSByteCodeDumper.h"
#include "ByteCode\ScopeInfo.h"
#include "ByteCode\StatementReader.h"

#include "Language\AsmJsTypes.h"
#include "Language\AsmJsUtils.h"
#include "Language\AsmJsLink.h"
#include "Language\AsmJsByteCodeGenerator.h"
#include "Language\AsmJsModule.h"
#include "Language\AsmJs.h"
#ifdef ASMJS_PLAT
#include "Language\AsmJSJitTemplate.h"
#include "Language\AsmJSEncoder.h"
#include "Language\AsmJSCodeGenerator.h"
#endif

#include "Language\InterpreterStackFrame.h"
#include "Language\DiagStackFrame.h"
#include "Language\LeaveScriptObject.h"
#include "Language\ByteCodeSerializer.h"
#include "Language\ProfilingHelpers.h"

#include "activdbg100.h"
#include "Language\MutationBreakpoint.h"

#ifdef DYNAMIC_PROFILE_STORAGE
#include "Language\DynamicProfileStorage.h"
#endif

#include "Library\SameValueComparer.h"
#include "Library\MapOrSetDataList.h"
#include "Library\JavascriptMap.h"
#include "Library\JavascriptMapIterator.h"
#include "Library\JavascriptSet.h"
#include "Library\JavascriptSetIterator.h"
#include "Library\JavascriptWeakMap.h"
#include "Library\JavascriptWeakSet.h"

//
// .inl files
//

#include "commoninl.h"

#include "Types\RecyclableObject.inl"
#include "ByteCode\ByteBlock.inl"
#include "Library\JavascriptString.inl"
#include "Library\JavascriptStringIterator.inl"
#include "Library\ConcatString.inl"
#include "Language\JavascriptConversion.inl"
#include "Types\FunctionBody.inl"
#include "Library\JavascriptBoolean.inl"
#include "Library\JavascriptVariantDate.inl"
#include "Types\DynamicObject.inl"
#include "Library\GlobalObject.inl"
#include "Library\JavascriptArray.inl"
#include "Library\JavascriptArrayIterator.inl"
#include "Library\SparseArraySegment.inl"
#include "Library\JavascriptFunction.inl"
#include "Library\JavascriptNumber.inl"
#include "Library\JavascriptRegularExpression.inl"
#include "Library\JavascriptRegularExpressionResult.inl"
#include "Library\RegexHelper.inl"
#include "Types\DynamicType.inl"
#include "Types\TypeHandler.inl"
#include "Library\JavascriptLibrary.inl"
#include "Math\JavascriptSSE2MathOperators.inl"
#include "Language\JavascriptMathOperators.inl"
#include "Language\InlineCache.inl"
#include "Language\InlineCachePointerArray.inl"
#include "Language\CacheOperators.inl"
#include "Language\JavascriptOperators.inl"
#include "Language\JavascriptExceptionOperators.inl"
#include "Library\TaggedInt.inl"
#include "Language\ScriptContext.inl"
#include "ByteCode\ByteCodeReader.inl"
#include "Language\InterpreterStackFrame.inl"
#include "Library\JavascriptDate.inl"
#include "Library\JavascriptWinRTDate.inl"
#include "Language\JavascriptObject.inl"
#include "Library\JavascriptPixelArray.inl"
#include "Library\JavascriptMap.inl"
#include "Library\JavascriptMapIterator.inl"
#include "Library\JavascriptSet.inl"
#include "Library\JavascriptSetIterator.inl"
#include "Library\JavascriptWeakMap.inl"
#include "Library\JavascriptWeakSet.inl"
#include "Library\JavascriptSymbol.inl"
#include "Language\DiagHelperMethodWrapper.inl"
#include "Library\CharStringCache.inl"





#if _M_IX86 || _M_X64
// This CRT routines skip some special condition checks for FPU state
// These routines are not expected to be called outside of the CRT, so using these should
// be re-evaluated when upgrading toolsets.
// Mark them explicitly as dllimport to workaround VC bug (Dev11:909888)
// This won't work if statically linking to the CRT.
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_acos(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_asin(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_atan(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_atan2(double,double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_cos(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_exp(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_pow(double,double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_log(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_log10(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_sin(double);
extern "C" double __declspec(dllimport) __cdecl __libm_sse2_tan(double);

#endif
