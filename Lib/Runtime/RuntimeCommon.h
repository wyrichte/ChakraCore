//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// Runtime.h has both definitions and #include other runtime files.
// Definitions here are extracted definitions (not #include's) from Runtime.h that core Runtime .h's can be used without #include'ing full Runtime.h

namespace Js
{
    // Forwards
    class RecyclableObject;
    struct CallInfo;
    class PropertyRecord;
    class JavascriptString;
    struct FrameDisplay;
    class TypedArrayBase;

#if _M_IX86
#define unaligned
#elif _M_X64 || _M_ARM || _M_ARM64
#define unaligned __unaligned
#else
#error Must define alignment capabilities for processor
#endif

    typedef uint32 RegSlot;
    typedef uint16 ArgSlot;
    typedef uint16 PropertyIndex;
    typedef int32 BigPropertyIndex;
    typedef unsigned char PropertyAttributes;

    // Inline cache flags when property of the object is not writable
    enum InlineCacheFlags : char {
        InlineCacheNoFlags              = 0x0,
        InlineCacheGetterFlag           = 0x1,
        InlineCacheSetterFlag           = 0x2,
    };

    #define PropertyNone            0x00
    #define PropertyEnumerable      0x01
    #define PropertyConfigurable    0x02
    #define PropertyWritable        0x04
    #define PropertyDeleted         0x08
    #define PropertyLetConstGlobal  0x10
    #define PropertyDeclaredGlobal  0x20
    #define PropertyLet             0x40
    #define PropertyConst           0x80
    // No more flags will fit unless PropertyAttributes is bumped up to a short instead of char
    #define PropertyBuiltInMethodDefaults (PropertyConfigurable|PropertyWritable)
    #define PropertyDynamicTypeDefaults (PropertyConfigurable|PropertyWritable|PropertyEnumerable)
    #define PropertyLetDefaults   (PropertyEnumerable|PropertyConfigurable|PropertyWritable|PropertyLet)
    #define PropertyConstDefaults (PropertyEnumerable|PropertyConfigurable|PropertyConst)
    #define PropertyDeletedDefaults (PropertyDeleted | PropertyWritable | PropertyConfigurable)
    #define PropertyNoRedecl        (PropertyLet | PropertyConst)

    BEGIN_ENUM_UINT(InternalPropertyIds)
#define INTERNALPROPERTY(n) n,
#include "InternalPropertyList.h"
        Count,
    END_ENUM_UINT()

    inline BOOL IsInternalPropertyId(PropertyId propertyId)
    {
        return propertyId < InternalPropertyIds::Count;
    }

    BEGIN_ENUM_UINT(PropertyIds)
        _none = InternalPropertyIds::Count,
#define ENTRY_INTERNAL_SYMBOL(n) n,
#define ENTRY_SYMBOL(n, d) n,
#define ENTRY(n) n,
#define ENTRY2(n, s) n,
#include "Library\JnDirectFields.h"
        _countJSOnlyProperty,
    END_ENUM_UINT()

    inline BOOL IsBuiltInPropertyId(PropertyId propertyId)
    {
        return propertyId < TotalNumberOfBuiltInProperties;
    }

    #define PropertyTypesNone                      0x00
    #define PropertyTypesReserved                  0x01  // This bit is always to prevent the DWORD in DynamticTypeHandler looking like a pointer.
    #define PropertyTypesWritableDataOnly          0x10  // Indicates that a type handler has only writable data properties
                                                         // (no accessors or non-writable properties)
    #define PropertyTypesWritableDataOnlyDetection 0x20  // Set on each call to DynamicTypeHandler::SetHasOnlyWritableDataProperties.
    #define PropertyTypesInlineSlotCapacityLocked  0x40  // Indicates that the inline slot capacity has been shrunk already and should't be touched again.
    #define PropertyTypesAll                       0x70
    typedef unsigned char PropertyTypes;                 // Holds flags that represent general information about the types of properties
                                                         // handled by a type handler.
    BEGIN_ENUM_UINT(JavascriptHint)
        None,                                   // no hint. use the default for that object
        HintString  = 0x00000001,               // 'string' hint in ToPrimitiveValue()
        HintNumber  = 0x00000002,               // 'number' hint
    END_ENUM_UINT()

    enum DescriptorFlags
    {
        None = 0x0,      // No data/accessor descriptor
        Accessor = 0x1,  // An accessor descriptor is present
        Data = 0x2,      // A data descriptor is present
        Writable = 0x4,  // Data descriptor is writable
        Const = 0x8,     // Data is const, meaning we throw on attempt to write to it
        Proxy = 0x10,    // data returned from proxy.
        WritableData = Data | Writable // Data descriptor is writable
    };

    BEGIN_ENUM_BYTE(BuiltinFunction)
#define LIBRARY_FUNCTION(obj, name, argc, flags) obj##_##name,
#include "LibraryFunction.h"
#undef LIBRARY_FUNCTION
        Count,
        None,
    END_ENUM_BYTE()

    typedef void * Var;
    typedef WriteBarrierPtr<void> WriteBarrierVar;

    typedef Var(__cdecl *JavascriptMethod)(RecyclableObject*, CallInfo, ...);


    const uintptr AtomTag_Object    = 0x0;

#if INT32VAR
    // The 49th bit is set in this representation
    const int32 VarTag_Shift        = 48;
    const uintptr AtomTag_IntPtr    = (((uintptr)0x1i64) << VarTag_Shift);
    const int32 AtomTag_Int32       = 0x0;     // lower 32-bits of a tagged integer
    const uintptr AtomTag           = 0x1;
    const int32 AtomTag_Multiply    = 1;
    const int32 AtomTag_Pair        = 0x00010001;  // Pair of tags
#else
    const uintptr AtomTag_IntPtr     = 0x1;
    const int32 AtomTag_Int32        = 0x1;    // lower 32-bits of a tagged integer
    const uintptr AtomTag            = 0x1;
    const int32 VarTag_Shift         = 1;
    const int32 AtomTag_Multiply     = 1 << VarTag_Shift;
#endif

#if FLOATVAR
    const uint64 FloatTag_Value      = 0xFFFCull << 48;
#endif
    template <bool IsPrototypeTemplate> class NullTypeHandler;

    template <typename TPropertyIndex, typename TMapKey, bool IsNotExtensibleSupported> class SimpleDictionaryTypeHandlerBase;
    template <typename TPropertyIndex, typename TMapKey, bool IsNotExtensibleSupported> class SimpleDictionaryUnorderedTypeHandler;
    template <typename TPropertyIndex> class DictionaryTypeHandlerBase;
    template <typename TPropertyIndex> class ES5ArrayTypeHandlerBase;

    typedef NullTypeHandler<false> NonProtoNullTypeHandler;
    typedef NullTypeHandler<true> ProtoNullTypeHandler;

    typedef SimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, false>    SimpleDictionaryTypeHandler;
    typedef SimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, true>     SimpleDictionaryTypeHandlerNotExtensible;
    typedef SimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, false> BigSimpleDictionaryTypeHandler;
    typedef SimpleDictionaryTypeHandlerBase<BigPropertyIndex, const PropertyRecord*, true>  BigSimpleDictionaryTypeHandlerNotExtensible;

    typedef SimpleDictionaryUnorderedTypeHandler<PropertyIndex, const PropertyRecord*, false>    SimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler;
    typedef SimpleDictionaryUnorderedTypeHandler<PropertyIndex, const PropertyRecord*, true>     SimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible;
    typedef SimpleDictionaryUnorderedTypeHandler<BigPropertyIndex, const PropertyRecord*, false> BigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler;
    typedef SimpleDictionaryUnorderedTypeHandler<BigPropertyIndex, const PropertyRecord*, true>  BigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible;

    typedef SimpleDictionaryUnorderedTypeHandler<PropertyIndex, JavascriptString*, false>    SimpleDictionaryUnorderedStringKeyedTypeHandler;
    typedef SimpleDictionaryUnorderedTypeHandler<PropertyIndex, JavascriptString*, true>     SimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible;
    typedef SimpleDictionaryUnorderedTypeHandler<BigPropertyIndex, JavascriptString*, false> BigSimpleDictionaryUnorderedStringKeyedTypeHandler;
    typedef SimpleDictionaryUnorderedTypeHandler<BigPropertyIndex, JavascriptString*, true>  BigSimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible;

    typedef DictionaryTypeHandlerBase<PropertyIndex> DictionaryTypeHandler;
    typedef DictionaryTypeHandlerBase<BigPropertyIndex> BigDictionaryTypeHandler;

    typedef ES5ArrayTypeHandlerBase<PropertyIndex> ES5ArrayTypeHandler;
    typedef ES5ArrayTypeHandlerBase<BigPropertyIndex> BigES5ArrayTypeHandler;

    template <int N> class ConcatStringN;
    typedef ConcatStringN<2> ConcatStringN2;
    typedef ConcatStringN<4> ConcatStringN4;
    typedef ConcatStringN<6> ConcatStringN6;
    typedef ConcatStringN<7> ConcatStringN7;

    template <wchar_t L, wchar_t R> class ConcatStringWrapping;
    typedef ConcatStringWrapping<L'[', L']'> ConcatStringWrappingSB;
    typedef ConcatStringWrapping<L'{', L'}'> ConcatStringWrappingB;
    typedef ConcatStringWrapping<L'"', L'"'> ConcatStringWrappingQ;

} // namespace Js.

namespace JSON
{
    class JSONParser;
}

//
// Below was moved from ByteCodeGenerator.h to share with jscript9diag.
//
#define REGSLOT_TO_VARREG(r) (r)
// To map between real reg number and const reg number, add 2 and negate.
// This way, 0xFFFF (no register) maps to itself, and 0xFFFF is never a valid number.
#define REGSLOT_TO_CONSTREG(r) ((Js::RegSlot)(0 - (r + 2)))
#define CONSTREG_TO_REGSLOT(r) ((Js::RegSlot)(0 - (r + 2)))

//
// Shared string literals
//
#define JS_DISPLAY_STRING_NAN           L"NaN"
#define JS_DISPLAY_STRING_DATE          L"Date"
#define JS_DISPLAY_STRING_INVALID_DATE  L"Invalid Date"
#define JS_DISPLAY_STRING_FUNCTION_ANONYMOUS        L"\012function() {\012    [native code]\012}\012"
#define JS_DISPLAY_STRING_FUNCTION_HEADER           L"function "
#define JS_DISPLAY_STRING_FUNCTION_BODY             L"() { [native code] }"

#define JS_DIAG_TYPE_JavascriptRegExp               L"Object, (Regular Expression)"

#define JS_DIAG_VALUE_JavascriptRegExpConstructor   L"{...}"
#define JS_DIAG_TYPE_JavascriptRegExpConstructor    L"Object, (RegExp constructor)"

#define JS_DEFAULT_CTOR_DISPLAY_STRING              L"constructor() {}"
#define JS_DEFAULT_EXTENDS_CTOR_DISPLAY_STRING      L"constructor(...args) { super(...args); }"

#ifdef SIMD_JS_ENABLED

#define SIMD_JS_FLAG Js::Configuration::Global.flags.Simdjs


#define SIMD_X 0
#define SIMD_Y 1
#define SIMD_Z 2
#define SIMD_W 3

struct _SIMDValue

{
    union{
        int     i32[4];
        float   f32[4];
        double  f64[2];
    };

    void SetValue(_SIMDValue value)
    {
        f64[SIMD_X] = value.f64[SIMD_X];
        f64[SIMD_Y] = value.f64[SIMD_Y];
    }
    void Zero()
    {
        f64[SIMD_X] = f64[SIMD_Y] = 0;
    }
    bool operator==(const _SIMDValue& r)
    {
        // don't compare f64/f32 because NaN bit patterns will not be considered equal.
        return (this->i32[SIMD_X] == r.i32[SIMD_X] &&
            this->i32[SIMD_Y] == r.i32[SIMD_Y] &&
            this->i32[SIMD_Z] == r.i32[SIMD_Z] &&
            this->i32[SIMD_W] == r.i32[SIMD_W]);
    }
    bool IsZero()
    {
        return (i32[SIMD_X] == 0 && i32[SIMD_Y] == 0 && i32[SIMD_Z] == 0 && i32[SIMD_W] == 0);
    }
};
typedef _SIMDValue SIMDValue;

// For dictionary use
template <>
struct DefaultComparer<_SIMDValue>
{
    __forceinline static bool Equals(_SIMDValue x, _SIMDValue y)
    {
        return x == y;
    }

    __forceinline static hash_t GetHashCode(_SIMDValue d)
    {
        return (hash_t)(d.i32[SIMD_X] ^ d.i32[SIMD_Y] ^ d.i32[SIMD_Z] ^ d.i32[SIMD_W]);
    }
};

#if _M_IX86 || _M_AMD64
struct _x86_SIMDValue
{
    union{
        _SIMDValue simdValue;
        __m128  m128_value;
        __m128d m128d_value;
        __m128i m128i_value;
    };

    static _x86_SIMDValue ToX86SIMDValue(const SIMDValue& val)
    {
        _x86_SIMDValue result;
        result.simdValue.i32[SIMD_X] = val.i32[SIMD_X];
        result.simdValue.i32[SIMD_Y] = val.i32[SIMD_Y];
        result.simdValue.i32[SIMD_Z] = val.i32[SIMD_Z];
        result.simdValue.i32[SIMD_W] = val.i32[SIMD_W];
        return result;
    }

    static SIMDValue ToSIMDValue(const _x86_SIMDValue& val)
    {
        SIMDValue result;
        result.i32[SIMD_X] = val.simdValue.i32[SIMD_X];
        result.i32[SIMD_Y] = val.simdValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = val.simdValue.i32[SIMD_Z];
        result.i32[SIMD_W] = val.simdValue.i32[SIMD_W];
        return result;
    }
};

// These global values are 16-byte aligned.
const _x86_SIMDValue X86_ABS_MASK_F4 = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const _x86_SIMDValue X86_ABS_MASK_I4 = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const _x86_SIMDValue X86_ABS_MASK_D2 = { 0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff };

const _x86_SIMDValue X86_NEG_MASK_F4 = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const _x86_SIMDValue X86_NEG_MASK_D2 = { 0x00000000, 0x80000000, 0x00000000, 0x80000000 };

const _x86_SIMDValue X86_ALL_ONES_F4 = { 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000 }; // {1.0, 1.0, 1.0, 1.0}
const _x86_SIMDValue X86_ALL_ONES_I4 = { 0x00000001, 0x00000001, 0x00000001, 0x00000001 }; // {1, 1, 1, 1}
const _x86_SIMDValue X86_ALL_ONES_D2 = { 0x00000000, 0x3ff00000, 0x00000000, 0x3ff00000 }; // {1.0, 1.0}

const _x86_SIMDValue X86_ALL_NEG_ONES = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

typedef _x86_SIMDValue X86SIMDValue;
CompileAssert(sizeof(X86SIMDValue) == 16);
#endif

typedef SIMDValue     AsmJsSIMDValue; // alias for asmjs
CompileAssert(sizeof(SIMDValue) == 16);

#endif // SIMD_JS_ENABLED
