//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "JDUtil.h"
class JDBackend : public JDUtil
{
public:
    JDBackend(EXT_CLASS_BASE::PropertyNameReader& propertyNameReader);
    void DumpFunc(ExtRemoteTyped func);
    void DumpInstr(ExtRemoteTyped instr);
private:
    void DumpLabelInstr(ExtRemoteTyped labelInstr);
    void DumpBranchInstr(ExtRemoteTyped branchInstr);
    void DumpInstrBase(ExtRemoteTyped instr);

    char * GetOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);

    char * GetIntConstOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetFloatConstOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetHelperCallOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetSymOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetRegOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetAddrOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetIndirOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetLabelOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetMemRefOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetRegBVOpndDumpString(ExtRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);

    char * GetPropertySymDumpString(ExtRemoteTyped stackSym, char * buffer = nullptr, size_t len = 0);
    char * GetStackSymDumpString(ExtRemoteTyped stackSym, char * buffer = nullptr, size_t len = 0);

    char * GetLabelInstrDumpString(ExtRemoteTyped labelInstr, char * buffer = nullptr, size_t len = 0);

    ULONG64 GetInt(ExtRemoteTyped value);
private:
    EXT_CLASS_BASE::PropertyNameReader& propertyNameReader;
};
