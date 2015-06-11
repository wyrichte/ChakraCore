#include "stdafx.h"
#include <stdio.h>
#include <windowsstringp.h>
#include "DodoBirdServer.h"

using namespace Microsoft::WRL;

Animals::DodoBirdServer::DodoBirdServer()
{}

IFACEMETHODIMP
Animals::DodoBirdServer::IsExtinct(__out boolean* res) {
    *res = true;
    return S_OK;
}

