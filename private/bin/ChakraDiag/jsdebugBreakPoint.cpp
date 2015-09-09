//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace JsDiag
{
    void JsDebugBreakPoint::Init(JsDebugProcess* process, UINT64 documentId, DWORD characterOffset, DWORD characterCount, bool isEnabled)
    {
        this->m_process = process;

        ThreadContext* threadContext = process->GetDebugClient()->GetGlobalValue<ThreadContext*>(Globals_ThreadContextList);
        auto reader = process->GetReader();
        while(threadContext)
        {
            RemoteThreadContext remoteThreadContext(reader, threadContext);
            ScriptContext* scriptContext = remoteThreadContext->scriptContextList;
            while(scriptContext != nullptr)
            {
                RemoteScriptContext remoteScriptContext(reader, scriptContext);
                if(remoteScriptContext->isClosed)
                {
                    scriptContext = remoteScriptContext->next;
                    continue;
                }
                ScriptContext::SourceList * sourceList = remoteScriptContext->sourceList;
                Assert(sourceList);
                RemoteSourceList remoteSourceList(reader, sourceList);
                bool breakPointBound = remoteSourceList.MapUntil( [=] ( uint index, RecyclerWeakReference<Utf8SourceInfo>* sourceInfoWeakRef) 
                {
                    RemoteRecyclerWeakReference<Utf8SourceInfo> remoteSourceInfoWeakRef(reader, sourceInfoWeakRef);
                    Utf8SourceInfo* utf8SourceInfo = remoteSourceInfoWeakRef.Get();
                    if(utf8SourceInfo)
                    {
                        RemoteUtf8SourceInfo remoteUtf8SourceInfo(reader, utf8SourceInfo);
                        if((UINT64)remoteUtf8SourceInfo.GetDocumentId() == documentId &&
                            remoteUtf8SourceInfo.Contains(characterOffset))
                        {
                            BindBreakPoint(remoteUtf8SourceInfo, characterOffset, characterCount);
                            if(isEnabled)
                            {
                                SetByteCodeBreakPoint(m_byteCodeOffset);
                            }
                            this->m_isEnabled = isEnabled;
                            this->m_documentId = documentId;
                            return true;
                        }
                    }
                    return false;
                });

                if(breakPointBound)
                {
                    Assert(this->m_documentId != NULL);
                    return;
                }
                
                scriptContext = remoteScriptContext->next;
            }
            threadContext = (ThreadContext*)remoteThreadContext->next;
        }
        // No breakpoint was bound
        DiagException::Throw(E_JsDEBUG_SOURCE_LOCATION_NOT_FOUND);
    }

    void JsDebugBreakPoint::BindBreakPoint(const RemoteUtf8SourceInfo& sourceInfo, DWORD characterOffset, DWORD characterCount)
    {
        StatementLocation breakPointLocation = {};
        StatementLocation candidateMatch1 = {};
        StatementLocation candidateMatch2 = {};

        RemoteSRCINFO srcInfo(sourceInfo.GetReader(), sourceInfo->m_srcInfo);
        Assert(characterOffset >= srcInfo->ulCharOffset);
        characterOffset = characterOffset - srcInfo->ulCharOffset;

        sourceInfo.MapFunctionUntil( [&] (RemoteFunctionBody& remoteFunctionBody) -> bool
        {
            RemoteFunctionBody_SourceInfo remoteSourceInfo = RemoteFunctionBody_SourceInfo(this->m_process->GetReader(), &remoteFunctionBody);
            charcount_t startInDocument = remoteFunctionBody->m_cchStartOffset;
            charcount_t functionEnd = startInDocument + remoteFunctionBody->m_cchLength;
            bool funcContains = startInDocument <= characterOffset && characterOffset < functionEnd;
        
            if (candidateMatch1.function == NULL
                || ((candidateMatch1.statement.begin <= static_cast<int>(startInDocument) || candidateMatch1.statement.end <= static_cast<int>(functionEnd))
                    && characterOffset > startInDocument
                    )
                || candidateMatch2.function == NULL
                || (candidateMatch2.statement.begin > static_cast<int>(startInDocument)
                    && characterOffset <= startInDocument
                    )
                || funcContains
                )
            {
                remoteFunctionBody.FindClosestStatements(characterOffset, &candidateMatch1, &candidateMatch2);
            }
            return false;
        });

        if (candidateMatch1.function == NULL && candidateMatch2.function == NULL)
        {
            DiagException::Throw(E_JsDEBUG_SOURCE_LOCATION_NOT_FOUND);
        }

        if (candidateMatch1.function == NULL || candidateMatch2.function == NULL)
        {
            breakPointLocation = (candidateMatch1.function == NULL) ? candidateMatch2 : candidateMatch1;
        }
        else
        {
            RemoteFunctionBody functionBody1(m_process->GetReader(), candidateMatch1.function);

            // If one of the func is inner to another one, and ibos is in the inner one, disregard the outer one/let the inner one win. 
            // See WinBlue 575634. Scenario is like this: var foo = function () {this;} -- and BP is set to 'this;' 'function'.
            if (candidateMatch1.function != candidateMatch2.function)
            {
                Assert(candidateMatch1.function && candidateMatch2.function);
                RemoteFunctionBody functionBody2(m_process->GetReader(), candidateMatch2.function);

                regex::Interval func1Range(functionBody1->m_cchStartOffset);
                func1Range.End(func1Range.Begin() + functionBody1->m_cchLength);
                regex::Interval func2Range(functionBody2->m_cchStartOffset);
                func2Range.End(func2Range.Begin() + functionBody2->m_cchLength);

                if (func1Range.Includes(func2Range) && func2Range.Includes(characterOffset))
                {
                    breakPointLocation = candidateMatch2;
                    Assert(breakPointLocation.function);
                }
                else if (func2Range.Includes(func1Range) && func1Range.Includes(characterOffset))
                {
                    breakPointLocation = candidateMatch1;
                    Assert(breakPointLocation.function);
                }
            }

            if (breakPointLocation.function == nullptr)
            {
                // At this point we have both candidate to consider.
                Assert(candidateMatch1.statement.begin < candidateMatch2.statement.begin);
                Assert(candidateMatch1.statement.begin >= 0);
                Assert(static_cast<uint>(candidateMatch1.statement.begin) < characterOffset);
                Assert(static_cast<uint>(candidateMatch2.statement.begin) >= characterOffset);

                // Default selection
                breakPointLocation = candidateMatch1;

                RemoteFunctionBody_SourceInfo remoteSourceInfo = RemoteFunctionBody_SourceInfo(m_process->GetReader(), &functionBody1);
                // if the second candidate start at ibos or
                // If the first candidate has line break between ibos, and the second candidate is on the same line as characterOffset.
                // consider the second one.
                BOOL fNextHasLineBreak = remoteSourceInfo.HasLineBreak(characterOffset, candidateMatch2.statement.begin);

                if ((static_cast<uint>(candidateMatch2.statement.begin) == characterOffset)
                    || (remoteSourceInfo.HasLineBreak(candidateMatch1.statement.begin, characterOffset) && !fNextHasLineBreak))
                {
                    breakPointLocation = candidateMatch2;
                }
                // If characterOffset is out of the range of first candidate, choose second candidate if  ibos is on the same line as second candidate 
                // or characterOffset is not on the same line of the end of the first candidate.
                else if (candidateMatch1.statement.end < (int)characterOffset && (!fNextHasLineBreak || remoteSourceInfo.HasLineBreak(candidateMatch1.statement.end, characterOffset)))
                {
                    breakPointLocation = candidateMatch2;
                }
            }
        }
        Assert(breakPointLocation.function != NULL);
        this->m_functionBody = breakPointLocation.function;
        this->m_characterOffset = srcInfo.ConvertInternalOffsetToHost(breakPointLocation.statement.begin) + srcInfo->ulCharOffset;
        this->m_characterCount = breakPointLocation.statement.end - breakPointLocation.statement.begin;
        this->m_byteCodeOffset = breakPointLocation.bytecodeSpan.begin;
    }

    void JsDebugBreakPoint::SetByteCodeBreakPoint(DWORD bytecodeOffset)
    {
        RemoteFunctionBody functionBody(m_process->GetReader(), this->m_functionBody);
        //TODO: Do breakpoints need to ref counted?
        functionBody.InstallProbe(bytecodeOffset, m_process->GetRemoteAllocator());
    }

    STDMETHODIMP JsDebugBreakPoint::Delete()
    {
        if(!m_functionBody)
        {
            return S_FALSE;
        }
        return JsDebugApiWrapper([=] 
        { 
            if(m_isEnabled)
            {
                this->Disable();
            }
            // indicates deletion
            m_functionBody = NULL;
            return S_OK;
        });
    }

    STDMETHODIMP JsDebugBreakPoint::Enable()
    {
        if(!m_functionBody)
        {
            return E_UNEXPECTED;
        }
        if(m_isEnabled)
        {
            return S_FALSE;
        }
        return JsDebugApiWrapper([=] 
        { 
            SetByteCodeBreakPoint(m_byteCodeOffset);
            m_isEnabled = true;
            return S_OK;
        });
    }

    
    STDMETHODIMP JsDebugBreakPoint::IsEnabled( 
        /* [out] */ BOOL *pIsEnabled)
    {
        if(!m_functionBody)
        {
            return E_UNEXPECTED;
        }
        if(!pIsEnabled)
        {
            return E_POINTER;
        }
        *pIsEnabled = m_isEnabled;
        return S_OK;
    }

    
    STDMETHODIMP JsDebugBreakPoint::Disable()
    {
        if(!m_functionBody)
        {
            return E_UNEXPECTED;
        }
        if(!m_isEnabled)
        {
            return S_FALSE;
        }
        return JsDebugApiWrapper([=] 
        { 
            RemoteFunctionBody functionBody(m_process->GetReader(), this->m_functionBody);
            functionBody.UninstallProbe(m_byteCodeOffset);
            m_isEnabled = false;
            return S_OK;
        });
    }


        
    STDMETHODIMP JsDebugBreakPoint::GetDocumentPosition( 
        /* [out] */ UINT64 *pDocumentId,
        /* [out] */ DWORD *pCharacterOffset,
        /* [out] */ DWORD *pStatementCharCount)
    {
        if(pDocumentId)
        {
            *pDocumentId = m_documentId;    
        }

        if(pCharacterOffset)
        {
            *pCharacterOffset = m_characterOffset;
        }
        
        if(pStatementCharCount)
        {
            *pStatementCharCount = m_characterCount;
        }
        return S_OK;
    }
}