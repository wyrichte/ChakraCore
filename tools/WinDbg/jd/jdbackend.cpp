//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE
#include "jdbackend.h"
#include "jdbackendutil.h"
#define TEMP_BUFFER_SIZE 1024
static char s_tempBuffer[TEMP_BUFFER_SIZE];
#define RETURNBUFFER(fmt, ...) if (buffer == nullptr) { buffer = s_tempBuffer; len = sizeof(s_tempBuffer); }; sprintf_s(buffer, len, fmt, __VA_ARGS__); return buffer

char * JDBackend::GetStackSymDumpString(ExtRemoteTyped stackSym, char * buffer, size_t len)
{
    ushort slotNum;
    if (stackSym.HasField("m_slotNum"))
    {
        // after commit b627afff0a244f45d0f5be4de9d6cce2e3eb3c5e
        slotNum = stackSym.Field("m_slotNum").GetUshort();
        if (stackSym.Field("m_isParamSym").GetChar())
        {
            RETURNBUFFER("prm%u", slotNum);
        }
    }
    else
    {
        // before commit b627afff0a244f45d0f5be4de9d6cce2e3eb3c5e
        ushort paramSlotNum = stackSym.Field("m_paramSlotNum").GetUshort();
        if (paramSlotNum != (ushort)-1)
        {
            RETURNBUFFER("prm%u", paramSlotNum);
        }

        slotNum = stackSym.Field("m_argSlotNum").GetUshort();
    }

    ulong symId = stackSym.Field("m_id").GetUlong();
    if (stackSym.Field("m_isArgSlotSym").GetChar())
    {
        if (stackSym.Field("m_isInlinedArgSlot").GetChar())
        {
            RETURNBUFFER("iarg%d(s%d)", slotNum, symId);
        }
        RETURNBUFFER("arg%d(s%d)", slotNum, symId);
    }
    RETURNBUFFER("s%d", symId);
}

char * JDBackend::GetIntConstOpndDumpString(ExtRemoteTyped intConstOpnd, char * buffer, size_t len)
{
    if (g_Ext->m_PtrSize == 4)
    {
        RETURNBUFFER("%d", intConstOpnd.Field("m_value").GetLong());
    }
    else
    {
        Assert(g_Ext->m_PtrSize == 8);
        RETURNBUFFER("%I64d", intConstOpnd.Field("m_value").GetLong64());
    }
}

char * JDBackend::GetFloatConstOpndDumpString(ExtRemoteTyped floatConstOpnd, char * buffer, size_t len)
{
    RETURNBUFFER("%e", floatConstOpnd.Field("m_value").GetDouble());    
}

char * JDBackend::GetHelperCallOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    RETURNBUFFER("%s", GetEnumString(opnd.Field("m_fnHelper")));
}

char * JDBackend::GetPropertySymDumpString(ExtRemoteTyped propertySym, char* buffer, size_t len)
{
    char tempBuffer[TEMP_BUFFER_SIZE];
    char * stackSymDumpStr = GetStackSymDumpString(propertySym.Field("m_stackSym"), tempBuffer, _countof(tempBuffer));

    char * fieldKind = propertySym.Field("m_fieldKind").GetSimpleValue();
    int32 propertyId = propertySym.Field("m_propertyId").GetLong();
    if (ENUM_EQUAL(fieldKind, PropertyKindData))
    {
        RETURNBUFFER("%s->%s", stackSymDumpStr, propertyNameReader.GetNameStringByPropertyId(propertyId).c_str());        
    }
    else if (ENUM_EQUAL(fieldKind, PropertyKindSlots)
        || ENUM_EQUAL(fieldKind, PropertyKindSlotArray))
    {
        RETURNBUFFER("%s[%d]", stackSymDumpStr, propertyId);        
    }
    else if (ENUM_EQUAL(fieldKind, PropertyLocalSlots))
    {
        RETURNBUFFER("%sl[%d]", stackSymDumpStr, propertyId);
    }
    else
    {
        return "<Unknown property sym field kind>";
    }
}

char * JDBackend::GetSymOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{    
    ExtRemoteTyped sym = opnd.Field("m_sym");
    char * kind = sym.Field("m_kind").GetSimpleValue();
    if (ENUM_EQUAL(kind, SymKindProperty))
    {
        return GetPropertySymDumpString(ExtRemoteTyped(GetExtension()->FillModule("(%s!PropertySym *)@$extin"), sym.GetPtr()), buffer, len);
    }
    ExtRemoteTyped stackSym = ExtRemoteTyped(GetExtension()->FillModule("(%s!StackSym *)@$extin"), sym.GetPtr());
    int32 stackSymOffset = stackSym.Field("m_offset").GetLong();
    bool hasOffset = stackSym.Field("m_isArgSlotSym").GetChar() ?
        stackSymOffset != -1 || !stackSym.Field("m_isInlinedArgSlot").GetChar() : stackSymOffset != 0;

    if (!hasOffset)
    {
        return GetStackSymDumpString(stackSym, buffer, len);
    }
    char tempBuffer[TEMP_BUFFER_SIZE];
    char * stackSymDumpStr = GetStackSymDumpString(stackSym, tempBuffer, _countof(tempBuffer));

    int32 opndOffset = opnd.Field("m_offset").GetLong();

    RETURNBUFFER("%s<%d>", stackSymDumpStr, stackSymOffset + opndOffset);    
}

ULONG64 JDBackend::GetInt(ExtRemoteTyped value)
{
    ULONG64 integer = value.GetData(value.GetTypeSize());
    return integer;
}


char * JDBackend::GetRegOpndDumpString(ExtRemoteTyped regOpnd, char * buffer, size_t len)
{
    ExtRemoteTyped sym = regOpnd.Field("m_sym");

    ExtRemoteTyped reg = regOpnd.Field("m_reg");
    if (GetInt(reg))
    {
        char tempBuffer[TEMP_BUFFER_SIZE];
        char * stackSymDumpStr = "";
        if (sym.GetPtr())
        {
            stackSymDumpStr = GetStackSymDumpString(sym, tempBuffer, _countof(tempBuffer));
        }
        RETURNBUFFER("%s(%s)", stackSymDumpStr, GetEnumString(reg) + 3 /* skip the prefix "Reg" */);
    }
    else if (sym.GetPtr())
    {
        return GetStackSymDumpString(sym, buffer, len);
    }
    return "<RegOpnd with no reg or sym>";
}

char * JDBackend::GetAddrOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    if (GetExtension()->IsCurMachine64())
    {
        RETURNBUFFER("0x%I64X", opnd.Field("m_address").GetPtr());
    }
    else
    {
        RETURNBUFFER("0x%X", (ULONG)opnd.Field("m_address").GetPtr());
    }
}

char * JDBackend::GetIndirOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    char tempBuffer[TEMP_BUFFER_SIZE];
    char * stackSymDumpStr = GetRegOpndDumpString(opnd.Field("m_baseOpnd"), tempBuffer, _countof(tempBuffer));

    ExtRemoteTyped indexOpnd = opnd.Field("m_indexOpnd");
    if (indexOpnd.GetPtr())
    {
        char tempBuffer2[TEMP_BUFFER_SIZE];
        char * indexSymDumpStr = GetRegOpndDumpString(indexOpnd, tempBuffer2, _countof(tempBuffer));
        uint8 scale = opnd.Field("m_scale").GetUchar();
        if (scale != 0)
        {
            RETURNBUFFER("[%s+%s*%d]", stackSymDumpStr, indexSymDumpStr, 1 << scale);
        }

        RETURNBUFFER("[%s+%s]", stackSymDumpStr, indexSymDumpStr);
    }
    int32 offset = opnd.Field("m_offset").GetLong();
    if (offset)
    {
        RETURNBUFFER(offset >= 0 ? "[%s+%d]" : "[%s%d]", stackSymDumpStr, offset);
    }

    RETURNBUFFER("[%s]", stackSymDumpStr);
}
char * JDBackend::GetLabelOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    return GetLabelInstrDumpString(opnd.Field("m_label"), buffer, len);
}
char * JDBackend::GetMemRefOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    if (GetExtension()->IsCurMachine64())
    {
        RETURNBUFFER("[0x%I64X]", opnd.Field("m_memLoc").GetPtr());
    }
    else
    {
        RETURNBUFFER("[0x%X]", (ULONG)opnd.Field("m_memLoc").GetPtr());
    }
}
char * JDBackend::GetRegBVOpndDumpString(ExtRemoteTyped opnd, char * buffer, size_t len)
{
    return "<RegBV>";
}

char * JDBackend::GetOpndDumpString(JDRemoteTyped opnd, char * buffer, size_t len)
{        
    char const * typeName;
    JDRemoteTyped typedOpnd = opnd.CastWithVtable(&typeName);
    if (!typeName)
    {
        // Opnd doesn't have vtable on retail build.  Use the kind to infer the type.
        const char * opndKindStr = JDUtil::GetEnumString(opnd.Field("m_kind"));
        if (strcmp(opndKindStr, "OpndKindIntConst") == 0)
        {
            typeName = "IR::IntConstOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindInt64Const") == 0)
        {
            typeName = "IR::Int64ConstOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindFloatConst") == 0)
        {
            typeName = "IR::FloatConstOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindSimd128Const") == 0)
        {
            typeName = "IR::Simd128ConstOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindHelperCall") == 0)
        {
            typeName = "IR::HelperCallOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindSym") == 0)
        {
            typeName = "IR::SymOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindReg") == 0)
        {
            typeName = "IR::RegOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindAddr") == 0)
        {
            typeName = "IR::AddrOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindIndir") == 0)
        {
            typeName = "IR::IndirOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindLabel") == 0)
        {
            typeName = "IR::LabelOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindMemRef") == 0)
        {
            typeName = "IR::MemRefOpnd";
        }
        else if (strcmp(opndKindStr, "OpndKindRegBV") == 0)
        {
            typeName = "IR::RegBVOpnd";
        }
        else
        {
            return "<No Vtable>";
        } 
        typedOpnd = opnd.Cast(typeName);
    }
    else
    {
        typeName = JDUtil::StripModuleName(typeName);
    }

    if (strcmp(typeName, "IR::IntConstOpnd") == 0)
    {
        return GetIntConstOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::FloatConstOpnd") == 0)
    {
        return GetFloatConstOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::HelperCallOpnd") == 0)
    {
        return GetHelperCallOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::SymOpnd") == 0 || strcmp(typeName, "IR::PropertySymOpnd") == 0)
    {
        return GetSymOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::RegOpnd") == 0)
    {
        return GetRegOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::AddrOpnd") == 0)
    {
        return GetAddrOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::IndirOpnd") == 0)
    {
        return GetIndirOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::LabelOpnd") == 0)
    {
        return GetLabelOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::MemRefOpnd") == 0)
    {
        return GetMemRefOpndDumpString(typedOpnd, buffer, len);
    }
    if (strcmp(typeName, "IR::RegBVOpnd") == 0)
    {
        return GetRegBVOpndDumpString(typedOpnd, buffer, len);
    }
    return "<UnknownKind>";
   
}

char * JDBackend::GetLabelInstrDumpString(ExtRemoteTyped labelInstr, char * buffer, size_t len)
{
    RETURNBUFFER("$L%u", labelInstr.Field("m_id").GetUlong());
}

void JDBackend::DumpLabelInstr(ExtRemoteTyped labelInstr)
{
    GetExtension()->Out(GetLabelInstrDumpString(labelInstr));
    if (labelInstr.Field("m_isLoopTop").GetStdBool())
    {
      GetExtension()->Out(">>>>>>>>>>>>>  LOOP TOP  >>>>>>>>>>>>>");
    }
}

void JDBackend::DumpBranchInstr(ExtRemoteTyped branchInstr)
{
    DumpInstrBase(branchInstr);
    GetExtension()->Out(" => ");
    ExtRemoteTyped branchTarget = branchInstr.Field("m_branchTarget");
    if (branchTarget.GetPtr() != 0)
    {
        GetExtension()->Out(GetLabelInstrDumpString(branchTarget));
        if (branchTarget.Field("m_isLoopTop").GetStdBool())
        {
            GetExtension()->Out("<<<<<<<<<<<<<  Loop Tail >>>>>>>>>>>>>");
        }
    }
    else
    {
        // This may happen when haven't finish filling the label in, such as in IRBuilder
        GetExtension()->Out("<NO TARGET LABEL>");
    }
}

void JDBackend::DumpInstrBase(ExtRemoteTyped instr)
{
    ExtRemoteTyped dst = instr.Field("m_dst");
    ExtRemoteTyped opcode = ExtRemoteTyped(GetExtension()->FillModule("(%s!Js::OpCode)@$extin"), instr.Field("m_opcode").GetUshort());
    ExtRemoteTyped src1 = instr.Field("m_src1");
    ExtRemoteTyped src2 = instr.Field("m_src2");

    if (dst.GetPtr() != NULL)
    {
        GetExtension()->Out("%15s = ", GetOpndDumpString(dst));
    }
    else
    {
        GetExtension()->Out("%18s", "");
    }

    // Trim out the enum numeric value
    GetExtension()->Out("%-20s", GetEnumString(opcode));

    if (src1.GetPtr() != NULL)
    {
        GetExtension()->Out(" %-8s", GetOpndDumpString(src1));
        if (src2.GetPtr() != NULL)
        {
            GetExtension()->Out(", %-15s", GetOpndDumpString(src2));
        }
        else
        {
            GetExtension()->Out("%18s", "");
        }
    }
    else
    {
        GetExtension()->Out("%19s", "");
    }
}

void JDBackend::DumpInstr(ExtRemoteTyped instr)
{
    char * v = instr.Field("m_kind").GetSimpleValue();
    if (ENUM_EQUAL(v, InstrKindLabel) || ENUM_EQUAL(v, InstrKindProfiledLabel))
    {
        DumpLabelInstr(ExtRemoteTyped(GetExtension()->FillModule("(%s!IR::LabelInstr *)@$extin"), instr.GetPtr()));
        GetExtension()->Out(":");
    }
    else if (ENUM_EQUAL(v, InstrKindBranch))
    {
        DumpBranchInstr(ExtRemoteTyped(GetExtension()->FillModule("(%s!IR::BranchInstr *)@$extin"), instr.GetPtr()));
    }
    else if (ENUM_EQUAL(v, InstrKindPragma))
    {
        ExtRemoteTyped opcode = ExtRemoteTyped(GetExtension()->FillModule("(%s!Js::OpCode)@$extin"), instr.Field("m_opcode").GetUshort());
        if (ENUM_EQUAL(opcode.GetSimpleValue(), StatementBoundary))
        {
            GetExtension()->Out(_u("%20s StatementBoundary #%d"), "", ExtRemoteTyped(GetExtension()->FillModule("(%s!IR::PragmaInstr *)@$extin"), instr.GetPtr()).Field("m_statementIndex").GetUlong());
        }
        else
        {
            DumpInstrBase(instr);
        }
    }
    else
    {
        DumpInstrBase(instr);
    }

    GetExtension()->Out("\n");
}

void
JDBackend::DumpFunc(ExtRemoteTyped func)
{
    ExtRemoteTyped curr = func.Field("m_headInstr");

    while (curr.GetPtr())
    {
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml("<link cmd=\"dt IR::Instr 0x%p\">0x%p</link>:", curr.GetPtr(), curr.GetPtr());
        }
        else
        {
            GetExtension()->Out("0x%p /*\"dt IR::Instr 0x%p\" to display*/:", curr.GetPtr(), curr.GetPtr());
        }
        DumpInstr(curr);
        curr = curr.Field("m_next");
    }
}

JDBackend::JDBackend(EXT_CLASS_BASE::PropertyNameReader& propertyNameReader) :
    propertyNameReader(propertyNameReader)
{}



JD_PRIVATE_COMMAND(irinstr,
    "Dump an IR instr",
    "{;s;instr;The pointer to the IR::Instr* to dump}{;edn=(10),o,d=1;count;Number of instructions to dump}")
{
    ExtRemoteTyped varTyped(GetUnnamedArgStr(0));
    ULONG64 pointer = varTyped.GetPtr();
    ExtRemoteTyped instr("(IR::Instr *)@$extin", pointer);
    PropertyNameReader propertyNameReader = JDBackendUtil::GetPropertyNameReaderFromFunc(instr.Field("m_func"));
    JDBackend jdbackend(propertyNameReader);
    ULONG64 count = GetUnnamedArgU64(1);
    ULONG64 halfCount = count / 2;
    ULONG64 i = 0;
    ExtRemoteTyped currentInstr = instr;
    ExtRemoteTyped prevInstr = instr.Field("m_prev");
    while (i < halfCount && prevInstr.GetPtr())
    {
        currentInstr = prevInstr;
        prevInstr = prevInstr.Field("m_prev");
        i++;
    }

    while (currentInstr.GetPtr() != instr.GetPtr())
    {
        Out("   ");
        jdbackend.DumpInstr(currentInstr);
        currentInstr = currentInstr.Field("m_next");
    }

    Out("=> ");
    jdbackend.DumpInstr(instr);

    i = 0;
    currentInstr = instr.Field("m_next");
    while (i < halfCount && currentInstr.GetPtr())
    {
        Out("   ");
        jdbackend.DumpInstr(currentInstr);
        currentInstr = currentInstr.Field("m_next");
        i++;
    }
}

static ExtRemoteTyped GetTopFunc(ExtRemoteTyped func)
{
  ExtRemoteTyped parent = func.Field("parentFunc");
  while (parent.GetPtr() != 0)
  {
    func = parent;
    parent = parent.Field("parentFunc");
  }
  return func;
}

JD_PRIVATE_COMMAND(irfunc,
    "Dump an IR func",
    "{;s;instr;C++ Expression that evalues to func}")
{
    ExtRemoteTyped varTyped(GetUnnamedArgStr(0));
    ULONG64 pointer = varTyped.GetPtr();
    ExtRemoteTyped func = GetTopFunc(ExtRemoteTyped(GetExtension()->FillModule("(%s!Func*)@$extin"), pointer));
    PropertyNameReader propertyNameReader = JDBackendUtil::GetPropertyNameReaderFromFunc(func);
    JDBackend jdbackend(propertyNameReader);
    jdbackend.DumpFunc(func);
}

#endif
