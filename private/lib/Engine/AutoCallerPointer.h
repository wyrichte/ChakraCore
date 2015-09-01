//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once


class AutoCallerPointer
{
public:
    AutoCallerPointer(ScriptSite* scriptSite, IUnknown* newCaller) :
      scriptSite(scriptSite),
      library(NULL),
      previousCaller(NULL)
    {
        if (!scriptSite->IsClosed())
        {
            library = scriptSite->GetScriptSiteContext()->GetLibrary();
        }
        scriptSite->SetCaller(newCaller, &originalCaller);
    }
    ~AutoCallerPointer()
    {
        scriptSite->SetCaller(originalCaller, &previousCaller);
        RELEASEPTR(previousCaller);
        RELEASEPTR(originalCaller);
    }
private:
    Js::JavascriptLibrary* library;
    IUnknown* originalCaller;
    IUnknown* previousCaller;
    ScriptSite* scriptSite;
};
