#include "stdafx.h"
#include <stdio.h>
#include "DinoServer.h"

using namespace Microsoft::WRL;
#define IfFailedReturn(expr) if(FAILED(expr)) { return expr; }

Animals::DinoServer::DinoServer() : m_canRoar(false)
{}

IFACEMETHODIMP
Animals::DinoServer::CanRoar(__out boolean* result)
{
    *result = m_canRoar;
    return S_OK;
}

IFACEMETHODIMP
Animals::DinoServer::Roar(int numtimes) {
    for(auto i = 0; i < numtimes; i++) {
        wprintf(L"Roar!\n");
    }
    return S_OK;
}

IFACEMETHODIMP
Animals::DinoServer::IsExtinct(__out boolean* res) {
    *res = true;
    return S_OK;
}

IFACEMETHODIMP
Animals::DinoFactory::InspectDino(__in IDino* specimen, __out HSTRING* results)
{
    HRESULT hr = S_OK;
    boolean hasTeeth;
    int height;
    hr = specimen->get_Height(&height);
    IfFailedReturn(hr);
    hr = specimen->hasTeeth(&hasTeeth);
    IfFailedReturn(hr);

    wchar_t resultStr[40];
    wchar_t hasTeethStr[6];
    int numChar;
    if (hasTeeth)
    {
        swprintf_s(hasTeethStr, 6, L"true");
    }
    else
    {
        swprintf_s(hasTeethStr, 6, L"false");
    }
    numChar = swprintf_s(resultStr, 40, L"Height: %d\nHasTeeth: %s", height, hasTeethStr);
    if (numChar <= 0)
    {
        return E_POINTER;
    }
    WindowsCreateString(resultStr, (UINT32)wcslen(resultStr), results);
    return S_OK;
}

IFACEMETHODIMP
Animals::DinoFactory::add_FossilsFoundEvent(
    __in Animals::IFossilsFoundHandler *clickHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtFossilsFound.Add(clickHandler, pCookie);
}
                
IFACEMETHODIMP
Animals::DinoFactory::remove_FossilsFoundEvent(
    __in EventRegistrationToken iCookie)
{
    return _evtFossilsFound.Remove(iCookie);
}

