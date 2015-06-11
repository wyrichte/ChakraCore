/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

// Mock CanvasImageData
class MockImageData:
    public ComObjectRoot,
    public ISCASerializable
{
private:
    Var m_instance; // Bound JS instance

    int m_width;
    int m_height;
    Var m_pixelArray; // PixelArray

    // Optional attributes
    CComBSTR m_type; // image type string
    ULONGLONG m_size; // image file size
    double m_compression; // compression ratio
    double m_lastModifiedDate;

    HRESULT InitInternal(int width, int height, Var pixelArray);

    template <class T>
    HRESULT InitOptionalProperty(ScriptDirect& pScriptDirect, Var options, LPCWSTR name, T* value);

public:
    MockImageData();
    ~MockImageData();

    // Bind this object to JS instance
    HRESULT Init(Var instance)
    {
        m_instance = instance;
        return S_OK;
    }

    HRESULT InitFrom(Var options);

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP GetObjectData(ISCAContext* context, SCATypeId* typeId, ISCAPropBag* propBag);
    STDMETHODIMP InitializeObject(ISCAContext* context, ISCAPropBag* propBag);
};

