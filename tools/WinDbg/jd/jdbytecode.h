//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "JDUtil.h"

class JDByteCode : public JDUtil
{
public:
    JDByteCode(bool dumpProbeBackingBlock, bool verbose);
    ~JDByteCode();
    void DumpForInterpreterStackFrame(ExtRemoteTyped interpreterStackFrame);
    void DumpForFunctionBody(ExtRemoteTyped functionBody);
    void DumpForJavascriptFunction(ExtRemoteTyped functionObject);
    void DumpForScriptFunction(ExtRemoteTyped functionObject);
    void DumpForRecyclableObject(ExtRemoteTyped recyclableObject);
    void DumpForFunc(ExtRemoteTyped func);
    void DumpForIRBuilder(ExtRemoteTyped irbuilder);    
    void DumpBytes(ExtRemoteTyped bytes);
    void DumpByteCodeBuffer(ULONG64 address);
public:


private:
    bool dumpProbeBackingBlock;
    bool verbose;
    bool hasFunctionBody;

    RemoteFunctionBody functionBody;
    EXT_CLASS_BASE::PropertyNameReader * propertyNameReader;
    JDByteCodeCachedData const& cachedData;
    uint RootObjectRegSlot;
    ULONG64 readerOffset;
    unsigned char CallIExtended_SpreadArgs;

    uint GetUnsigned(ExtRemoteTyped unsignedField);
    int GetSigned(ExtRemoteTyped signedField);
    void DumpReg(ExtRemoteTyped regSlot);
    void DumpU2(ExtRemoteTyped value);
    void DumpI4(ExtRemoteTyped value);
    void DumpU4(ExtRemoteTyped value);
    void DumpOffset(ExtRemoteTyped relativeJumpOffset, uint nextOffset);

    void DumpEmpty(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset) {};
    void DumpReg1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2B1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3B1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3C(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg4(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg5(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg1Unsigned1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpUnsigned1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpW1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg1Int2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2Int1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallI(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIExtended(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    
    void DumpReg2WithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIWithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIExtendedWithICIndex(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpBrLong(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBr(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrS(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg1Unsigned1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpElementRootCP(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementCP(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementP(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlot(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlotI1(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlotI2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementScopedC2(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementI(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpStartCall(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpArg(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpAuxiliary(ExtRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpConstantTable();
    void DumpImplicitArgIns();

    char16 * GetPropertyNameFromCacheId(uint inlineCacheIndex, char16 * buffer, ULONG bufferSize);
    char16 * GetPropertyNameFromReferencedIndex(uint referencedIndex, char16 * buffer, ULONG bufferSize);
};