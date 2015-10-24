//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "EnginePch.h"
#include "BackEnd.h"
#include "ExternalLowerer.h"

void
GenerateFastExternalEqTest(
    CustomExternalObjectOperations *pData,
    IR::RegOpnd *typeRegOpnd,
    IR::BranchInstr *instrBranch,
    IR::LabelInstr *labelHelper,
    IR::LabelInstr *labelSuccess,
    bool isStrictBr,
    bool fallThroughOnSuccess,
    Func * func)
{
    // Given a type, generate check for CustomExternalType and look at the operation usage flags to see
    // if we can do a fast inline compare or need to call into the DOM for special handling.

    // Check external type flag.
    IR::Opnd *opnd = IR::IndirOpnd::New(typeRegOpnd, Js::Type::GetOffsetOfFlags(), TyUint8, func);
    Lowerer::InsertTestBranch(
        opnd,
        IR::IntConstOpnd::New(TypeFlagMask_External, TyUint8, func),
        Js::OpCode::BrEq_A,
        labelHelper,
        instrBranch);

    // Check for CustomExternalType -- see if operations field is non-null.
    opnd =
        IR::IndirOpnd::New(typeRegOpnd, Js::ExternalType::GetOffsetOfOperations(), TyMachReg, func);
    Lowerer::InsertCompareBranch(
        opnd,
        IR::AddrOpnd::New(nullptr, IR::AddrOpndKindConstantVar, func, true),
        Js::OpCode::BrEq_A,
        labelSuccess,
        instrBranch);

    // Check CustomExternalType's operations usage. We need data from the hosting layer for this.
    IntConstType operationFlag = isStrictBr ? pData->operationFlagStrictEquals : pData->operationFlagEquals;
    opnd = IR::IndirOpnd::New(typeRegOpnd, pData->offsetOfOperationsUsage, TyUint32, func);
    if (fallThroughOnSuccess)
    {
        Lowerer::InsertTestBranch(
            opnd,
            IR::IntConstOpnd::New(operationFlag, TyUint32, func),
            Js::OpCode::BrNeq_A,
            labelHelper,
            instrBranch);
    }
    else
    {
        Lowerer::InsertTestBranch(
            opnd,
            IR::IntConstOpnd::New(operationFlag, TyUint32, func),
            Js::OpCode::BrEq_A,
            labelSuccess,
            instrBranch);

        Lowerer::InsertBranch(Js::OpCode::Br, labelHelper, instrBranch);
    }
}

bool
ExternalLowerer::TryGenerateFastExternalEqTest(IR::Opnd * src1, IR::Opnd * src2, IR::BranchInstr * instrBranch, IR::LabelInstr * labelHelper, 
    IR::LabelInstr * labelBooleanCmp, Lowerer * lowerer, bool isStrictBr)
{
    CustomExternalObjectOperations data;
    HRESULT hr = S_FALSE;    
    HostScriptContext *hsc = lowerer->GetScriptContext()->GetHostScriptContext();
    if (hsc)
    {
        hr = hsc->GetExternalJitData(ExternalJitData_CustomExternalObjectOperations, &data);
    }
    if (hr == S_OK)
    {
        if (!isStrictBr)
        {
            IR::LabelInstr *labelContinue = IR::LabelInstr::New(Js::OpCode::Label, lowerer->m_func);
            IR::RegOpnd *typeRegOpnd = lowerer->GenerateIsBuiltinRecyclableObject(src1->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/, labelContinue);
            GenerateFastExternalEqTest(&data, typeRegOpnd, instrBranch, labelHelper, labelBooleanCmp, false, false, lowerer->m_func);

            instrBranch->InsertBefore(labelContinue);
            lowerer->GenerateIsBuiltinRecyclableObject(src2->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/);
        }
        else
        {
            IR::RegOpnd *typeRegOpnd = lowerer->GenerateIsBuiltinRecyclableObject(src2->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/, labelBooleanCmp);
            GenerateFastExternalEqTest(&data, typeRegOpnd, instrBranch, labelHelper, labelBooleanCmp, true, true, lowerer->m_func);
        }
        return true;
    }
    return false;
}
 