#include "stdafx.h"
#include "RecyclerObjectGraph.h"
#include "RecyclerRoots.h"

#ifdef JD_PRIVATE

RecyclerObjectGraph::RecyclerObjectGraph(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler, bool verbose) :
    _ext(extension),
    _verbose(verbose),
    _heapBlockHelper(extension, recycler),
    _recycler(recycler),
    m_hasTypeName(false),
    m_hasTypeNameAndFields(false),
    m_trident(false)
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

void RecyclerObjectGraph::PushMark(ULONG64 object, ULONG64 prev)
{
    if (object != 0)
    {
        MarkStackEntry entry;
        entry.first = object;
        entry.second = prev;
        _markStack.push(entry);
    }
}

void RecyclerObjectGraph::Construct(Addresses& roots)
{
    auto start = _time64(nullptr);
    roots.Map([&](ULONG64 root)
    {
        PushMark(root, 0);
    });

    // Ensure the cached heap block map
    _ext->recyclerCachedData.GetHeapBlockMap(_recycler);

    int iters = 0;

    while (_markStack.size() != 0)
    {
        const MarkStackEntry& object = _markStack.top();
        _markStack.pop();

        ULONG64 objectAddress = object.first;
        ULONG64 prevAddress = object.second;

        MarkObject(objectAddress, prevAddress);

        iters++;
        if (iters % 0x10000 == 0)
        {
            iters = 0;
            _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rTraversing object graph, object count - stack: %6d, visited: %d", _markStack.size(), _objectGraph.Count());
        }
    }

    _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, 
        "\rObject graph construction complete - elapsed time: %us                                                    \n",
        (ULONG)(_time64(nullptr) - start));
}

void RecyclerObjectGraph::ClearTypeInfo()
{
    this->MapAllNodes([&](ULONG64 objectAddress, RecyclerObjectGraph::GraphImplNodeType* node)
    {
        node->aux.isScanned = true;
        node->aux.typeName = nullptr;
        node->aux.typeNameOrField = nullptr;
        node->aux.hasVtable = false;
        node->aux.isPropagated = false;
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
        node->aux.typeName = typeName;
        node->aux.typeNameOrField = typeNameOrField;
        node->aux.isScanned = false;
        node->aux.hasVtable = hasVtable;
        node->aux.isPropagated = isPropagated;
        if (!infer)
        {
            return false;
        }
        auto i = noScanFieldVtable.find(typeName);
        return (i == noScanFieldVtable.end());
    };
    
    ULONG numNodes = 0;
    this->MapAllNodes([&](ULONG64 objectAddress, RecyclerObjectGraph::GraphImplNodeType* node)
    {       
        numNodes++;
        if (numNodes % 10000 == 0)
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rProcessing objects for type info - %11d/%11d", numNodes, this->NumNodes());
        }

        if (!node->aux.isScanned)
        {
            return;
        }

        char const * typeName = nullptr;
        if (!mayHaveVtable(objectAddress, node->aux.objectSize))
        {
            return;
        }

        ExtRemoteTyped remoteTyped = GetExtension()->CastWithVtable(objectAddress, &typeName);
        if (typeName == nullptr)
        {
            return;
        }

        if (setNodeData(typeName, typeName, node, true, false))
        {
            auto addField = [&](ExtRemoteTyped field, char const * name)
            {
                if (field.GetPtr() != 0)
                {
                    auto fieldNode = this->FindNode(field.GetPtr());
                    if (fieldNode && fieldNode->aux.isScanned)
                    {
                        setNodeData(typeName, name, fieldNode, false, false);
                        return true;
                    }
                }
                return false;
            };

            auto addDynamicTypeField = [&](ExtRemoteTyped obj)
            {
                ExtRemoteTyped type = obj.Field("type");
                addField(type, "Js::DynamicType");
                addField(type.Field("propertyCache"), "Js::DynamicType.propertyCache");
            };

            auto addArrayFields = [&](ExtRemoteTyped arr, char const * segmentName)
            {
                addDynamicTypeField(arr);
                ExtRemoteTyped segment = arr.Field("head");
                while (segment.GetPtr() != 0)
                {
                    if (!addField(segment, segmentName))
                    {
                        break;
                    }
                    segment = segment.Field("next");
                }

                addField(arr.Field("segmentUnion.segmentBTreeRoot"), "Js::JavascriptArray.segmentBTreeRoot");
            };

            char const * simpleTypeName = JDUtil::StripStructClass(remoteTyped.GetTypeName());

            if (strcmp(simpleTypeName, "Js::RecyclableObject *") == 0)
            {
                addField(remoteTyped.Field("type"), "Js::StaticType");
            }
            else if (strcmp(simpleTypeName, "Js::DynamicObject *") == 0)
            {
                addDynamicTypeField(remoteTyped);
                ExtRemoteTyped auxSlots = remoteTyped.Field("auxSlots");
                if (auxSlots.GetPtr() != 0)
                {
                    // Aux slots might be use as a inline slot for object with small number of properties
                    char const * auxSlotTypeName;
                    ExtRemoteTyped remoteTyped = GetExtension()->CastWithVtable(objectAddress, &auxSlotTypeName);
                    if (auxSlotTypeName == nullptr)
                    {
                        addField(remoteTyped.Field("auxSlots"), "Js::DynamicObject.auxSlots");
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
                ExtRemoteTyped constructorCacheList = remoteTyped.Field("constructorCaches");
                while (addField(constructorCacheList, "Js::FunctionEntryPointInfo.<constructorCache>"))
                {
                    constructorCacheList = constructorCacheList.Field("next");
                }
            }
            else if (strcmp(simpleTypeName, "Js::FunctionBody *") == 0)
            {
                RemoteFunctionBody functionBody(remoteTyped);
                ExtRemoteTyped byteCodeBlock = functionBody.GetByteCodeBlock();
                if (byteCodeBlock.GetPtr() != 0)
                {
                    addField(byteCodeBlock, "Js::FunctionBody.byteCodeBlock");
                    addField(byteCodeBlock.Field("m_content"), "Js::FunctionBody.byteCodeBlock.m_content");
                }

                ExtRemoteTyped auxBlock = functionBody.GetAuxBlock();
                if (auxBlock.GetPtr() != 0)
                {
                    addField(auxBlock, "Js::FunctionBody.auxBlock");
                    addField(auxBlock.Field("m_content"), "Js::FunctionBody.auxBlock.m_content");
                }

                ExtRemoteTyped auxContextBlock = functionBody.GetAuxContextBlock();
                if (auxContextBlock.GetPtr() != 0)
                {
                    addField(auxContextBlock, "Js::FunctionBody.auxContextBlock");
                    addField(auxContextBlock.Field("m_content"), "Js::FunctionBody.auxContextBlock.m_content");
                }

                ExtRemoteTyped probeBackingStore = functionBody.GetProbeBackingStore();
                if (probeBackingStore.GetPtr() != 0)
                {
                    addField(probeBackingStore, "Js::FunctionBody.m_sourceInfo.probeBackingBlock");
                    addField(probeBackingStore.Field("m_content"), "Js::FunctionBody.m_sourceInfo.probeBackingBlock.m_content");
                }

                ExtRemoteTyped statementMaps = functionBody.GetStatementMaps();
                if (statementMaps.GetPtr() != 0)
                {
                    addField(statementMaps, "Js::FunctionBody.statementMaps");
                    addField(statementMaps.Field("buffer"), "Js::FunctionBody.statementMaps.buffer");
                }

                ExtRemoteTyped entryPoints = functionBody.GetEntryPoints();
                if (entryPoints.GetPtr() != 0)
                {
                    // Don't need to add the entryPoints list itself, as that has a vtable
                    int count = entryPoints.Field("count").GetLong();
                    ExtRemoteTyped buffer = entryPoints.Field("buffer");
                    for (int i = 0; i < count; i++)
                    {
                        addField(buffer.ArrayElement(i), "Js::FunctionBody.entryPoints.<functionEntryPointInfoWeakRef>");
                    }
                }

                addField(JDUtil::GetWrappedField(functionBody, "loopHeaderArray"), "Js::FunctionBody.loopHeaderArray");
                addField(JDUtil::GetWrappedField(functionBody, "dynamicProfileInfo"), "Js::FunctionBody.dynamicProfileInfo");

                std::list<ExtRemoteTyped> functionCodeGenRuntimeDataArrayStack;
                auto addFunctionCodeGenRuntimeDataArray = [&](ExtRemoteTyped arr, uint count)
                {
                    if (arr.GetPtr() != 0)
                    {
                        addField(arr, "Js::FunctionBody.<FunctionCodeGenRuntimeData[]>");
                        for (uint i = 0; i < count; i++)
                        {
                            ExtRemoteTyped functionCodeGenRuntimeData = arr.ArrayElement(i);
                            if (functionCodeGenRuntimeData.GetPtr() != 0)
                            {
                                functionCodeGenRuntimeDataArrayStack.push_back(functionCodeGenRuntimeData);
                            }
                        }
                    }
                };

                addFunctionCodeGenRuntimeDataArray(JDUtil::GetWrappedField(functionBody, "m_codeGenRuntimeData"), functionBody.GetProfiledCallSiteCount());
                addFunctionCodeGenRuntimeDataArray(JDUtil::GetWrappedField(functionBody, "m_codeGenGetSetRuntimeData"), functionBody.GetInlineCacheCount());

                while (!functionCodeGenRuntimeDataArrayStack.empty())
                {
                    ExtRemoteTyped curr = functionCodeGenRuntimeDataArrayStack.back();
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

                addField(JDUtil::GetWrappedField(functionBody, "inlineCaches"), "Js::FunctionBody.<inlineCaches[]>");
                addField(JDUtil::GetWrappedField(functionBody.Field("polymorphicInlineCaches"), "inlineCaches"), "Js::FunctionBody.<polymorphicInlineCaches[]>");

                addField(functionBody.GetSourceInfo().Field("pSpanSequence"), "Js::FunctionBody.sourceInfo.pSpanSequence");
                addField(functionBody.GetConstTable(), "Js::FunctionBody.m_constTable");
                addField(JDUtil::GetWrappedField(functionBody, "cacheIdToPropertyIdMap"), "Js::FunctionBody.cacheIdToPropertyIdMap");
                addField(JDUtil::GetWrappedField(functionBody, "referencedPropertyIdMap"), "Js::FunctionBody.referencedPropertyIdMap");
                addField(JDUtil::GetWrappedField(functionBody, "literalRegexes"), "Js::FunctionBody.literalRegexes");

                addField(JDUtil::GetWrappedField(functionBody, "m_boundPropertyRecords"), "Js::FunctionBody.m_boundPropertyRecords");
                addField(JDUtil::GetWrappedField(functionBody, "m_displayName"), "Js::FunctionBody.m_displayName");
                addField(JDUtil::GetWrappedField(functionBody, "m_scopeInfo"), "Js::FunctionBody.m_scopeInfo");

            }
            else if (strcmp(simpleTypeName, "Js::ParseableFunctionInfo *") == 0)
            {
                addField(JDUtil::GetWrappedField(remoteTyped, "m_boundPropertyRecords"), "Js::ParseableFunctionInfo.m_boundPropertyRecords");
                addField(JDUtil::GetWrappedField(remoteTyped, "m_displayName"), "Js::ParseableFunctionInfo.m_displayName");
                addField(JDUtil::GetWrappedField(remoteTyped, "m_scopeInfo"), "Js::ParseableFunctionInfo.m_scopeInfo");
            }
            else if (strcmp(simpleTypeName, "Js::SimpleSourceHolder *") == 0)
            {
                addField(remoteTyped.Field("source"), "Js::SimpleSourceHolder.source");
            }
            else if (strcmp(simpleTypeName, "Js::SimplePathTypeHandler *") == 0)
            {
                addField(remoteTyped.Field("typePath"), "Js::SimplePathTypeHandler.typePath");
                addField(remoteTyped.Field("predecessorType"), "Js::DynamicType");
                ExtRemoteTyped successorTypeWeakRef = remoteTyped.Field("successorTypeWeakRef");
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
                ExtRemoteTyped propertySuccessors = remoteTyped.Field("propertySuccessors");
                if (propertySuccessors.GetPtr() != 0)
                {
                    addField(propertySuccessors, "Js::PathTypeHandler.propertySuccessors");
                    addField(propertySuccessors.Field("buckets"), "Js::PathTypeHandler.propertySuccessors.buckets");
                    ExtRemoteTyped propertySucessorsEntries = propertySuccessors.Field("entries");
                    if (propertySucessorsEntries.GetPtr() != 0)
                    {
                        addField(propertySucessorsEntries, "Js::PathTypeHandler.propertySuccessors.entries");
                        int count = propertySuccessors.Field("count").GetLong();
                        for (int i = 0; i < count; i++)
                        {
                            ExtRemoteTyped successorTypeWeakRef = propertySucessorsEntries.ArrayElement(i).Field("value");
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
                ExtRemoteTyped lineOffsetCache = remoteTyped.Field("m_lineOffsetCache");
                if (lineOffsetCache.GetPtr() != 0)
                {
                    addField(lineOffsetCache, "Js::Utf8SourceInfo.m_lineOffsetCache");
                    ExtRemoteTyped lineOffsetCacheList = lineOffsetCache.Field("lineOffsetCacheList");
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
        this->MapAllNodes([&](ULONG64 objectAddress, RecyclerObjectGraph::GraphImplNodeType* node)
        {
            if (!node->aux.isScanned)
            {
                return;
            }

            if (node->aux.isRoot)
            {
                setNodeData("<Root>", "<Root>", node, false, true);
            }

            if (!node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType* pred)
            {
                if (pred->aux.isScanned)
                {
                    return false;
                }
                
                setNodeData(pred->aux.typeName, pred->aux.typeNameOrField, node, pred->aux.hasVtable, true);
                return true;
            }))
            {
                missedRef = true;
            }
        });
    }

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

void RecyclerObjectGraph::MarkObject(ULONG64 address, ULONG64 prev)
{
    if (address != 0 && _heapBlockHelper.IsAlignedAddress(address))
    {
        ULONG64 heapBlockAddress = _heapBlockHelper.FindHeapBlock(address, _recycler);
        if (heapBlockAddress != 0)
        {
            GraphNode<ULONG64, RecyclerGraphNodeAux>* node = _objectGraph.GetNode(address);
            if (prev)
            {
                _objectGraph.AddEdge(prev, node);
            }
            else
            {
                node->aux.isRoot = true;
            }

            if (!node->aux.isScanned)
            {
                ExtRemoteTyped heapBlock = _ext->recyclerCachedData.GetAsHeapBlock(heapBlockAddress);
                auto type = _heapBlockHelper.GetHeapBlockType(heapBlock);
                ULONG64 objectSize = 0;

                if (type == _ext->enum_LargeBlockType())
                {
                    ExtRemoteTyped largeBlock = _ext->recyclerCachedData.GetAsLargeHeapBlock(heapBlockAddress);
                    objectSize = GetLargeObjectSize(largeBlock, address);
                }
                else if (type != _ext->enum_SmallLeafBlockType() && type != _ext->enum_MediumLeafBlockType())
                {
                    ExtRemoteTyped smallBlock = _ext->recyclerCachedData.GetAsSmallHeapBlock(heapBlockAddress);

                    // Hack- make this a friend of SmallHeapBlock
                    if (_heapBlockHelper.GetSmallHeapBlockObjectIndex(smallBlock, address) != 0xffff)
                    {
                        objectSize = (ULONG64)smallBlock.Field("objectSize").GetUshort();
                    }
                }

                if (objectSize != 0)
                {
                    ScanBytes(address, objectSize);
                }

                if (type != _ext->enum_SmallLeafBlockType())
                {
                    node->aux.objectSize = (uint)objectSize;
                }
                else
                {
                    ExtRemoteTyped smallBlock = _ext->recyclerCachedData.GetAsSmallHeapBlock(heapBlockAddress);
                    node->aux.objectSize = (ULONG64)smallBlock.Field("objectSize").GetUshort();
                }
                node->aux.isScanned = true;
            }
        }
    }
}

ULONG64 RecyclerObjectGraph::GetLargeObjectSize(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress)
{
    ULONG64 heapBlock = heapBlockObject.GetPointerTo().GetPtr();
    ULONG64 blockAddress = heapBlockObject.Field("address").GetPtr();

    ULONG64 sizeOfHeapBlock = _ext->EvalExprU64(_ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeHeapBlock))"));
    ULONG64 sizeOfObjectHeader = _ext->EvalExprU64(_ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));

    ULONG64 headerAddress = objectAddress - sizeOfObjectHeader;

    if (headerAddress < blockAddress)
    {
        if (_verbose)
            _ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        return 0;
    }

    ExtRemoteTyped largeObjectHeader(_ext->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);

    HeapObject heapObject;
    ULONG64 objectCount = EXT_CLASS_BASE::GetSizeT(heapBlockObject.Field("objectCount"));
    heapObject.index = (ushort)largeObjectHeader.Field("objectIndex").m_Typed.Data; // Why does calling UShort not work?

    if (heapObject.index > objectCount)
    {
        return 0;
    }

    ULONG largeObjectHeaderPtrSize = _ext->m_PtrSize;
    ULONG64 headerArrayAddress = heapBlock + sizeOfHeapBlock + (heapObject.index * largeObjectHeaderPtrSize);
    ExtRemoteData  headerData(headerArrayAddress, largeObjectHeaderPtrSize);

    if (headerData.GetPtr() != headerAddress)
    {
        if (_verbose)
        {
            _ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
            _ext->Out("Header address: 0x%p, Header in index %d is 0x%p\n", headerAddress, heapObject.index, headerData.GetPtr());
        }

        return 0;
    }

    return EXT_CLASS_BASE::GetSizeT(largeObjectHeader.Field("objectSize"));
}

void RecyclerObjectGraph::ScanBytes(ULONG64 address, ULONG64 size)
{
    char* object = (char*)malloc((size_t)size);
    if (!object)
    {
        _ext->ThrowOutOfMemory();
    }

    ExtRemoteData data(address, (ULONG)size);

    data.ReadBuffer(object, (ULONG)size);
    char* end = object + size;
    char* current = object;
    ulong ptrSize = this->_ext->m_PtrSize;

    while (current < end)
    {
        ULONG64 value;

        if (ptrSize == 8)
        {
            value = *((ULONG64*)current);
        }
        else
        {
            value = *((ULONG32*)current);
        }

        ULONG64 objectAlignmentMask = this->_heapBlockHelper.GetObjectAlignmentMask();
        if ((0 == (((size_t)address) & objectAlignmentMask)) && (value != address))
        {
            PushMark(value, address);
        }

        current += ptrSize;
    }

    free(object);
}

#endif