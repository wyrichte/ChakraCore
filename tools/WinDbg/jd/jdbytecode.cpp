//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE
#include "jdbytecode.h"
#define MAX_LAYOUT_TYPE_NAME 255

JDByteCode::JDByteCode(EXT_CLASS_BASE * ext, bool dumpProbeBackingBlock, bool verbose)
    : ext(ext), layoutTable(ext->FillModule("%s!Js::OpCodeUtil::OpCodeLayouts")),
    extendedLayoutTable(ext->FillModule("%s!Js::OpCodeUtil::ExtendedOpCodeLayouts")),
    attributesTable(ext->FillModule("%s!OpcodeAttributes")),
    extendedAttributesTable(ext->FillModule("%s!ExtendedOpcodeAttributes")),
    OpcodeAttr_OpHasMultiSizeLayout(ExtRemoteTyped(ext->FillModule("%s!OpHasMultiSizeLayout")).GetLong()),    
    LayoutSize_SmallLayout(ExtRemoteTyped(ext->FillModule("%s!SmallLayout")).GetLong()),
    LayoutSize_MediumLayout(ExtRemoteTyped(ext->FillModule("%s!MediumLayout")).GetLong()),
    LayoutSize_LargeLayout(ExtRemoteTyped(ext->FillModule("%s!LargeLayout")).GetLong()),
    
    readerOffset((ULONG64)-1), propertyNameReader(nullptr), dumpProbeBackingBlock(dumpProbeBackingBlock), verbose(verbose),
    RootObjectRegSlot(1),           // TODO: how do find this symbolically?    
    CallIExtended_SpreadArgs(1)     // TODO: how do find this symbolically?    
{        
}

JDByteCode::~JDByteCode()
{
    if (propertyNameReader)
    {
        delete propertyNameReader;
    }
}


uint
JDByteCode::GetUnsigned(ExtRemoteTyped unsignedField)
{    
    switch (unsignedField.GetTypeSize())
    {
    case 1:
        return unsignedField.GetUchar();
    case 2:
        return unsignedField.GetUshort();        
    case 4:
        return unsignedField.GetUlong();
    default:
        throw ExtException(E_FAIL, "Invalid size of unsigned byte code field");
    }
}

int
JDByteCode::GetSigned(ExtRemoteTyped signedField)
{
    switch (signedField.GetTypeSize())
    {
    case 1:
        return signedField.GetChar();
    case 2:
        return signedField.GetShort();
    case 4:
        return signedField.GetLong();
    default:
        throw ExtException(E_FAIL, "Invalid size of signed byte code field");
    }
}

void
JDByteCode::DumpReg(ExtRemoteTyped regSlot)
{
    ext->Out(" R%u ", GetUnsigned(regSlot));
}

void
JDByteCode::DumpU2(ExtRemoteTyped value)
{
    ext->Out(" ushort:%d ", GetSigned(value));
}

void
JDByteCode::DumpI4(ExtRemoteTyped value)
{
    ext->Out(" int:%d ", GetSigned(value));
}

void
JDByteCode::DumpU4(ExtRemoteTyped value)
{
    ext->Out(" uint:%d ", GetUnsigned(value));
}

void
JDByteCode::DumpOffset(ExtRemoteTyped relativeJumpOffset, uint nextOffset)
{
    int jumpOffset = relativeJumpOffset.GetTypeSize() == 2 ? relativeJumpOffset.GetShort() : relativeJumpOffset.GetLong();
    ext->Out(" x:%04x (%4d)", nextOffset + jumpOffset, jumpOffset);
}

void
JDByteCode::DumpReg1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{    
    DumpReg(layout.Field("R0"));
}

void
JDByteCode::DumpReg2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
}

void
JDByteCode::DumpReg2B1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
    DumpI4(layout.Field("B2"));
}

void
JDByteCode::DumpReg3(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
    DumpReg(layout.Field("R2"));
}

void
JDByteCode::DumpReg3B1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
    DumpReg(layout.Field("R2"));
    DumpI4(layout.Field("B3"));
}

void
JDByteCode::DumpReg4(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
    DumpReg(layout.Field("R2"));
    DumpReg(layout.Field("R3"));
}

void
JDByteCode::DumpReg5(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpReg(layout.Field("R1"));
    DumpReg(layout.Field("R2"));
    DumpReg(layout.Field("R3"));
    DumpReg(layout.Field("R4"));
}

void
JDByteCode::DumpW1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{    
    DumpU2(layout.Field("C0"));
}

void
JDByteCode::DumpReg1Int2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg(layout.Field("R0"));
    DumpI4(layout.Field("C0"));
    DumpI4(layout.Field("C1"));
}

void
JDByteCode::DumpCallI(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    ExtRemoteTyped returnRegField = layout.Field("Return");
    uint returnReg = (uint)-1;
    switch (returnRegField.GetTypeSize())
    {
    case 1:
        if (returnRegField.GetChar() != -1)
        {
            returnReg = (uint)returnRegField.GetChar();
        }
        break;
    case 2:
        if (returnRegField.GetShort() != -1)
        {
            returnReg = (uint)returnRegField.GetShort();
        }
        break;
    case 4:
        if (returnRegField.GetUlong() != (uint)-1)
        {
            returnReg = (uint)returnRegField.GetShort();
        }
    };
    if (returnReg != (uint)-1)
    {        
        ext->Out(" R%u =", returnReg);
    }
    DumpReg(layout.Field("Function"));
    ext->Out("(ArgCount: %u)", GetUnsigned(layout.Field("ArgCount")));
}

void
JDByteCode::DumpCallIExtended(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpCallI(layout, opcodeStr, nextOffset);
    if (layout.Field("Options").GetUchar() & CallIExtended_SpreadArgs)
    {
        // TODO: dump spread args
        ext->Out(L" spreadArgs [???]");
    }
}

void
JDByteCode::DumpBrLong(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpBr(layout, opcodeStr, nextOffset);
}

void
JDByteCode::DumpBr(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpOffset(layout.Field("RelativeJumpOffset"), nextOffset);
}

void
JDByteCode::DumpBrS(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpBr(layout, opcodeStr, nextOffset);
    DumpI4(layout.Field("val"));
}

void
JDByteCode::DumpBrReg1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpBr(layout, opcodeStr, nextOffset);
    DumpReg(layout.Field("R1"));
}

void
JDByteCode::DumpBrReg2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpBr(layout, opcodeStr, nextOffset);
    DumpReg(layout.Field("R1"));
    DumpReg(layout.Field("R2"));
}

void
JDByteCode::DumpReg2Int1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    bool isGetCachedFunc = ENUM_EQUAL(opcodeStr, GetCachedFunc);
    DumpReg(layout.Field("R0"));
    ext->Out("=");
    if (isGetCachedFunc)
    {
        ext->Out(" func(");
    }
    DumpReg(layout.Field("R1"));
    ext->Out(", %d", GetSigned(layout.Field("C1")));
    if (isGetCachedFunc)
    {
        ext->Out(")");
    }   
}

wchar_t *
JDByteCode::GetPropertyNameFromCacheId(uint inlineCacheIndex, wchar_t * buffer, ULONG bufferSize)
{
    if (this->propertyNameReader)
    {
        int32 propertyId = this->functionBody.GetCacheIdToPropertyIdMap()[(ULONG)inlineCacheIndex].GetLong();
        ExtRemoteTyped propertyName("(wchar_t *)@$extin", this->propertyNameReader->GetNameByPropertyId(propertyId));
        return (*propertyName).GetString(buffer, bufferSize, bufferSize);
    }
    return L"???";
}


void
JDByteCode::DumpElementRootCP(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    uint value = GetUnsigned(layout.Field("Value"));    
    uint inlineCacheIndex = GetUnsigned(layout.Field("inlineCacheIndex"));
    wchar_t tempBuffer[1024];
    wchar_t * propertyName = this->GetPropertyNameFromCacheId(inlineCacheIndex, tempBuffer, _countof(tempBuffer));
    if (ENUM_EQUAL(opcodeStr, LdRoot) || ENUM_EQUAL(opcodeStr, ProfiledLdRoot))
    {
        ext->Out(" R%u = root.%S #%u",
            value,
            propertyName,
            inlineCacheIndex);
    }
    else if (ENUM_EQUAL(opcodeStr, StRoot) || ENUM_EQUAL(opcodeStr, InitRoot)
        || ENUM_EQUAL(opcodeStr, ProfiledStRoot) || ENUM_EQUAL(opcodeStr, ProfiledInitRoot))
    {
        ext->Out(" root.%S = R%u #%u",
            propertyName,
            value,
            inlineCacheIndex);
    }
    else
    {
        ext->Out("<ElementRootCP>");
    }

    if (ENUM_EQUAL(opcodeStr, Profiled))
    {
        ext->Out(" <%u>", inlineCacheIndex);
    }
}

void
JDByteCode::DumpElementCP(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{

    uint value = GetUnsigned(layout.Field("Value"));
    uint instance = GetUnsigned(layout.Field("Instance"));
    uint inlineCacheIndex = GetUnsigned(layout.Field("inlineCacheIndex"));
    wchar_t tempBuffer[1024];
    wchar_t * propertyName = this->GetPropertyNameFromCacheId(inlineCacheIndex, tempBuffer, _countof(tempBuffer));
    if (ENUM_EQUAL(opcodeStr, Ld) || ENUM_EQUAL(opcodeStr, ProfiledLd) || ENUM_EQUAL(opcodeStr, ScopedLdMethodFld))
    {
        ext->Out(" R%u = R%u.%S #%u",
            value,
            instance,
            propertyName,
            inlineCacheIndex);
    }
    else if (ENUM_EQUAL(opcodeStr, St) || ENUM_EQUAL(opcodeStr, Init)
        || ENUM_EQUAL(opcodeStr, ProfiledSt) || ENUM_EQUAL(opcodeStr, ProfiledInit))
    {
        ext->Out(" R%u.%S = R%u #%u",
            instance,
            propertyName,
            value,
            inlineCacheIndex);
    }
    else if (ENUM_EQUAL(opcodeStr, ScopedLd))
    {
        ext->Out(" R%u = R%u.%S, R%u #%u",
            value,
            instance,
            propertyName,
            RootObjectRegSlot,
            inlineCacheIndex);
    }
    else if (ENUM_EQUAL(opcodeStr, ScopedSt))
    {
        ext->Out(" R%u.%S = R%u, R%u #%u",
            instance,
            propertyName, 
            value,            
            RootObjectRegSlot,
            inlineCacheIndex);
    }
    else
    {
        ext->Out("<ElementCP>");
    }

    if (ENUM_EQUAL(opcodeStr, Profiled))
    {
        ext->Out(" <%u>", inlineCacheIndex);
    }
}

void
JDByteCode::DumpElementSlot(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    if (ENUM_EQUAL(opcodeStr, New))
    {
        DumpReg(layout.Field("Value"));
        ext->Out("= env:");
        DumpReg(layout.Field("Instance"));
        ext->Out("<FuncName???>");        // TODO: dump function name
    }
    else if (ENUM_EQUAL(opcodeStr, St))
    {
        ext->Out(" R%u[%u] = R%u", GetUnsigned(layout.Field("Instance")),
            GetUnsigned(layout.Field("SlotIndex")),
            GetUnsigned(layout.Field("Value")));
    }
    else
    {
        ext->Out(" R%u = R%u[%u]", GetUnsigned(layout.Field("Value")),
            GetUnsigned(layout.Field("Instance")),
            GetUnsigned(layout.Field("SlotIndex")));    
    }

}

void
JDByteCode::DumpElementI(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    uint value = GetUnsigned(layout.Field("Value"));
    uint instance = GetUnsigned(layout.Field("Instance"));
    uint element = GetUnsigned(layout.Field("Element"));

    if (ENUM_EQUAL(opcodeStr, Ld) || ENUM_EQUAL(opcodeStr, ProfiledLd) || ENUM_EQUAL(opcodeStr, Typeof))
    {
        ext->Out(" R%u = R%u[R%u]", value, instance, element);
    }
    else if (ENUM_EQUAL(opcodeStr, St) || ENUM_EQUAL(opcodeStr, ProfiledSt))
    {
        ext->Out(" R%u[R%u] = R%u", instance, element, value);
    }
    else if (ENUM_EQUAL(opcodeStr, Delete))
    {
        ext->Out(" R%u[R%u]", instance, element);
    }
    else
    {
        ext->Out("<ElementI>");
    }
}

void
JDByteCode::DumpReg1Unsigned1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    if (ENUM_EQUAL(opcodeStr, InitUndeclSlot) || ENUM_EQUAL(opcodeStr, InvalCachedScope)
        || ENUM_EQUAL(opcodeStr, NewScopeSlots))
    {
        ext->Out(" R%u[%u]", GetUnsigned(layout.Field("R0")), GetUnsigned(layout.Field("C1")));
    }
    else if (ENUM_EQUAL(opcodeStr, NewRegEx))
    {
        DumpReg(layout.Field("R0"));
        ext->Out("=<regex #%u>", GetUnsigned(layout.Field("C1")));
    }
    else
    {
        DumpReg(layout.Field("R0"));
        DumpU4(layout.Field("C1"));
    }
}

void
JDByteCode::DumpUnsigned1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpU4(layout.Field("C1"));
}

void
JDByteCode::DumpReg2WithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpReg2(layout, opcodeStr, nextOffset);
    ext->Out(" <%u>", GetUnsigned(layout.Field("inlineCacheIndex")));
}

void
JDByteCode::DumpCallIWithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpCallI(layout, opcodeStr, nextOffset);
    ext->Out(" <%u>", GetUnsigned(layout.Field("inlineCacheIndex")));
}

void
JDByteCode::DumpCallIExtendedWithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    DumpCallIExtended(layout, opcodeStr, nextOffset);
    ext->Out(" <%u>", GetUnsigned(layout.Field("inlineCacheIndex")));
}

void
JDByteCode::DumpArg(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    ext->Out(" Out%u =", GetUnsigned(layout.Field("Arg")));
    DumpReg(layout.Field("Reg"));
}

void
JDByteCode::DumpStartCall(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset)
{
    ext->Out(" ArgCount: %u", GetUnsigned(layout.Field("ArgCount")));
}

void
JDByteCode::DumpConstantTable()
{
    if (!this->hasFunctionBody)
    {
        return;
    }

    ext->Out("    Constant Table:\n    ======== =====\n");
    uint count = this->functionBody.GetConstCount();
    const uint firstRegSlot = 1; // FunctionBody::FirstRegSlot
    for (uint reg = firstRegSlot; reg < count; reg++)
    {
        // TODO: better interpretation of these constants instead of dumping the address?
        ext->Out("     R%u = 0x%p\n", reg, this->functionBody.GetConstTable()[(ULONG)(reg - firstRegSlot)].GetPtr());
    }    
}


void
JDByteCode::DumpImplicitArgIns()
{
    if (!this->hasFunctionBody)
    {
        return;
    }

    ushort inParamCount = this->functionBody.GetParamCount();
    if (inParamCount <= 1 || !this->functionBody.HasImplicitArgIns())
    {
        return;
    }
    ext->Out(L"    Implicit Arg Ins:\n    ======== =====\n");
    uint constCount = this->functionBody.GetConstCount();
    
    for (uint reg = 1; reg < inParamCount; reg++)
    {
        ext->Out("     R%u = ArgIn_A In%u\n", reg + constCount - 1, reg);
    }    
}
void
JDByteCode::DumpBytes(ExtRemoteTyped bytes)
{
    if (this->verbose)
    {
        ext->Out("Byte code buffer: 0x%p\n", bytes.GetPtr());
    }
    DumpConstantTable();
    DumpImplicitArgIns();

    LONG currentOffset = 0;
    bool foundReaderOffset = false;
    while (true)
    {
        bool isBreakPoint = 0;
        LONG layoutStart = currentOffset + 1;
        unsigned char opcodeByte = bytes[currentOffset].GetUchar();        
        char * opcodeStr = GetEnumString(ExtRemoteTyped(ext->FillModule("(%s!Js::OpCode)@$extin"), opcodeByte));
        if (ENUM_EQUAL(opcodeStr, EndOfBlock))
        {
            break;
        }

        if (ENUM_EQUAL(opcodeStr, Break) && this->hasFunctionBody)
        {
            isBreakPoint = true;
            ExtRemoteTyped probeBackingBlock = functionBody.GetProbeBackingStore();
            if (probeBackingBlock.GetPtr() != 0)
            {
                // If the debugger created a probe backing block, use that instead.
                opcodeByte = probeBackingBlock.Field("m_content")[currentOffset].GetUchar();
                opcodeStr = GetEnumString(ExtRemoteTyped(ext->FillModule("(%s!Js::OpCode)@$extin"), opcodeByte));
            }
        }

        // Process opcode prefix
        unsigned char opcodePrefix = 0;
        int layoutSizeEnum = LayoutSize_SmallLayout;
        bool isExtendOpcode = false;        
        if (ENUM_EQUAL(opcodeStr, ExtendedOpcodePrefix))
        {            
            opcodePrefix = opcodeByte;                    
            isExtendOpcode = true;
        }
        else if (ENUM_EQUAL(opcodeStr, MediumLayoutPrefix))
        {
            layoutSizeEnum = LayoutSize_MediumLayout;
            opcodePrefix = opcodeByte;
        }
        else if (ENUM_EQUAL(opcodeStr, ExtendedMediumLayoutPrefix))
        {
            layoutSizeEnum = LayoutSize_MediumLayout;
            opcodePrefix = opcodeByte;
            isExtendOpcode = true;
        }
        else if (ENUM_EQUAL(opcodeStr, LargeLayoutPrefix))
        {
            layoutSizeEnum = LayoutSize_LargeLayout;
            opcodePrefix = opcodeByte;
        }
        else if (ENUM_EQUAL(opcodeStr, ExtendedLargeLayoutPrefix))
        {
            layoutSizeEnum = LayoutSize_LargeLayout;
            opcodePrefix = opcodeByte;
            isExtendOpcode = true;
        }

        ExtRemoteTyped opCodeLayoutTable;
        ExtRemoteTyped opCodeAttrTable;
        if (opcodePrefix != 0)
        {
            // Read the opcode byte if there is a prefix
            layoutStart++;
            opcodeByte = bytes[currentOffset + 1].GetUchar();

            if (isExtendOpcode)
            {
                opcodeStr = GetEnumString(ExtRemoteTyped(ext->FillModule("(%s!Js::OpCode)@$extin"), opcodeByte + (opcodePrefix << 8)));
                opCodeLayoutTable = extendedLayoutTable;
                opCodeAttrTable = extendedAttributesTable;
            }
            else
            {
                opcodeStr = GetEnumString(ExtRemoteTyped(ext->FillModule("(%s!Js::OpCode)@$extin"), opcodeByte));
                opCodeLayoutTable = layoutTable;
                opCodeAttrTable = attributesTable;
            }
        }
        else
        {
            opCodeLayoutTable = layoutTable;
            opCodeAttrTable = attributesTable;
        }


        uint layoutType = opCodeLayoutTable[(ULONG)opcodeByte].Field("_value").GetUlong();
        ExtRemoteTyped layoutEnum(ext->FillModule("(%s!Js::OpLayoutType::_E)@$extin"), layoutType);
        char layoutTypeName[MAX_LAYOUT_TYPE_NAME];
        int attribute = attributesTable[(ULONG)opcodeByte].GetLong();
        char * layoutStr = GetEnumString(layoutEnum);
        char * plainLayoutStr = layoutStr;

        bool isProfiledLayout = (strncmp(layoutStr, "Profiled", _countof("Profiled") - 1) == 0);
        if (isProfiledLayout)
        {
            plainLayoutStr = layoutStr + _countof("Profiled") - 1;
            if (attribute & OpcodeAttr_OpHasMultiSizeLayout)
            {
                sprintf_s(layoutTypeName, MAX_LAYOUT_TYPE_NAME, "%%s!Js::OpLayoutDynamicProfile<Js::OpLayoutT_%s<Js::LayoutSizePolicy<%d> > >", plainLayoutStr, layoutSizeEnum);
            }
            else
            {
                if (layoutSizeEnum != LayoutSize_SmallLayout)
                {
                    ext->ThrowLastError("Non-multisize layout opcode shouldn't have layout size prefix");
                }
                sprintf_s(layoutTypeName, MAX_LAYOUT_TYPE_NAME, "%%s!Js::OpLayoutDynamicProfile<Js::OpLayout%s>", plainLayoutStr);
            }
        }
        else
        {
            if (attribute & OpcodeAttr_OpHasMultiSizeLayout)
            {
                sprintf_s(layoutTypeName, MAX_LAYOUT_TYPE_NAME, "%%s!Js::OpLayoutT_%s<Js::LayoutSizePolicy<%d> >", layoutStr, layoutSizeEnum);
            }
            else
            {
                if (layoutSizeEnum != LayoutSize_SmallLayout)
                {
                    ext->ThrowLastError("Non-multisize layout opcode shouldn't have layout size prefix");
                }
                sprintf_s(layoutTypeName, MAX_LAYOUT_TYPE_NAME, "%%s!Js::OpLayout%s", layoutStr);
            }
        }

        ExtRemoteTyped layout(ext->FillModule(layoutTypeName), bytes.GetPtr() + layoutStart, false);
        uint layoutSize = layout.GetTypeSize();
        uint nextOffset = layoutStart + layoutSize;

        ext->Out("%s %s%04x", 
            isBreakPoint ? "BP " : "   ",
            (readerOffset == currentOffset) ? "=>" : "  ", currentOffset);
        
        foundReaderOffset = foundReaderOffset || (readerOffset == currentOffset);

        if (verbose)
        {
            if (opcodePrefix != 0)
            {
                ext->Out(" %02x", opcodePrefix);
            }
            else
            {
                ext->Out("   ");
            }
            ext->Out(" %02x", opcodeByte);
            for (uint i = 0; i < layout.GetTypeSize(); i++)
            {
                ext->Out(" %02x", bytes[layoutStart + i].GetUchar());
            }

            const int MaxLayoutBytes = 10;
            if (layoutSize < MaxLayoutBytes)
            {
                for (uint i = 0; i < MaxLayoutBytes - layoutSize; i++)
                {
                    ext->Out("   ");
                }
            }
        }
        ext->Out(" %-30s", opcodeStr);

#define PROCESS_LAYOUT(layoutName) \
        if (strcmp(plainLayoutStr, #layoutName) == 0) \
        { \
            Dump##layoutName(layout, opcodeStr, nextOffset); \
        }


        if (strcmp(plainLayoutStr, "Empty") == 0)
        {
            currentOffset += 1;
            ext->Out("\n");
            continue;
        }
        else PROCESS_LAYOUT(CallI)
        else PROCESS_LAYOUT(CallIWithICIndex)
        else PROCESS_LAYOUT(CallIExtended)
        else PROCESS_LAYOUT(CallIExtendedWithICIndex)
        else PROCESS_LAYOUT(Reg1)
        else PROCESS_LAYOUT(Reg2)
        else PROCESS_LAYOUT(Reg2B1)
        else PROCESS_LAYOUT(Reg2WithICIndex)
        else PROCESS_LAYOUT(Reg3)
        else PROCESS_LAYOUT(Reg3B1)
        else PROCESS_LAYOUT(Reg4)
        else PROCESS_LAYOUT(Reg5)        
        else PROCESS_LAYOUT(W1)
        else PROCESS_LAYOUT(Reg1Int2)
        else PROCESS_LAYOUT(Reg1Unsigned1)
        else PROCESS_LAYOUT(Reg2Int1)
        else PROCESS_LAYOUT(Unsigned1)
        else PROCESS_LAYOUT(ElementRootCP)
        else PROCESS_LAYOUT(ElementCP)
        else PROCESS_LAYOUT(ElementSlot)
        else PROCESS_LAYOUT(ElementI)
        else PROCESS_LAYOUT(BrLong)
        else PROCESS_LAYOUT(Br)
        else PROCESS_LAYOUT(BrS)
        else PROCESS_LAYOUT(BrReg1)
        else PROCESS_LAYOUT(BrReg2)
        else PROCESS_LAYOUT(StartCall)
        else PROCESS_LAYOUT(Arg)
        else
        {
            ext->Out(" <%s>", plainLayoutStr);
        }
     
        if (isProfiledLayout)
        {
            ext->Out(" <%u>", GetUnsigned(layout.Field("profileId")));
        }        
        currentOffset = nextOffset;

        ext->Out("\n");
    }

    if (readerOffset != (ULONG64)-1 && !foundReaderOffset)
    {
        ext->Out("WARNING: Current interpreter offset not found at 0x%x\n", readerOffset);
    }
}

void
JDByteCode::DumpForFunctionBody(ExtRemoteTyped funcBody)
{
    this->hasFunctionBody = true;
    this->functionBody = funcBody;
    this->propertyNameReader = new EXT_CLASS_BASE::PropertyNameReader(ext, RemoteFunctionBody(funcBody).GetThreadContext());
    RemoteFunctionBody(funcBody).PrintNameAndNumberWithLink(ext);
    ext->Out("\n");
    if (dumpProbeBackingBlock)
    {
        ExtRemoteTyped probeBackingBlock = this->functionBody.GetProbeBackingStore();
        if (probeBackingBlock.GetPtr() == 0)
        {
            ext->ThrowLastError("No probe backing block to dump");
        }
        DumpBytes(probeBackingBlock.Field("m_content"));
        return;
    }   
    DumpBytes(functionBody.GetByteCodeBlock().Field("m_content"));
}

void
JDByteCode::DumpForJavascriptFunction(ExtRemoteTyped functionObject)
{
    // TODO: Need script function check
    DumpForScriptFunction(functionObject);
}

void
JDByteCode::DumpForScriptFunction(ExtRemoteTyped functionObject)
{
    // TODO: Need script function check
    DumpForFunctionBody(ExtRemoteTyped(ext->FillModule("(%s!Js::FunctionBody *)@$extin"),
        functionObject.Field("functionInfo").GetPtr()));
}

void
JDByteCode::DumpForRecyclableObject(ExtRemoteTyped recyclableObject)
{
    if (!ENUM_EQUAL(recyclableObject.Field("type.typeId").GetSimpleValue(), TypeIds_Function))
    {
        throw ExtException(E_FAIL, "Not JavascriptFunction");
    }
    DumpForJavascriptFunction(ExtRemoteTyped(ext->FillModule("(%s!Js::JavascriptFunction *)@$extin"),
        recyclableObject.GetPtr()));
}

void
JDByteCode::DumpForFunc(ExtRemoteTyped func)
{
    DumpForFunctionBody(func.Field("m_jnFunction"));    
}

void
JDByteCode::DumpForIRBuilder(ExtRemoteTyped irbuilder)
{
    ExtRemoteTyped reader = irbuilder.Field("m_jnReader");
    this->readerOffset = reader.Field("m_currentLocation").GetPtr() - reader.Field("m_startLocation").GetPtr();

    DumpForFunctionBody(irbuilder.Field("m_functionBody"));
}

void
JDByteCode::DumpForInterpreterStackFrame(ExtRemoteTyped interpreterStackFrame)
{
    ExtRemoteTyped reader = interpreterStackFrame.Field("m_reader");
    this->readerOffset = reader.Field("m_currentLocation").GetPtr() - reader.Field("m_startLocation").GetPtr();
    DumpForFunctionBody(interpreterStackFrame.Field("m_functionBody"));
}

JD_PRIVATE_COMMAND(bc,
    "Dump bytecode",
    "{p;b,o;probe;dump byte code at the probe backing block}"
    "{v;b,o;verbose;dump the bytes of the bytecode}"
    "{;x;bytecode;expression or address of bytecode or InterpreterStackFrame/FunctionBody/JavascriptFunction}")
{
    const bool dumpProbeBackingBlock = HasArg("p");
    const bool verbose = HasArg("v");
    PCSTR arg = GetUnnamedArgStr(0);
    ExtRemoteTyped input = ExtRemoteTyped(arg);
    JDByteCode jdbytecode(this, dumpProbeBackingBlock, verbose);
    PCSTR inputType = input.GetTypeName();
    if (strcmp(inputType, "int") == 0 || strcmp(inputType, "int64") == 0)
    {
        // Just an address
        Out("0x%u treated as the byte code buffer", input.GetLong64());
        jdbytecode.DumpBytes(ExtRemoteTyped("(unsigned char *)@$extin", input.GetLong64()));
    }
    else
    {
        inputType = JDUtil::StripStructClass(inputType);        
        if (strcmp(inputType, "Js::FunctionBody") == 0 || strcmp(inputType, "Js::FunctionBody *") == 0)
        {
            jdbytecode.DumpForFunctionBody(input);
        }
        else if (strcmp(inputType, "Js::InterpreterStackFrame") == 0 || strcmp(inputType, "Js::InterpreterStackFrame *") == 0)
        {
            jdbytecode.DumpForInterpreterStackFrame(input);
        }
        else if (strcmp(inputType, "Js::JavascriptFunction") == 0 || strcmp(inputType, "Js::JavascriptFunction *") == 0)
        {
            jdbytecode.DumpForJavascriptFunction(input);
        }
        else if (strcmp(inputType, "Js::ScriptFunction") == 0 || strcmp(inputType, "Js::ScriptFunction *") == 0)
        {
            jdbytecode.DumpForScriptFunction(input);
        }
        else if (strcmp(inputType, "Js::RecyclableObject") == 0 || strcmp(inputType, "Js::RecyclableObject *") == 0)
        {
            jdbytecode.DumpForRecyclableObject(input);
        }
        else if (strcmp(inputType, "Func") == 0 || strcmp(inputType, "Func *") == 0)
        {
            jdbytecode.DumpForFunc(input);
        }
        else if (strcmp(inputType, "IRBuilder") == 0 || strcmp(inputType, "IRBuilder *") == 0)
        {
            jdbytecode.DumpForIRBuilder(input);
        }
        else
        {
            Out(input.GetTypeName());
            throw ExtException(E_FAIL, "Unknown type to get output from");
        }
    }
}
#endif
