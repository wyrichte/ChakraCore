//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

// Maximum 'weight' of a scriptContext object that can be taken over by a child ScriptContextPath.
// If the ScriptContext is bigger than that, it has to be copied.
#define SCRIPTCONTEXT_TAKEOVER_LIMIT 1048576    // 1024 * 1024

namespace Authoring
{
    // A IActiveScriptAuthoring (an authoring) is created by passing an IAuthorFileContext to the authoring
    // services. The IAuthorFileContext is the hosts way of communicating a list of context files (a file context) 
    // and a primary file. The host can create multiple authorings each with their own file context. A script 
    // context path is a way of sharing the state created by executing the context files in between authorings that 
    // that have files in common in their file contexts.
    //
    // The script context paths form a path of execution from the first file in the the file context to the last file 
    // in the context file. At each point in the path is represented as a script context that contains the global 
    // object value at the point after the associated file is executed. Each script context is a copy-on-write copy 
    // of the previous script context meaning that any object that is modified by the associated script will be first 
    // copied then the modification will be applied (using the ScriptContext::CopyOnWriteCopy()).
    //
    // All the paths form a tree of contexts. For example, consider four files A, B, C & D. Given the following 
    // contexts [A], [A, B], [A, B, C], [A, B, D] (where the bracketed list represents a file context). This will form 
    // the following tree of script contexts:
    //
    //    A -> B -> C
    //         \ -> D
    //
    // where each node in the tree is the end of a path containing the parent nodes. Note A and B are shared by all 
    // the paths. If we add the context [A, E] then the tree becomes:
    //
    //    A -> B -> C
    //    |    \ -> D
    //    \ -> E
    //
    // which shares A but not B. Adding the context [A, B, C, D, F] the tree becomes:
    //
    //    A -> B -> C -> D -> F
    //    |    \ -> D
    //    \ -> E
    //
    // Note that D is included twice because, in one state only A and B were executed prior to D, and in the other A, 
    // B, and C were executed prior to D. Note also that this implies that memory usage will be better if the path
    // branches are few, which can be ensured by the host if the context files are generally in the same order for all
    // authoring contexts. Also note that if the context [A, B, D] where changed to [A, B, C, D] above, the duplicate
    // D would have been avoided.
    // 
    // The nodes are reference counted. Releasing one of the nodes might cause the parent nodes to also be released
    // if they are no longer referenced. For example, releasing E above will dispose of E but not A. Releasing F
    // will release F and the second copy of D and maybe C if C is no longer being used.
    //
    // When the content of a file is modified for which there is a script context node, the node is invalidated
    // as well as all script contexts after it in any paths that contain it. For example, if A is modified then all
    // script contexts are invalidated (and discarded, the node is retained but the script context it represents
    // is discarded). Requesting the script context for F, after this occurs, for example, will cause A, B, C & D's 
    // contexts to be created and their scripts applied to their contexts. If C is modified, on the other hand, only 
    // C, the duplicated D and F are discarded. A, B, the original D, and E are all retained. If D is modified, both 
    // copies of D are discarded as well as F, everything else is retained.
    //
    // This implies that large and/or infrequently modified files should always occur first in the path to avoid 
    // them being invalidated indirectly.

    class ScriptContextPath;

    // These are used to tag ScriptContextPath nodes with what the node is created for.
    // We use tagging so that we can invalidate script contexts based used only for GetStructure in the end of GetStructure.
    enum ScriptContextUsageFlags
    {
        None = 0,
        UsedForStructure,
        UsedForCompletions,
    };

    // A leaf script context is a script context that is based on a path but will not be
    // the bases of another path. The primary file's script context is a good example of this.
    // This class encapsulates the code to manage the lifetime of the script context as 
    // well as ensuring the path it is based on is kept alive and appropriately released.
    class LeafScriptContext 
    {
        friend ScriptContextPath;

#if TRACK_TYPE_STATS
        static TypeStats _typeStats;
#endif

    private:
        ScriptContextPath *m_path;
        ArenaAllocator *m_alloc;
        ScriptContextAutoPtr m_scriptContext;
        LeafScriptContext *m_nextLeaf;
        ParseNodeTree *m_dependentTree;
        FileAuthoring *m_fileAuthoring;

        // This list will be used for pinning utf8sourceinfo till the current LeafScriptContext is alive.
        JsUtil::List<Js::Utf8SourceInfo*> *m_sourceInfoList;

        LeafScriptContext(ArenaAllocator *alloc, ScriptContextPath *path, ParseNodeTree *dependentTree);

        void RemoveUtf8SourceInfo();
        void PopulateUtf8SourceInfo();

    public:
        ~LeafScriptContext();

        bool IsValid() { return m_scriptContext != null; }
        HRESULT GetScriptContext(FileAuthoring *fileAuthoring, Js::ScriptContext **result);
        void Invalidate();

        LeafScriptContext *CreateSnapshot(ArenaAllocator *alloc, FileAuthoring *fileAuthoring);

        // Asserts the script context is valid. Written as a function that always returns true
        // so it can be added at the end of a Boolean expression (e.g. ... && ValidateScriptContext(scriptContext)).
        bool ValidateScriptContext(Js::ScriptContext *scriptContext)
        {
            Assert(scriptContext && m_scriptContext == scriptContext);
            return true;
        }
#if DEBUG
        void DumpCopyOnWriteTableSizes();
#endif
    };

    typedef JsUtil::BaseDictionary<AuthoringFileHandle*, ScriptContextPath *, ArenaAllocator, PrimeSizePolicy> ScriptContextPaths;

    class AuthoringFileHandle;
    class ScriptContextManager;

    class ScriptContextPath : public RefCounted<DeletePolicy::ArenaDelete>
    {
    public:
        friend AuthoringFileHandle;
        friend LeafScriptContext;

    private:
        ScriptContextPath *m_invalidationLink;
        LeafScriptContext *m_activeLeaves;
        ArenaAllocator *m_alloc;
        int m_version;
        RefCountedPtr<ScriptContextPath>   m_parent;
        RefCountedPtr<AuthoringFileHandle> m_handle;
        ScriptContextAutoPtr m_scriptContext;
        ScriptContextPaths m_nexts;
        Js::HostType m_hostType;        
        ScriptContextManager* m_scriptContextManager;
        RefCountedPtr<AsyncRequestList> m_dependentAsyncRequests;
        size_t m_scriptContextSize;
        bool m_scriptContextCopiedFromParent;
        bool m_contextFileHalted;
        ScriptContextUsageFlags m_usageFlags;   // Whether this node is used for structure/completions/both, etc.

        // Construct the path as the next link from the parent path.
        ScriptContextPath(ArenaAllocator *alloc, ScriptContextPath *parent, AuthoringFileHandle *handle, ScriptContextManager *scriptContextManager): m_alloc(alloc),
            m_parent(parent), m_handle(handle), m_scriptContext(nullptr), m_invalidationLink(nullptr),
            m_nexts(alloc), m_activeLeaves(nullptr), m_version(0), m_scriptContextManager(scriptContextManager),
            m_dependentAsyncRequests(nullptr), m_scriptContextSize(0), m_scriptContextCopiedFromParent(false), m_contextFileHalted(false),
            m_usageFlags(ScriptContextUsageFlags::None)
        {
            Assert(parent && handle);
            Assert(alloc);
        }

        // Construct the root path. 
        ScriptContextPath(ArenaAllocator *alloc, Js::HostType hostType, ScriptContextManager *scriptContextManager): m_alloc(alloc),
            m_parent(nullptr), m_handle(nullptr), m_scriptContext(nullptr), m_invalidationLink(nullptr),
            m_nexts(alloc), m_activeLeaves(nullptr), m_version(0), m_hostType(hostType), m_scriptContextManager(scriptContextManager)
        {
            Assert(alloc);
        }

        static ScriptContextPath *Create(ArenaAllocator *alloc, ScriptContextPath *parent, AuthoringFileHandle *handle, ScriptContextManager *scriptContextManager)
        {
            return Anew(alloc, ScriptContextPath, alloc, parent, handle, scriptContextManager);
        }

        void AddLeaf(LeafScriptContext *leaf);
        void RemoveLeaf(LeafScriptContext *leaf);
        static void RuntimeParsingCallback(void *context, Parser *parser, LPCUTF8 pszSrc, size_t offset, size_t length, ParseNodePtr pnodeProg);
        void ProcessRuntimeParseTree(Parser *parser, LPCUTF8 pszSrc, size_t offset, size_t length, ParseNodePtr pnodeProg, AuthoringFileHandle* primaryFile);
        AuthoringFileHandle* GetAuthoringFileByBufferLocation(LPCUTF8 pszSrc);
        bool CanTakeoverScriptContext();
        void TakeoverScriptContext(ScriptContextAutoPtr &newScriptContext);
        size_t GetScriptContextWeight();
    public:
        ~ScriptContextPath();

        ArenaAllocator* Alloc() { return m_alloc; } 

        static ScriptContextPath *CreateRoot(ArenaAllocator *alloc, Js::HostType hostType, ScriptContextManager *scriptContextManager)
        {
            return Anew(alloc, ScriptContextPath, alloc, hostType, scriptContextManager);
        }

        bool IsUpToDate() { return m_scriptContext; }
        int GetVersion() { return m_version; }
        bool WasContextFileHalted() { return m_contextFileHalted; }

        static Js::ScriptContext *CreateEmptyScriptContext(Js::HostType hostType);
        HRESULT EnsureScriptContext(FileAuthoring *fileAuthoring, Js::ScriptContext **result);
        HRESULT ScriptContextPath::EnsureCompletePath(PhaseReporter *reporter);

        void Invalidate();
        void InstallInlineBreakpointProbes(Js::HaltCallback *pProbe);
        void UninstallInlineBreakpointProbes(Js::HaltCallback *pProbe);
        ScriptContextPath* Next(AuthoringFileHandle *handle);
        AuthoringFileHandle* GetAuthoringFileById(int fileId);
        AuthoringFileHandle* GetAuthoringFileByIndex(Js::ScriptContext *scriptContext, uint sourceIndex);
        AuthoringFileHandle* GetAuthoringFileBySourceInfo(Js::Utf8SourceInfo * sourceInfo);

        LeafScriptContext *CreateLeafScriptContext(ArenaAllocator *alloc, ParseNodeTree *dependantTree);
        void InstallAuthoringCallback(Js::ScriptContext *scriptContext, PhaseReporter *reporter, FileAuthoring *fileAuthoring);
        void RefreshAuthoringCallback(Js::ScriptContext *scriptContext, FileAuthoring *fileAuthoring);

        ScriptContextPath* CreateSnapshot(ArenaAllocator *alloc);

        void RecordDependentAsyncRequest(AsyncRequest *dependentAsyncRequest);
        void ClearDependentAsyncRequest();
        void CleanupScriptPropertyCaches();

        ScriptContextUsageFlags GetUsageFlags() { return m_usageFlags; }
        void SetUsageFlags(ScriptContextUsageFlags value) { m_usageFlags = value; }
        void OrUsageFlags(ScriptContextUsageFlags value)
        {
            m_usageFlags = static_cast<ScriptContextUsageFlags>(m_usageFlags | value);
        }
        void ApplyUsageFlagsUp(ScriptContextUsageFlags flags);
        void InvalidateUpForStructure();

#if DEBUG
        void DumpCopyOnWriteTableSizes();
        LPCWSTR GetScriptContextFileName();
#endif
    };
}
