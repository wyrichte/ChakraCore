//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
// Shared intepreter loop
//
// This holds the single definition of the interpreter loop.
// It allows for configurable copies of the loop that do extra work without
// impacting the mainline performance. (for example the debug loop can simply
// check a bit without concern for impacting the nondebug mode.)
#ifdef INTERPRETER_ASMJS
#define INTERPRETER_OPCODE OpCodeAsmJs
#else
#define INTERPRETER_OPCODE OpCode
#endif
#ifdef PROVIDE_DEBUGGING
#define DEBUGGING_LOOP 1
#else
#define DEBUGGING_LOOP 0
#endif
#ifdef PROVIDE_LANGUAGESERVICE
#define LANGUAGESERVICE_LOOP 1
#define SETTARGET(r) target = r
#define SETTARGET_Small(r) target = (RegSlot)r
#define SETTARGET_Medium SETTARGET_Small
#define SETTARGET_Large SETTARGET
#define STARTCALL(a) popCount = a
#define DONECALL() popCount = 0
#define UNSETTARGET() target = Js::Constants::NoRegister
#define INTERPRETERLOOP_PARAMETERS_FORMAL RegSlot& target, ArgSlot& popCount
// The language service skips exceptions at the instruction level. These
// language service specific definitions record the register that is supposed to
// be initialized by these instructions and therefore which register should be
// initialized when an exception is thrown.
#define PROCESS_CUSTOM_LS(name, func, layout) \
    case INTERPRETER_OPCODE::name: \
    { \
        AssertMsg(OpCodeUtil::GetOpCodeLayout(INTERPRETER_OPCODE::name) == OpLayoutType::layout, "Validate correct layout"); \
        func##_LS(m_reader.layout(ip), target); \
        break; \
    }

#else
#define LANGUAGESERVICE_LOOP 0
#define SETTARGET(r)
#define SETTARGET_Small(r)
#define SETTARGET_Medium(r)
#define SETTARGET_Large(r)
#define STARTCALL(a)
#define DONECALL()
#define UNSETTARGET()
#define INTERPRETERLOOP_PARAMETERS_FORMAL
#define PROCESS_CUSTOM_LS(name, func, layout) PROCESS_CUSTOM(name, func, layout)
#endif
#ifdef PROVIDE_INTERPRETERPROFILE
#define INTERPRETERPROFILE 1
#define PROFILEDOP(prof, unprof) prof
#else
#define INTERPRETERPROFILE 0
#define PROFILEDOP(prof, unprof) unprof
#endif
#if defined (DBG)
// Win8 516184: Huge switch with lots of labels each having a few locals on ARM.DBG causes each occurrence
// of this function (call of a javascript function in interpreter mode) to take 7+KB stack space
// (without optimizations the compiler accounts for ALL locals inside case labels when allocating space on stack
// for locals - SP does not change inside the function). On other platforms this is still huge but better than ARM.
// The issue was surfaced in SunSpider\controlflow-recursive.js -- AV/stack overflow in interpreter mode.
// So, for DBG turn on optimizations to prevent this huge loss of stack.
#pragma optimize("g", on)
#endif
Var Js::InterpreterStackFrame::INTERPRETERLOOPNAME(INTERPRETERLOOP_PARAMETERS_FORMAL)
{
    PROBE_STACK(scriptContext, Js::Constants::MinStackInterpreter);

    if (this->m_reader.GetCurrentOffset() == 0)
    {
        // If this is the start of the function, then we've waited until after the stack probe above
        // to set up the FD/SS pointers, so do it now.
        if (this->m_functionBody->DoStackFrameDisplay())
        {
            this->SetLocalFrameDisplay(this->localFrameDisplay);
        }
        if (this->m_functionBody->DoStackScopeSlots())
        {
            this->SetLocalScopeSlots(this->localScopeSlots);
        }
    }

    Assert(this->returnAddress != null);
    AssertMsg(m_arguments == NULL || Js::ArgumentsObject::Is(m_arguments), "Bad arguments!");
    // IP Passing in the interpreter:
    // We keep a local copy of the bytecode's instruction pointer and
    // pass it by reference to the bytecode reader.
    // This allows the optimizer to recognize that the local (held in a register)
    // dominates the copy in the reader.
    // The effect is our dispatch loop is significantly smaller in the common case
    // on optimized builds.
    //
    // For checked builds this does mean we are incrementing 2 different counters to
    // track the ip.
    const byte* ip = m_reader.GetIP();
    while (true)
    {
        INTERPRETER_OPCODE op = ReadByteOp<INTERPRETER_OPCODE>(ip);

#ifdef TELEMETRY
        if( TELEMETRY_OPCODE_OFFSET_ENABLED )
        {
            OpcodeTelemetry& opcodeTelemetry = this->scriptContext->GetTelemetry().GetOpcodeTelemetry();
            opcodeTelemetry.ProgramLocationFunctionId    ( this->function->GetFunctionInfo()->GetLocalFunctionId() );
            opcodeTelemetry.ProgramLocationBytecodeOffset( this->m_reader.GetCurrentOffset() );
        }
#endif
        
#if DEBUGGING_LOOP || LANGUAGESERVICE_LOOP
        if (this->scriptContext->GetThreadContext()->Diagnostics->stepController.IsActive() &&
            this->scriptContext->GetThreadContext()->Diagnostics->stepController.IsStepComplete_AllowingFalsePositives(this))
        {
            // BrLong is used for branch island, we don't want to break over there, as they don't belong to any statement. Just skip this.
            if (!InterpreterStackFrame::IsBrLong(op, ip) && !this->m_functionBody->GetUtf8SourceInfo()->GetIsLibraryCode())
            {
                uint prevOffset = m_reader.GetCurrentOffset();
                InterpreterHaltState haltState(STOP_STEPCOMPLETE, m_functionBody);
                this->scriptContext->diagProbesContainer.DispatchStepHandler(&haltState, &op);
                if (prevOffset != m_reader.GetCurrentOffset())
                {
                    // The location of the statement has been changed, setnextstatement was called.
                    // Reset m_outParams and m_outSp as before SetNext was called, we could be in the middle of StartCall.
                    // It's fine to do because SetNext can only be done to a statement -- function-level destination, 
                    // and can't land to an expression inside call.
                    ResetOut();
                    ip = m_reader.GetIP();
                    continue;
                }
            }
        }
        // The break opcode will be handled later in the switch block.
        if (op != OpCode::Break && this->scriptContext->GetThreadContext()->Diagnostics->asyncBreakController.IsBreak())
        {
            if (!InterpreterStackFrame::IsBrLong(op, ip) && !this->m_functionBody->GetUtf8SourceInfo()->GetIsLibraryCode())
            {
                uint prevOffset = m_reader.GetCurrentOffset();
#if LANGUAGESERVICE_LOOP
                // Laungauge service does not handle ASYNCBREAK very well.
                InterpreterHaltState haltState(STOP_BREAKPOINT, m_functionBody);
#else
                InterpreterHaltState haltState(STOP_ASYNCBREAK, m_functionBody);
#endif
                this->scriptContext->diagProbesContainer.DispatchAsyncBreak(&haltState);
                if (prevOffset != m_reader.GetCurrentOffset())
                {
                    // The location of the statement has been changed, setnextstatement was called.
                    ip = m_reader.GetIP();
                    continue;
                }
            }
        }
SWAP_BP_FOR_OPCODE:
#endif
        switch (op)
        {
        case INTERPRETER_OPCODE::Ret:
            {
                //
                // Return "Reg: 0" as the return-value.
                // - JavaScript functions always return a value, and this value is always
                //   accessible to the caller.  For an empty "return;" or exiting the end of the
                //   function's body, it is assumed that the byte-code author
                //   (ByteCodeGenerator) will load 'undefined' into R0.
                // - If R0 has not explicitly been set, it will contain whatever garbage value
                //   was last set.
                //
                this->retOffset = m_reader.GetCurrentOffset();
                m_reader.Empty(ip);
                return GetReg((RegSlot)0);
            }

        case INTERPRETER_OPCODE::Yield:
            {
                m_reader.Reg2_Small(ip);
                return GetReg(GetFunctionBody()->GetYieldRegister());
            }

#define DEF2(x, op, func) PROCESS_##x(op, func)
#define DEF3(x, op, func, y) PROCESS_##x(op, func, y)
#define DEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Small)
#define DEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Small)
#define DEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Small, t)

#include "InterpreterHandler.inl"
            
            case INTERPRETER_OPCODE::Leave:
                // Return the continuation address to the helper.
                // This tells the helper that control left the scope without completing the try/handler,
                // which is particularly significant when executing a finally.
                m_reader.Empty(ip);
                return (Var)this->m_reader.GetCurrentOffset();
            case INTERPRETER_OPCODE::LeaveNull:
                // Return to the helper without specifying a continuation address,
                // indicating that the handler completed without jumping, so exception processing
                // should continue.
                m_reader.Empty(ip);
                return null;

            case INTERPRETER_OPCODE::ExtendedOpcodePrefix:
            {
                ip = [this
#if LANGUAGESERVICE_LOOP
                    , &target, &popCount
#endif
                ](const byte * ip) -> const byte *
                {
                    INTERPRETER_OPCODE op = (INTERPRETER_OPCODE)(ReadByteOp<INTERPRETER_OPCODE>(ip
#if DBG_DUMP
                    , true
#endif
                    ) + (INTERPRETER_OPCODE::ExtendedOpcodePrefix << 8));                    
                    switch (op)
                    {
#define EXDEF2(x, op, func) PROCESS_##x(op, func)
#define EXDEF3(x, op, func, y) PROCESS_##x(op, func, y)
#define EXDEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Small)
#define EXDEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Small)
#define EXDEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Small, t)
#include "InterpreterHandler.inl"
                        default:
                            // Help the C++ optimizer by declaring that the cases we
                            // have above are sufficient
                            AssertMsg(false, "dispatch to bad opcode");
                            __assume(false);
                    };
                    return ip;
                }(ip);

                if (switchProfileMode)
                {
                    // Aborting the current interpreter loop to switch the profile mode
                    return nullptr;
                }
                break;
            }
            case INTERPRETER_OPCODE::MediumLayoutPrefix:
            {
                Var yieldValue = nullptr;
                ip = [this, &yieldValue
#if LANGUAGESERVICE_LOOP
                    , &target, &popCount
#endif
                ](const byte * ip) -> const byte *
                {
                    INTERPRETER_OPCODE op = ReadByteOp<INTERPRETER_OPCODE>(ip);
                    switch (op)
                    {
                        case INTERPRETER_OPCODE::Yield:
                            m_reader.Reg2_Medium(ip);
                            yieldValue = GetReg(GetFunctionBody()->GetYieldRegister());
                            break;

#define DEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Medium)
#define DEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Medium)
#define DEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Medium, t)
#include "InterpreterHandler.inl"
                        default:
                            // Help the C++ optimizer by declaring that the cases we
                            // have above are sufficient
                            AssertMsg(false, "dispatch to bad opcode");
                            __assume(false);
                    }
                    return ip;
                }(ip);

                if (yieldValue != nullptr)
                {
                    return yieldValue;
                }

                if (switchProfileMode)
                {
                    // Aborting the current interpreter loop to switch the profile mode
                    return nullptr;
                }
                break;
            }
            case INTERPRETER_OPCODE::ExtendedMediumLayoutPrefix:
            {
#ifndef INTERPRETER_ASMJS  // Asmjs doesn't have any extended opcodes for now, remove that case
                ip = [this
#if LANGUAGESERVICE_LOOP
                    , &target, &popCount
#endif
                ](const byte * ip) -> const byte *
                {
                    INTERPRETER_OPCODE op = (INTERPRETER_OPCODE)(ReadByteOp<INTERPRETER_OPCODE>(ip
#if DBG_DUMP
                    , true
#endif
                    ) + (INTERPRETER_OPCODE::ExtendedOpcodePrefix << 8));
                    switch (op)
                    {
#define EXDEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Medium)
#define EXDEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Medium)
#define EXDEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Medium, t)
#include "InterpreterHandler.inl"
                        default:
                            // Help the C++ optimizer by declaring that the cases we
                            // have above are sufficient
                            AssertMsg(false, "dispatch to bad opcode");
                            __assume(false);
                    };
                    return ip;
                }(ip);

                if (switchProfileMode)
                {
                    // Aborting the current interpreter loop to switch the profile mode
                    return nullptr;
                }
#endif
                break;
            } 
            case INTERPRETER_OPCODE::LargeLayoutPrefix:
            {
                Var yieldValue = nullptr;

                ip = [this, &yieldValue
#if LANGUAGESERVICE_LOOP
                    , &target, &popCount
#endif
                ](const byte * ip) -> const byte *
                {
                    INTERPRETER_OPCODE op = ReadByteOp<INTERPRETER_OPCODE>(ip);
                    switch (op)
                    {
                        case INTERPRETER_OPCODE::Yield:
                            m_reader.Reg2_Large(ip);
                            yieldValue = GetReg(GetFunctionBody()->GetYieldRegister());
                            break;

#define DEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Large)
#define DEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Large)
#define DEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Large, t)
#include "InterpreterHandler.inl"
                        default:
                            // Help the C++ optimizer by declaring that the cases we
                            // have above are sufficient
                            AssertMsg(false, "dispatch to bad opcode");
                            __assume(false);
                    }
                    return ip;
                }(ip);

                if (yieldValue != nullptr)
                {
                    return yieldValue;
                }

                if(switchProfileMode)
                {
                    // Aborting the current interpreter loop to switch the profile mode
                    return nullptr;
                }
                break;
            }
            case INTERPRETER_OPCODE::ExtendedLargeLayoutPrefix:
            {
#ifndef INTERPRETER_ASMJS  // Asmjs doesn't have any extended opcodes for now, remove that case
                ip = [this
#if LANGUAGESERVICE_LOOP
                    , &target, &popCount
#endif
                ](const byte * ip) -> const byte *
                {
                    INTERPRETER_OPCODE op = (INTERPRETER_OPCODE)(ReadByteOp<INTERPRETER_OPCODE>(ip
#if DBG_DUMP
                    , true
#endif
                    ) + (INTERPRETER_OPCODE::ExtendedOpcodePrefix << 8));
                    switch (op)
                    {
#define EXDEF2_WMS(x, op, func) PROCESS_##x##_COMMON(op, func, _Large)
#define EXDEF3_WMS(x, op, func, y) PROCESS_##x##_COMMON(op, func, y, _Large)
#define EXDEF4_WMS(x, op, func, y, t) PROCESS_##x##_COMMON(op, func, y, _Large, t)
#include "InterpreterHandler.inl"
                        default:
                            // Help the C++ optimizer by declaring that the cases we
                            // have above are sufficient
                            AssertMsg(false, "dispatch to bad opcode");
                            __assume(false);
                    };
                    return ip;
                }(ip);

                if(switchProfileMode)
                {
                    // Aborting the current interpreter loop to switch the profile mode
                    return nullptr;
                }
#endif
                break;
            } 

            case INTERPRETER_OPCODE::EndOfBlock:
            {
                // Note that at this time though ip was advanced by 'OpCode op = ReadByteOp<INTERPRETER_OPCODE>(ip)', 
                // we haven't advanced m_reader.m_currentLocation yet, thus m_reader.m_currentLocation still points to EndOfBLock,
                // and that +1 will point to 1st byte past the buffer.
                Assert(m_reader.GetCurrentOffset() + sizeof(byte) == m_functionBody->GetByteCode()->GetLength());

                //
                // Reached an "OpCode::EndOfBlock" so need to exit this interpreter loop because
                // there is no more byte-code to execute.
                // - This prevents us from accessing random memory as byte-codes.
                // - Functions should contain an "OpCode::Ret" instruction to organize an
                //   orderly return.
                //
#if DEBUGGING_LOOP || LANGUAGESERVICE_LOOP
                // However, during debugging an exception can be skipped which causes the
                // statement that caused to exception to be skipped. If this statement is
                // the statement that contains the OpCode::Ret then the EndOfBlock will
                // be executed. In these cases it is sufficient to return undefined.
                return this->scriptContext->GetLibrary()->GetUndefined();
#else
                return null;
#endif
            }  

            case INTERPRETER_OPCODE::Break:
            {
#if DEBUGGING_LOOP || LANGUAGESERVICE_LOOP
                // The reader has already advanced the IP:
                if (this->m_functionBody->ProbeAtOffset(m_reader.GetCurrentOffset(), &op))
                {
                    uint prevOffset = m_reader.GetCurrentOffset();
                    InterpreterHaltState haltState(STOP_BREAKPOINT, m_functionBody);
                    this->scriptContext->diagProbesContainer.DispatchProbeHandlers(&haltState);
                    if (prevOffset != m_reader.GetCurrentOffset())
                    {
                        // The location of the statement has been changed, setnextstatement was called.
                        ip = m_reader.GetIP();
                        continue;
                    }
                    // Yes this is an ugly jump back to the start of the switch.
                    // If you have a better idea on how to do this that doesn't impact perf,
                    // or maintainability of the interpreter loop I'll by happy to change this.
                    goto SWAP_BP_FOR_OPCODE;
                }
                else
                {
#if DEBUGGING_LOOP
                    // an inline break statement rather than a probe
                    if (!this->scriptContext->GetThreadContext()->Diagnostics->stepController.ContinueFromInlineBreakpoint())
                    {
                        uint prevOffset = m_reader.GetCurrentOffset();
                        InterpreterHaltState haltState(STOP_INLINEBREAKPOINT, m_functionBody);
                        this->scriptContext->diagProbesContainer.DispatchInlineBreakpoint(&haltState);
                        if (prevOffset != m_reader.GetCurrentOffset())
                        {
                            // The location of the statement has been changed, setnextstatement was called.
                            ip = m_reader.GetIP();
                            continue;
                        }
                    }
#endif
                    // Consume after dispatching
                    m_reader.Empty(ip);
                }
#else
                m_reader.Empty(ip);
#endif
                break;
            }
            default:
                // Help the C++ optimizer by declaring that the cases we
                // have above are sufficient
                AssertMsg(false, "dispatch to bad opcode");
                __assume(false);
        }
    }
}
#if defined (DBG)
// Restore optimizations to what's specified by the /O switch.
#pragma optimize("", on)
#endif
#undef DEBUGGING_LOOP
#undef LANGUAGESERVICE_LOOP
#undef SETTARGET
#undef SETTARGET_Small
#undef SETTARGET_Medium
#undef SETTARGET_Large
#undef STARTCALL
#undef DONECALL
#undef UNSETTARGET
#undef INTERPRETERLOOP_PARAMETERS_FORMAL
#undef INTERPRETERPROFILE
#undef PROFILEDOP
#undef PROCESS_CUSTOM_LS
#undef INTERPRETER_OPCODE
