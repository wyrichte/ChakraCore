/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

static const LPCWSTR NAME_WIDTH = _u("width");
static const LPCWSTR NAME_HEIGHT = _u("height");
static const LPCWSTR NAME_DATA = _u("data");
static const LPCWSTR NAME_TYPE = _u("type");
static const LPCWSTR NAME_SIZE = _u("size");
static const LPCWSTR NAME_COMPRESSION = _u("compression");
static const LPCWSTR NAME_LASTMODIFIEDDATE = _u("lastModifiedDate");

STDMETHODIMP MockImageData::QueryInterface(REFIID riid, void** ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);

    QI_IMPL_INTERFACE(IUnknown);
    QI_IMPL_INTERFACE(ISCASerializable);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP MockImageData::GetObjectData(ISCAContext* context, SCATypeId* typeId, ISCAPropBag* propBag)
{
    IfNullReturnError(context, E_INVALIDARG);

    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(m_instance));

    *typeId = SCA_ImageDataObject;

    Var width, height;
    IfFailGo(pScriptDirect->IntToVar(m_width, &width));
    IfFailGo(pScriptDirect->IntToVar(m_height, &height));

    IfFailGo(propBag->Add(NAME_WIDTH, width));
    IfFailGo(propBag->Add(NAME_HEIGHT, height));
    IfFailGo(propBag->Add(NAME_DATA, m_pixelArray));

    // Optional properties (still save)
    Var var;
    IfFailGo(pScriptDirect->StringToVar(m_type, m_type.Length(), &var));
    IfFailGo(propBag->Add(NAME_TYPE, var));

    IfFailGo(pScriptDirect->UInt64ToVar(m_size, &var));
    IfFailGo(propBag->Add(NAME_SIZE, var));

    IfFailGo(pScriptDirect->DoubleToVar(m_compression, &var));
    IfFailGo(propBag->Add(NAME_COMPRESSION, var));

    IfFailGo(pScriptDirect->DateToVar(m_lastModifiedDate, &var));
    IfFailGo(propBag->Add(NAME_LASTMODIFIEDDATE, var));

Error:
    return pScriptDirect.CheckRecordedException(hr);
}

STDMETHODIMP MockImageData::InitializeObject(ISCAContext* context, ISCAPropBag* propBag)
{
    IfNullReturnError(context, E_INVALIDARG);

    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(m_instance));

    Var widthVar, heightVar;
    IfFailGo(propBag->Get(NAME_WIDTH, &widthVar));
    IfFailGo(propBag->Get(NAME_HEIGHT, &heightVar));

    int width, height;
    IfFailGo(pScriptDirect->VarToInt(widthVar, &width));
    IfFailGo(pScriptDirect->VarToInt(heightVar, &height));

    Var data;
    IfFailGo(propBag->Get(NAME_DATA, &data));

    IfFailGo(InitInternal(width, height, data));

    // Optional properties (still has value)
    Var var;
    IfFailGo(propBag->Get(NAME_TYPE, &var));
    IfFailGo(pScriptDirect->VarToString(var, &m_type));

    IfFailGo(propBag->Get(NAME_SIZE, &var));
    IfFailGo(pScriptDirect->VarToUInt64(var, &m_size));

    IfFailGo(propBag->Get(NAME_COMPRESSION, &var));
    IfFailGo(pScriptDirect->VarToDouble(var, &m_compression));

    IfFailGo(propBag->Get(NAME_LASTMODIFIEDDATE, &var));
    IfFailGo(pScriptDirect->VarToDate(var, &m_lastModifiedDate));

Error:
    return pScriptDirect.CheckRecordedException(hr);
}

template <class T>
HRESULT MockImageData::InitOptionalProperty(ScriptDirect& pScriptDirect, Var options, LPCWSTR name, T* value)
{
    HRESULT hr = S_OK;

    BOOL hasProperty;
    IfFailGo(pScriptDirect.HasProperty(options, name, &hasProperty));

    if (hasProperty)
    {
        IfFailGo(pScriptDirect.GetOwnProperty(options, name, value));
    }
    else
    {
        *value = T();
    }

Error:
    return hr;
}

HRESULT MockImageData::InitFrom(Var options)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(m_instance));

    int width, height;
    IfFailGo(pScriptDirect.GetOwnProperty(options, NAME_WIDTH, &width));
    IfFailGo(pScriptDirect.GetOwnProperty(options, NAME_HEIGHT, &height));

    // Read data into pixelArray
    Var pixelArray;
    {
        Var data;
        int length;
        IfFailGo(pScriptDirect.GetOwnProperty(options, NAME_DATA, &data));
        IfFailGo(pScriptDirect.GetOwnProperty(data, _u("length"), &length));

        IfFailGo(pScriptDirect->CreatePixelArray(static_cast<UINT>(length), &pixelArray));

        BYTE* pBuffer;
        UINT bufferLen;
        IfFailGo(pScriptDirect->GetPixelArrayBuffer(pixelArray, &pBuffer, &bufferLen));

        // Read data into pixel array buffer
        {
            ByteBufferContainer byteBuffer(pBuffer, bufferLen);
            IfFailGo(pScriptDirect.ReadArray(data, &byteBuffer));
        }
    }

    IfFailGo(InitInternal(width, height, pixelArray));

    // Optional properties
    IfFailGo(InitOptionalProperty(pScriptDirect, options, NAME_TYPE, &m_type));
    IfFailGo(InitOptionalProperty(pScriptDirect, options, NAME_SIZE, &m_size));
    IfFailGo(InitOptionalProperty(pScriptDirect, options, NAME_COMPRESSION, &m_compression));

    Var dateVar = NULL;
    IfFailGo(InitOptionalProperty(pScriptDirect, options, NAME_LASTMODIFIEDDATE, &dateVar));
    if (dateVar)
    {
        IfFailGo(pScriptDirect->VarToDate(dateVar, &m_lastModifiedDate));
    }
    else
    {
        m_lastModifiedDate = 0;
    }

Error:
    return hr;
}

//
// Init internal width, height, and PixelArray data.
//
HRESULT MockImageData::InitInternal(int width, int height, Var pixelArray)
{
    ScriptDirect pScriptDirect;
    PropertyId propertyId;
    HRESULT hr = S_OK;

    m_width = width;
    m_height = height;
    m_pixelArray = pixelArray;

    IfFailGo(ScriptDirect::JsVarAddRef(pixelArray)); //Pin pixel array

    // Hacky way to expose the pixelArray
    IfFailGo(pScriptDirect.From(m_instance));
    IfFailGo(pScriptDirect->GetOrAddPropertyId(_u("data"), &propertyId));
    IfFailGo(pScriptDirect.SetProperty(m_instance, propertyId, pixelArray));

Error:
    return hr;
}

MockImageData::MockImageData()
    : m_instance(NULL), m_pixelArray(NULL)
{
}

MockImageData::~MockImageData()
{
    if (m_pixelArray)
    {
        ScriptDirect::JsVarRelease(m_pixelArray); //UnPin pixel array
    }
}
