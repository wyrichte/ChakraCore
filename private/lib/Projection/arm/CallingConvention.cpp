//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"

namespace ProjectionModel
{

// Returns val rounded up as necessary to be a multiple of alignment.
// In:
//   - val: the value to align.
//   - alignment: number of bytes to align to. Must be a power of 2.
// Out:
//   - returns the aligned value.
inline size_t ALIGN_UP(size_t val, int alignment)
{
    // alignment must be a power of 2 for this implementation to work (need modulo otherwise).
    Assert(0 == (alignment & (alignment - 1)));
    size_t result = (val + (alignment - 1)) & ~(alignment - 1);
    Assert(result >= val);      // check for overflow.
    return result;
}

inline void* ALIGN_UP(void* val, int alignment)
{
    return (void*)ALIGN_UP((size_t)val, alignment);
}

// Get abstract (ParameterLocation) location of next argument and update internal state which will be used for next argument.
// Updates internal state so that when you call it next time, you'll get location of next argument and not this one.
// In:
//   - the argument to get the location of.
// Out: 
//   - loc receives (is filled with) the location.
void CallingConventionHelper::GetNextParameterLocation(RtABIPARAMETER argument, ParameterLocation *loc)
{
    Assert(argument);
    Assert(argument->type);
    Assert(loc);

    if (!ConcreteType::Is(argument->type))
    {
        AssertMsg(FALSE, "This scenario is not implemented: !ConcreteType");
        Js::Throw::FatalInternalError();
    }
    RtCONCRETETYPE concreteType = ConcreteType::From(argument->type);

    // Note: for tcMissingNamedType, the data is unreliable/garbage, we'll report the error later 
    //       when the arg is actually used (like WriteInType), here assuming 4-bytes/non floating point/non 64 bit.
    bool isMissingType = argument->type->typeCode == tcMissingNamedType;
    int byteCount = isMissingType ? 4 : argument->GetSizeOnStack();
    bool isFloatingPoint = isMissingType ? false : CallingConventionHelper::IsFloatingPoint(concreteType);
    bool is64BitAlignRequired = isMissingType ? false : CallingConventionHelper::Is64BitAlignRequired(concreteType);

    return GetNextParameterLocation(byteCount, isFloatingPoint, is64BitAlignRequired, loc);
}

// Get abstract location of next argument and update internal state which will be used for next argument.
// In: 
//   - byteCount: size of argument in bytes, aka "size on stack".
//   - isFloatingPoint: whether we should use VFP registers for the argument.
//   - is64BitAlignRequired: whether to align to 8 bytes (default alignment is 4 bytes).
// Out: 
//   - loc receives the location.
// Notes:
//   - see also: http://windows/planning/w8themes/arm/Shared%20Documents/Windows%20ARM%20ABI.docx
//   - see also: http://infocenter.arm.com/help/topic/com.arm.doc.espc0002/ATPCS.pdf
//   - see also: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf
//   - see also: \\cpvsbuild\DROPS\dev11\Main\raw\current\sources\ndp\clr\src\VM\CallingConvention.cpp.
void CallingConventionHelper::GetNextParameterLocation(int byteCount, bool isFloatingPoint, bool is64BitAlignRequired, ParameterLocation *loc)
{
    // Attempt to place the argument into some combination of floating point or general registers and
    // the stack.

    // Internally we mostly deal with stack slots rather than bytes (we assume the argument size has been
    // rounded up to the nearest multiple of 4 already).
    Assert((byteCount % 4) == 0);
    int argSlotCount = byteCount / 4;

    // Note: for WinRT, vararg functions are not supported, so we don't have to take special care of them
    //       (per ABI, float args of vararg functions go to the stack rather than VFP registers).
    if (isFloatingPoint)
    {
        // Handle floating point (primitive) arguments.

        // First determine whether we can place the argument in VFP registers. There are 16 32-bit
        // and 8 64-bit argument registers that share the same register space (e.g. D0 overlaps S0 and
        // S1). The ABI specifies that VFP values will be passed in the lowest sequence of registers that
        // haven't been used yet and have the required alignment. So the sequence (float, double, float)
        // would be mapped to (S0, D1, S1) or (S0, S2/S3, S1).
        //
        // We use a 16-bit bitmap to record which registers have been used so far.
        // So we can use the same basic loop for each argument type (float, double or HFP struct) we set up
        // the following input parameters based on the size and alignment requirements of the arguments:
        //   allocMask  : bitmask of the number of 32-bit registers we need (1 for 1, 3 for 2, 7 for 3 etc.)
        //   stepCount  : number of loop iterations it'll take to search the 16 registers
        //   shiftCount : how many bits to shift the allocation mask on each attempt

        WORD allocMask = (1 << (byteCount / 4)) - 1;
        WORD stepCount = (WORD)(is64BitAlignRequired ? 9 - (byteCount / 8) : 17 - (byteCount / 4));
        WORD shiftCount = is64BitAlignRequired ? 2 : 1;

        // Look through the availability bitmask for a free register or register pair.
        for (WORD i = 0; i < stepCount; i++)
        {
            if ((m_usageData.AvailableFloatRegsMask & allocMask) == 0)
            {
                // We found one, mark the register or registers as used. 
                m_usageData.AvailableFloatRegsMask |= allocMask;

                // Indicate the registers used to the caller and return.
                loc->floatRegIndex = i * shiftCount;
                loc->floatRegCount = argSlotCount;

                return;
            }
            allocMask <<= shiftCount;
        }

        // The FP argument is going to live on the stack. Once this happens the ABI demands we mark all VFP
        // registers as unavailable.
        m_usageData.AvailableFloatRegsMask = 0xFFFF;

        // Doubles or HFP structs containing doubles need the stack aligned appropriately.
        if (is64BitAlignRequired)
        {
            m_usageData.StackSlotIndex = ALIGN_UP(m_usageData.StackSlotIndex, 2);
        }

        // Indicate the stack location of the argument to the caller.
        loc->StackSlotIndex = m_usageData.StackSlotIndex;
        loc->stackSlotCount = argSlotCount;

        // Record the stack usage.
        m_usageData.StackSlotIndex += argSlotCount;

        return;
    }
    else
    {
        // Handle the non-floating point case.

        if (is64BitAlignRequired)
        {
            // The argument requires 64-bit alignment. Align either the next general argument register if
            // we have any left or the stack. Note that if aligning the next register causes us to run out
            // of registers (i.e. the next register was R3), we need to need to consider aligning the
            // stack as well (i.e. don't use an if/else). That's because although the stack starts out
            // 64-bit aligned according to the ABI, it's possible that floating point arguments ran out of
            // VFP registers and spilled onto the stack first, possibly mis-aligning it in the process.
            if (m_usageData.GenRegIndex < 4)
            {
                m_usageData.GenRegIndex = ALIGN_UP(m_usageData.GenRegIndex, 2);
            }

            if (m_usageData.GenRegIndex >= 4)
            {
                m_usageData.StackSlotIndex = ALIGN_UP(m_usageData.StackSlotIndex, 2);
            }
        }

        // The ABI supports splitting a non-VFP argument across registers and the stack. But this is
        // disabled if the FP arguments already overflowed onto the stack (i.e. the stack index is not
        // zero). The following code marks the general argument registers as exhausted if this condition
        // holds.
        // Note that single int64 argument will never split (because it requires 8-byte alignliment), 
        // what can split are structs.
        int availableSlotCount = 4 - m_usageData.GenRegIndex;
        if (argSlotCount > availableSlotCount && m_usageData.StackSlotIndex != 0)
        {
            m_usageData.GenRegIndex = 4;
        }

        // Indicate to the caller whether the first part (or possibly all) of the argument fits into one
        // or more general registers.
        if (m_usageData.GenRegIndex < 4)
        {
            loc->GenRegIndex = m_usageData.GenRegIndex;
            loc->genRegCount = min(argSlotCount, 4 - m_usageData.GenRegIndex);

            // Mark the registers just allocated as used.
            m_usageData.GenRegIndex += loc->genRegCount;

            // Drop from consideration the portion of the argument that we just placed.
            argSlotCount -= loc->genRegCount;
        }

        Assert(argSlotCount >= 0);

        // Specify the portion of the argument (if any) that lives on the stack.
        if (argSlotCount)
        {
            loc->StackSlotIndex = m_usageData.StackSlotIndex;
            loc->stackSlotCount = argSlotCount;

            // Advance the stack pointer over the argument just placed.
            m_usageData.StackSlotIndex += argSlotCount;
        }
    }
}

// Returns true if and only if the given type is a floating point type or HFP struct.
bool CallingConventionHelper::IsFloatingPoint(RtCONCRETETYPE type) // static.
{
    Assert(type);
    bool isFloatingPoint = false;

    if (BasicType::Is(type))
    {
        CorElementType baseType = BasicType::From(type)->typeCor;
        isFloatingPoint = baseType == ELEMENT_TYPE_R4 || baseType == ELEMENT_TYPE_R8;
    }
    else if (StructType::Is(type))
    {
        // HFP structs are passes through VFP registers f0-f3/d0-d3, i.e. there can be only 4 fields, either float or double.
        auto structType = StructType::From(type);
        isFloatingPoint = structType->structType == structFieldTypeHFPFloat || structType->structType == structFieldTypeHFPDouble;
    }

    return isFloatingPoint;
}

// Returns whether 8-byte alignment is required for the type.
bool CallingConventionHelper::Is64BitAlignRequired(RtCONCRETETYPE type) // static.
{
    // Note: for tcMissingNamedType, naturalAlignment is unreliable/garbage, we'll report the error later 
    //       when the arg is actually used (like WriteInType), here assuming 4-byte alingment is fine.
    // Note: for tcByRefType, naturalAlignment unreliable/garbage, the actual alignment used is 4 byte.
    // TODO: Evanesco/Projection: see maybe it's better to fix that in ConcreteType and not here.
    AssertMsg(type->typeCode == tcMissingNamedType || type->typeCode == tcByRefType || type->naturalAlignment / 8 <= 1, 
        "How come we have more than 8 byte alignment?");

    bool is64BitAlignRequired = type->typeCode != tcByRefType && type->naturalAlignment / 8 > 0;
    return is64BitAlignRequired;
}

// Get actual location in memory for abstract parameter location.
// In:
// - Abstract parameter location.
// Out:
//   - Returns actual location in memory (somewhere within "this" ApcsCallLayout instance) for abstract parameter location.
// ASSUMPTIONS:
// - loc has already been populated.
byte* ApcsCallLayout::GetParameterLocation(const ParameterLocation* loc)
{
    Assert(loc);
    byte* locationInMemory = NULL;

    if (loc->floatRegIndex != ParameterLocation::invalidIndex)
    {
        Assert(this->FloatRegisters);
        locationInMemory = this->FloatRegisters + loc->floatRegIndex * 4;
    }
    else 
    {
        // When argument is split across general registers and stack, this is possible only when FP registers 
        // were not placed on the stack yet. Thus, since in the data structure stack is right after general registers,
        // we can just place the value to general regs and it will span to the stack region continuosly.
        if (loc->GenRegIndex != ParameterLocation::invalidIndex)
        {
            AssertMsg(this->GeneralRegisters, "callLayout.GeneralRegisters must have been allocated.");
            AssertMsg(loc->StackSlotIndex == ParameterLocation::invalidIndex || loc->StackSlotIndex == 0,
                "When argument is split across general registers and stack, there must be nothing on stack yet (i.e. argument goes to beginning of stack).");
            AssertMsg(loc->StackSlotIndex == ParameterLocation::invalidIndex || this->Stack, 
                "When argument is split across general registers and stack, callLayout.Stack must have been allocated.");
            locationInMemory = this->GeneralRegisters + loc->GenRegIndex * 4;
        }
        else if (loc->StackSlotIndex != ParameterLocation::invalidIndex)
        {
            AssertMsg(this->Stack, "callLayout.Stack must have been allocated.");
            locationInMemory = this->Stack + loc->StackSlotIndex * 4;
        }
    }

    Assert(locationInMemory);
    return locationInMemory;
}

// Allocate data for registers and stack.
// Note that for registers, for simplicity, unless regs are not used, we allocate data for all regs, 
// rather than just for used amount of regs.
// TODO: Evanesco: consider allocating only required number of registers rather than "either none or all registers".
void ApcsCallLayout::AllocateData(ArenaAllocator* alloc, int generalRegisterCount, int usedFloatRegisterMask, int stackSize)
{
    Assert(alloc);

    bool areGeneralRegistersUsed = generalRegisterCount > 0;
    bool areFloatRegistersUsed = usedFloatRegisterMask != 0; 

    if (areFloatRegistersUsed)
    {
        this->FloatRegisters = AnewPlusZ(alloc, 4 * 16, byte);   // 16 VFP regs are used to pass data via registers (f0-f15/d0-d7).
    }

    if (stackSize > 0)
    {
        this->StackSize = stackSize;
    }

    if (areGeneralRegistersUsed && stackSize > 0)
    {
        // Allocate general regs and stack as one continuos region.
        this->GeneralRegisters = AnewPlusZ(alloc, 4 * 4 + stackSize, byte);
        this->Stack = this->GeneralRegisters + 4 * 4;
    }
    else if (areGeneralRegistersUsed)
    {
        this->GeneralRegisters = AnewPlusZ(alloc, 4 * 4, byte);
    }
    else if (stackSize > 0)
    {
        this->Stack = AnewPlusZ(alloc, stackSize, byte);
    }
}

} // namespace.
