//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

ChakraEngine::ChakraEngine(ScriptEngineBase *scriptEngine) :
    scriptEngine(scriptEngine),
    library(nullptr)
{
    // Set up recycler links between ChakraEngine and JavascriptLibrary
    library = scriptEngine->GetScriptContext()->GetLibrary();
    library->chakraEngine = this;
    // Let the scriptEngine know that it was created as part of a ChakraEngine.
    scriptEngine->SetChakraEngine(this);
}
