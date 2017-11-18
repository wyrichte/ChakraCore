//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnCPch.h"

#ifdef EDIT_AND_CONTINUE

HRESULT EditAndContinue::InitializeScriptEdit(ScriptEngine * scriptEngine, IActiveScriptEdit ** scriptEdit)
{
    HRESULT hr = S_OK;
    CComPtr<Js::ScriptEdit> spScriptEdit;
    IFFAILRET(Js::ScriptEdit::CreateInstance(&spScriptEdit));
    IFFAILRET(spScriptEdit->Init(scriptEngine));

    *scriptEdit = spScriptEdit.Detach();

    return hr;
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

void EditAndContinue::InitializeEditTest(Js::ScriptContext * scriptContext)
{
    if (!CONFIG_FLAG(EditTest))
    {
        return;
    }
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    Js::DynamicObject* editTest = library->CreateObject();
    {
        const Js::PropertyRecord* propertyRecord;
        scriptContext->GetOrAddPropertyRecord(_u("EditTest"), &propertyRecord);
        library->AddMember(library->GetGlobalObject(), propertyRecord->GetPropertyId(), editTest);
    }

    library->AddFunctionToLibraryObjectWithPropertyName(editTest, _u("LoadTextFile"), &Js::EditTest::EntryInfo::LoadTextFile, 1);
    library->AddFunctionToLibraryObjectWithPropertyName(editTest, _u("LCS"), &Js::EditTest::EntryInfo::LCS, 2);
    library->AddFunctionToLibraryObjectWithPropertyName(editTest, _u("Ast"), &Js::EditTest::EntryInfo::Ast, 1);
    library->AddFunctionToLibraryObjectWithPropertyName(editTest, _u("AstDiff"), &Js::EditTest::EntryInfo::AstDiff, 2);
}
#endif
#endif