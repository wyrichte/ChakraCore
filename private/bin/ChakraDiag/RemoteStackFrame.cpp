//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag
{
    RemoteStackFrame::RemoteStackFrame() :
        m_effectiveFrameBase(nullptr),
        m_returnAddress(nullptr),
        m_frameEnd(nullptr),
        m_frameStart(nullptr),
        m_functionAddr(nullptr),
        m_argvAddr(nullptr),
        m_callInfo(0),
        m_isInlineFrame(false),
        m_loopNumber(LoopHeader::NoLoop),
        m_frameId(0)
    {
    }

    RemoteStackFrame::~RemoteStackFrame()
    {
    }

    ULONG RemoteStackFrame::GetRow() { return m_rowColumn.GetRow(this); }
    ULONG RemoteStackFrame::GetColumn() { return m_rowColumn.GetColumn(this); }

    // Returned memory must be freed by the caller using delete[].
    char16* RemoteStackFrame::GetUri() const
    {
        if (IsAmd64FakeEHFrame())
        {
            return WcsDup(_u("JS Internal"), DiagConstants::MaxUriLength);
        }

        char16 buf[DiagConstants::MaxUriLength];
        GetRemoteFunctionBody()->GetUri(buf, _countof(buf));
        return WcsDup(buf, DiagConstants::MaxUriLength);
    }

    // Returned memory must be freed by the caller using delete[].
    char16* RemoteStackFrame::GetFunctionName() const
    {
        if (IsAmd64FakeEHFrame())
        {
            return WcsDup(_u("Amd64EHFrame"), DiagConstants::MaxFunctionNameLength);
        }

        char16 buf[DiagConstants::MaxFunctionNameLength];
        GetRemoteFunctionBody()->GetFunctionName(buf, _countof(buf));

        if (m_loopNumber != LoopHeader::NoLoop)
        {
            static const char16 loop[] = _u("Loop");
            size_t len = wcslen(buf);
            if (len + /*length of largest int32*/ 10 + _countof(loop) + /*nullptr*/ 1 <= _countof(buf))
            {
                swprintf_s(buf + len, _countof(buf) - len, _u("Loop%d"), m_loopNumber + 1);
            }
        }

        return WcsDup(buf, DiagConstants::MaxFunctionNameLength);
    }

    bool RemoteStackFrame::IsInlineFrame() const { return m_isInlineFrame; }

    void* RemoteStackFrame::GetEffectiveFrameBase() const { return m_effectiveFrameBase; }
    void* RemoteStackFrame::GetReturnAddress() const { return m_returnAddress; }
    void* RemoteStackFrame::GetInstructionPointer() const { return m_instructionPointer; }
    void* RemoteStackFrame::GetFrameBase() const { return m_frameBase; }
    void* RemoteStackFrame::GetStackPointer() const { return m_stackPointer; }

    void RemoteStackFrame::SetEnd(void* frameEnd) { m_frameEnd = frameEnd; }
    void RemoteStackFrame::SetStart(void* frameStart) { m_frameStart = frameStart; }
    void RemoteStackFrame::SetReturnAddress(void* returnAddress) { m_returnAddress = returnAddress; }

    void RemoteStackFrame::SetByteCodeOffset(int byteCodeOffset) { m_byteCodeOffset = byteCodeOffset; }
    void RemoteStackFrame::SetIsInlineFrame(bool isInlineFrame) { m_isInlineFrame = isInlineFrame; }
    void RemoteStackFrame::SetLoopBodyNumber(uint loopNumber)
    {
        if (!m_interpreterFrame) // This is the JIT loop body frame itself
        {
            m_loopNumber = loopNumber;
        }
    }

    // WARNING: This attaches to and owns given interpreterFrame.
    void RemoteStackFrame::SetInterpreterFrame(RemoteInterpreterStackFrame* frame) { m_interpreterFrame = frame; }

    // Similar to wcsdup but with limit on char count in destination.
    // Allocates memory using operator new, which the caller must free.
    // - src: the sring to copy
    // - maxDstCharCount: buffer size of dst (which is to be created) in chars.
    // static
    char16* RemoteStackFrame::WcsDup(_In_z_ const char16* src, size_t maxDstCharCount)
    {
        Assert(src);
        Assert(maxDstCharCount > 0);

        size_t dstCharCount = wcsnlen_s(src, maxDstCharCount - 1) + 1;
        char16* dst = new(oomthrow) char16[dstCharCount];

        // Copy src including NULL-terminator with truncate if needed, note: NULL-terminator is always appended.
        wcsncpy_s(dst, dstCharCount, src, _TRUNCATE);

        return dst;
    }

    

    void RemoteStackFrame::Init(const RefCounted<RemoteFunctionBody>& functionBody, const RefCounted<RemoteScriptContext>& scriptContext, 
        JavascriptFunction* function, void** argvAddr, Js::CallInfo callInfo, const InternalStackFrame* frame)
    {
        m_functionBody = functionBody;
        m_scriptContext = scriptContext;
        m_functionAddr = function;
        m_effectiveFrameBase = frame->EffectiveFrameBase;
        m_returnAddress = frame->ReturnAddress;
        m_instructionPointer = frame->InstructionPointer;
        m_frameBase = frame->FrameBase;
        m_stackPointer = frame->StackPointer;
        m_argvAddr = argvAddr;
        m_callInfo = callInfo;
    }

    

    void RemoteStackFrame::GetRowAndColumn(ULONG* pRow, ULONG* pColumn)
    {
        ULONG rowOffset = 0; // aka row/line but starts from 0.
        ULONG colOffset = 0;  // aka column but starts from 0.

        if (!IsAmd64FakeEHFrame())
        {
            GetRemoteFunctionBody()->GetRowColumn(m_byteCodeOffset, &rowOffset, &colOffset);
        }

        *pRow = rowOffset + 1;
        *pColumn = colOffset + 1;
    }

    void RemoteStackFrame::GetStatementStartAndEndOffset(ULONG* startOffset, ULONG* endOffset)
    {
        RemoteFunctionBody* functionBody = GetRemoteFunctionBody();
        RemoteFunctionBody_SourceInfo sourceInfo(functionBody->GetReader(), functionBody);
        sourceInfo.GetStatementOffsets(m_byteCodeOffset, startOffset, endOffset);
        Assert(*endOffset >= *startOffset);
    }

    RemoteStackFrame::RowColumnInfo::RowColumnInfo() : m_isInitialized(false), m_row(0), m_column(0)
    {
    }

    ULONG RemoteStackFrame::RowColumnInfo::GetRow(RemoteStackFrame* frame)
    {
        Assert(frame);
        if (!m_isInitialized)
        {
            frame->GetRowAndColumn(&m_row, &m_column);
            m_isInitialized = true;
        }
        return m_row;
    }

    ULONG RemoteStackFrame::RowColumnInfo::GetColumn(RemoteStackFrame* frame)
    {
        Assert(frame);
        if (!m_isInitialized)
        {
            frame->GetRowAndColumn(&m_row, &m_column);
            m_isInitialized = true;
        }
        return m_column;
    }

    RemoteStackFrame::StatementStartEndOffsetInfo::StatementStartEndOffsetInfo() : m_isInitialized(false), m_startOffset(0)
    {
    }

    ULONG RemoteStackFrame::StatementStartEndOffsetInfo::GetStartOffset(RemoteStackFrame* frame)
    {
        Assert(frame);
        if (!m_isInitialized)
        {
            frame->GetStatementStartAndEndOffset(&m_startOffset, &m_endOffset);
            m_isInitialized = true;
        }
        return m_startOffset;
    }

    ULONG RemoteStackFrame::StatementStartEndOffsetInfo::GetEndOffset(RemoteStackFrame* frame)
    {
        Assert(frame);
        if (!m_isInitialized)
        {
            frame->GetStatementStartAndEndOffset(&m_startOffset, &m_endOffset);
            m_isInitialized = true;
        }
        return m_endOffset;
    }

    void** RemoteStackFrame::GetArgvAddr()
    {
        return m_argvAddr;
    }

    Js::CallInfo RemoteStackFrame::GetCallInfo()
    {
        return m_callInfo;
    }
} // namespace JsDiag.
