#include "StdAfx.h"

namespace Authoring
{
    bool AuthoringFactory::GetResourceText(UINT resourceId, LPCUTF8 *text, DWORD *textLen)
    {
        HMODULE currentModule = ::GetModuleHandle(L"chakrals");
        if (currentModule == nullptr)
        {
#if ENABLE_DEBUG_CONFIG_OPTIONS
            currentModule = ::GetModuleHandle(L"chakralstest");
            if (currentModule == nullptr)
#endif
            {
                currentModule = ::GetModuleHandle(L"chakra");
                if (currentModule == nullptr)
                {
                    return false;
                }
            }
        }

        auto resource = ::FindResource(currentModule, MAKEINTRESOURCE(resourceId), MAKEINTRESOURCE(RT_HTML)); 
        if(resource == nullptr)
        {
            return false;
        }

        auto loadedResource = ::LoadResource(currentModule, resource);
        if(loadedResource == nullptr)
        {
            return false;
        }

        auto js = (LPCUTF8)::LockResource(loadedResource);
        if(js == nullptr)
        {
            return false;
        }
#if DEBUG
        // Verify the file is encoded as utf8 and not utf16
        Assert(js[1] != 0);
#endif

        *text = js;
        *textLen = ::SizeofResource(currentModule, resource);

        return true;
    }

    //
    // AuthoringFactory::Resources
    //

    UINT AuthoringFactory::Resources::HelpersJs()
    {
        return IDR_HELPERS_JS;
    }

    UINT AuthoringFactory::Resources::IntlHelpersJs()
    {
        return IDR_INTL_HELPERS_JS;
    }

    UINT AuthoringFactory::Resources::IntlJs()
    {
        return IDR_INTL_JS;
    }
}
