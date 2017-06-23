//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "EnginePch.h"
#include "BackEnd.h"
#include "ExternalLowerer.h"

void
GenerateFastExternalEqTest(
    Lowerer* lowerer,
    CustomExternalObjectOperations *pData,
    IR::RegOpnd *typeRegOpnd,
    IR::Instr *instrBranch,
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
    lowerer->InsertTestBranch(
        opnd,
        IR::IntConstOpnd::New(TypeFlagMask_External, TyUint8, func),
        Js::OpCode::BrEq_A,
        labelHelper,
        instrBranch);

    // Check for CustomExternalType -- see if operations field is non-null.
    opnd =
        IR::IndirOpnd::New(typeRegOpnd, Js::ExternalType::GetOffsetOfOperations(), TyMachReg, func);
    lowerer->InsertCompareBranch(
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
        lowerer->InsertTestBranch(
            opnd,
            IR::IntConstOpnd::New(operationFlag, TyUint32, func),
            Js::OpCode::BrNeq_A,
            labelHelper,
            instrBranch);
    }
    else
    {
        lowerer->InsertTestBranch(
            opnd,
            IR::IntConstOpnd::New(operationFlag, TyUint32, func),
            Js::OpCode::BrEq_A,
            labelSuccess,
            instrBranch);

        lowerer->InsertBranch(Js::OpCode::Br, labelHelper, instrBranch);
    }
}

HRESULT GetExternalJitData(ExternalJitData id, void *data)
{
    switch (id)
    {
    case ExternalJitData_CustomExternalObjectOperations:
    {
        CustomExternalObjectOperations *ceoData = (CustomExternalObjectOperations*)data;

        ceoData->offsetOfOperationsUsage = Js::CustomExternalType::GetOffsetOfUsage();
        ceoData->operationFlagEquals = OperationFlag_Equals;
        ceoData->operationFlagStrictEquals = OperationFlag_StrictEquals;
        return S_OK;
    }

    default:
        return E_NOTIMPL;
    }
}

bool
ExternalLowerer::TryGenerateFastExternalEqTest(IR::Opnd * src1, IR::Opnd * src2, IR::Instr * instrBranch, IR::LabelInstr * labelHelper, 
    IR::LabelInstr * labelBooleanCmp, Lowerer * lowerer, bool isStrictBr, bool isInHelper)
{
    CustomExternalObjectOperations data;
    HRESULT hr = GetExternalJitData(ExternalJitData_CustomExternalObjectOperations, &data);
    if (hr == S_OK)
    {
        if (!isStrictBr)
        {
            IR::LabelInstr *labelContinue = IR::LabelInstr::New(Js::OpCode::Label, lowerer->m_func, isInHelper);
            IR::RegOpnd *typeRegOpnd = lowerer->GenerateIsBuiltinRecyclableObject(src1->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/, labelContinue, isInHelper);
            GenerateFastExternalEqTest(lowerer, &data, typeRegOpnd, instrBranch, labelHelper, labelBooleanCmp, false, false, lowerer->m_func);

            instrBranch->InsertBefore(labelContinue);
            lowerer->GenerateIsBuiltinRecyclableObject(src2->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/, nullptr, isInHelper);
        }
        else
        {
            IR::RegOpnd *typeRegOpnd = lowerer->GenerateIsBuiltinRecyclableObject(src2->AsRegOpnd(), instrBranch, labelHelper, false /*checkObjectAndDynamicObject*/, labelBooleanCmp, isInHelper);
            GenerateFastExternalEqTest(lowerer, &data, typeRegOpnd, instrBranch, labelHelper, labelBooleanCmp, true, true, lowerer->m_func);
        }
        return true;
    }
    return false;
}
 