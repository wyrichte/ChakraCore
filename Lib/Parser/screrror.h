//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

/***************************************************************************
Exception blocks
***************************************************************************/
class ErrHandler;
struct ParseNode;
class COleScript;

inline void FreeExcepInfo(EXCEPINFO *pei)
{
    if (pei->bstrSource)
        SysFreeString(pei->bstrSource);
    if (pei->bstrDescription)
        SysFreeString(pei->bstrDescription);
    if (pei->bstrHelpFile)
        SysFreeString(pei->bstrHelpFile);
    memset(pei, 0, sizeof(*pei));
}

void CopyException (EXCEPINFO *pexcepinfoDest, const EXCEPINFO *pexcepinfoSource);

BOOL FSupportsErrorInfo(IUnknown *punk, REFIID riid);
HRESULT GetErrorInfo(EXCEPINFO *pexcepinfo);

HRESULT MapHr(HRESULT hr, ErrorTypeEnum * errorTypeOut = null);

class SRCINFO;
class ActiveScriptError;

class ScriptException
{
public:
    long ichMin;
    long ichLim;
    EXCEPINFO ei;

public:
    ScriptException()
    { memset(this, 0, sizeof(*this)); }
    ~ScriptException(void);

public:
    void CopyInto(ScriptException *pse);
    void Free();
    void GetError(HRESULT *phr, EXCEPINFO *pei); // Clears error.
};

class CompileScriptException : public ScriptException
{
public:
    long line;       // line number of error (zero based)
    long ichMinLine; // starting char of the line
    bool hasLineNumberInfo;
    // TODO: if the line contains \0 character the substring following \0 will not be included:
    BSTR bstrLine;   // source line (if available)

public:
    CompileScriptException(void) : ScriptException(), line(0), ichMinLine(0), hasLineNumberInfo(false),
        bstrLine(NULL)
    { }
    ~CompileScriptException();

public:
    void Clear();
    void Free();
    void GetError(HRESULT *phr, EXCEPINFO *pei)
    {
        ScriptException::GetError(phr, pei);
        Free();
    }

    HRESULT  ProcessError(IScanner * pScan, HRESULT hr, ParseNode * pnodeBase);

    friend class ActiveScriptError;
};
