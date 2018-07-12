//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "RecyclerLibraryGraph.h"
#include "RecyclerObjectGraph.h"
#include "ProgressTracker.h"
#include "RemoteJavascriptLibrary.h"

struct LibrarySummary
{
    LibrarySummary(ULONG64 library)
        : library(library), objectCount(0), objectSize(0), rootCount(0), pinnedRootCount(0), isPrimary(false),
        arenaRootCount(0), crossReferencedObjectCount(0), isClosed(false), reachableFromNonClosed(false), groupId(0), subGroupId(0)
    {};

    ULONG64 library;
    ULONG64 objectCount;
    ULONG64 objectSize;
    ULONG64 rootCount;
    ULONG64 pinnedRootCount;
    ULONG64 arenaRootCount;
    ULONG64 crossReferencedObjectCount;
    bool isPrimary;
    bool isClosed;
    bool reachableFromNonClosed;
    uint groupId;
    uint subGroupId;

    std::set<ULONG64> successors;
    std::set<ULONG64> predecessors;
};


int __cdecl LibrarySummaryComparer(const void * a, const void * b)
{
    auto ptrA = *(LibrarySummary **)a;
    auto ptrB = *(LibrarySummary **)b;
    if (ptrA->library == 0) { return -1; }
    if (ptrB->library == 0) { return 1; }
    if (ptrA->library == RemoteJavascriptLibrary::GlobalLibrary) { return -1; }
    if (ptrB->library == RemoteJavascriptLibrary::GlobalLibrary) { return 1; }
    int diff = (int)ptrB->isPrimary - (int)ptrA->isPrimary;
    if (diff != 0) { return diff; }
    diff = (int)ptrB->reachableFromNonClosed - (int)ptrA->reachableFromNonClosed;
    if (diff != 0) { return diff; }
    diff = (int)ptrA->groupId - (int)ptrB->groupId;
    if (diff != 0) { return diff; }
    diff = (int)ptrA->subGroupId - (int)ptrB->subGroupId;
    if (diff != 0) { return diff; }
    diff = (int)ptrA->isClosed - (int)ptrB->isClosed;
    if (diff != 0) { return diff; }
    diff = (int)ptrA->predecessors.size() - (int)ptrB->predecessors.size();
    if (diff != 0) { return diff; }
    diff = (int)ptrB->crossReferencedObjectCount - (int)ptrA->crossReferencedObjectCount;
    return diff;
}

RecyclerLibraryGraph::RecyclerLibraryGraph(RecyclerObjectGraph& objectGraph)
{
    ProgressTracker progress("Accumulating library stats", 0x10000, objectGraph.GetNodeCount());
    objectGraph.MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType * node)
    {
        ULONG64 library = node->GetAssociatedJavascriptLibrary();
        LibrarySummary * summary = libraryMap[library];
        if (summary == nullptr)
        {
            summary = new LibrarySummary(library);
            libraryMap[library] = summary;
        }
        summary->objectCount++;
        summary->objectSize += node->GetObjectSize();
        if (node->IsRoot())
        {
            summary->rootCount++;

            if (RootTypeUtils::IsType(node->GetRootType(), RootType::RootTypePinned))
            {
                summary->pinnedRootCount++;
            }
            if (RootTypeUtils::IsType(node->GetRootType(), RootType::RootTypeArena))
            {
                summary->arenaRootCount++;
            }
        }

        // Ignore unknown library
        if (library != 0)
        {
            bool hasCrossReference = false;
            std::stack<RecyclerObjectGraph::GraphImplNodeType *> work;
            auto processNode = [&](RecyclerObjectGraph::GraphImplNodeType * pred)
            {
                ULONG64 predLibrary = pred->GetAssociatedJavascriptLibrary();
                if (predLibrary == 0)   // Unknown library, take the predecessor
                {
                    work.push(pred);
                }
                else if (predLibrary != library)
                {
                    hasCrossReference = true;
                    summary->predecessors.insert(predLibrary);
                }
            };

            node->MapAllPredecessors(processNode);
            if (!work.empty())
            {
                std::set< RecyclerObjectGraph::GraphImplNodeType *> visited;
                visited.insert(node);
                do
                {
                    auto currNode = work.top();
                    work.pop();
                    if (visited.find(currNode) != visited.end())
                    {
                        return;
                    }
                    visited.insert(currNode);
                    currNode->MapAllPredecessors(processNode);
                } while (!work.empty());
            }
            summary->crossReferencedObjectCount += hasCrossReference;
        }
        progress.Inc();
    });

    // Populate successors and isClosed
    for (auto i : libraryMap)
    {
        if (i.first != 0 && i.first != RemoteJavascriptLibrary::GlobalLibrary)
        {
            JDRemoteTyped javascriptLibrary = JDRemoteTyped::FromPtrWithVtable(i.first);
            RemoteScriptContext scriptContext = javascriptLibrary.Field("scriptContext");
            i.second->isClosed = scriptContext.IsClosed();
            if (!i.second->isClosed)
            {
                i.second->isPrimary = scriptContext.IsPrimaryEngine();
            }
        }
        for (auto j : i.second->predecessors)
        {
            libraryMap[j]->successors.insert(i.first);
        }
    }

    sortedArray.reset(new LibrarySummary *[libraryMap.size()]);
    int c = 0;
    for (auto i : libraryMap)
    {
        sortedArray.get()[c++] = i.second;
    }
    qsort(sortedArray.get(), c, sizeof(LibrarySummary *), LibrarySummaryComparer);

    // Populate groupId
    WalkGraph(
        [](LibrarySummary& summary, uint iter)
        {
            if (summary.library == 0) { return false; }
            if (summary.groupId != 0) { return false; }
            summary.groupId = iter;
            return true;
        },
        true
        );
    qsort(sortedArray.get(), c, sizeof(LibrarySummary *), LibrarySummaryComparer);

    // Populate subGroupId
    WalkGraph(
        [](LibrarySummary& summary, uint iter)
        {
            if (summary.library == 0) { return false; }
            if (summary.subGroupId != 0) { return false; }
            summary.subGroupId = iter;
            return true;
        },
        false
        );

    // Populate reachableFromNonClosed
    WalkGraph(
        [](LibrarySummary& summary, uint iter)
        {
            if (iter == 0) { return !summary.isClosed; }
            if (summary.reachableFromNonClosed) { return false; }
            summary.reachableFromNonClosed = true;
            return true;
        },
        false
        );

    qsort(sortedArray.get(), c, sizeof(LibrarySummary *), LibrarySummaryComparer);
    progress.Done("Accumulating library stats completed");
}

RecyclerLibraryGraph::~RecyclerLibraryGraph()
{
    for (auto i : libraryMap)
    {
        delete i.second;
    }
}

static void DumpLibrarySummary(LibrarySummary const& i, bool filterClosed)
{
    RemoteJavascriptLibrary javascriptLibrary(i.library);
    if (filterClosed && !javascriptLibrary.IsClosed())
    {
        return;
    }

    if (i.reachableFromNonClosed)
    {
        g_Ext->Out("*");
    }
    else
    {
        g_Ext->Out(" ");
    }
    g_Ext->Out("%3d-%03d:", i.groupId, i.subGroupId);
    javascriptLibrary.PrintStateAndLink(true);

    g_Ext->Out(" %7I64u %11I64u | %5I64u %5I64u %5I64u | %7I64u %4I64u %4I64u", i.objectCount, i.objectSize, i.rootCount,
        i.pinnedRootCount, i.arenaRootCount,
        i.crossReferencedObjectCount, (ULONG64)i.predecessors.size(), (ULONG64)i.successors.size());

    if (GetExtension()->PreferDML())
    {
        if (!javascriptLibrary.IsInternal())
        {
            g_Ext->Dml(" <link cmd=\"!jd.traceroots -lib 0x%p\">(root)</link>", i.library);
        }
        else
        {
            g_Ext->Out("       ");
        }
        if (i.predecessors.size() != 0)
        {
            g_Ext->Dml(" <link cmd=\"!jd.jslibpreds 0x%p\">(libpreds)</link>", i.library);
        }
        else
        {
            g_Ext->Out("           ");
        }
        if (i.successors.size() != 0)
        {
            g_Ext->Dml(" <link cmd=\"!jd.jslibsuccs 0x%p\">(libsuccs)</link>", i.library);
        }
    }
    g_Ext->Out("\n");
}

JD_PRIVATE_COMMAND(jslibstats,
    "Dump a library stats",
    "{;e,o,d=0;recycler;Recycler address}"
    "{fc;b,o;FilterClosed;Filter to closed context}")
{
    const bool filterClosed = HasArg("fc");
    ULONG64 recyclerArg = GetUnnamedArgU64(0);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, GetStackTop(),
        RecyclerObjectGraph::TypeInfoFlags::Infer));


    RecyclerLibraryGraph& libraryGraph = *objectGraph.GetLibraryGraph();


    if (this->m_PtrSize == 8)
    {
        this->Out("            Library              Count        Size |  Root   Pin Arena |    XRef XPre XSuc\n");
    }
    else
    {
        this->Out("            Library      Count        Size |  Root   Pin Arena |    XRef XPre XSuc\n");
    }

    libraryGraph.ForEach([&](LibrarySummary const& i)
    {
        DumpLibrarySummary(i, filterClosed);
    });
}

JD_PRIVATE_COMMAND(jslibpreds,
    "Dump a library predecessor",
    "{;e,d=0;library;Library address}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 libraryArg = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, GetStackTop(),
        RecyclerObjectGraph::TypeInfoFlags::Infer));
    RecyclerLibraryGraph& libraryGraph = *objectGraph.GetLibraryGraph();
    LibrarySummary const * summary = libraryGraph.GetLibrarySummary(libraryArg);
    if (summary == nullptr)
    {
        g_Ext->ThrowInvalidArg("Invalid library address 0x%p", libraryArg);
    }
    for (auto i : summary->predecessors)
    {
        DumpLibrarySummary(*libraryGraph.GetLibrarySummary(i), false);
    }
}

JD_PRIVATE_COMMAND(jslibsuccs,
    "Dump a library successors",
    "{;e,d=0;library;Library address}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 libraryArg = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, GetStackTop(),
        RecyclerObjectGraph::TypeInfoFlags::Infer));
    RecyclerLibraryGraph& libraryGraph = *objectGraph.GetLibraryGraph();
    LibrarySummary const * summary = libraryGraph.GetLibrarySummary(libraryArg);
    if (summary == nullptr)
    {
        g_Ext->ThrowInvalidArg("Invalid library address 0x%p", libraryArg);
    }
    for (auto i : summary->successors)
    {
        DumpLibrarySummary(*libraryGraph.GetLibrarySummary(i), false);
    }
}