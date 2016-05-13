#include "stdafx.h"
#include "RecyclerObjectGraph.h"
#include "RecyclerRoots.h"

#ifdef JD_PRIVATE

RecyclerObjectGraph * RecyclerObjectGraph::New(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext, RecyclerObjectGraph::TypeInfoFlags typeInfoFlags)
{
    RecyclerObjectGraph * recyclerObjectGraph = GetExtension()->recyclerCachedData.GetCachedRecyclerObjectGraph(recycler.GetPtr());
    if (recyclerObjectGraph == nullptr)
    {
        Addresses *rootPointerManager = GetExtension()->recyclerCachedData.GetRootPointers(recycler, threadContext);
        ExtRemoteTyped heapBlockMap = recycler.Field("heapBlockMap");
        AutoDelete<RecyclerObjectGraph> newObjectGraph(new RecyclerObjectGraph(GetExtension(), recycler));
        newObjectGraph->Construct(heapBlockMap, *rootPointerManager);

        recyclerObjectGraph = newObjectGraph.Detach();
        GetExtension()->recyclerCachedData.CacheRecyclerObjectGraph(recycler.GetPtr(), recyclerObjectGraph);
    }

    recyclerObjectGraph->EnsureTypeInfo(typeInfoFlags);

    GetExtension()->recyclerCachedData.DisableCachedDebuggeeMemory();
    return recyclerObjectGraph;
}

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

void RecyclerObjectGraph::EnsureTypeInfo(RecyclerObjectGraph::TypeInfoFlags typeInfoFlags)
{
    bool infer = (typeInfoFlags & TypeInfoFlags::Infer) != 0;
    bool trident = (typeInfoFlags & TypeInfoFlags::Trident) != 0;
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

    GetExtension()->recyclerCachedData.EnableCachedDebuggeeMemory();

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
        JDRemoteTyped remoteTyped = GetExtension()->CastWithVtable(objectAddress, &typeName);
        if (typeName == nullptr)
        {
            return;
        }

        if (setNodeData(typeName, typeName, node, true, false))
        {
            auto addField = [&](JDRemoteTyped field, char const * name, bool overwriteVtable = false)
            {
                if (field.GetPtr() != 0)
                {
                    auto fieldNode = this->FindNode(field.GetPtr());
                    if (fieldNode && (!fieldNode->HasTypeInfo() || (fieldNode->HasVtable() && overwriteVtable)))
                    {
                        setNodeData(typeName, name, fieldNode, false, false);
                        return true;
                    }
                }
                return false;
            };

            auto addDynamicTypeField = [&](JDRemoteTyped obj, char const * dynamicTypeName)
            {
                JDRemoteTyped type = obj.Field("type");
                addField(type, dynamicTypeName == nullptr? "Js::DynamicType" : dynamicTypeName);
                addField(type.Field("propertyCache"), "Js::DynamicType.propertyCache");
            };

            auto addAuxSlots = [&](JDRemoteTyped remoteTyped)
            {
                JDRemoteTyped auxSlots = remoteTyped.Field("auxSlots");
                if (auxSlots.GetPtr() != 0)
                {
                    auto fieldNode = this->FindNode(auxSlots.GetPtr());
                    if (fieldNode && !fieldNode->HasTypeInfo())
                    {
                        // Aux slots might be use as a inline slot for object with small number of properties
                        char const * auxSlotTypeName;
                        JDRemoteTyped remoteTypedAuxSlot = GetExtension()->CastWithVtable(auxSlots.GetPtr(), &auxSlotTypeName);
                        if (auxSlotTypeName == nullptr)
                        {
                            setNodeData(typeName, "Js::DynamicObject.auxSlots[]", fieldNode, false, false);
                        }
                    }
                }
            };

            auto addDynamicObjectFields = [&](JDRemoteTyped obj, char const * dynamicTypeName = nullptr)
            {
                addDynamicTypeField(obj, dynamicTypeName);
                addAuxSlots(obj);
            };

            auto addArrayFields = [&](JDRemoteTyped arr, char const * segmentName)
            {
                addDynamicObjectFields(arr);
                JDRemoteTyped segment = arr.Field("head");
                while (segment.GetPtr() != 0)
                {
                    if (!addField(segment, segmentName))
                    {
                        break;
                    }
                    segment = segment.Field("next");
                }

                if (GetExtension()->IsJScript9())
                {
                    addField(arr.Field("segmentMap"), "Js::JavascriptArray.segmentBTreeRoot");
                }
                else
                {
                    addField(arr.Field("segmentUnion").Field("segmentBTreeRoot"), "Js::JavascriptArray.segmentBTreeRoot");
                }
            };

            auto addPathTypeBase = [&](JDRemoteTyped pathType, char const * typePathName)
            {
                JDRemoteTyped typePath = remoteTyped.Field("typePath");
                addField(typePath, typePathName);
                if (typePath.HasField("data"))
                {
                    addField(typePath.Field("data"), typePathName);
                }
                addField(remoteTyped.Field("predecessorType"), "Js::DynamicType");
            };

            char const * simpleTypeName = JDUtil::StripStructClass(remoteTyped.GetTypeName());
            bool isCrossSiteObject = strncmp(simpleTypeName, "Js::CrossSiteObject<", _countof("Js::CrossSiteObject<") - 1) == 0;

#define IsTypeOrCrossSite(type) strcmp(simpleTypeName,  isCrossSiteObject? "Js::CrossSiteObject<" ## type ## "> *" : type ## " *") == 0
#define TypeOrCrossSiteFieldName(type, field) isCrossSiteObject? "Js::CrossSiteObject<" ## type ## ">." ## field : type ## "." ## field

#define AddDictionaryField(dictionary, name) \
            if (addField(dictionary, name)) \
            { \
                addField(dictionary.Field("buckets"), name ## ".buckets"); \
                addField(dictionary.Field("entries"), name ## ".entries"); \
            }

            if (strcmp(simpleTypeName, "Js::RecyclableObject *") == 0
                || strcmp(simpleTypeName, "Js::JavascriptBoolean *") == 0)
            {
                addField(remoteTyped.Field("type"), "Js::StaticType");
            }
            else if (strcmp(simpleTypeName, "Js::LiteralString *") == 0)
            {
                addField(remoteTyped.Field("type"), "Js::StaticType");
                addField(remoteTyped.Field("m_pszValue"), "Js::LiteralString.m_pszValue");
            }
            else if (IsTypeOrCrossSite("Js::DynamicObject")
                || IsTypeOrCrossSite("Js::JavascriptStringObject")
                || IsTypeOrCrossSite("Js::JavascriptBooleanObject")
                || IsTypeOrCrossSite("Js::JavascriptNumberObject")
                || IsTypeOrCrossSite("Js::JavascriptSIMDObject"))
            {
                addDynamicObjectFields(remoteTyped);
            }
            else if (IsTypeOrCrossSite("Js::ScriptFunction"))
            {
                addDynamicObjectFields(remoteTyped, "Js::ScriptFunctionType");
                addField(remoteTyped.Field("environment"), "Js::FrameDisplay");
            }
            else if (IsTypeOrCrossSite("Js::ScriptFunctionWithInlineCache"))
            {
                addDynamicObjectFields(remoteTyped, "Js::ScriptFunctionType");
                addField(remoteTyped.Field("environment"), "Js::FrameDisplay");
                addField(remoteTyped.Field("m_inlineCaches"), TypeOrCrossSiteFieldName("Js::ScriptFunctionWithInlineCache", "m_inlineCaches"));
            }
            else if (IsTypeOrCrossSite("Js::BoundFunction"))
            {
                addDynamicObjectFields(remoteTyped);
                addField(remoteTyped.Field("boundArgs"), TypeOrCrossSiteFieldName("Js:BoundFunction", "boundArgs"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptArray"))
            {
                addArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptNativeIntArray"))
            {
                addArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptNativeIntArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptNativeFloatArray"))
            {
                addArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptNativeFloatArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptCopyOnAccessNativeIntArray"))
            {
                addArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptCopyOnAccessNativeIntArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptCopyOnAccessNativeFloatArray"))
            {
                addArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptCopyOnAccessNativeFloatArray", "{SparseArraySegments}"));
            }
            else if (strcmp(simpleTypeName, "Js::CustomExternalObject *") == 0)
            {
                addDynamicObjectFields(remoteTyped, "Js::ExternalType");
            }
            else if (IsTypeOrCrossSite("Js::GlobalObject"))
            {
                addDynamicObjectFields(remoteTyped);
                JDRemoteTyped loadInlineCacheMap = remoteTyped.Field("loadInlineCacheMap");
                AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");

                JDRemoteTyped loadMethodInlineCacheMap = remoteTyped.Field("loadMethodInlineCacheMap");
                AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");
                
                JDRemoteTyped storeInlineCacheMap = remoteTyped.Field("storeInlineCacheMap");
                AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");
                
                addField(remoteTyped.Field("reservedProperties"), "Js::GlobalObject.{ReservedPropertiesHashSet}");
            }
            else if (strcmp(simpleTypeName, "Js::FunctionEntryPointInfo *") == 0)
            {
                JDRemoteTyped constructorCacheList = remoteTyped.Field("constructorCaches");
                while (addField(constructorCacheList, "Js::FunctionEntryPointInfo.{ConstructorCacheList}"))
                {
                    constructorCacheList = constructorCacheList.Field("next");
                }
            }
            else if (strcmp(simpleTypeName, "Js::FunctionBody *") == 0)
            {
                RemoteFunctionBody functionBody(remoteTyped);

                if (functionBody.HasField("auxPtrs"))
                {
                    addField(JDUtil::GetWrappedField(functionBody, "auxPtrs"), "Js::FunctionBody.auxPtrs[]");
                }

                if (functionBody.HasField("counters"))
                {
                    addField(JDUtil::GetWrappedField(functionBody.Field("counters"), "fields"), "Js::FunctionBody.counters.fields");
                }

                if (functionBody.HasField("nestedArray"))
                {
                    addField(JDUtil::GetWrappedField(functionBody, "nestedArray"), "Js::FunctionBody.nestedArray");
                }

                JDRemoteTyped byteCodeBlock = functionBody.GetByteCodeBlock();
                if (addField(byteCodeBlock, "Js::FunctionBody.byteCodeBlock"))
                {
                    addField(byteCodeBlock.Field("m_content"), "Js::FunctionBody.byteCodeBlock.m_content");
                }

                JDRemoteTyped auxBlock = functionBody.GetAuxBlock();
                if (addField(auxBlock, "Js::FunctionBody.auxBlock"))
                {
                    addField(auxBlock.Field("m_content"), "Js::FunctionBody.auxBlock.m_content");
                }

                JDRemoteTyped auxContextBlock = functionBody.GetAuxContextBlock();
                if (addField(auxContextBlock, "Js::FunctionBody.auxContextBlock"))
                {
                    addField(auxContextBlock.Field("m_content"), "Js::FunctionBody.auxContextBlock.m_content");
                }

                JDRemoteTyped probeBackingStore = functionBody.GetProbeBackingStore();
                if (addField(probeBackingStore, "Js::FunctionBody.m_sourceInfo.probeBackingBlock"))
                {
                    addField(probeBackingStore.Field("m_content"), "Js::FunctionBody.m_sourceInfo.probeBackingBlock.m_content");
                }

                JDRemoteTyped statementMaps = functionBody.GetStatementMaps();
                if (addField(statementMaps, "Js::FunctionBody.statementMaps", true))
                {
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

                addField(functionBody.GetLoopHeaderArray(), "Js::FunctionBody.loopHeaderArray");
                addField(JDUtil::GetWrappedField(functionBody, "dynamicProfileInfo"), "Js::FunctionBody.dynamicProfileInfo");

                std::list<JDRemoteTyped> functionCodeGenRuntimeDataArrayStack;
                auto addFunctionCodeGenRuntimeDataArray = [&](JDRemoteTyped arr, uint count)
                {
                    if (addField(arr, "Js::FunctionBody.<FunctionCodeGenRuntimeData[]>"))
                    {
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

                addFunctionCodeGenRuntimeDataArray(functionBody.GetCodeGenRuntiemData(), functionBody.GetProfiledCallSiteCount());
                addFunctionCodeGenRuntimeDataArray(functionBody.GetCodeGenGetSetRuntimeData(), functionBody.GetInlineCacheCount());

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

                addField(functionBody.GetInlineCaches(), "Js::FunctionBody.<inlineCaches[]>");
                addField(JDUtil::GetWrappedField(functionBody.GetPolymorphicInlineCaches(), "inlineCaches"), "Js::FunctionBody.{PolymorphicInlineCaches[]}");

                addField(functionBody.GetConstTable(), "Js::FunctionBody.m_constTable");
                addField(functionBody.GetCacheIdToPropertyIdMap(), "Js::FunctionBody.cacheIdToPropertyIdMap");
                addField(functionBody.GetReferencedPropertyIdMap(), "Js::FunctionBody.referencedPropertyIdMap");
                addField(functionBody.GetLiteralRegexes(), "Js::FunctionBody.literalRegexes");
                addField(functionBody.GetPropertyIdsForScopeSlotArray(), "Js::FunctionBoredy.propertyIdsForScopeSlotArray");                
                addField(functionBody.GetDisplayName(), "Js::FunctionBody.m_displayName");
                addField(functionBody.GetScopeInfo(), "Js::FunctionBody.m_scopeInfo");

                JDRemoteTyped boundPropertyRecords = functionBody.GetBoundPropertyRecords();
                AddDictionaryField(boundPropertyRecords, "Js::FunctionBody.{BoundPropertyRecordsDictionary}");

                JDRemoteTyped sourceInfo = functionBody.GetSourceInfo();
                addField(sourceInfo.Field("pSpanSequence"), "Js::FunctionBody.sourceInfo.pSpanSequence");
                addField(JDUtil::GetWrappedField(sourceInfo, "m_auxStatementData"), "Js::FunctionBody.sourceInfo.m_auxStatementData");
                JDRemoteTyped scopeObjectChain = JDUtil::GetWrappedField(sourceInfo, "pScopeObjectChain");
                if (addField(scopeObjectChain, "Js::FunctionBody.sourceInfo.pScopeObjectChain"))
                {
                    JDRemoteTyped scopeObjectChainList = scopeObjectChain.Field("pScopeChain");
                    addField(scopeObjectChainList, "Js::FunctionBody.sourceInfo.pScopeObjectChain.pScopeChain", true);
                    int scopeObjectChainCount = scopeObjectChainList.Field("count").GetLong();
                    JDRemoteTyped scopeObjectChainListBuffer = scopeObjectChainList.Field("buffer");
                    addField(scopeObjectChainListBuffer, "Js::FunctionBody.sourceInfo.pScopeObjectChain.pScopeChain.buffer");
                    for (int i = 0; i < scopeObjectChainCount; i++)
                    {
                        JDRemoteTyped debuggerScope = scopeObjectChainListBuffer.ArrayElement(i);
                        addField(debuggerScope, "Js::DebuggerScope");
                        JDRemoteTyped debuggerScopePropertyList = debuggerScope.Field("scopeProperties");
                        if (addField(debuggerScopePropertyList, "Js::DebuggerScope.scopeProperties", true))
                        {
                            addField(debuggerScopePropertyList.Field("buffer"), "Js::DebuggerScope.scopeProperties.buffer");
                        }
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::ParseableFunctionInfo *") == 0)
            {
                RemoteParseableFunctionInfo parseableFunctionInfo(remoteTyped);
                JDRemoteTyped boundPropertyRecords = parseableFunctionInfo.GetBoundPropertyRecords();
                AddDictionaryField(boundPropertyRecords, "Js::ParseableFunctionInfo.{BoundPropertyRecordsDictionary}");                
                addField(parseableFunctionInfo.GetDisplayName(), "Js::ParseableFunctionInfo.m_displayName");
                addField(parseableFunctionInfo.GetScopeInfo(), "Js::ParseableFunctionInfo.m_scopeInfo");
            }
            else if (strcmp(simpleTypeName, "Js::SimpleSourceHolder *") == 0)
            {
                addField(remoteTyped.Field("source"), "Js::SimpleSourceHolder.source");
            }
            else if (strcmp(simpleTypeName, "Js::SimplePathTypeHandler *") == 0)
            {
                addPathTypeBase(remoteTyped, "Js::SimplePathTypeHandler.typePath");
                JDRemoteTyped successorTypeWeakRef = remoteTyped.Field("successorTypeWeakRef");
                if (addField(successorTypeWeakRef, "Js::SimplePathTypeHandler.successorTypeWeakRef"))
                {
                    addField(successorTypeWeakRef.Field("strongRef"), "Js::DynamicType");
                }
            }
            else if (strcmp(simpleTypeName, "Js::PathTypeHandler *") == 0)
            {
                addPathTypeBase(remoteTyped, "Js::PathTypeHandler.typePath");
                JDRemoteTyped propertySuccessors = remoteTyped.Field("propertySuccessors");
                if (addField(propertySuccessors, "Js::PathTypeHandler.propertySuccessors"))
                {
                    addField(propertySuccessors.Field("buckets"), "Js::PathTypeHandler.propertySuccessors.buckets");
                    JDRemoteTyped propertySucessorsEntries = propertySuccessors.Field("entries");
                    if (addField(propertySucessorsEntries, "Js::PathTypeHandler.propertySuccessors.entries"))
                    {
                        int count = propertySuccessors.Field("count").GetLong();
                        for (int i = 0; i < count; i++)
                        {
                            JDRemoteTyped successorTypeWeakRef = propertySucessorsEntries.ArrayElement(i).Field("value");
                            if (addField(successorTypeWeakRef, "Js::PathTypeHandler.successorTypeWeakRef"))
                            {
                                addField(successorTypeWeakRef.Field("strongRef"), "Js::DynamicType");
                            }
                        }
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::Utf8SourceInfo *") == 0)
            {
                if (!GetExtension()->IsJScript9())
                {
                    JDRemoteTyped lineOffsetCache = remoteTyped.Field("m_lineOffsetCache");
                    if (addField(lineOffsetCache, "Js::Utf8SourceInfo.m_lineOffsetCache"))
                    {
                        JDRemoteTyped lineOffsetCacheList = lineOffsetCache.Field("lineOffsetCacheList");
                        if (addField(lineOffsetCacheList, "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList", true))
                        {
                            addField(lineOffsetCacheList.Field("buffer"), "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList.buffer");
                        }
                    }
                    JDRemoteTyped deferredFunctionsDictionary = remoteTyped.Field("m_deferredFunctionsDictionary");
                    AddDictionaryField(deferredFunctionsDictionary, "Js::Utf8SourceInfo.{DeferredFunctionsDictionary}");
                }
                JDRemoteTyped functionBodyDictionary = remoteTyped.Field("functionBodyDictionary");
                AddDictionaryField(functionBodyDictionary, "Js::Utf8SourceInfo.{FunctionBodyDictionary}");
            }
            else if (strcmp(simpleTypeName, "UnifiedRegex::RegexPattern *") == 0)
            {
                JDRemoteTyped unifiedRep = remoteTyped.Field("rep").Field("unified");
                JDRemoteTyped program = unifiedRep.Field("program");
                addField(program, "UnifiedRegex::Program");
                addField(program.Field("source"), "UnifiedRegex::Program.source");
                addField(unifiedRep.Field("matcher"), "UnifiedRegex::Matcher");
                addField(unifiedRep.Field("trigramInfo"), "UnifiedRegex::TrigramInfo");
            }
            else
            {
                noScanFieldVtable.insert(typeName);
            }
        }
#undef IsTypeOrCrossSite
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
    Assert(node->GetObjectSize() == info.objectSize);
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
