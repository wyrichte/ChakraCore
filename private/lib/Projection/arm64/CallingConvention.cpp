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
        //       when the arg is actually used (like WriteInType), here assuming 8-bytes/non floating point.
        bool isMissingType = argument->type->typeCode == tcMissingNamedType;
        AssertMsg(argument->GetSizeOnStack() < INT_MAX, "No argument should exceed available stack space.");
        int byteCount = isMissingType ? 8 : (int)argument->GetSizeOnStack();
        bool isFloatingPoint = isMissingType ? false : CallingConventionHelper::IsFloatingPoint(concreteType);
        return GetNextParameterLocation(byteCount, isFloatingPoint, loc, concreteType);
    }

    // Get abstract location of next argument and update internal state which will be used for next argument.
    // In:
    //   - byteCount: size of argument in bytes, aka "size on stack".
    //   - isFloatingPoint: whether we should use VFP registers for the argument.
    //   - type: type information about the argument.
    // Out:
    //   - loc receives the location.
    // Notes:
    //   - see also: http://windows/planning/w8themes/arm/Shared%20Documents/Windows%20ARM%20ABI.docx
    //   - see also: http://infocenter.arm.com/help/topic/com.arm.doc.espc0002/ATPCS.pdf
    //   - see also: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf
    //   - see also: \\cpvsbuild\DROPS\dev11\Main\raw\current\sources\ndp\clr\src\VM\CallingConvention.cpp.
    void CallingConventionHelper::GetNextParameterLocation(int byteCount, bool isFloatingPoint, ParameterLocation *loc, RtCONCRETETYPE type)
    {
        // Attempt to place the argument into some combination of floating point or general registers and
        // the stack.

        // Internally we mostly deal with stack slots rather than bytes (we assume the argument size has been
        // rounded up to the nearest multiple of 8 already).
        Assert((byteCount % 8) == 0);
        int argSlotCount = (byteCount + 7) / 8;

        // Note: for WinRT, vararg functions are not supported, so we don't have to take special care of them
        //       (per ABI, float args of vararg functions go to the stack rather than VFP registers).
        if (isFloatingPoint)
        {

            int floatRegCount;

            if (StructType::Is(type))
            {

                // If this is an HFA then the registers required area count of
                // the fields in the struct.

                auto structType = StructType::From(type);
                floatRegCount = structType->hfpFieldCount;
            }
            else
            {
                // If this is not an HFA then the floating point case should
                // be a float or a double passed by value.
                floatRegCount = argSlotCount;
                Assert(floatRegCount == 1);
            }

            // Handle floating point (primitive) arguments.

            int availableRegCount = 8 - m_usageData.FloatRegIndex;
            if (floatRegCount > availableRegCount)
            {
                m_usageData.FloatRegIndex = 8;
            }

            // Indicate to the caller whether the argument fits into one
            // or more floating point registers.
            if (m_usageData.FloatRegIndex < 8)
            {
                loc->FloatRegIndex = m_usageData.FloatRegIndex;
                loc->floatRegCount = floatRegCount;

                // Mark the registers just allocated as used.
                m_usageData.FloatRegIndex += loc->floatRegCount;

                // Drop from consideration the portion of the argument that we just placed.
                // It is invalid to have some portion of an argument in float registers and
                // some spilled onto the stack so arg slot count must be 0 at this point if
                // any values are going in registers.
                argSlotCount = 0;
            }
        }
        else
        {
            // Handle the non-floating point case.

            int availableSlotCount = 8 - m_usageData.GenRegIndex;
            if (argSlotCount > availableSlotCount && m_usageData.StackSlotIndex != 0)
            {
                m_usageData.GenRegIndex = 8;
            }

            // Indicate to the caller whether the first part (or possibly all) of the argument fits into one
            // or more general registers.
            if (m_usageData.GenRegIndex < 8)
            {
                loc->GenRegIndex = m_usageData.GenRegIndex;
                loc->genRegCount = min(argSlotCount, 8 - m_usageData.GenRegIndex);

                // Mark the registers just allocated as used.
                m_usageData.GenRegIndex += loc->genRegCount;

                // Drop from consideration the portion of the argument that we just placed.
                argSlotCount -= loc->genRegCount;
            }
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
            // HFP structs are passes through VFP registers q0-q7, i.e. there can be only 4 fields, either float or double.
            auto structType = StructType::From(type);
            isFloatingPoint = structType->structType == structFieldTypeHFPFloat || structType->structType == structFieldTypeHFPDouble;
        }

        return isFloatingPoint;
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

        if (loc->FloatRegIndex != ParameterLocation::invalidIndex)
        {
            Assert(this->FloatRegisters);
            locationInMemory = this->FloatRegisters + loc->FloatRegIndex * 16;
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
                locationInMemory = this->GeneralRegisters + loc->GenRegIndex * 8;
            }
            else if (loc->StackSlotIndex != ParameterLocation::invalidIndex)
            {
                AssertMsg(this->Stack, "callLayout.Stack must have been allocated.");
                locationInMemory = this->Stack + loc->StackSlotIndex * 8;
            }
        }

        Assert(locationInMemory);
        return locationInMemory;
    }

    // Allocate data for registers and stack.
    // Note that for registers, for simplicity, unless regs are not used, we allocate data for all regs,
    // rather than just for used amount of regs.
    // TODO: Evanesco: consider allocating only required number of registers rather than "either none or all registers".
    void ApcsCallLayout::AllocateData(ArenaAllocator* alloc, int generalRegisterCount, int floatRegisterCount, int stackSize)
    {
        Assert(alloc);

        bool areGeneralRegistersUsed = generalRegisterCount > 0;
        bool areFloatRegistersUsed = floatRegisterCount > 0;

        if (areFloatRegistersUsed)
        {
            this->FloatRegisters = AnewPlusZ(alloc, 8 * 16, byte);   // 16 VFP regs are used to pass data via registers (f0-f15/d0-d7).
        }

        if (stackSize > 0)
        {
            this->StackSize = stackSize;
        }

        if (areGeneralRegistersUsed && stackSize > 0)
        {
            // Allocate general regs and stack as one continuos region.
            this->GeneralRegisters = AnewPlusZ(alloc, 8 * 8 + stackSize, byte);
            this->Stack = this->GeneralRegisters + 8 * 8;
        }
        else if (areGeneralRegistersUsed)
        {
            this->GeneralRegisters = AnewPlusZ(alloc, 8 * 8, byte);
        }
        else if (stackSize > 0)
        {
            this->Stack = AnewPlusZ(alloc, stackSize, byte);
        }
    }

} // namespace.
