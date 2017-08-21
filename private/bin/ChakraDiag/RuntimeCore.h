//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

// TODO: consider moving this file under lib\runtime and runtime.h including this file & removing definitions in runtime.h which are defined here.

// Core definitions from Runtime used in DAC

// Forwards
class SourceContextInfo;                // Used in Utf8SourceInfo.h
class ActiveScriptProfilerHeapEnum;     // Used in ArgumentsObject.h
namespace Js
{
    // TODO: consider extracting and pulling enums (instead of forward), as changing their underlying type may break us.
    class DebugDocument;                    // Used by Utf8SourceInfo.h
    class FunctionProxy;
    class ParseableFunctionInfo;            // Used by Utf8SourceInfo.h
    class FunctionBody;                     // Used by Utf8SourceInfo.h
    struct Utf8SourceInfo;                  // Used by FunctionBody.h
    class RecyclableObject;                 // Used by RuntimeCommon.h
    struct CallInfo;                        // Used by RuntimeCommon.h
    struct InlineCache;                     // Used by RecyclableObject.h
    class PolymorphicInlineCache;
    class PropertyRecord;                   // Used by ThreadContext.h
    class CodeGenNumberChunk;               // Used by FunctionBody.h
    class JavascriptLibrary;                // Used by FunctionBody.h
    class ByteBlock;                        // Used by FunctionBody.h
    struct InterpreterStackFrame;           // Used by FunctionBody.h
    class DynamicObject;                    // Used by FunctionBody.h
    class ArrayObject;
    class DynamicProfileInfo;               // Used by FunctionBody.h
    struct PolymorphicCallSiteInfo;         // Used by FunctionBody.h
    enum ImplicitCallFlags : BYTE;          // Used by FunctionBody.h
    struct LoopHeader;                      // Used by FunctionBody.h
    enum FldInfoFlags : BYTE;               // Used by InterpreterStackFrame.h
    enum CacheType;                         // Used by InterpreterStackFrame.h
    class Amd64StackFrame;                  // Used by InterpreterStackFrame.h
    enum class OpCodeAsmJs : unsigned short; // Used by FunctionBody.h
    struct ConstructorCache;                // Used by FunctionBody.h
    struct JitTimeInlineCache;              // Used by FunctionBody.h
    class JavascriptStackWalker;            // Used by StackTraceArguments.h
    class JavascriptFunction;               // Used by JavascriptExceptionObject.h
    class JavascriptGeneratorFunction;      // Used by JavascriptLibrary.h
    class JavascriptAsyncFunction;          // Used by JavascriptLibrary.h
    class ScriptFunctionType;               // used by FunctionBody.h
    class ScriptFunction;                   // Used by InterpreterStackFrame.h
    class GeneratorVirtualScriptFunction;   // Used by JavascriptLibrary.h
    class StackScriptFunction;              // Used by InterpreterStackFrame.h
    struct PropertyRecordPointerComparer;   // Used by ThreadContext.h
    class CaseInvariantPropertyListWithHashCode;    // Used by ThreadContext.h
    struct PropertyDescriptor;
    class Type;                             // Used by ThreadContext.h
    class TempArenaAllocatorObject;         // Used by ThreadContext.h
    class TempGuestArenaAllocatorObject;    // Used by ThreadContext.h
    class JavascriptExceptionObject;        // Used by ThreadContext.h
    class CodeGenRecyclableData;            // Used by ThreadContext.h
    struct ScriptEntryExitRecord;           // Used by ThreadContext.h
    class DynamicType;                      // Used by ThreadContext.h
    class DynamicObject;
    class JavascriptString;                 // Used by ThreadContext.h
    class StringCopyInfo;
    class ProbeManager;                     // Used by ThreadContext.h
    class ProbeContainer;
    class PropertyString;                   // Used by ScriptContext.h
    class TypePath;                         // Used by ScriptContext.h
    class RegexPatternMruMap;               // Used by ScriptContext.h
    struct IDebugDocumentContext;           // Used by ScriptContext.h
    enum SideEffects;                       // Used by ScriptContext.h
    class StaticType;                       // Used by JavascriptNumber.h
    class CodeGenNumberAllocator;           // Used by JavascriptNumber.h
    class JavascriptNumber;                 // Used by CodeGenNumberallocator.h
    class RootObjectBase;                   // Used by FunctionBody.h
    class GlobalObject;                     // Used by ScriptContext.h
    class ModuleRoot;                       // Used by ScriptContext.h
    class SourceMutator;                    // Used by ScriptContext.h
    class ScriptContext;                    // Used by ProbeContainer.h
    struct NativeModule;
    struct HaltCallback;                    // Used by ProbeContainer.h
    struct IRemoteDebugApplication110;      // Used by ProbeContainer.h
    struct InterpreterHaltState;            // Used by ProbeContainer.h
    struct Probe;                           // Used by ProbeContainer.h
    struct ByteCodeReader;                  // Used by ProbeContainer.h
    struct AsmJsCallStackLayout;            // Used by InterpreterStackFrame.h
    class JavascriptCallStackLayout;        // Used by InterpreterStackFrame.h
    struct PropertyIdArray;                 // Used by InterpreterStackFrame.h
    enum class DynamicObjectFlags : unsigned short; // Used by DynamicObject.h
    class JavascriptArray;                  // Used by DynamicObject.h
    class JavascriptNativeIntArray;         // Used by JavascriptLibrary.h
    class JavascriptCopyOnAccessNativeIntArray;         // Used by JavascriptLibrary.h
    template<typename T> class SparseArraySegment;               // Used by JavascriptLibrary.h
    class JavascriptNativeFloatArray;       // Used by JavascriptLibrary.h
    template<typename T> struct AuxArray;   // Used by JavascriptArray.h
    struct VarArrayVarCount;                // used by ByteCodeReader.h
    struct FrameDisplay;                    // Used by CrossSite.h
    struct ConstructorCache;                // Used by JavascriptFunction.h
    class ActivationObjectEx;               // Used by JavascriptFunction.h
    class TypePropertyCache;                // Used by Type.h
    class ES5Array;                         // Used by TypeHandler.h
    class DeferredTypeHandlerBase;          // Used by TypeHandler.h
    class IndexPropertyDescriptor;          // Used by SimpleTypeHandler.h
    class JavascriptRegExp;                     // Used by JavascriptLibrary.h
    class JavascriptRegExpConstructor;      // Used by JavascriptLibrary.h
    class JavascriptBoolean;                // Used by JavascriptLibrary.h
    class JavascriptEnumerator;             // Used by JavascriptLibrary.h
    class PropertyStringCacheMap;           // Used by JavascriptLibrary.h
    class JavascriptExternalFunction;       // Used by JavascriptLibrary.h
    class HeapArgumentsObject;              // Used by JavascriptLibrary.h
    class ArrayBuffer;                      // Used by JavascriptLibrary.h
    class DataView;                         // Used by JavascriptLibrary.h
    class JavascriptDate;                   // Used by JavascriptLibrary.h
    class JavascriptMap;                    // Used by JavascriptLibrary.h
    class JavascriptSet;                    // Used by JavascriptLibrary.h
    class JavascriptWeakMap;                // Used by JavascriptLibrary.h
    class JavascriptWeakSet;                // Used by JavascriptLibrary.h
    class JavascriptError;                  // Used by JavascriptLibrary.h
    class JavascriptVariantDate;            // Used by JavascriptLibrary.h
    class JavascriptBooleanObject;          // Used by JavascriptLibrary.h
    class JavascriptNumberObject;           // Used by JavascriptLibrary.h

    // SIMD
    class JavascriptSIMDType;               // Used by JavascriptLibrary.h
    class SIMDFloat32x4Lib;                 // Used by JavascriptLibrary.h
    class JavascriptSIMDFloat32x4;          // Used by JavascriptLibrary.h
    class SIMDFloat64x2Lib;                 // Used by JavascriptLibrary.h
    class JavascriptSIMDFloat64x2;          // Used by JavascriptLibrary.h
    class SIMDInt32x4Lib;                   // Used by JavascriptLibrary.h
    class JavascriptSIMDInt32x4;            // Used by JavascriptLibrary.h
    class SIMDInt16x8Lib;                   // Used by JavascriptLibrary.h
    class JavascriptSIMDInt16x8;            // Used by JavascriptLibrary.h
    class SIMDInt8x16Lib;                   // Used by JavascriptLibrary.h
    class JavascriptSIMDInt8x16;            // Used by JavascriptLibrary.h
    class SIMDUint32x4Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDUint32x4;           // Used by JavascriptLibrary.h
    class SIMDUint16x8Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDUint16x8;           // Used by JavascriptLibrary.h
    class SIMDUint8x16Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDUint8x16;           // Used by JavascriptLibrary.h
    class SIMDBool32x4Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDBool32x4;           // Used by JavascriptLibrary.h
    class SIMDBool16x8Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDBool16x8;           // Used by JavascriptLibrary.h
    class SIMDBool8x16Lib;                  // Used by JavascriptLibrary.h
    class JavascriptSIMDBool8x16;           // Used by JavascriptLibrary.h

    class JavascriptSIMDObject;             // Used by JavascriptLibrary.h
    class JavascriptStringObject;           // Used by JavascriptLibrary.h
    class ObjectPrototypeObject;            // Used by JavascriptLibrary.h
    class JavascriptSymbol;                 // Used by JavascriptLibrary.h
    class JavascriptSymbolObject;           // Used by JavascriptLibrary.h
    class JavascriptArrayIterator;          // Used by JavascriptLibrary.h
    enum class JavascriptArrayIteratorKind; // Used by JavascriptLibrary.h
    class JavascriptMapIterator;            // Used by JavascriptLibrary.h
    enum class JavascriptMapIteratorKind;   // Used by JavascriptLibrary.h
    class JavascriptSetIterator;            // Used by JavascriptLibrary.h
    enum class JavascriptSetIteratorKind;   // Used by JavascriptLibrary.h
    class JavascriptStringIterator;         // Used by JavascriptLibrary.h
    class JavascriptListIterator;           // Used by JavascriptLibrary.h
    class JavascriptPromise;                // Used by JavascriptLibrary.h
    class JavascriptPromiseCapability;      // Used by JavascriptLibrary.h
    class JavascriptPromiseReaction;        // Used by JavascriptLibrary.h
    class JavascriptPromiseAsyncSpawnExecutorFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseAsyncSpawnStepArgumentExecutorFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseCapabilitiesExecutorFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseResolveOrRejectFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseReactionTaskFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseResolveThenableTaskFunction; // Used by JavascriptLibrary.h
    class JavascriptPromiseAllResolveElementFunction; // Used by JavascriptLibrary.h
    struct JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper; // Used by JavascriptLibrary.h
    struct JavascriptPromiseResolveOrRejectFunctionAlreadyResolvedWrapper; // Used by JavascriptLibrary.h
    class JavascriptProxy;                  // used by JavascriptLibrary.h
    class JavascriptGenerator;              // Used by JavascriptLibrary.h
    class FunctionCodeGenJitTimeData;       // Used by NativeCodeGenerator.h
    class FunctionCodeGenRuntimeData;       // Used by NativeCodeGenerator.h
    class CodeGenRecyclableData;            // Used by NativeCodeGenerator.h
    class ScriptContextProfiler;            // Used by CodeGenAllocators.h
    class CustomEnumerator;                 // Used by ExternalObject.h
    class Lowerer;                          // Used by JavascriptString.h
    class ActivationObject;                 // Used by ArgumentsObject.h
    class ES5ArgumentsObjectEnumerator;     // Used by ArgumentsObject.h
    class JavascriptStaticEnumerator;       // Used by RecyclableObject.h
    enum class EnumeratorFlags : byte;      // Used by RecyclableObject.h
    struct ForInCache;                      // Used by RecyclableObject.h
    template <typename Key> struct SameValueZeroComparer;   // Used by JavascriptSet.h and JavascriptMap.h
    struct IsInstInlineCache;
    class FunctionInfo;
    class DetachedStateBase;                // Used by ArrayBuffer.h
    class CharClassifier;                   // Used by ScriptContext.h
    //////////////////////////////////////////////////////////////////////////
    // asm.js
    struct EmitExpressionInfo;
    namespace AsmJsLookupSource
    {
        enum Source;
    }
    namespace ArrayBufferView
    {
        enum ViewType : unsigned char;
    }
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
    class AsmJsMathFunction;
    class AsmJsMathConst;
    class AsmJsCodeGenerator;
    class AsmJsEncoder;
    struct MathBuiltin;
    struct ExclusiveContext;
    class AsmJsModuleCompiler;
    class AsmJSCompiler;
    class AsmJSByteCodeGenerator;
    //////////////////////////////////////////////////////////////////////////
    class EntryPointInfo;
    class PolymorphicInlineCacheInfo;
    class PropertyGuard;
    enum class OpCode : unsigned short;
    template <class TKey, class TValue, class SecondaryDictionary, class NestedKey>  class TwoLevelHashRecord;
    template <class T> struct FastEvalMapStringComparer;
    template <class Key, class Value, class EntryRecord, class TopLevelDictionary, class NestedKey> class TwoLevelHashDictionary;
} // namespace Js.
namespace UnifiedRegex
{
    struct RegexPattern;                    // Used by FunctionBody.h
    template <typename T> class StandardChars; // Used by ThreadContext.h
    class DebugWriter;                      // Used by ScriptContext.h
    class RegexStatsDatabase;               // Used by ScriptContext.h
    struct Program;
    struct TrigramAlphabet;                  // Used by ScriptContext.h
    struct RegexStacks;                      // Used by ScriptContext.h
} // Used by UnifiedRegex.
class SRCINFO;                              // Used by FunctionBody.h
class FunctionCodeGenRuntimeData;           // Used by FunctionBody.h
class DynamicProfileMutator;                // Used by ThreadContext.h
class CompileScriptException;               // Used by SctiptContext.h
class Parser;                               // Used by SctiptContext.h
class ScriptContextProfiler;                // Used by SctiptContext.h
class StringProfiler;                       // Used by SctiptContext.h
class ByteCodeGenerator;                    // Used by ScriptContext.h
class InterpreterThunkEmitter;              // Used by ScriptContext.h
struct IActiveScriptDataCache;              // Used by ScriptContext.h
struct JsFunctionCodeGen;                   // Used by NativeCodeGenerator.h
struct JsLoopBodyCodeGen;                   // Used by NativeCodeGenerator.h
class InliningDecider;                      // Used by NativeCodeGenerator.h
class JavascriptDispatch;
class StackSym;                             // Used by InlineeFrameInfo.h
class Func;                                 // Used by InlineeFrameInfo.h
struct InlinedFrameLayout;                  // Used by InlineeFrameInfo.h

typedef intptr_t  IntConstType;
typedef uintptr_t UIntConstType;
typedef double  FloatConstType;
struct LazyBailOutRecord;

namespace Projection
{
    class ProjectionContext;
    class EventProjectionHandler;
    class ProjectionMemoryInformation;
}

class WebAssemblyMemory;
namespace Wasm
{
    class WasmSignature;
}

// Indicate that we compile Runtime #include's under DAC
// Pull runtinme header dependencies
#include "Common.h"
#include "ParserCommon.h"
#include "RuntimeCommon.h"

#include "Debug/TTSupport.h"
#include "Debug/TTSerialize.h"

#include "ChakraPlatform.h"
#include "DelayLoadLibrary.h"             // Used by ThreadContext
#include "WindowsGlobalizationAdapter.h"  // Used by ThreadContext::windowsGlobalizationAdapter
#include "WindowsFoundationAdapter.h"     // Used by ThreadContext::windowsFoundationAdapter
#include "activdbg.h"
#include "activdbg100.h"
#include "SourceHolder.h"
#include "Utf8SourceInfo.h"
#include "JITClient.h"
#include "Constants.h"                  // Used by FunctionInfo::FunctionInfo
#include "CallInfo.h"                   // Used by Arguments.h
#include "ExecutionMode.h"              // Used by FunctionBody.h
#include "Arguments.h"                  // Used by RecyclableObject.h
#include "TypeId.h"                     // Used by ThreadContext::nextTypeId
#include "DetachedStateBase.h"
#include "RecyclableObject.h"           // Used by JavascriptNumber.h. Note: we actually use this one in remote proxies
#include "Type.h"
#include "CrossSite.h"                  // Used by CrossSiteObject.h -- can be worked around using extract to .inl, but is easy to pull
#include "CrossSiteObject.h"            // Used by DynamicObject: DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT
#include "BackEndAPI.h"                 // Used by NativeCodeGenerator.h
#include "DynamicObject.h"
#include "ArrayObject.h"
#include "TypeHandler.h"
#include "DynamicType.h"                // Needed by inspection
#include "InlineCache.h"                // Used by FunctionBody
#include "InlineCachePointerArray.h"    // Used by FunctionBody, PolymorphicInlineCacheInfo
#include "Base\ExpirableObject.h"
#include "..\Backend\NativeCodeData.h"  // Used by FunctionBody
#include "FunctionInfo.h"               // Used by FunctionBody : FunctionInfo
#include "..\Backend\IRType.h"
#include "InlineeFrameInfo.h"
#include "FunctionBody.h"               // Used by JavascriptExceptionContext::functionBody. Note: we actually use this one in remote proxies
#include "StackTraceArguments.h"        // Used by JavascriptExceptionContext::argumentTypes. Note: we actually use this one in remote proxies
#include "JavascriptExceptionContext.h" // Used by JavascriptExceptionObject::exceptionContext
#include "JavascriptExceptionObject.h"  // Used by ThreadContext::RecyclableData::soErrorObject
#include "Entropy.h"                    // Used by ThreadContext::entropy
#include "PropertyRecord.h"             // Used by TypePath.h has inline functions that use PropertyRecord, easy to pull
#include "TypePath.h"                   // Used by ScriptContext::objectLiteralCount is using TypePath::MaxPathTypeHandlerLength, easy to pull
#include "OpLayoutsCommon.h"
#include "OpLayouts.h"                  // Used by ScriptContext::byteCodeHistogram
#include "OpLayoutsAsmJs.h"
#include "PropertyDescriptor.h"         // Used by RecyclableObject.h
#include "JavascriptNumber.h"           // Used by LibraryCommon.h

// SIMD types
#include "JavascriptSimdType.h"
#include "JavascriptSIMDFloat32x4.h"
#include "JavascriptSIMDFloat64x2.h"
#include "JavascriptSIMDInt32x4.h"
#include "JavascriptSIMDInt16x8.h"
#include "JavascriptSIMDInt8x16.h"
#include "JavascriptSIMDUint32x4.h"
#include "JavascriptSIMDUint16x8.h"
#include "JavascriptSIMDUint8x16.h"
#include "JavascriptSIMDBool32x4.h"
#include "JavascriptSIMDBool16x8.h"
#include "JavascriptSIMDBool8x16.h"

#include "RecyclerFastAllocator.h"      // Used by LibraryCommon.h
#include "Base\SourceContextInfo.h"    // Used by ScriptContext::noContextSourceContextInfo
#include "Debug\DebuggingFlags.h"
#include "Debug\DiagProbe.h"            // Used by ThreadContext::Diagnostics
#include "Debug\DebugManager.h"         // Used by ThreadContext::DebugManager
#include "Debug\ProbeContainer.h"       // Used by ScriptContext::diagProbesContainer
#include "Debug\DebugContext.h"         // Used by ScriptContext::debugContext
#include "StackProber.h"                // Used by ThreadContextTLSEntry::prober
#include "ByteCodeReader.h"             // Used by InterpreterStackFrame::m_reader
#ifdef _M_X64
#include "amd64\stackframe.h"           // Used by InterpreterStackFrame::amd64ContextsManager
#endif
#include "TypeHandler.h"                // Used by SimpleTypeHandler : DynamicTypeHandler
#include "SimplePropertyDescriptor.h"   // Used by SimpleTypeHandler::descriptors
#include "SimpleTypeHandler.h"          // Used by JavascriptLibrary::SharedPrototypeTypeHandler & others
#include "MissingPropertyTypeHandler.h" // Used by JavascriptLibrary::MissingPropertyHolderTypeHandler
#include "DeferredTypeHandler.h"        // Used by JavascriptLibrary.h
#include "BuiltInFlags.h"               // Used by JavascriptLibrary.h
#include "QueuedFullJitWorkItem.h"      // Used by NativeCodeGenerator.h
#include "CodeGenWorkItemType.h"        // Used by NativeCodeGenerator.h
#include "ValueType.h"                  // Needed by DynamicProfileInfo.h
#include "DynamicProfileInfo.h"         // Needed by FunctionBody.inl, InterpreterStackFrame.h
#include "EHBailoutData.h"              // Needed by InterpreterStackFrame.h

// Pull runtinme headers that DAC actually uses which have not been pulled yet
#include "Type.h"
#include "DynamicType.h"                // Needed by inspection
#include "DynamicObject.h"
#include "JavascriptFunction.h"
#include "ScriptFunctionType.h"
#include "ScriptFunction.h"
#include "RuntimeFunction.h"
#include "BoundFunction.h"
#include "JavascriptExternalFunction.h" // Not really using this one, needed for JavascriptLibrary and is using JavascriptFunction which is using DynamicObject.
#include "CustomExternalIterator.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"
#include "CharStringCache.h"
#include "ExternalLibraryBase.h"
#include "JavascriptLibrarybase.h"
#include "JavascriptLibrary.h"

// SIMD libs
#include "SIMDFloat32x4Lib.h"
#include "SIMDFloat64x2Lib.h"
#include "SIMDInt32x4Lib.h"
#include "SIMDInt16x8Lib.h"
#include "SIMDInt8x16Lib.h"
#include "SIMDUint32x4Lib.h"
#include "SIMDUint16x8Lib.h"
#include "SIMDUint8x16Lib.h"
#include "SIMDBool32x4Lib.h"
#include "SIMDBool16x8Lib.h"
#include "SIMDBool8x16Lib.h"

#include "MathLibrary.h"

#ifdef ENABLE_BASIC_TELEMETRY
#include "DirectCall.h"
#include "LanguageTelemetry.h"
#else
#define CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(builtin)
#define CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(feature, m_scriptContext)
#endif

#include "ThreadContextInfo.h"
#include "ThreadContext.h"
#include "ThreadContextTLSEntry.h"
#include "evalmaprecord.h"
#include "ScriptContextOptimizationOverrideInfo.h"
#include "JavascriptError.h"
#include "ScriptContextBase.h"
class ScriptContextTelemetry;
#include "ScriptContextInfo.h"
#include "ScriptContext.h"
#include "InterpreterStackFrame.h"
#include "CodeGenAllocators.h"
#include "EmitBuffer.h"
#include "NativeCodeGenerator.h"
#include "JavascriptFunctionArgIndex.h"
#include "ByteBlock.h"

#include "TaggedInt.h"
#include "JavascriptConversion.h"       // Needed by TaggedInt.inl
#include "TaggedInt.inl"                // Used by inspection
#include "StaticType.h"                 // Needed by JavascriptBoolean.h
#include "JavascriptBoolean.h"          // Used for DAC
#include "NullTypeHandler.h"
#include "PathTypeHandler.h"
#include "SimpleDictionaryPropertyDescriptor.h"
#include "SimpleDictionaryUnorderedTypeHandler.h"
#include "PropertyIndexRanges.h"
#include "SimpleDictionaryTypeHandler.h"
#include "DictionaryPropertyDescriptor.h"
#include "DictionaryTypeHandler.h"
#include "JavascriptTypedNumber.h"
#include "JavascriptBooleanObject.h"
#include "JavascriptNumberObject.h"
#include "JavascriptSimdObject.h"

#include "..\..\..\private\lib\staticlib\base\MockExternalObject.h"             // Needed by CustomExternalType.h
#include "ExternalObject.h"
#include "CustomExternalType.h"         // Needed by mshtmldac
#include "JavascriptString.h"           // Needed by JavascriptString DAC
#define IsJsDiag
#include "StringCopyInfo.h"
#include "StringCopyInfo.cpp"
#undef IsJsDiag
#include "LiteralString.h"
#include "SingleCharString.h"
#include "PropertyString.h"
#include "SubString.h"
#include "ConcatString.h"
#define IsJsDiag
#include "CompoundString.h"
#include "CompoundString.cpp"
#undef IsJsDiag
#include "BufferStringBuilder.h"
#include "JavascriptStringObject.h"     // Needed by JavascriptStringObject DAC
#include "SparseArraySegment.h"         // Needed by JavascriptArray.h
#include "JavascriptArray.h"            // Needed by JavascriptArray DAC
#include "ES5ArrayTypeHandler.h"
#include "ES5Array.h"                   // Needed by ES5Array DAC
#include "ArrayBuffer.h"
#include "TypedArray.h"
#include "JavascriptSymbol.h"
#include "JavascriptSymbolObject.h"
#include "ArgumentsObject.h"
#include "ObjectPrototypeObject.h"
#include "weakreference.h"
#include "ProjectionObjectInstance.h"
#include "HostObjectBase.h"
#include "RootObjectBase.h"             // Needed by GlobalObject.h
#include "..\..\lib\runtime\library\GlobalObject.h" // There is another globalobject.h in publics
#include "JavascriptError.h"
#include "DateImplementation.h"
#include "JavascriptDate.h"
#include "JavascriptVariantDate.h"
#include "MapOrSetDataList.h"
#include "JavascriptMap.h"
#include "JavascriptSet.h"
#include "JavascriptWeakMap.h"
#include "JavascriptWeakSet.h"
#define IsJsDiag
#include "JavascriptProxy.h"
#undef IsJsDiag
#include "RegexFlags.h"                 // Needed by regexp.h
#include "Chars.h"
#include "CharMap.h"
#include "CharSet.h"
#include "RegexStats.h"
#include "CharTrie.h"
#include "TextbookBoyerMoore.h"
#include "CaseInsensitive.h"            // Needed by RegexRunTime.h and OctoquadIdentifier.h
#include "OctoquadIdentifier.h"
#include "RegexRunTime.h"               // Needed by RegexPattern.h
#include "RegexPattern.h"               // Needed by JavascriptRegularExpression.h
#include "JavascriptRegularExpression.h"
#include "JavascriptRegExpConstructor.h"    // Used by DAC.h
#include "JSONString.h"
#include "CharClassifier.h"
#include "WithScopeObject.h"
#include "BreakpointProbe.h"                // Used by DebugDocument.h
#include "DebugDocument.h"                  // Used by ScriptDebugDocument.h
#include "scrpting.h"                       // Used by ScriptDebugDocument.h
#include "ScriptDebugDocument.h"            // Used by DAC.h

#ifdef _M_X64_OR_ARM64
// TODO: Clean this warning up
#pragma warning(disable:4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
#endif
