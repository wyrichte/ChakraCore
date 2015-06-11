//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(Completions, L"Completions")

    namespace Names
    {
        const wchar_t nonremovable[] = L"_$isNonRemovable";
    }

    #define ValidateState() \
    do { \
      if (!(_fileAuthoring)) { \
        hr = E_UNEXPECTED; \
        goto Error; \
      } \
    } while (0)

    static BOOL IsValidIdentifier(wchar_t const *name, charcount_t len)
    {
        if (len == 0) return FALSE;
        Assert(name[len] == NULL);     // the name has to be null terminated
        const OLECHAR* t = FileAuthoring::GetLSCharClassifier()->SkipIdentifier(name);
        return ((charcount_t)(t-name) == len);
    }

    Completions::Completions(FileAuthoring *fileAuthoring) 
        : SimpleComObjectWithAlloc<IAuthorCompletionSet>(fileAuthoring->GetScriptContext()->GetThreadContext()->GetPageAllocator(), L"ls: Completions"), 
        ids(Alloc()), 
        _fileAuthoring(fileAuthoring), 
        kind(acskMember),
        objectKind(acokNone),
        diagFlags(AuthorCompletionDiagnosticFlags(acdfNone)),
        nonRemovablePropertyId(0)
    {
        results = CompletionList::New(Alloc());
    }

    Completions* Completions::New(FileAuthoring *fileAuthoring)
    {
        Assert(fileAuthoring);
        return new Completions(fileAuthoring);
    }

    Completions::InternalCompletion* Completions::AddCompletion(
        Js::RecyclableObject* parentObject,
        Js::Var value, 
        AuthorCompletionKind kind, 
        AuthorCompletionFlags group, 
        Js::PropertyId id, 
        wchar_t const *name, 
        charcount_t len,
        LPCWSTR glyph,
        HintInfo* hintInfo)
    {
        if (name &&  (len < 2 || name[0] != '_' || name[1] != '$') && IsValidIdentifier(name, len) && !IsExtensionObjectName(name, len))
        {
            InternalCompletion *completion = Anew(Alloc(), InternalCompletion);
            completion->kind = kind;
            completion->id = id;
            completion->group = group;
            completion->name.Set(String::Copy(Alloc(), name, len), len);
            completion->displayText = completion->name;
            completion->insertionText = completion->name;
            completion->hintInfo = hintInfo;
            completion->parentObject = parentObject;
            completion->value = value;
            if(!String::IsNullOrEmpty(glyph))
                completion->glyph.Set(String::Copy(Alloc(), glyph));
            results->Add(completion);
            return completion;
        }

        return nullptr;
    }

    Completions::InternalCompletion* Completions::AddUniqueCompletion(
        Js::RecyclableObject* parentObject,
        Js::Var value, 
        AuthorCompletionKind kind, 
        AuthorCompletionFlags group, 
        Js::PropertyId id, 
        wchar_t const *name, 
        charcount_t len,
        LPCWSTR glyph,
        HintInfo* hintInfo)
    {
        if (!ids.TestAndSet(id))
            return AddCompletion(parentObject, value, kind, group, id, name, len, glyph, hintInfo);
        else 
            UpdateFlags(id, group);
        return nullptr;
    }

    void Completions::AddUnique(Js::RecyclableObject* parentObject, Js::Var value, AuthorCompletionKind kind, AuthorCompletionFlags group, Js::PropertyId id, LPCWSTR glyph, HintInfo* hintInfo)
    {
        Js::PropertyRecord const *name = _fileAuthoring->GetScriptContext()->GetPropertyName(id);
        if (name)
        {
            AddUniqueCompletion(parentObject, value, kind, group, id, name->GetBuffer(), name->GetLength(), glyph, hintInfo);
        }
    }

    void Completions::AddUnique(AuthorCompletionKind kind, AuthorCompletionFlags group, LPCWSTR name, charcount_t len, LPCWSTR glyph, HintInfo* hintInfo)
    {
        if (name)
        {
            Js::PropertyId id = _fileAuthoring->GetScriptContext()->GetOrAddPropertyIdTracked(name, len);
            AddUniqueCompletion(nullptr, nullptr, kind, group, id, name, len, glyph, hintInfo);
        }
    }

    void Completions::AddUnique(AuthorCompletionKind kind, AuthorCompletionFlags group, IdentPtr pid, LPCWSTR glyph, HintInfo* hintInfo)
    {
        if (pid)
        {
            AddUnique(kind, group, pid->Psz(), pid->Cch(), glyph, hintInfo);
        }
    }

    Completions::InternalCompletion *Completions::Find(Js::InternalString* name)
    {
        Assert(name);
        auto id = _fileAuthoring->GetScriptContext()->GetOrAddPropertyIdTracked(name->GetBuffer(), name->GetLength());
        if (ids.Test(id))
        {
            for (int i = 0; i < results->Count(); i++)
            {
                auto result = results->Item(i);
                if (result->id == id)
                    return result;
            }
        }
        return nullptr;
    }

    void Completions::UpdateFlags(Js::PropertyId id, AuthorCompletionFlags group)
    {
        int count = results->Count();
        for (int i = 0; i < count; i++)
        {
            InternalCompletion *completion = results->Item(i);
            if (completion->id == id)
            {
                completion->group = AuthorCompletionFlags(completion->group | group);
                break;
            }
        }
    }

    void Completions::SetExtent(long offset, long length)
    {
        extent.offset = offset;
        extent.length = length;
    }

    void Completions::SetKind(AuthorCompletionSetKind kind)
    {
        this->kind = kind;
    }

    void Completions::SetObjectKind(AuthorCompletionObjectKind objectKind)
    {
        this->objectKind = objectKind;
    }

    void Completions::SetDiagnosticFlags(AuthorCompletionDiagnosticFlags flags)
    {
        // Set given flags in addition to flags that are already set. 
        // Diagnostic flags are never reset on a completion result.
        this->diagFlags = AuthorCompletionDiagnosticFlags(this->diagFlags | flags);
    }


    STDMETHODIMP Completions::get_Count(int *result)
    {
        STDMETHOD_PREFIX;

        ValidatePtr(result, E_POINTER);
        ValidateState();

        *result = results->Count();

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::GetItems(int startIndex, int count, AuthorCompletion *completions)
    {
        STDMETHOD_PREFIX;

        ValidatePtr(completions, E_POINTER);
        ValidateArg(startIndex + count <= results->Count());
        ValidateState();

        for (int i = 0; i < count; i++)
        {
            int sourceIndex = startIndex + i; 
            InternalCompletion *src = results->Item(sourceIndex);
            AuthorCompletion *dest = &completions[i];
            
            dest->kind = src->kind;
            dest->group = src->group;
            dest->completionCookie = src->id;
            dest->name = src->name.ToBSTR();
            dest->insertionText = src->insertionText.ToBSTR();
            dest->displayText = src->displayText.ToBSTR();
            dest->glyph = src->glyph.ToBSTR();
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::GetExtent(AuthorFileRegion *extent)
    {
        STDMETHOD_PREFIX;
        ValidateState();
        if (extent)
            *extent = this->extent;

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::GetHintFor(int index, IAuthorSymbolHelp **result)
    {
        STDMETHOD_PREFIX;

        ValidateArg(index <= results->Count());
        ValidateArg(result);
        ValidateState();

        auto item = results->Item(index);
        Assert(item != nullptr);

        Js::ScriptContext* scriptContext = _fileAuthoring->GetScriptContext(); 
        Assert(scriptContext);
        
        // For ackProperty property items, the value is not set for performance reasons.
        // Initialize it now since it is needed to produce completion hint.
        IfFailGo(item->FetchPropertyValue(_fileAuthoring));

        IAuthorSymbolHelp* symbolHelp = nullptr;
        
        IfFailGo(_fileAuthoring->CreateSymbolHelp(
            scriptContext,
            item->parentObject,
            item->value, 
            item->name.Buffer(), 
            item->hintInfo ? item->hintInfo->scope : ascopeUnknown,
            item->hintInfo ? item->hintInfo->type : atUnknown,
            item->hintInfo ? item->hintInfo->fileId : -1, 
            item->hintInfo ? item->hintInfo->funcSourceMin : 0,
            &symbolHelp));

        auto pageAlloc = scriptContext->GetThreadContext()->GetPageAllocator();
        ArenaAllocator localArena(L"ls: Completions::GetHintFor", pageAlloc, Js::Throw::OutOfMemory);
        
        // Allow extensions to modify the result
        LangSvcExtensibility extensibility(&localArena, scriptContext, _fileAuthoring);
        extensibility.FireOnCompletionHint(pageAlloc, this, index, symbolHelp);
        
        *result = symbolHelp;

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::get_Kind(AuthorCompletionSetKind *result)
    {
        STDMETHOD_PREFIX;

        *result = this->kind;

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::get_ObjectKind(AuthorCompletionObjectKind *result)
    {
        STDMETHOD_PREFIX;

        *result = this->objectKind;

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP Completions::get_DiagnosticFlags(AuthorCompletionDiagnosticFlags *result)
    {
        STDMETHOD_PREFIX;

        *result = this->diagFlags;

        STDMETHOD_POSTFIX;
    }

    void Completions::EngineClosing()
    {
        OnDelete();
    }

    void Completions::Clear(bool preserveNonRemovable)
    {
        CompletionList* nonRemovableItems = nullptr;
        if(preserveNonRemovable)
        {
            // Create a list of non-removable items in the original set
            nonRemovableItems = CompletionList::New(Alloc());
            ListOperations::ForEach(Items(), [&](int, InternalCompletion* item)
            {
                if(IsNonRemovable(item))
                    nonRemovableItems->Add(item);
            });
        }

        Items()->Clear();
        Ids()->ClearAll();

        if(nonRemovableItems)
        {
            Items()->AddRange(*nonRemovableItems);
            ListOperations::ForEach(nonRemovableItems, [&](int, InternalCompletion* item)
            {
                Ids()->Set(item->id);
            });
        }
    }

    void Completions::PreventJsObjectsRecycling()
    {
        // Push all JS objects to a global array to prevent recycling. 
        // When Completions object is released, the ScriptContext object is likely to be released as well (we're holding a reference to it), 
        // so we never need to free the objects. 
        Js::ScriptContext* scriptContext = _fileAuthoring->GetScriptContext();
        Assert(scriptContext);
        ListOperations::ForEach(results, [&] (int, InternalCompletion* item) 
        { 
            Assert(item);
            if(item->parentObject) 
                JsHelpers::PreventRecycling(scriptContext, item->parentObject);
            if(item->value) 
                JsHelpers::PreventRecycling(scriptContext, item->value);
        });
    }

    void Completions::ExtendLifetime()
    {
        Assert(_fileAuthoring);
        _fileAuthoring.TakeOwnership(
            _fileAuthoring->CreateSnapshot());
        _fileAuthoring->AddCompletionList(this);
        PreventJsObjectsRecycling();
    }

    void Completions::OnDelete()
    {
        if(_fileAuthoring)
        {
            Clear();

            _fileAuthoring->RemoveCompletionList(this);
            _fileAuthoring.ReleaseAndNull();

            // Call the base class's OnDelete
            SimpleComObjectWithAlloc<IAuthorCompletionSet>::OnDelete();
        }
    }

    Js::PropertyId Completions::ToId(LPCWSTR name)
    {
        Assert(!String::IsNullOrEmpty(name));
        Js::ScriptContext* scriptContext = _fileAuthoring->GetScriptContext();
        Assert(scriptContext);
        return scriptContext->GetOrAddPropertyIdTracked(name, wcslen(name));
    }

    bool Completions::IsNonRemovable(InternalCompletion* completion)
    {
        Assert(completion);
        
        // Local variables and parameters are non-removable
        auto scope = completion->hintInfo ? completion->hintInfo->scope : ascopeUnknown;
        if(scope == ascopeLocal || scope == ascopeParameter)
            return true;

        auto value = completion->value;
        if(value && !Js::TaggedInt::Is(value))
        {
            if(!this->nonRemovablePropertyId) 
                this->nonRemovablePropertyId = ToId(Names::nonremovable);
            Js::RecyclableObject* obj = nullptr;
            Convert::FromVar(nullptr, value, obj);
            auto typeId = Js::JavascriptOperators::GetTypeId(obj);
            if (typeId != Js::TypeIds_Null && typeId != Js::TypeIds_Undefined)
            {
                BEGIN_ENTER_SCRIPT(this->_fileAuthoring->GetScriptContext(), false, false, false);
                {
                    if(obj->HasOwnProperty(this->nonRemovablePropertyId))
                        return true;
                }
                END_ENTER_SCRIPT;
            }
        }
        return false;
    }

    HRESULT Completions::InternalCompletion::FetchPropertyValue(FileAuthoring* fileAuthoring)
    {
        Assert(fileAuthoring);
        METHOD_PREFIX;
        if(kind == ackProperty && parentObject && !value)
        {
            // Execute the getter to obtain property value
            IfFailGo(fileAuthoring->ExecuteGetter(parentObject, name.Buffer(), &value));
            JsHelpers::PreventRecycling(fileAuthoring->GetScriptContext(), value);
        }
        METHOD_POSTFIX;
    }
}