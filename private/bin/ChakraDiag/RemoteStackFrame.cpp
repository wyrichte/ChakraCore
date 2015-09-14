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
        m_frameId(0),
        m_diagFrame(nullptr)
    {
    }

    RemoteStackFrame::~RemoteStackFrame()
    {
    }

    ULONG RemoteStackFrame::GetRow() { return m_rowColumn.GetRow(this); }
    ULONG RemoteStackFrame::GetColumn() { return m_rowColumn.GetColumn(this); }

    // Returned memory must be freed by the caller using delete[].
    wchar_t* RemoteStackFrame::GetUri() const
    {
        if (IsAmd64FakeEHFrame())
        {
            return WcsDup(L"JS Internal", DiagConstants::MaxUriLength);
        }

        wchar_t buf[DiagConstants::MaxUriLength];
        GetRemoteFunctionBody()->GetUri(buf, _countof(buf));
        return WcsDup(buf, DiagConstants::MaxUriLength);
    }

    // Returned memory must be freed by the caller using delete[].
    wchar_t* RemoteStackFrame::GetFunctionName() const
    {
        if (IsAmd64FakeEHFrame())
        {
            return WcsDup(L"Amd64EHFrame", DiagConstants::MaxFunctionNameLength);
        }

        wchar_t buf[DiagConstants::MaxFunctionNameLength];
        GetRemoteFunctionBody()->GetFunctionName(buf, _countof(buf));

        if (m_loopNumber != LoopHeader::NoLoop)
        {
            static const wchar_t loop[] = L"Loop";
            size_t len = wcslen(buf);
            if (len + /*length of largest int32*/ 10 + _countof(loop) + /*nullptr*/ 1 <= _countof(buf))
            {
                swprintf_s(buf + len, _countof(buf) - len, L"Loop%d", m_loopNumber + 1);
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
    wchar_t* RemoteStackFrame::WcsDup(_In_z_ const wchar_t* src, size_t maxDstCharCount)
    {
        Assert(src);
        Assert(maxDstCharCount > 0);

        size_t dstCharCount = wcsnlen_s(src, maxDstCharCount - 1) + 1;
        wchar_t* dst = new(oomthrow) wchar_t[dstCharCount];

        // Copy src including NULL-terminator with truncate if needed, note: NULL-terminator is always appended.
        wcsncpy_s(dst, dstCharCount, src, _TRUNCATE);

        return dst;
    }

    //
    // Obtains start/end of the frame on the stack.
    // For x86/ARM these would be frame base addresses, for current and child frame.
    // For amd64 these would be parent callsite SP and this frame's callsite SP.
    // Parameters:
    // - pStart: frame pointer of 
    //          - the current javascript frame, or
    //          - the frame pointer of the script runtime entry frame
    // - pEnd: frame pointer of 
    //         -the previous (one above the current one) Javascript frame.
    //         - script exit record (when out of script).
    //         - the top most physical frame (when in script and being the topmost javascript frame).
    //
    STDMETHODIMP RemoteStackFrame::GetStackRange(
        /* [out] */ __RPC__out UINT64 *pStart,
        /* [out] */ __RPC__out UINT64 *pEnd)
    {
        if(pStart)
        {
            Assert(m_frameStart != nullptr);
           *pStart = (UINT64)m_frameStart;
        }
        if(pEnd)
        {
            Assert(m_frameEnd != nullptr);
            *pEnd = (UINT64)m_frameEnd;
        }
        return S_OK;
    }

    STDMETHODIMP RemoteStackFrame::GetReturnAddress(
        /* [out] */ __RPC__out UINT64 *pReturnAddress)
    {
        if(pReturnAddress)
        {
            *pReturnAddress = (UINT64)m_returnAddress;
        }
        return S_OK;
    }

    STDMETHODIMP RemoteStackFrame::GetName(
        /* [out] */ __RPC__deref_out_opt BSTR *pbstrName)
    {
        if(!pbstrName)
        {
            return E_POINTER;
        }

        return JsDebugApiWrapper([=]
        {
            wchar_t functionName[DiagConstants::MaxFunctionNameLength];
            GetRemoteFunctionBody()->GetFunctionName(functionName, _countof(functionName));

            CComBSTR name(functionName);
            *pbstrName = name.Detach();
            return S_OK;
        });
    }

    STDMETHODIMP RemoteStackFrame::GetDocumentPositionWithId(
        /* [out] */ __RPC__out UINT64 *pDocumentId,
        /* [out] */ __RPC__out DWORD *pCharacterOffset,
        /* [out] */ __RPC__out DWORD *pStatementCharCount)
    {
        return JsDebugApiWrapper([=]
        {
            RemoteFunctionBody* functionBody = GetRemoteFunctionBody();
            if(pDocumentId)
            {
                *pDocumentId = functionBody->GetDocumentId();
            }
            if(pCharacterOffset)
            {
                RemoteUtf8SourceInfo sourceInfo(functionBody->GetReader(), functionBody->ToTargetPtr()->m_utf8SourceInfo);
                ULONG charOffset = 0;
                if(sourceInfo->m_srcInfo)
                {
                    RemoteSRCINFO srcInfo(functionBody->GetReader(), sourceInfo->m_srcInfo);
                    charOffset = srcInfo->ulCharOffset;
                }
                *pCharacterOffset = m_statementStartOffset.GetStartOffset(this) + charOffset;
            }
            if(pStatementCharCount)
            {
                *pStatementCharCount = m_statementStartOffset.GetEndOffset(this) - m_statementStartOffset.GetStartOffset(this);
            }

            return S_OK;
        });
    }

    STDMETHODIMP RemoteStackFrame::GetDocumentPositionWithName(
        /* [out] */ __RPC__deref_out_opt BSTR *pDocumentName,
        /* [out] */ __RPC__out DWORD *pLine,
        /* [out] */ __RPC__out DWORD *pColumn)
    {
        if(pDocumentName)
        {
            wchar_t uri[DiagConstants::MaxUriLength];
            GetRemoteFunctionBody()->GetUri(uri, _countof(uri));

            CComBSTR documentName(uri);
            *pDocumentName = documentName.Detach();
        }
        if(pLine)
        {
            *pLine = this->GetRow();
        }
        if(pColumn)
        {
            *pColumn = this->GetColumn();
        }

        return S_OK;
    }

    STDMETHODIMP RemoteStackFrame::GetDebugProperty(
        /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty)
    {
        return JsDebugApiWrapper([=]
        {
            CreateComObject<RemoteStackFrameLocals>(this, ppDebugProperty);
            return S_OK;
        });
    }

    STDMETHODIMP RemoteStackFrame::Evaluate(
        /* [in] */ __RPC__in LPCOLESTR pExpressionText,
        /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty,
        /* [out] */ __RPC__deref_out_opt BSTR *pError)
    {
        return JsDebugApiWrapper([=]
        {
            CComPtr<IJsDebugPropertyInternal> root;
            CreateComObject<RemoteStackFrameLocals>(this, &root);

            SimpleExpressionEvaluator evaluator(GetInspectionContext(), root);
            CString error;

            if (evaluator.TryEvaluate(pExpressionText, ppDebugProperty, error))
            {
                *pError = nullptr;
                return S_OK;
            }
            else
            {
                *pError = error.AllocSysString();
                return S_FALSE;
            }
        });
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

    // Returns an instance of RemoteDiagFrame that is valid only within lifetime of this RemoteStackFrame instance,
    // that's why is has "temp" in the name.
    // The instance is owned by RemoteStackFrame, so do not call destructor outside.
    RemoteDiagFrame* RemoteStackFrame::GetTempDiagFrame()
    {
        if (m_diagFrame == nullptr)
        {
            m_diagFrame = this->CreateDiagFrame();
        }
        return m_diagFrame;
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

    RemoteDiagFrame* RemoteStackFrame::CreateDiagFrame()
    {
        if (this->IsInterpreterFrame())
        {
            return new RemoteDiagInterpreterFrame(this);
        }
        else
        {
            return new RemoteDiagNativeFrame(this);
        }
    }

} // namespace JsDiag.
