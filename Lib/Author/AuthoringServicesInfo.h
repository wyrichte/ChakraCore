//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class FileAuthoring;

    class AuthoringServicesInfo : public RefCounted<DeletePolicy::OperatorDelete>
    {
        typedef JsUtil::List<FileAuthoring *, ArenaAllocator> FileAuthorings;

    private:
        ArenaAllocator *m_alloc;
        ScriptContextManager *m_scriptContextManager;
        FileAuthorings m_fileAuthorings;
    private:
        AuthoringServicesInfo(ArenaAllocator *alloc, ScriptContextManager *scriptContextManager);
    public:
        static AuthoringServicesInfo* New(ArenaAllocator *alloc, ScriptContextManager *scriptContextManager);
        ~AuthoringServicesInfo();

        ScriptContextManager *GetScriptContextManager() { return m_scriptContextManager; }

        void RegisterFileAuthoring(FileAuthoring *fileAuthoring);
        void UnregisterFileAuthoring(FileAuthoring *fileAuthoring);
        void NotifyEngineClosing();
    };

}