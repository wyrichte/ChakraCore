#include <windows.h>
#include "iesettings.h"
#include "IEConfig.h"

namespace SettingStore
{   
    IESSAPI GetBOOL(VALUEID<BOOL> Id, __out BOOL* pfValue)
    {
        return E_FAIL;
    }

    VALUEID<int> const IEVALUE_ExperimentalFeatures_ExperimentalJS = { 0 };
    VALUEID<int> const IEVALUE_ExperimentalFeatures_Asmjs = { 0 };
};

HRESULT IEConfiguration_SetBool(__in IEConfigurationID iecID, __in bool fValue)
{
    return E_FAIL;
}