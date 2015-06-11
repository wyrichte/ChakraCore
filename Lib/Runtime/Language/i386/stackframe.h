//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once


namespace Js {

#ifndef _M_IX86
#error This is only for x86
#endif

    class X86StackFrame {
    public:
        X86StackFrame() : frame(null), codeAddr(null), stackCheckCodeHeight(0), addressOfCodeAddr(nullptr) {};

        bool InitializeByFrameId(void * frameAddress, ScriptContext* scriptContext);
        bool InitializeByReturnAddress(void * returnAddress, ScriptContext* scriptContext);

        bool Next();

        void *  GetInstructionPointer() { return codeAddr; }
        void ** GetArgv() { return frame + 2; }
        void *  GetReturnAddress() { return frame[1]; }
        void *  GetAddressOfReturnAddress() { return &frame[1]; }
        void *  GetAddressOfInstructionPointer() const { return addressOfCodeAddr; }
        void *  GetFrame() const { return (void *)frame; }

        void SetReturnAddress(void * address) { frame[1] = address; }
        bool SkipToFrame(void * frameAddress);

        bool IsInStackCheckCode(void *entry) const;

    private:
        void ** frame;      // ebp
        void * codeAddr;    // eip
        void * addressOfCodeAddr;
        size_t stackCheckCodeHeight;

        static const size_t stackCheckCodeHeightThreadBound = StackFrameConstants::StackCheckCodeHeightThreadBound;
        static const size_t stackCheckCodeHeightNotThreadBound = StackFrameConstants::StackCheckCodeHeightNotThreadBound;
        static const size_t stackCheckCodeHeightWithInterruptProbe = StackFrameConstants::StackCheckCodeHeightWithInterruptProbe;
    };

};
