//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace ProjectionModel
{

// Describes how a single parameter is laid out in registers and/or stack locations.
struct ParameterLocation
{
    static const int invalidIndex = -1;

    int FloatRegIndex;      // First floating point register used (or -1)
    int floatRegCount;      // Count of floating point registers used (or 0)

    int GenRegIndex;        // First general register used (or -1)
    int genRegCount;        // Count of general registers used (or 0)

    int StackSlotIndex;     // First stack slot (each slot = 4 bytes) used (or -1)
    int stackSlotCount;     // Count of stack slots used (or 0).

    ParameterLocation()
    {
        FloatRegIndex = invalidIndex;
        floatRegCount = 0;
        GenRegIndex = invalidIndex;
        genRegCount = 0;
        StackSlotIndex = invalidIndex;
        stackSlotCount = 0;
    }
};

// ARM64 Procedure Call Standard -- layout for parameters.
// Simple data structure that we pass to assembler code to make the actual call.
// WARNING: if you modify this structure, make sure to modify projectioncall.asm and unknownimplhelper.asm as well, as .asm relies on this layout.
struct ApcsCallLayout
{
    byte* FloatRegisters;       // NULL if the call does not use parameters in VFP regs, otherwise pointer to memory allocated for float regs.
    byte* GeneralRegisters;     // NULL if the call does not use gemeral regs, otherwise pointer to memory allocated for generanl regs.
    byte* Stack;                // NULL if the call does not use parameters stack, otherwise pointer to memory allocated for data on the stack.
    int StackSize;              // Actual size on the stack needed for parameters.

    ApcsCallLayout()
    {
        this->Clear();
    }

    void Clear()
    {
        ZeroMemory(this, sizeof(ApcsCallLayout));
    }

    // For given parameter, returns its location in memory (according to the layout defined "this").
    byte* GetParameterLocation(const ParameterLocation* loc);

    // Allocate data for registers and stack.
    void ApcsCallLayout::AllocateData(ArenaAllocator* alloc, int generalRegisterCount, int floatRegisterCount, int stackSize);
};

// Helper to determine placement of next parameter (registers/stack) and register/stack usage so far.
// How to use:
//   CallingConventionHelper cch(1);
//   'foreach' (RtABIPARAMETER parameter) {
//       ParameterLocation loc;
//       cch.GetNextParameterLocation(parameter, &loc);
//       // use the loc as needed.
//   }
//   int stackSize = cch.GetStackSlotCount() * 4;
class CallingConventionHelper
{
private:
    // Contains current state of used registers/stack.
    struct RegisterAndStackState
    {
        int FloatRegIndex;          // Next floating point register to be assigned a value.
        int GenRegIndex;            // Next general register to be assigned a value.
        int StackSlotIndex;         // Next stack slot to be assigned a value (each slot = 4 bytes).

        RegisterAndStackState(int reservedGeneralRegisterCount) :
            FloatRegIndex(0), GenRegIndex(reservedGeneralRegisterCount), StackSlotIndex(0)
        {
        }
    };

private:
    RegisterAndStackState m_usageData;

public:
    CallingConventionHelper(int reservedGeneralRegisterCount) : m_usageData(reservedGeneralRegisterCount)
    {
    }

    // Get placement of the parameter into loc, and update register/stack usage so far.
    // To get actual location in memory, you can use ApcsCallLayout::GetParameterLocation().
    void GetNextParameterLocation(RtABIPARAMETER parameter, ParameterLocation *loc);
    void GetNextParameterLocation(int byteCount, bool isFloatingPoint, ParameterLocation *loc, RtCONCRETETYPE type);

    int GetStackSlotCount()
    {
        return m_usageData.StackSlotIndex;
    }

    int GetGeneralRegisterCount()
    {
        return m_usageData.GenRegIndex;
    }

    int GetFloatRegisterCount()
    {
        return m_usageData.FloatRegIndex;
    }

    static bool IsFloatingPoint(RtCONCRETETYPE type);
};

} // namespace.
