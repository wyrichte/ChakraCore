//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "Backend.h"

namespace IR
{
    
const void * const JnHelperMethodAddresses[] =
{
#define HELPERCALL(Name, Address, Attributes) static_cast<void *>(Address),
// Because of order-of-initialization problems with the vtable address static field
// and this array, we're going to have to fill these in as we go along.
#include "JnHelperMethodList.h"
#undef HELPERCALL

    NULL
};

#if defined(_M_IX86) 
const void * const JnHelperMethodAddresses_SSE2[] =
{
#define SSE2MATH
#define HELPERCALL(Name, Address, Attributes) static_cast<void *>(Address),
// Because of order-of-initialization problems with the vtable address static field
// and this array, we're going to have to fill these in as we go along.
#include "JnHelperMethodList.h"
#undef HELPERCALL
#undef SSE2MATH
    NULL
};

const void * const*GetHelperMethods()
{
    if (AutoSystemInfo::Data.SSE2Available())
    {
        return JnHelperMethodAddresses_SSE2;
    }
    return JnHelperMethodAddresses;
}
static const void * const * pJnHelperMethodAddresses = GetHelperMethods();
#else

const void *const*GetHelperMethods()
{
    return JnHelperMethodAddresses;
}

static const void *const* pJnHelperMethodAddresses = JnHelperMethodAddresses;
#endif

static void const* helperMethodWrappers[] = {
    &Js::HelperMethodWrapper0,
    &Js::HelperMethodWrapper1,
    &Js::HelperMethodWrapper2,
    &Js::HelperMethodWrapper3,
    &Js::HelperMethodWrapper4,
    &Js::HelperMethodWrapper5,
    &Js::HelperMethodWrapper6,
    &Js::HelperMethodWrapper7,
    &Js::HelperMethodWrapper8,
    &Js::HelperMethodWrapper9,
    &Js::HelperMethodWrapper10,
    &Js::HelperMethodWrapper11,
    &Js::HelperMethodWrapper12,
    &Js::HelperMethodWrapper13,
    &Js::HelperMethodWrapper14,
    &Js::HelperMethodWrapper15,
    &Js::HelperMethodWrapper16,
};

///----------------------------------------------------------------------------
///
/// GetMethodAddress
///     
///     returns the memory address of the helperMethod, 
///     which can the address of debugger wrapper that intercept the original helper.
///
///----------------------------------------------------------------------------
void const* 
GetMethodAddress(IR::HelperCallOpnd* opnd)
{
    Assert(opnd);

#if defined(_M_ARM32_OR_ARM64)
#define LowererMDFinal LowererMD
#else
#define LowererMDFinal LowererMDArch
#endif

    CompileAssert(_countof(helperMethodWrappers) == LowererMDFinal::MaxArgumentsToHelper + 1);

    if (opnd->IsDiagHelperCallOpnd())
    {
        // Note: all arguments are already loaded for the original helper. Here we just return the address.
        IR::DiagHelperCallOpnd* diagOpnd = (IR::DiagHelperCallOpnd*)opnd;

        if (0 <= diagOpnd->m_argCount && diagOpnd->m_argCount <= LowererMDFinal::MaxArgumentsToHelper)
        {
            return helperMethodWrappers[diagOpnd->m_argCount];
        }
        else
        {
            AssertMsg(FALSE, "Unsupported arg count (need to implement).");
        }
    }

    return GetMethodOriginalAddress(opnd->m_fnHelper);
}

///----------------------------------------------------------------------------
///
/// GetMethodOriginalAddress
///     
///     returns the memory address of the the helperMethod, 
///     this one is never the intercepted by debugger helper.
///
///----------------------------------------------------------------------------
void const * GetMethodOriginalAddress(JnHelperMethod helperMethod)
{
    return pJnHelperMethodAddresses[static_cast<WORD>(helperMethod)];
}


#if DBG_DUMP || defined(ENABLE_IR_VIEWER)



wchar_t const * const JnHelperMethodNames[] =
{
#define HELPERCALL(Name, Address, Attributes) L"" STRINGIZEW(Name) L"",
#include "JnHelperMethodList.h"
#undef HELPERCALL

    NULL
};

///----------------------------------------------------------------------------
///
/// GetMethodName
///     
///     returns the string representing the name of the helperMethod.
///
///----------------------------------------------------------------------------

wchar_t const*
GetMethodName(JnHelperMethod helperMethod)
{
    return JnHelperMethodNames[static_cast<WORD>(helperMethod)];
}

#endif  //#if DBG_DUMP


} //namespace IR

#if DBG_DUMP || defined(ENABLE_IR_VIEWER)
const wchar_t *GetVtableName(VTableValue value)
{
    switch (value)
    {
#if !defined(_M_X64)
    case VtableJavascriptNumber:
        return L"vtable JavascriptNumber";
        break;
#endif
    case VtableDynamicObject:
        return L"vtable DynamicObject";
        break;
    case VtableInvalid:
        return L"vtable Invalid";
        break;
    case VtablePropertyString:
        return L"vtable PropertyString";
        break;
    case VtableJavascriptBoolean:
        return L"vtable JavascriptBoolean";
        break;
    case VtableSmallDynamicObjectSnapshotEnumeratorWPCache:
        return L"vtable SmallDynamicObjectSnapshotEnumeratorWPCache";
        break;
    case VtableJavascriptArray:
        return L"vtable JavascriptArray";
        break;
    case VtableInt8Array:
        return L"vtable Int8Array";
        break;
    case VtableUint8Array:
        return L"vtable Uint8Array";
        break;
    case VtableUint8ClampedArray:
        return L"vtable Uint8ClampedArray";
        break;
    case VtableInt16Array:
        return L"vtable Int16Array";
        break;
    case VtableUint16Array:
        return L"vtable Uint16Array";
        break;
    case VtableInt32Array:
        return L"vtable Int32Array";
        break;
    case VtableUint32Array:
        return L"vtable Uint32Array";
        break;
    case VtableFloat32Array:
        return L"vtable Float32Array";
        break;
    case VtableFloat64Array:
        return L"vtable Float64Array";
        break;
    case VtableJavascriptPixelArray:
        return L"vtable JavascriptPixelArray";
        break;
    case VtableInt64Array:
        return L"vtable Int64Array";
        break;
    case VtableUint64Array:
        return L"vtable Uint64Array";
        break;
    case VtableInt8VirtualArray:
        return L"vtable Int8VirtualArray";
        break;
    case VtableUint8VirtualArray:
        return L"vtable Uint8VirtualArray";
        break;
    case VtableUint8ClampedVirtualArray:
        return L"vtable Uint8ClampedVirtualArray";
        break;
    case VtableInt16VirtualArray:
        return L"vtable Int16VirtualArray";
        break;
    case VtableUint16VirtualArray:
        return L"vtable Uint16VirtualArray";
        break;
    case VtableInt32VirtualArray:
        return L"vtable Int32VirtualArray";
        break;
    case VtableUint32VirtualArray:
        return L"vtable Uint32VirtualArray";
        break;
    case VtableFloat32VirtualArray:
        return L"vtable Float32VirtualArray";
        break;
    case VtableFloat64VirtualArray:
        return L"vtable Float64VirtualArray";
        break;
    case VtableBoolArray:
        return L"vtable BoolArray";
        break;
    case VtableCharArray:
        return L"vtable CharArray";
        break;
    case VtableNativeIntArray:
        return L"vtable NativeIntArray";
        break;
    case VtableNativeFloatArray:
        return L"vtable NativeFloatArray";
        break;
    case VtableJavascriptNativeIntArray:
        return L"vtable JavascriptNativeIntArray";
        break;
    case VtableJavascriptRegExp:
        return L"vtable JavascriptRegExp";
        break;
    case VtableStackScriptFunction:
        return L"vtable StackScriptFunction";
        break;
    case VtableConcatStringMulti:
        return L"vtable ConcatStringMulti";
        break;
    case VtableCompoundString:
        return L"vtable CompoundString";
        break;
    default:
        Assert(false);
        break;
    }

    return L"vtable unknown";
}
#endif

namespace HelperMethodAttributes
{

// Position: same as in JnHelperMethod enum.
// Value: one or more of OR'ed HelperMethodAttribute values.
static const BYTE JnHelperMethodAttributes[] =
{
#define HELPERCALL(Name, Address, Attributes) Attributes,
#include "JnHelperMethodList.h"
#undef HELPERCALL
};

// Returns true if the helper can throw non-OOM / non-SO exception.
bool CanThrow(IR::JnHelperMethod helper)
{
    return (JnHelperMethodAttributes[helper] & AttrCanThrow) != 0;
}

bool IsInVariant(IR::JnHelperMethod helper)
{
    return (JnHelperMethodAttributes[helper] & AttrInVariant) != 0;
}

} //namespace HelperMethodAttributes
