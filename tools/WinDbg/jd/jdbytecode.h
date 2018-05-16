//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "JDUtil.h"

class JDByteCode : public JDUtil
{
public:
    JDByteCode(ULONG64 bytesWindow, bool dumpProbeBackingBlock, bool verbose);
    ~JDByteCode();
    bool DumpForInterpreterStackFrame(JDRemoteTyped interpreterStackFrame);
    bool DumpForFunctionBody(JDRemoteTyped functionBody);
    bool DumpForJavascriptFunction(JDRemoteTyped functionObject);
    bool DumpForScriptFunction(JDRemoteTyped functionObject);
    bool DumpForRecyclableObject(JDRemoteTyped recyclableObject);
    bool DumpForFunc(JDRemoteTyped func);
    bool DumpForIRBuilder(JDRemoteTyped irbuilder);
    bool DumpBytes(JDRemoteTyped bytes);
    bool DumpByteCodeBuffer(ULONG64 address);

private:
    bool dumpProbeBackingBlock;
    bool verbose;
    bool hasFunctionBody;

    RemoteFunctionBody functionBody;
    EXT_CLASS_BASE::PropertyNameReader * propertyNameReader;
    JDByteCodeCachedData const& cachedData;
    uint RootObjectRegSlot;
    ULONG64 readerOffset;
    ULONG64 bytesWindow;
    unsigned char CallIExtended_SpreadArgs;

    uint GetUnsigned(JDRemoteTyped unsignedField);
    int GetSigned(JDRemoteTyped signedField);
    void DumpReg(JDRemoteTyped regSlot);
    void DumpU2(JDRemoteTyped value);
    void DumpI4(JDRemoteTyped value);
    void DumpU4(JDRemoteTyped value);
    void DumpOffset(JDRemoteTyped relativeJumpOffset, uint nextOffset);

    void DumpEmpty(JDRemoteTyped layout, char * opcodeStr, uint nextOffset) {};
    void DumpReg1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2B1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3B1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg3C(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg4(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg5(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg1Unsigned1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpUnsigned1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpW1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg1Int2(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpReg2Int1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallI(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIExtended(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    
    void DumpReg2WithICIndex(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIWithICIndex(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpCallIExtendedWithICIndex(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpBrLong(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBr(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrS(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg1Unsigned1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpBrReg2(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpElementRootCP(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementCP(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementP(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlot(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlotI1(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementSlotI2(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementScopedC2(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpElementI(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpStartCall(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpArg(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);
    void DumpAuxiliary(JDRemoteTyped layout, char * opcodeStr, uint nextOffset);

    void DumpConstantTable();
    void DumpImplicitArgIns();

    char16 * GetPropertyNameFromCacheId(uint inlineCacheIndex, char16 * buffer, ULONG bufferSize);
    char16 * GetPropertyNameFromReferencedIndex(uint referencedIndex, char16 * buffer, ULONG bufferSize);
};