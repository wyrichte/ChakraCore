/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// ScpNode.h : Declaration of the CStandardScriptSourceNode
#pragma once

#ifndef _SCPNODE_H
#define _SCPNODE_H

#ifdef __cplusplus
extern "C"{
#endif


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

extern CLSID CLSID_StandardScriptSourceNode;

#ifdef __cplusplus
}
#endif

class CStandardScriptSourceNode : public IUnknown
    {
protected:
    long m_refCount;
    LPOLESTR m_pszShort;
    LPOLESTR m_pszLong;
    BSTR m_bstrUrl;
    IDebugApplicationNode *m_pdan;

public:
    CStandardScriptSourceNode(void);
    virtual ~CStandardScriptSourceNode(void);

    HRESULT Close(void);

    HRESULT SetNode(IDebugDocumentProvider *pddp, IDebugApplicationNode *pdan);
    HRESULT GetNode(IDebugApplicationNode **ppdan);

    ulong InternalAddRef(void)
        { return InterlockedIncrement(&m_refCount); }

    ulong InternalRelease(void)
        { return InterlockedDecrement(&m_refCount); }

    // IUnknown:
    STDMETHOD_(ULONG, AddRef)(void)
        { return InternalAddRef(); }

    STDMETHOD_(ULONG, Release)(void)
        {
        ULONG lu = InternalRelease();
        if (lu == 0)
            delete this;
        return lu;
        }

    STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);

    virtual STDMETHODIMP GetName(DOCUMENTNAMETYPE dnt, BSTR *pbstr);
    virtual STDMETHODIMP SetLongName(LPCOLESTR psz);
    virtual STDMETHODIMP SetShortName(LPCOLESTR psz);   
    };

#endif // _SCPNODE_H
