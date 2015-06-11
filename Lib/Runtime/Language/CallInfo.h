//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    enum CallFlags : unsigned
    {
        CallFlags_None    = 0,
        CallFlags_New     = 1,
        CallFlags_Value   = 2,
        CallFlags_Eval    = 4,
        CallFlags_CallEval = 8,
        CallFlags_NotUsed = 0x10,
        CallFlags_Wrapped = 0x20,
        CallFlags_CallPut = 0x40,
        CallFlags_InternalFrame = 0x80
    };

    struct CallInfo
    {
        /*
         * Removed the copy constructor because it forced the 64 bit compiler
         * to pass this object by reference. Interpreter stack setup code expects
         * CallInfo to be passed by value.
         */
        CallInfo(ushort count)
            : Flags(CallFlags_None)
            , Count(count)
#if defined(_M_X64_OR_ARM64)
            , unused(0)
#endif
        {
        }

        CallInfo(CallFlags flags, ushort count)
            : Flags(flags)
            , Count(count)
#if defined(_M_X64_OR_ARM64)
            , unused(0)
#endif
        {
        }

        // Assumes big-endian layout
        // If the size of the count is changed, change should happen at following places also
        //  - scriptdirect.idl
        //  - LowererMDArch::LoadInputParamCount
        //
        unsigned  Count : 24;
        CallFlags Flags : 8;
#if defined(_M_X64_OR_ARM64)
        unsigned unused : 32;
#endif

    public:
        static const ushort ksizeofCount;
        static const ushort ksizeofCallFlags;
        static const uint kMaxCountArgs;
    };

    struct InlineeCallInfo
    {
        // Assumes big-endian layout. 
        size_t Count: 4;
#if defined(_M_X64_OR_ARM64)
        size_t InlineeStartOffset: 60;
#else
        size_t InlineeStartOffset: 28;
#endif
        static size_t const MaxInlineeArgoutCount = 0xF;

        static bool Encode(Js::Var &callInfo, size_t count, size_t offset)
        {
#if defined(_M_X64_OR_ARM64)
            const size_t offsetMask = 0x0FFFFFFFFFFFFFFF;
            const size_t countMask  = 0x000000000000000F;
#else
            const size_t offsetMask = 0x0FFFFFFF;
            const size_t countMask  = 0x0000000F;
#endif
            if (count != (count & countMask))
            {
                return false;
            }

            if (offset != (offset & offsetMask))
            {
                return false;
            }

            callInfo = (Js::Var)((offset << 4) | count);

            return true;
        }

        void Clear()
        {
            this->Count = 0;
            this->InlineeStartOffset = 0;
        }
    };
}
