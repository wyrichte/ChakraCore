#include "stdafx.h"
#include "ContractVersioning.h"
#include <stdio.h>

IFACEMETHODIMP
DevTests::ContractVersioned::XyzServer::Method1(int param1)
{
    printf("Param: %d\n", param1);
    return S_OK;
}

IFACEMETHODIMP
DevTests::ContractVersioned::XyzPlatformVersionedServer::Method1(int param1)
{
    printf("[Platform Versioned] Param: %d\n", param1);
    return S_OK;
}
