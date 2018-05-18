//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "JDUtil.h"
class JDBackend : public JDUtil
{
public:
    JDBackend(EXT_CLASS_BASE::PropertyNameReader& propertyNameReader);
    void DumpFunc(JDRemoteTyped func);
    void DumpInstr(JDRemoteTyped instr);
private:
    void DumpLabelInstr(JDRemoteTyped labelInstr);
    void DumpBranchInstr(JDRemoteTyped branchInstr);
    void DumpInstrBase(JDRemoteTyped instr);

    char * GetOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);

    char * GetIntConstOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetFloatConstOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetHelperCallOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetSymOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetRegOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetAddrOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetIndirOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetLabelOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetMemRefOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);
    char * GetRegBVOpndDumpString(JDRemoteTyped opnd, char * buffer = nullptr, size_t len = 0);

    char * GetPropertySymDumpString(JDRemoteTyped stackSym, char * buffer = nullptr, size_t len = 0);
    char * GetStackSymDumpString(JDRemoteTyped stackSym, char * buffer = nullptr, size_t len = 0);

    char * GetLabelInstrDumpString(JDRemoteTyped labelInstr, char * buffer = nullptr, size_t len = 0);

    ULONG64 GetInt(JDRemoteTyped value);
private:
    EXT_CLASS_BASE::PropertyNameReader& propertyNameReader;
};
