#include "stdafx.h"
#include "RecyclerObjectGraph.h"
#include "RecyclerRoots.h"

#ifdef JD_PRIVATE

RecyclerObjectGraph::RecyclerObjectGraph(EXT_CLASS_BASE* extension, JDRemoteTyped recycler, bool verbose) :
    _ext(extension),
    _alignmentUtility(recycler),
    _verbose(verbose),
    m_hbm(recycler.Field("heapBlockMap")),
    m_hasTypeName(false),
    m_hasTypeNameAndFields(false),
    m_trident(false),
    m_interior(recycler.HasField("enableScanInteriorPointers") && recycler.Field("enableScanInteriorPointers").GetStdBool())
{
}

RecyclerObjectGraph::~RecyclerObjectGraph()
{
}

// Dumps the object graph for manipulation in python
void RecyclerObjectGraph::DumpForPython(const char* outfile)
{
    _objectGraph.ExportToPython(outfile);
}

// Dumps the object graph for manipulation in js
void RecyclerObjectGraph::DumpForJs(const char* outfile)
{
    _objectGraph.ExportToJs(outfile);
}

// Dumps the object graph as a CSV file
void RecyclerObjectGraph::DumpForCsv(const char* outfile)
{
    _objectGraph.ExportToCsv(outfile);
}

// Dumps the object graph as a CSV file with extra info
void RecyclerObjectGraph::DumpForCsvExtended(EXT_CLASS_BASE *ext, const char* outfile)
{
    _objectGraph.ExportToCsvExtended(ext, outfile);
}

void RecyclerObjectGraph::Construct(ExtRemoteTyped& heapBlockMap, Addresses& roots)
{
    auto start = _time64(nullptr);
    GetExtension()->recyclerCachedData.EnableCachedDebuggeeMemory();
    roots.Map([&](ULONG64 root)
    {
        MarkObject(root, 0, roots.GetRootType(root));
    });

    int iters = 0;

    while (_markStack.size() != 0)
    {
        const MarkStackEntry object = _markStack.top();
        _markStack.pop();

        ScanBytes(object.first, object.second);

        iters++;
        if (iters % 0x10000 == 0)
        {
            _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
                "\rTraversing object graph, object count - stack: %6d, visited: %d",
                _markStack.size(), _objectGraph.GetNodeCount());
        }
    }

    GetExtension()->recyclerCachedData.DisableCachedDebuggeeMemory();    

    _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
        "\rObject graph construction completed - elapsed time: %us                                                    \n",
        (ULONG)(_time64(nullptr) - start));
}

void RecyclerObjectGraph::ClearTypeInfo()
{
    this->MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        node->ClearTypeInfo();
    });
}

void RecyclerObjectGraph::EnsureTypeInfo(bool infer, bool trident, bool verbose)
{
    if (m_hasTypeName)
    {
        if (!trident && !infer)
        {
            return;
        }

        if (trident)
        {
            if (m_trident && !infer)
            {
                return;
            }
        }
        else
        {
            Assert(infer);
            if (m_hasTypeNameAndFields)
            {
                return;
            }
        }
    }

    auto start = _time64(nullptr);

    if (m_hasTypeName)
    {
        ClearTypeInfo();
    }

    ULONG moduleIndex = 0;
    ULONG64 baseAddress = 0;
    ULONG64 endAddress = 0;

    if (FAILED(g_Ext->m_Symbols3->GetModuleByModuleName(GetExtension()->GetModuleName(), 0, &moduleIndex, &baseAddress)))
    {
        g_Ext->Out("Unable to get range for module '%s'. Is Chakra loaded?\n", GetExtension()->GetModuleName());
        return;
    }

    IMAGEHLP_MODULEW64 moduleInfo;
    g_Ext->GetModuleImagehlpInfo(baseAddress, &moduleInfo);
    endAddress = baseAddress + moduleInfo.ImageSize;
    if (verbose)
    {
        g_Ext->Out("Chakra Vtables are in the range %p to %p\n", baseAddress, endAddress);
    }

    ULONG64 tridentBaseAddress = 0;
    ULONG64 tridentEndAddress = 0;
    if (trident)
    {
        if (FAILED(g_Ext->m_Symbols3->GetModuleByModuleName("mshtml", 0, &moduleIndex, &tridentBaseAddress)) &&
            FAILED(g_Ext->m_Symbols3->GetModuleByModuleName("edgehtml", 0, &moduleIndex, &tridentBaseAddress)))
        {
            g_Ext->Out("Unable to get range for module 'mshtml' or 'edgehtml. Is Trident loaded?\n");
            return;
        }
        g_Ext->GetModuleImagehlpInfo(tridentBaseAddress, &moduleInfo);
        tridentEndAddress = tridentBaseAddress + moduleInfo.ImageSize;

        if (verbose)
        {
            g_Ext->Out("Trident Vtables are in the range %p to %p\n", tridentBaseAddress, tridentEndAddress);
        }
    }
    auto mayHaveVtable = [&](ULONG64 address, ULONG64 objectSize)
    {
        ExtRemoteData data(address, g_Ext->m_PtrSize);
        data.Read();
        ULONG64 vtable = data.GetPtr();

        bool maybeVtable = vtable % 4 == 0 && vtable >= baseAddress && vtable < endAddress;

        if (!maybeVtable && trident && objectSize >= 0x10)
        {
            // REVIEW: 0xc the start object offset?
            ExtRemoteData tridentdata(address + 0xc, g_Ext->m_PtrSize);
            tridentdata.Read();
            vtable = tridentdata.GetPtr();
            maybeVtable = vtable % 4 == 0 && vtable >= tridentBaseAddress && vtable < tridentEndAddress;
        }
        return maybeVtable;
    };

    stdext::hash_set<char const *> noScanFieldVtable;

    auto setNodeData = [&](char const * typeName, char const * typeNameOrField, RecyclerObjectGraph::GraphImplNodeType * node, bool hasVtable, bool isPropagated)
    {
        node->SetTypeInfo(typeName, typeNameOrField, hasVtable, isPropagated);
        if (!infer)
        {
            return false;
        }
        auto i = noScanFieldVtable.find(typeName);
        return (i == noScanFieldVtable.end());
    };

    ULONG numNodes = 0;
    this->MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {        
        numNodes++;
        if (numNodes % 10000 == 0)
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rProcessing objects for type info - %11d/%11d", numNodes, this->GetNodeCount());
        }

        if (node->HasTypeInfo())
        {
            return;
        }

        char const * typeName = nullptr;
        ULONG64 objectAddress = node->Key();
        if (!mayHaveVtable(objectAddress, node->GetObjectSize()))
        {
            return;
        }

        JDRemoteTyped remoteTyped = GetExtension()->CastWithVtable(objectAddress, &typeName);
        if (typeName == nullptr)
        {
            return;
        }

        if (setNodeData(typeName, typeName, node, true, false))
        {
            auto addField = [&](JDRemoteTyped field, char const * name)
            {
                if (field.GetPtr() != 0)
                {
                    auto fieldNode = this->FindNode(field.GetPtr());
                    if (fieldNode && !fieldNode->HasTypeInfo())
                    {
                        setNodeData(typeName, name, fieldNode, false, false);
                        return true;
                    }
                }
                return false;
            };

            auto addDynamicTypeField = [&](JDRemoteTyped obj)
            {
                JDRemoteTyped type = obj.Field("type");
                addField(type, "Js::DynamicType");
                addField(type.Field("propertyCache"), "Js::DynamicType.propertyCache");
            };

            auto addArrayFields = [&](JDRemoteTyped arr, char const * segmentName)
            {
                addDynamicTypeField(arr);
                JDRemoteTyped segment = arr.Field("head");
                while (segment.GetPtr() != 0)
                {
                    if (!addField(segment, segmentName))
                    {
                        break;
                    }
                    segment = segment.Field("next");
                }

                addField(arr.Field("segmentUnion").Field("segmentBTreeRoot"), "Js::JavascriptArray.segmentBTreeRoot");
            };

            char const * simpleTypeName = JDUtil::StripStructClass(remoteTyped.GetTypeName());

            if (strcmp(simpleTypeName, "Js::RecyclableObject *") == 0)
            {
                addField(remoteTyped.Field("type"), "Js::StaticType");
            }
            else if (strcmp(simpleTypeName, "Js::DynamicObject *") == 0)
            {
                addDynamicTypeField(remoteTyped);
                JDRemoteTyped auxSlots = remoteTyped.Field("auxSlots");
                if (auxSlots.GetPtr() != 0)
                {
                    // Aux slots might be use as a inline slot for object with small number of properties
                    char const * auxSlotTypeName;
                    JDRemoteTyped remoteTyped = GetExtension()->CastWithVtable(objectAddress, &auxSlotTypeName);
                    if (auxSlotTypeName == nullptr)
                    {
                        addField(auxSlots, "Js::DynamicObject.auxSlots");
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::ScriptFunction *") == 0)
            {
                addDynamicTypeField(remoteTyped);
            }
            else if (strcmp(simpleTypeName, "Js::BoundFunction *") == 0)
            {
                addDynamicTypeField(remoteTyped);
                addField(remoteTyped.Field("boundArgs"), "Js:BoundFunction.boundArgs");
            }
            else if (strcmp(simpleTypeName, "Js::LiteralString *") == 0)
            {
                addDynamicTypeField(remoteTyped);
                addField(remoteTyped.Field("m_pszValue"), "Js::LiteralString.m_pszValue");
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptArray *") == 0)
            {
                addArrayFields(remoteTyped, "Js::JavascriptArray.<SparseArraySegments>");
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptNativeIntArray *") == 0)
            {
                addArrayFields(remoteTyped, "Js::JavascriptNativeIntArray.<SparseArraySegments>");
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptNativeFloatArray *") == 0)
            {
                addArrayFields(remoteTyped, "Js::JavascriptNativeFloatArray.<SparseArraySegments>");
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptCopyOnAccessNativeIntArray *") == 0)
            {
                addArrayFields(remoteTyped, "Js::JavascriptCopyOnAccessNativeIntArray.<SparseArraySegments>");
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptCopyOnAccessNativeFloatArray *") == 0)
            {
                addArrayFields(remoteTyped, "Js::JavascriptCopyOnAccessNativeFloatArray.<SparseArraySegments>");
            }
            else if (strcmp(simpleTypeName, "Js::FunctionEntryPointInfo *") == 0)
            {
                JDRemoteTyped constructorCacheList = remoteTyped.Field("constructorCaches");
                while (addField(constructorCacheList, "Js::FunctionEntryPointInfo.<constructorCache>"))
                {
                    constructorCacheList = constructorCacheList.Field("next");
                }
            }
            else if (strcmp(simpleTypeName, "Js::FunctionBody *") == 0)
            {
                RemoteFunctionBody functionBody(remoteTyped);
                JDRemoteTyped byteCodeBlock = functionBody.GetByteCodeBlock();
                if (byteCodeBlock.GetPtr() != 0)
                {
                    addField(byteCodeBlock, "Js::FunctionBody.byteCodeBlock");
                    addField(byteCodeBlock.Field("m_content"), "Js::FunctionBody.byteCodeBlock.m_content");
                }

                JDRemoteTyped auxBlock = functionBody.GetAuxBlock();
                if (auxBlock.GetPtr() != 0)
                {
                    addField(auxBlock, "Js::FunctionBody.auxBlock");
                    addField(auxBlock.Field("m_content"), "Js::FunctionBody.auxBlock.m_content");
                }

                JDRemoteTyped auxContextBlock = functionBody.GetAuxContextBlock();
                if (auxContextBlock.GetPtr() != 0)
                {
                    addField(auxContextBlock, "Js::FunctionBody.auxContextBlock");
                    addField(auxContextBlock.Field("m_content"), "Js::FunctionBody.auxContextBlock.m_content");
                }

                JDRemoteTyped probeBackingStore = functionBody.GetProbeBackingStore();
                if (probeBackingStore.GetPtr() != 0)
                {
                    addField(probeBackingStore, "Js::FunctionBody.m_sourceInfo.probeBackingBlock");
                    addField(probeBackingStore.Field("m_content"), "Js::FunctionBody.m_sourceInfo.probeBackingBlock.m_content");
                }

                JDRemoteTyped statementMaps = functionBody.GetStatementMaps();
                if (statementMaps.GetPtr() != 0)
                {
                    addField(statementMaps, "Js::FunctionBody.statementMaps");
                    addField(statementMaps.Field("buffer"), "Js::FunctionBody.statementMaps.buffer");
                }

                JDRemoteTyped entryPoints = functionBody.GetEntryPoints();
                if (entryPoints.GetPtr() != 0)
                {
                    // Don't need to add the entryPoints list itself, as that has a vtable
                    int count = entryPoints.Field("count").GetLong();
                    JDRemoteTyped buffer = entryPoints.Field("buffer");
                    for (int i = 0; i < count; i++)
                    {
                        addField(buffer.ArrayElement(i), "Js::FunctionBody.entryPoints.<functionEntryPointInfoWeakRef>");
                    }
                }

                addField(functionBody.GetWrappedField("loopHeaderArray"), "Js::FunctionBody.loopHeaderArray");
                addField(functionBody.GetWrappedField("dynamicProfileInfo"), "Js::FunctionBody.dynamicProfileInfo");

                std::list<JDRemoteTyped> functionCodeGenRuntimeDataArrayStack;
                auto addFunctionCodeGenRuntimeDataArray = [&](JDRemoteTyped arr, uint count)
                {
                    if (arr.GetPtr() != 0)
                    {
                        addField(arr, "Js::FunctionBody.<FunctionCodeGenRuntimeData[]>");
                        for (uint i = 0; i < count; i++)
                        {
                            JDRemoteTyped functionCodeGenRuntimeData = arr.ArrayElement(i);
                            if (functionCodeGenRuntimeData.GetPtr() != 0)
                            {
                                functionCodeGenRuntimeDataArrayStack.push_back(functionCodeGenRuntimeData);
                            }
                        }
                    }
                };

                addFunctionCodeGenRuntimeDataArray(functionBody.GetWrappedField("m_codeGenRuntimeData"), functionBody.GetProfiledCallSiteCount());
                addFunctionCodeGenRuntimeDataArray(functionBody.GetWrappedField("m_codeGenGetSetRuntimeData"), functionBody.GetInlineCacheCount());

                while (!functionCodeGenRuntimeDataArrayStack.empty())
                {
                    JDRemoteTyped curr = functionCodeGenRuntimeDataArrayStack.back();
                    functionCodeGenRuntimeDataArrayStack.pop_back();
                    while (true)
                    {
                        if (!addField(curr, "Js::FunctionBody.<FunctionCodeGenRuntimeData>"))
                        {
                            break;
                        }

                        addField(JDUtil::GetWrappedField(curr.Field("clonedInlineCaches"), "inlineCaches"), "Js::FunctionBody.<FunctionCodeGenRuntimeData.ClonedInlineCaches[]>");
                        RemoteFunctionBody currBody(curr.Field("functionBody"));
                        addFunctionCodeGenRuntimeDataArray(curr.Field("inlinees"), currBody.GetProfiledCallSiteCount());
                        addFunctionCodeGenRuntimeDataArray(curr.Field("ldFldInlinees"), currBody.GetInlineCacheCount());
                        curr = curr.Field("next");
                    }
                }

                addField(functionBody.GetWrappedField("inlineCaches"), "Js::FunctionBody.<inlineCaches[]>");
                addField(JDUtil::GetWrappedField(functionBody.Field("polymorphicInlineCaches"), "inlineCaches"), "Js::FunctionBody.<polymorphicInlineCaches[]>");

                addField(functionBody.GetSourceInfo().Field("pSpanSequence"), "Js::FunctionBody.sourceInfo.pSpanSequence");
                addField(functionBody.GetConstTable(), "Js::FunctionBody.m_constTable");
                addField(functionBody.GetWrappedField("cacheIdToPropertyIdMap"), "Js::FunctionBody.cacheIdToPropertyIdMap");
                addField(functionBody.GetWrappedField("referencedPropertyIdMap"), "Js::FunctionBody.referencedPropertyIdMap");
                addField(functionBody.GetWrappedField("literalRegexes"), "Js::FunctionBody.literalRegexes");

                addField(functionBody.GetWrappedField("m_boundPropertyRecords"), "Js::FunctionBody.m_boundPropertyRecords");
                addField(functionBody.GetWrappedField("m_displayName"), "Js::FunctionBody.m_displayName");
                addField(functionBody.GetWrappedField("m_scopeInfo"), "Js::FunctionBody.m_scopeInfo");

            }
            else if (strcmp(simpleTypeName, "Js::ParseableFunctionInfo *") == 0)
            {
                addField(JDUtil::GetWrappedField(remoteTyped, "m_boundPropertyRecords"), "Js::ParseableFunctionInfo.m_boundPropertyRecords");
                addField(JDUtil::GetWrappedField(remoteTyped, "m_displayName"), "Js::ParseableFunctionInfo.m_displayName");
                addField(RemoteParseableFunctionInfo(remoteTyped).GetScopeInfo(), "Js::ParseableFunctionInfo.m_scopeInfo");                
            }
            else if (strcmp(simpleTypeName, "Js::SimpleSourceHolder *") == 0)
            {
                addField(remoteTyped.Field("source"), "Js::SimpleSourceHolder.source");
            }
            else if (strcmp(simpleTypeName, "Js::SimplePathTypeHandler *") == 0)
            {
                addField(remoteTyped.Field("typePath"), "Js::SimplePathTypeHandler.typePath");
                addField(remoteTyped.Field("predecessorType"), "Js::DynamicType");
                JDRemoteTyped successorTypeWeakRef = remoteTyped.Field("successorTypeWeakRef");
                if (successorTypeWeakRef.GetPtr() != 0)
                {
                    addField(successorTypeWeakRef, "Js::SimplePathTypeHandler.successorTypeWeakRef");
                    addField(successorTypeWeakRef.Field("strongRef"), "Js::DynamicType");
                }
            }
            else if (strcmp(simpleTypeName, "Js::PathTypeHandler *") == 0)
            {
                addField(remoteTyped.Field("typePath"), "Js::PathTypeHandler.typePath");
                addField(remoteTyped.Field("predecessorType"), "Js::DynamicType");
                JDRemoteTyped propertySuccessors = remoteTyped.Field("propertySuccessors");
                if (propertySuccessors.GetPtr() != 0)
                {
                    addField(propertySuccessors, "Js::PathTypeHandler.propertySuccessors");
                    addField(propertySuccessors.Field("buckets"), "Js::PathTypeHandler.propertySuccessors.buckets");
                    JDRemoteTyped propertySucessorsEntries = propertySuccessors.Field("entries");
                    if (propertySucessorsEntries.GetPtr() != 0)
                    {
                        addField(propertySucessorsEntries, "Js::PathTypeHandler.propertySuccessors.entries");
                        int count = propertySuccessors.Field("count").GetLong();
                        for (int i = 0; i < count; i++)
                        {
                            JDRemoteTyped successorTypeWeakRef = propertySucessorsEntries.ArrayElement(i).Field("value");
                            if (addField(successorTypeWeakRef, "Js::PathTypeHandler.successorTypeWeakRef") != 0)
                            {
                                addField(successorTypeWeakRef.Field("strongRef"), "Js::DynamicType");
                            }
                        }
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::Utf8SourceInfo *") == 0)
            {
                JDRemoteTyped lineOffsetCache = remoteTyped.Field("m_lineOffsetCache");
                if (lineOffsetCache.GetPtr() != 0)
                {
                    addField(lineOffsetCache, "Js::Utf8SourceInfo.m_lineOffsetCache");
                    JDRemoteTyped lineOffsetCacheList = lineOffsetCache.Field("lineOffsetCacheList");
                    if (lineOffsetCacheList.GetPtr() != 0)
                    {
                        addField(lineOffsetCacheList, "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList");
                        addField(lineOffsetCacheList.Field("buffer"), "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList.buffer");
                    }
                }

                addField(remoteTyped.Field("functionBodyDictionary"), "Js::Utf8SourceInfo.functionBodyDictionary");
                addField(remoteTyped.Field("m_deferredFunctionsDictionary"), "Js::Utf8SourceInfo.m_deferredFunctionsDictionary");
            }
            else
            {
                noScanFieldVtable.insert(typeName);
            }
        }
    });

    bool missedRef = true;
    while (missedRef)
    {
        missedRef = false;
        this->MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
        {
            if (node->HasTypeInfo())
            {
                return;
            }

            if (node->IsRoot())
            {
                setNodeData("<Root>", "<Root>", node, false, true);
            }

            if (!node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType* pred)
            {
                if (!pred->HasTypeInfo())
                {
                    return false;
                }

                setNodeData(pred->GetTypeName(), pred->GetTypeNameOrField(), node, pred->HasVtable(), true);
                return true;
            }))
            {
                missedRef = true;
            }
        });
    }

    m_hasTypeName = true;
    m_trident = trident;
    m_hasTypeNameAndFields = infer;
    _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
        "\rObject graph type info complete - elapsed time: %us                                                    \n",
        (ULONG)(_time64(nullptr) - start));
}

#if ENABLE_MARK_OBJ
void RecyclerObjectGraph::FindPathTo(RootPointers& roots, ULONG64 address, ULONG64 root)
{
    if (_objectGraph.GetNode(address) != nullptr)
    {
        std::vector<ULONG64> shortestPath;
        int pathsFound = 0;

        if (root)
        {
            shortestPath = _objectGraph.FindPath(root, address);
        }
        else
        {
            roots.Map([&](ULONG64 rootAddress)
            {
                _ext->Out("Finding path from 0x%P\n", rootAddress);
                std::vector<ULONG64> path = _objectGraph.FindPath(rootAddress, address);

                if (shortestPath.size() == 0 || shortestPath.size() > path.size())
                {
                    shortestPath = path;
                    if (shortestPath.size() != 0) {
                        _ext->Out("Shortest path so far is from 0x%P\n", rootAddress);
                        pathsFound++;
                    }
                }
            });
        }

        _ext->Out("Shortest path has %d nodes\n", shortestPath.size());
        for (auto it = shortestPath.begin(); it != shortestPath.end(); it++)
        {
            _ext->Out("0x%P\n", (*it));
        }
    }
    else
    {
        _ext->Out("Object not found\n");
    }
}
#endif

void RecyclerObjectGraph::MarkObject(ULONG64 address, Set<GraphImplNodeType *> * successors, RootType rootType)
{
    if (address == NULL ||
        !this->_alignmentUtility.IsAlignedAddress(address))
    {
        return;
    }

    RemoteHeapBlock *remoteHeapBlock = m_hbm.FindHeapBlock(address);
    if (remoteHeapBlock == nullptr)
    {
        return;
    }

    HeapObjectInfo info;
    if (!remoteHeapBlock->GetRecyclerHeapObjectInfo(address, info, this->m_interior))
    {
        return;
    }

    GraphImplNodeType *node = _objectGraph.FindNode(info.objectAddress);
    bool found = node != nullptr;
    if (!found)
    {
        node = _objectGraph.AddNode(info.objectAddress);
        node->SetObjectSize(info.objectSize);
    }
    Assert(node->objectSize == info.objectSize);
    if (successors)
    {
        successors->Add(node);
        Assert(!RootTypeUtils::IsAnyRootType(rootType));
    }
    else
    {
        Assert(RootTypeUtils::IsAnyRootType(rootType));
        node->AddRootType(rootType); // propagate RootType info to ObjectGraph
    }
    
    if (!found && !info.IsLeaf())
    {
        MarkStackEntry entry;
        entry.first = remoteHeapBlock;
        entry.second = node;
        _markStack.push(entry);
    }
}

void RecyclerObjectGraph::ScanBytes(RemoteHeapBlock * remoteHeapBlock, GraphImplNodeType * node)
{
    ULONG64 objectAddress = node->Key();
    Assert(objectAddress != 0);

    uint objectSize = node->GetObjectSize();
    RemoteHeapBlock::AutoDebuggeeMemory object(remoteHeapBlock, objectAddress, objectSize);   
    char* current = (char *)object;
    char* end = current + objectSize;
    ulong ptrSize = this->_ext->m_PtrSize;
    Set<GraphImplNodeType *> successors;
    while (current < end)
    {
        ULONG64 value = (ptrSize == 8) ? *((ULONG64*)current) : *((ULONG32*)current);
        if (value != objectAddress)
        {
            MarkObject(value, &successors, RootType::RootTypeNone);
        }
        current += ptrSize;
    }

    this->_objectGraph.AddEdges(node, successors);
}

#endif
