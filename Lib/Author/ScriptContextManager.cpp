//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{

    ScriptContextManager::ScriptContextManager(AuthoringFactory *factory) 
        : m_factory(factory), m_alloc(m_factory->GetAuthoringScriptAllocator()), m_activePhaseReporter(nullptr)
    {
    }

    ScriptContextManager::~ScriptContextManager()
    {        
        m_browserRootInstance11.Release();     
        m_applicationRootInstance11.Release();
    }

    AuthoringFileHandle* ScriptContextManager::FromResource(UINT resourceId, AuthoringFileHandleKind handleKind
#if DEBUG
        , LPCWSTR resourceName
#endif
        )
    {
        LPCUTF8 text = nullptr;
        DWORD textLen = 0;
        if (!AuthoringFactory::GetResourceText(resourceId, &text, &textLen))
            return nullptr;

        TextBuffer js(m_alloc);
        js.AddUtf8(text, textLen);
        LPCWSTR jsText = js.Sz(true);
        auto file = new MemoryAuthoringFile(jsText, wcslen(jsText)
#if DEBUG
            , String::Copy(m_alloc, resourceName), resourceName ? wcslen(resourceName) : 0
#endif
            );
        if (!file) return nullptr;
        auto fileHandle =  new AuthoringFileHandle(m_factory, file, handleKind);
        // Mark the file as internal to avoid it being exposed to the host
        return fileHandle;
    }

    ScriptContextPath* ScriptContextManager::GetRootInstance(Js::HostType hostType)
    {
        Assert(hostType == Js::HostType::HostTypeBrowser || hostType == Js::HostType::HostTypeApplication);        

        ScriptContextRoot* rootInstance;
        if (hostType == Js::HostType::HostTypeApplication)
        {
            rootInstance = &m_applicationRootInstance11;            
        }     
        else
        {
            rootInstance = &m_browserRootInstance11;
        }

        if (!rootInstance->root)
        {
            Assert(!rootInstance->start);

            // Create a new root instance for the host type
            rootInstance->root.Assign(ScriptContextPath::CreateRoot(m_alloc, hostType, this));

            // Inject in to the beginning of the path the helper functions.
            ScriptContextPath * start = rootInstance->root;

            RefCountedPtr<AuthoringFileHandle> helpers;
            helpers.TakeOwnership(FromResource(AuthoringFactory::Resources::HelpersJs(), AuthoringFileHandleKind_Helper
#if DEBUG
                , L"helpers.js"
#endif
                ));

            if (helpers)
                start = start->Next(helpers);

            
            RefCountedPtr<AuthoringFileHandle> intlHelpers;
            intlHelpers.TakeOwnership(FromResource(AuthoringFactory::Resources::IntlHelpersJs(), AuthoringFileHandleKind_Helper
#if DEBUG
                , L"intlHelpers.js"
#endif
                ));

            RefCountedPtr<AuthoringFileHandle> intl;
            intl.TakeOwnership(FromResource(AuthoringFactory::Resources::IntlJs(), AuthoringFileHandleKind_Helper
#if DEBUG
                , L"intl.js"
#endif
                ));

            if (intlHelpers && intl)
            {
                start = start->Next(intlHelpers);
                start = start->Next(intl);
            }

            rootInstance->start.Assign(start);
        }

        Assert(rootInstance->start);

        return rootInstance->start;
    }

    ScriptContextPath* ScriptContextManager::CreatePathFor(__in_ecount(count) AuthoringFileHandle **handles, int count, Js::HostType hostType)
    {
        ScriptContextPath *current = GetRootInstance(hostType);

        // Find the script context instance that corresponds to the given context, creating a new path if necessary.
        for (int i = 0; i < count; i++)
        {
            current = current->Next(handles[i]);
        }
        return current;
    }

    PhaseReporter* ScriptContextManager::GetActivePhaseReporter()
    {
        return m_activePhaseReporter;
    }

    void ScriptContextManager::SetActivePhaseReporter(PhaseReporter* activePhaseReporter)
    {
        m_activePhaseReporter = activePhaseReporter;
    }

    void ScriptContextManager::CleanupScriptPropertyCaches()
    {        
        if (m_browserRootInstance11.root)
            m_browserRootInstance11.root->CleanupScriptPropertyCaches();     
        if (m_applicationRootInstance11.root)
            m_applicationRootInstance11.root->CleanupScriptPropertyCaches();
    }
}