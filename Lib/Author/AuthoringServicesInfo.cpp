//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(AuthoringServicesInfo, L"AuthoringServicesInfo")

    AuthoringServicesInfo::AuthoringServicesInfo(ArenaAllocator *alloc, ScriptContextManager *scriptContextManager) 
        : m_alloc(alloc), m_scriptContextManager(scriptContextManager), m_fileAuthorings(alloc)
    { 
    }

    AuthoringServicesInfo* AuthoringServicesInfo::New(ArenaAllocator *alloc, ScriptContextManager *scriptContextManager)
    {
        return new AuthoringServicesInfo(alloc, scriptContextManager);
    }

    AuthoringServicesInfo::~AuthoringServicesInfo()
    {
        DeletePtr(m_scriptContextManager);
    }

    void AuthoringServicesInfo::RegisterFileAuthoring(FileAuthoring *fileAuthoring)
    {
        m_fileAuthorings.Add(fileAuthoring);
    }

    void AuthoringServicesInfo::UnregisterFileAuthoring(FileAuthoring *fileAuthoring)
    {
        m_fileAuthorings.Remove(fileAuthoring);
    }

    void AuthoringServicesInfo::NotifyEngineClosing()
    {
        // Assuming FileAuthoring calls UnregisterFileAuthoring on disposal
        while(m_fileAuthorings.Count())
            m_fileAuthorings.Item(m_fileAuthorings.Count() - 1)->EngineClosing();
    }

}