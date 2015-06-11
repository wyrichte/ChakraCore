/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#ifndef DBGFMT_H
#define DBGFMT_H


/***************************************************************************
	Formatter
***************************************************************************/
class ComDebugFormatter sealed : public IDebugFormatter
	{
	DECLARE_IUNKNOWN()

protected:
	long m_cref;
	
protected:	
	ComDebugFormatter(void);
	~ComDebugFormatter(void);
	
public:
    // === ComDebugFormatter ===
	static HRESULT Create(ComDebugFormatter **ppdf);
	
    // === IDebugFormatter ===
	STDMETHOD(GetStringForVariant)(VARIANT *pvarIn, ULONG ulRadix, BSTR *pbstr);
	STDMETHOD(GetVariantForString)(LPCOLESTR bstrValue, VARIANT *pvar);
	STDMETHOD(GetStringForVarType)(VARTYPE vt, TYPEDESC *ptd, BSTR *pbstr);
	};


#endif // DBGFMT_H
