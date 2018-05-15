//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "..\private\lib\StaticLib\base\scriptenginebase.h"
#include "..\Common\Core\FinalizableObject.h"

class ChakraEngine : public FinalizableObject
{
private:
    ScriptEngineBase *scriptEngine;
    Js::JavascriptLibraryBase *library;

public:
    ChakraEngine(ScriptEngineBase *scriptEngine);

    ~ChakraEngine()
    {
        // Nothing needs to be explicitly cleaned up in the destructor.
        // The recycler will take care of cleaning up.
    }

    IActiveScriptDirect *GetActiveScriptDirect(__out HRESULT *hr = nullptr) const
    {
        HRESULT outHR = E_FAIL; // init in failure mode to protect against paths which did not explicitly succeed

        // We know that ScriptEngineBase is IASD, so there is no need to QI.
        // However, we still need to refcount, so call AddRef.
        // NOTE: AddRef takes care of rooting the global object, if necessary.
        IActiveScriptDirect *pIASD = static_cast<IActiveScriptDirect *>(scriptEngine);
        pIASD->AddRef();
        outHR = S_OK;

        if (hr)
        {
            *hr = outHR;
        }
        return pIASD;
    }

public:
    //
    // FinalizableObject methods
    //

    void Finalize(bool isShutdown) override
    {
    }

    void Dispose(bool isShutdown) override
    {
        if (isShutdown)
        {
            return;
        }

        scriptEngine->Release();
    }

    void Mark(Recycler* recycler) override
    {
    }
};
