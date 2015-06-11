//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class ScriptContextManager
    {
    private:

        struct ScriptContextRoot
        {
            RefCountedPtr<ScriptContextPath> root;
            RefCountedPtr<ScriptContextPath> start;

             void Release()
             {
                root.ReleaseAndNull();
                start.ReleaseAndNull(); 
             }
        };

        AuthoringFactory *m_factory;
        ArenaAllocator *m_alloc;        
        ScriptContextRoot m_browserRootInstance11;
        ScriptContextRoot m_applicationRootInstance11;
        PhaseReporter * m_activePhaseReporter;

    public:
        ScriptContextManager(AuthoringFactory *factory);
        ~ScriptContextManager();
        ScriptContextPath* CreatePathFor(__in_ecount(count) AuthoringFileHandle **handles, int count, Js::HostType hostType);
        PhaseReporter* GetActivePhaseReporter();
        void SetActivePhaseReporter(PhaseReporter* activePhaseReporter);
        void CleanupScriptPropertyCaches();

    private:
        AuthoringFileHandle* FromResource(UINT resourceId, AuthoringFileHandleKind handleKind
#if DEBUG
            , LPCWSTR resourceName = nullptr
#endif
            );
        ScriptContextPath* GetRootInstance(Js::HostType hostType);
    };
}