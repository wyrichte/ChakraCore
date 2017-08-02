//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "RecyclerObjectGraph.h"
#include "RecyclerRoots.h"

RecyclerObjectGraph * RecyclerObjectGraph::New(RemoteRecycler recycler, RemoteThreadContext * threadContext, ULONG64 stackTop, RecyclerObjectGraph::TypeInfoFlags typeInfoFlags)
{
    if (recycler.CollectionInProgress())
    {
        g_Ext->Out("WARNING: Recycler is in collection state.  Object graph may be inaccurate\n");
    }
    RecyclerObjectGraph * recyclerObjectGraph = GetExtension()->recyclerCachedData.GetCachedRecyclerObjectGraph(recycler.GetPtr());
    if (recyclerObjectGraph == nullptr)
    {
        Addresses *rootPointerManager = GetExtension()->recyclerCachedData.GetRootPointers(recycler, threadContext, stackTop);
        AutoDelete<RecyclerObjectGraph> newObjectGraph(new RecyclerObjectGraph(recycler));
        newObjectGraph->Construct(recycler, *rootPointerManager);

        recyclerObjectGraph = newObjectGraph.Detach();
        GetExtension()->recyclerCachedData.CacheRecyclerObjectGraph(recycler.GetPtr(), recyclerObjectGraph);
    }

    recyclerObjectGraph->EnsureTypeInfo(threadContext, typeInfoFlags);

    GetExtension()->recyclerCachedData.DisableCachedDebuggeeMemory();
    return recyclerObjectGraph;
}

RecyclerObjectGraph::RecyclerObjectGraph(RemoteRecycler recycler, bool verbose) :
    _verbose(verbose),
    m_hasTypeName(false),
    m_hasTypeNameAndFields(false),
    m_trident(false),
    m_interior(recycler.EnableScanInteriorPointers())
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

void RecyclerObjectGraph::Construct(RemoteRecycler recycler, Addresses& roots)
{
    auto start = _time64(nullptr);

    ConstructData constructData(recycler);
    GetExtension()->recyclerCachedData.EnableCachedDebuggeeMemory();
    roots.Map([&](ULONG64 root)
    {
        MarkObject(constructData, root, 0, roots.GetRootType(root));
    });

    int iters = 0;

    while (constructData.markStack.size() != 0)
    {
        const MarkStackEntry object = constructData.markStack.top();
        constructData.markStack.pop();

        ScanBytes(constructData, object.first, object.second);

        iters++;
        if (iters % 0x10000 == 0)
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
                "\rTraversing object graph, object count - stack: %6d, visited: %d",
                constructData.markStack.size(), _objectGraph.GetNodeCount());
        }
    }

    g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
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

ULONG64 RecyclerObjectGraph::TryInferVarJavascriptLibrary(JDRemoteTyped& remoteTyped)
{
    if (remoteTyped.HasField("type"))
    {
        JDRemoteTyped objType = remoteTyped.Field("type");
        if (objType.HasField("javascriptLibrary"))
        {
            return objType.Field("javascriptLibrary").GetPtr();
        }
    }
    return 0;
}

ULONG64 RecyclerObjectGraph::TryInferFunctionProxyJavascriptLibrary(JDRemoteTyped& remoteTyped)
{
    if (remoteTyped.HasField("m_scriptContext"))
    {
        // FunctionProxy and derived classes
        JDRemoteTyped scriptContext = remoteTyped.Field("m_scriptContext");
        // This may be NoWriteBarrier wrapped.
        if (scriptContext.HasField("value"))
        {
            scriptContext = scriptContext.Field("value");
        }
        if (scriptContext.HasField("javascriptLibrary"))
        {
            return scriptContext.Field("javascriptLibrary").GetPtr();
        }
    }
    return 0;
}

ULONG64 RecyclerObjectGraph::InferJavascriptLibrary(RecyclerObjectGraph::GraphImplNodeType* node, JDRemoteTyped remoteTyped, char const * simpleTypeName)
{
    ULONG64 library = TryInferVarJavascriptLibrary(remoteTyped);
    if (library != 0)
    {
        return library;
    }

    if (strcmp(simpleTypeName, "Js::JavascriptLibrary *") == 0)
    {
        return remoteTyped.GetPtr();
    }

    bool isPathTypeHandler = strcmp(simpleTypeName, "Js::PathTypeHandler *") == 0 || strcmp(simpleTypeName, "Js::SimplePathTypeHandler *") == 0;
    if (isPathTypeHandler)
    {
        JDRemoteTyped predecessorType = remoteTyped.Field("predecessorType");
        if (predecessorType.GetPtr() != 0)
        {
            return predecessorType.Field("javascriptLibrary").GetPtr();
        }
    }

    bool isTypeHandler = isPathTypeHandler
        || STR_START_WITH(simpleTypeName, "Js::SimpleTypeHandler")
        || STR_START_WITH(simpleTypeName, "Js::DictionaryTypeHandlerBase")
        || STR_START_WITH(simpleTypeName, "Js::SimpleDictionaryTypeHandlerBase");

    if (isTypeHandler)
    {
        node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType * pred)
        {
            if (pred->HasTypeInfo())
            {
                if (strcmp(pred->GetTypeNameOrField(), "Js::DynamicType") == 0
                    || strcmp(pred->GetTypeNameOrField(), "Js::ExternalType") == 0
                    || strcmp(pred->GetTypeNameOrField(), "Js::ScriptFunctionType") == 0
                    || strcmp(pred->GetTypeName(), "Js::JavascriptLibrary") == 0)
                {
                    library = pred->GetAssociatedJavascriptLibrary();
                    return true;
                }
                return false;
            }

            char const * typeName;
            JDRemoteTyped predRemoteTyped = JDRemoteTyped::FromPtrWithVtable(pred->Key(), &typeName);
            if (typeName != nullptr)
            {
                if (strcmp(JDUtil::StripModuleName(typeName), "Js::JavascriptLibrary") == 0)
                {
                    library = pred->Key();
                    return true;
                }
                return false;
            }

            // Pretend that this is a dynamic type and see if typeHandler and javascriptLibrary looks correct
            predRemoteTyped = JDRemoteTyped::FromPtrWithType(pred->Key(), "Js::DynamicType");
            library = TryMatchDynamicType("Js::DynamicType", pred, "typeHandler", node);
            return library != 0;
        });
        return library;
    }

    if (strcmp(simpleTypeName, "Js::FunctionInfo *") == 0)
    {
        // Get the library info from the functionBodyImpl
        remoteTyped = RemoteFunctionInfo(remoteTyped).GetFunctionBody();
        if (remoteTyped.GetPtr() == 0)
        {
            return 0;
        }
    }

    if (strcmp(simpleTypeName, "Js::PolymorphicInlineCache *") == 0
        || strcmp(simpleTypeName, "Js::FunctionBodyPolymorphicInlineCache *") == 0
        || strcmp(simpleTypeName, "Js::ScriptContextPolymorphicInlineCache *") == 0)
    {
        // Even though PolymorphicInlineCache has a functionBody, the data is leaf
        // So the pointer may not be valid memory if the PolymorphicInlineCache has a false reference to it
        // We will override the information in the main loop instead of infering the library here.
        return 0;
    }

    if (strcmp(simpleTypeName, "Js::IntlEngineInterfaceExtensionObject *") == 0)
    {
        return remoteTyped.Field("scriptContext").Field("javascriptLibrary").GetPtr();
    }

    library = TryInferFunctionProxyJavascriptLibrary(remoteTyped);
    if (library != 0)
    {
        return library;
    }

    if (remoteTyped.HasField("library"))
    {
        // EntryPointInfo and derived classes
        JDRemoteTyped libraryRemoteTyped = remoteTyped.Field("library");
        // sanity check
        if (libraryRemoteTyped.HasField("scriptContext"))
        {
            library = libraryRemoteTyped.GetPtr();
            if (library != 0)
            {
                return library;
            }
        }        
    }

    if (strcmp(simpleTypeName, "Js::FunctionEntryPointInfo *") == 0
        || strcmp(simpleTypeName, "Js::ProxyEntryPointInfo *") == 0)
    {
        node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType* pred)
        {
            if (pred->HasTypeInfo())
            {
                if (strcmp(pred->GetTypeNameOrField(), "Js::ScriptFunctionType") == 0
                    || strcmp(JDUtil::StripModuleName(pred->GetTypeName()), "Js::FunctionBody") == 0
                    || strcmp(JDUtil::StripModuleName(pred->GetTypeName()), "Js::ParseableFunctionInfo") == 0
                    || strcmp(JDUtil::StripModuleName(pred->GetTypeName()), "Js::DeferDeserializeFunctionInfo") == 0)
                {
                    library = pred->GetAssociatedJavascriptLibrary();
                    return true;
                }
                return false;
            }

            char const * typeName;
            JDRemoteTyped predRemoteTyped = JDRemoteTyped::FromPtrWithVtable(pred->Key(), &typeName);
            if (typeName != nullptr)
            {
                library = TryInferFunctionProxyJavascriptLibrary(predRemoteTyped);
            }
            else
            {
                library = TryMatchDynamicType("Js::ScriptFunctionType", pred, "entryPointInfo", node);
            }
            return library != 0;
        });
        return library;
    }

    if (strcmp(simpleTypeName, "JavascriptDispatch *") == 0)
    {
        JDRemoteTyped scriptObjectRemoteTyped = remoteTyped.Field("scriptObject");
        if (scriptObjectRemoteTyped.GetPtr())
        {
            return scriptObjectRemoteTyped.Field("type").Field("javascriptLibrary").GetPtr();
        }
        return 0;
    }

    if (strcmp(simpleTypeName, "Js::SimpleSourceHolder *") == 0)
    {
        node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType* pred)
        {
            if (pred->HasTypeInfo())
            {
                if (strcmp(JDUtil::StripModuleName(pred->GetTypeName()), "Js::Utf8SourceInfo") == 0)
                {
                    library = pred->GetAssociatedJavascriptLibrary();
                    return true;
                }
                return false;
            }

            char const * typeName;
            JDRemoteTyped predRemoteTyped = JDRemoteTyped::FromPtrWithVtable(pred->Key(), &typeName);
            if (typeName != nullptr && strcmp(JDUtil::StripModuleName(typeName), "Js::Utf8SourceInfo") == 0)
            {
                RemoteUtf8SourceInfo utf8SourceInfo(predRemoteTyped);
                library = utf8SourceInfo.GetScriptContext().GetJavascriptLibrary().GetPtr();
                return true;
            }
            return false;
        });
        return library;
    }
    return 0;

}

ULONG64 RecyclerObjectGraph::TryMatchDynamicType(char const * type, RecyclerObjectGraph::GraphImplNodeType* pred, char const * field, RecyclerObjectGraph::GraphImplNodeType* node)
{
    JDRemoteTyped remoteTyped = JDRemoteTyped::FromPtrWithType(pred->Key(), type);
    ULONG64 typeSize = remoteTyped.Dereference().GetTypeSize();
    if (pred->GetObjectSize() < typeSize)
    {
        return false;
    }
    if (remoteTyped.Field(field).GetPtr() != node->Key())
    {
        return false;
    }
    ULONG64 library = remoteTyped.Field("javascriptLibrary").GetPtr();
    char const * typeName;
    JDRemoteTyped::FromPtrWithVtable(library, &typeName);
    if (typeName == nullptr || strcmp(JDUtil::StripModuleName(typeName), "Js::JavascriptLibrary") != 0)
    {
        return 0;
    }
    return library;
}

void RecyclerObjectGraph::EnsureTypeInfo(RemoteThreadContext * threadContext, RecyclerObjectGraph::TypeInfoFlags typeInfoFlags)
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

    auto setNodeData = [&](char const * typeName, char const * typeNameOrField, RecyclerObjectGraph::GraphImplNodeType * node, bool hasVtable, bool isPropagated, ULONG64 javascriptLibrary)
    {
        node->SetTypeInfo(typeName, typeNameOrField, hasVtable, isPropagated, javascriptLibrary);
        if (!infer)
        {
            return false;
        }
        auto i = noScanFieldVtable.find(typeName);
        return (i == noScanFieldVtable.end());
    };

    GetExtension()->recyclerCachedData.EnableCachedDebuggeeMemory();

    auto setAddressData = [&](char const * typeName, JDRemoteTyped object, bool hasVtable = false, ULONG64 library = 0, bool requireLivePointer = true)
    {
        if (object.GetPtr())
        {
            RecyclerObjectGraph::GraphImplNodeType* node = this->FindNode(object.GetPtr());
            if (requireLivePointer || node)
            {
                setNodeData(typeName, typeName, node, hasVtable, false, library);
                return true;
            }
        }
        return false;
    };

    auto addFieldData = [&](JDRemoteTyped field, char const * typeName, char const * fieldTypeName, bool overwriteVtable, ULONG64 library)
    {
        if (field.GetPtr() != 0)
        {
            auto fieldNode = this->FindNode(field.GetPtr());
            if (fieldNode && (!fieldNode->HasTypeInfo() || (fieldNode->HasVtable() && overwriteVtable)))
            {
                setNodeData(typeName, fieldTypeName, fieldNode, false, false, library);
                return true;
            }
        }
        return false;
    };

    JDRemoteTyped threadRecyclableData = threadContext->GetExtRemoteTyped().Field("recyclableData.ptr");
    setAddressData("ThreadContext::RecyclableData", threadRecyclableData);
    RemoteBaseDictionary typesWithProtoPropertyCache = threadRecyclableData.Field("typesWithProtoPropertyCache");
    setAddressData("ThreadContext::RecyclableData.typesWithProtoPropertyCache.buckets", typesWithProtoPropertyCache.GetBuckets());
    setAddressData("ThreadContext::RecyclableData.typesWithProtoPropertyCache.entries", typesWithProtoPropertyCache.GetEntries());
    typesWithProtoPropertyCache.ForEachValue([&](RemoteWeaklyReferencedKeyDictionary typeHashSet)
    {
        setAddressData("ThreadContext::RecyclableData.typesWithProtoPropertyCache.entries.{TypeHashSet}", typeHashSet.GetExtRemoteTyped());
        setAddressData("ThreadContext::RecyclableData.typesWithProtoPropertyCache.entries.{TypeHashSet}.buckets", typeHashSet.GetBuckets());
        setAddressData("ThreadContext::RecyclableData.typesWithProtoPropertyCache.entries.{TypeHashSet}.entries", typeHashSet.GetEntries());
        typeHashSet.ForEachKey([&](JDRemoteTyped key)
        {
            setAddressData("RecyclerWeakReference<Js::Type>", key);
            return false;
        });
        return false;
    });
    RemoteWeaklyReferencedKeyDictionary propertyGuards = threadRecyclableData.Field("propertyGuards");
    setAddressData("ThreadContext::RecyclableData.propertyGuards.buckets", propertyGuards.GetBuckets());
    setAddressData("ThreadContext::RecyclableData.propertyGuards.entries", propertyGuards.GetEntries());
    propertyGuards.ForEachValue([&](JDRemoteTyped entry)
    {
        setAddressData("ThreadContext::PropertyGuardEntry", entry);
        setAddressData("ThreadContext::PropertyGuardEntry.sharedGuard", entry.Field("sharedGuard"));
        RemoteBaseDictionary uniqueGuards = entry.Field("uniqueGuards");
        setAddressData("ThreadContext::PropertyGuardEntry.uniqueGuards.buckets", uniqueGuards.GetBuckets());
        setAddressData("ThreadContext::PropertyGuardEntry.uniqueGuards.entries", uniqueGuards.GetEntries());
        uniqueGuards.ForEachValue([&](JDRemoteTyped value)
        {
            setAddressData("RecyclerWeakReference<Js::PropertyGuard>", value);
            return false;
        });
        if (entry.HasField("entryPoints"))  // IE doesn't have entryPoints
        {
            RemoteWeaklyReferencedKeyDictionary entryPoints = entry.Field("entryPoints");
            setAddressData("ThreadContext::PropertyGuardEntry.entryPoints", entryPoints.GetExtRemoteTyped());
            if (entryPoints.GetExtRemoteTyped().GetPtr() != 0)
            {
                entryPoints.ForEachKey([&](JDRemoteTyped key)
                {
                    setAddressData("RecyclerWeakReference<Js::EntryPointInfo>", key);
                    return false;
                });
            }
        }
        return false;
    });
    RemoteBaseDictionary caseInvariantPropertySet = threadRecyclableData.Field("caseInvariantPropertySet");
    if (setAddressData("ThreadContext::RecyclableData.caseInvariantPropertySet", caseInvariantPropertySet.GetExtRemoteTyped()))
    {
        setAddressData("ThreadContext::RecyclableData.caseInvariantPropertySet.buckets", caseInvariantPropertySet.GetBuckets());
        setAddressData("ThreadContext::RecyclableData.caseInvariantPropertySet.entries", caseInvariantPropertySet.GetEntries());
    }
    JDRemoteTyped boundPropertyStrings = threadRecyclableData.Field("boundPropertyStrings");
    setAddressData("ThreadContext::RecyclableData.boundPropertyStrings.buffer", boundPropertyStrings.Field("buffer"));   
    if (threadRecyclableData.HasField("symbolRegistrationMap"))  // IE doesn't have symbolRegistrationMap
    {
        RemoteBaseDictionary symbolRegistrationMap = threadRecyclableData.Field("symbolRegistrationMap");
        setAddressData("ThreadContext::RecyclableData.symbolRegistrationMap.buckets", symbolRegistrationMap.GetBuckets());
        setAddressData("ThreadContext::RecyclableData.symbolRegistrationMap.entries", symbolRegistrationMap.GetEntries());
    }

    // For RS2 and below Js::Cache has a vtable, but may be ICF with other type.  Process them first.
    RemoteThreadContext remoteThreadContext(*threadContext);
    remoteThreadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
    {
        JDRemoteTyped library = scriptContext.GetJavascriptLibrary();
        JDRemoteTyped cache;
        if (library.HasField("scriptContextCache"))
        {
            cache = library.Field("scriptContextCache").Cast("Js::Cache");
        }
        else
        {
            return false;
        }
        
        if (cache.GetPtr() != 0)
        {
            ULONG64 javascriptLibrary = library.GetPtr();
            JDRemoteTyped sourceContextInfoMap = cache.Field("sourceContextInfoMap");
            if (addFieldData(sourceContextInfoMap, "Js::Cache", "Js::Cache.sourceContextInfoMap", false, javascriptLibrary))
            {
                addFieldData(sourceContextInfoMap.Field("buckets"), "Js::Cache", "Js::Cache.sourceContextInfoMap.buckets", false, javascriptLibrary);
                JDRemoteTyped sourceContextInfoMapEntries = sourceContextInfoMap.Field("entries");

                if (addFieldData(sourceContextInfoMapEntries, "Js::Cache", "Js::Cache.sourceContextInfoMap.entries", false, javascriptLibrary))
                {
                    int count = sourceContextInfoMap.Field("count").GetLong();
                    for (int i = 0; i < count; i++)
                    {
                        JDRemoteTyped sourceContextInfo = sourceContextInfoMapEntries.ArrayElement(i).Field("value");
                        if (addFieldData(sourceContextInfo, "Js::Cache", "SourceContextInfo", false, javascriptLibrary))
                        {
                            addFieldData(sourceContextInfo.Field("sourceDynamicProfileManager"), "Js::Cache", "SourceDynamicProfileManager", false, javascriptLibrary);
                        }
                    }
                }
            }
        }
        return true;
    });

    class AutoError
    {
    public:        
        ~AutoError()
        {
            if (objectAddress != 0)
            {
                g_Ext->Err("ERROR: exception throw when processing %p with type %s\n", objectAddress, typeName);
            }
        }
        void Start(ULONG64 objectAddress, char const * typeName)
        {
            this->objectAddress = objectAddress;
            this->typeName = typeName;
        }
        void Clear()
        {
            this->objectAddress = 0;
        }
    private:
        ULONG64 objectAddress;
        char const * typeName;
    } autoError;

    ULONG numNodes = 0;
    this->MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        autoError.Clear();

        numNodes++;
        if (numNodes % 10000 == 0)
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rProcessing objects for type info %s- %11d/%11d", infer? "" : "without infer ", numNodes, this->GetNodeCount());
        }

        if (node->HasTypeInfo())
        {
            return;
        }

        char const * typeName = nullptr;
        ULONG64 objectAddress = node->Key();
        JDRemoteTyped remoteTyped = JDRemoteTyped::FromPtrWithVtable(objectAddress, &typeName);
        if (typeName == nullptr)
        {
            return;
        }
        
        autoError.Start(objectAddress, typeName);

        char const * simpleTypeName = JDUtil::StripStructClass(remoteTyped.GetTypeName());
        ULONG64 javascriptLibrary = InferJavascriptLibrary(node, remoteTyped, simpleTypeName);

        if (setNodeData(typeName, typeName, node, true, false, javascriptLibrary))
        {
            auto addField = [&](JDRemoteTyped field, char const * fieldTypeName, bool overwriteVtable = false, ULONG64 library = 0)
            {
                if (library == 0)
                {
                    library = javascriptLibrary;
                }
                return addFieldData(field, typeName, fieldTypeName, overwriteVtable, library);
            };

            auto addDynamicTypeField = [&](JDRemoteTyped type, char const * dynamicTypeName = nullptr)
            {
                ULONG64 library = javascriptLibrary;
                if (library == 0)
                {
                    library = type.Field("javascriptLibrary").GetPtr();
                }
                addField(type, dynamicTypeName == nullptr ? "Js::DynamicType" : dynamicTypeName, false, library);
                addField(type.Field("propertyCache"), "Js::DynamicType.propertyCache", false, library);
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
                        JDRemoteTyped remoteTypedAuxSlot = JDRemoteTyped::FromPtrWithVtable(auxSlots.GetPtr(), &auxSlotTypeName);
                        if (auxSlotTypeName == nullptr)
                        {
                            setNodeData(typeName, "Js::DynamicObject.auxSlots[]", fieldNode, false, false, javascriptLibrary);
                        }
                    }
                }
            };

            auto addDynamicObjectFields = [&](JDRemoteTyped obj, char const * dynamicTypeName = nullptr)
            {
                addDynamicTypeField(obj.Field("type"), dynamicTypeName);
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

            auto addNativeArrayFields = [&](JDRemoteTyped arr, char const * segmentName)
            {
                addArrayFields(arr, segmentName);
                addField(arr.Field("weakRefToFuncBody"), "RecyclerWeakReference<Js::FunctionBody>");
            };

            auto addPathTypeBase = [&](JDRemoteTyped pathType, char const * typePathName)
            {
                JDRemoteTyped typePath = remoteTyped.Field("typePath");
                addField(typePath, typePathName);
                if (typePath.HasField("data"))
                {
                    addField(typePath.Field("data"), typePathName);
                }
                JDRemoteTyped predecessorType = remoteTyped.Field("predecessorType");
                if (predecessorType.GetPtr() != 0)
                {
                    addDynamicTypeField(remoteTyped.Field("predecessorType"));
                }
                addField(typePath.Field("singletonInstance"), "RecyclerWeakReference<Js::DynamicObject>");
            };

#define AddDictionaryFieldOnlyBase(dictionary, name, MACRO) \
            addField(dictionary.GetBuckets(), MACRO(name ## ".buckets")); \
            addField(dictionary.GetEntries(), MACRO(name ## ".entries"))

#define AddDictionaryFieldBase(dictionary, name, MACRO) \
            if (addField(dictionary.GetExtRemoteTyped(), MACRO(name), true)) \
            { \
                AddDictionaryFieldOnlyBase(dictionary, name, MACRO); \
            }

#define IDENT_MACRO(n) n
#define AddDictionaryFieldOnly(dictionary, name) AddDictionaryFieldOnlyBase(dictionary, name, IDENT_MACRO)
#define AddDictionaryField(dictionary, name) AddDictionaryFieldBase(dictionary, name, IDENT_MACRO)

            enum FunctionProxyNameIndex
            {
                ParseableFunctionInfoNameIndex,
                FunctionBodyNameIndex,
                DeferDeserializedNameindex
            };

#define FUNCTION_PROXY_FIELD_NAME(n) (nameIndex == ParseableFunctionInfoNameIndex? "Js::ParseableFunctionInfo."##n \
    : nameIndex == FunctionBodyNameIndex ? "Js::FunctionBody."##n  : "Js::DeferDeserialized."##n)

            auto addFunctionProxyFields = [&](JDRemoteTyped remoteTyped, FunctionProxyNameIndex nameIndex)
            {
                RemoteFunctionProxy functionProxy(remoteTyped);
                if (functionProxy.HasField("auxPtrs"))
                {
                    addField(JDUtil::GetWrappedField(functionProxy, "auxPtrs"), FUNCTION_PROXY_FIELD_NAME("auxPtrs[]"));
                }

                // Even though function object type list has a vtable, it would be good to give it a better name
                JDRemoteTyped functionObjectTypeList = functionProxy.GetFunctionObjectTypeList();
                if (addField(functionObjectTypeList, FUNCTION_PROXY_FIELD_NAME("functionObjectTypeList"), true))
                {
                    JDRemoteTyped buffer = functionObjectTypeList.Field("buffer");
                    addField(buffer, FUNCTION_PROXY_FIELD_NAME("functionObjectTypeList.buffer"));
                    int count = functionObjectTypeList.Field("count").GetLong();
                    for (int i = 0; i < count; i++)
                    {
                        addField(buffer.ArrayElement(i), "RecyclerWeakReference<Js::ScriptFunctionType>");
                    }
                }
            };

            auto addParseableFunctionInfoFields = [&](JDRemoteTyped remoteTyped, FunctionProxyNameIndex nameIndex)
            {
                addFunctionProxyFields(remoteTyped, nameIndex);
                RemoteParseableFunctionInfo parseableFunctionInfo(remoteTyped);
                RemoteBaseDictionary boundPropertyRecords = parseableFunctionInfo.GetBoundPropertyRecords();
                AddDictionaryFieldBase(boundPropertyRecords, "{BoundPropertyRecordsDictionary}", FUNCTION_PROXY_FIELD_NAME);
                addField(parseableFunctionInfo.GetDisplayName(), FUNCTION_PROXY_FIELD_NAME("m_displayName"));
                addField(parseableFunctionInfo.GetScopeInfo(), FUNCTION_PROXY_FIELD_NAME("m_scopeInfo"));
                addField(parseableFunctionInfo.GetNestedArray(), FUNCTION_PROXY_FIELD_NAME("nestedArray"));
                addField(parseableFunctionInfo.GetDeferredPrototypeType(), "Js::ScriptFunctionType");
                addField(parseableFunctionInfo.GetPropertyIdsForScopeSlotArray(), FUNCTION_PROXY_FIELD_NAME("propertyIdsForScopeSlotArray"));
                addField(parseableFunctionInfo.GetDeferredStubs(), FUNCTION_PROXY_FIELD_NAME("deferredStubs"));
            };
#undef FUNCTION_PROXY_FIELD_NAME

            enum EntryPointInfoNameIndex
            {
                FunctionEntryPointInfoNameIndex,
                LoopEntryPointInfoNameIndex
            };
#define ENTRY_POINT_FIELD_NAME(n) (nameIndex == FunctionEntryPointInfoNameIndex? "Js::FunctionEntryPointInfo."##n : "Js::LoopEntryPointInfo."##n)

            auto addEntryPointInfoField = [&](JDRemoteTyped remoteTyped, EntryPointInfoNameIndex nameIndex)
            {
                RemoteBaseDictionary weakFuncRefSet = remoteTyped.Field("weakFuncRefSet");
                AddDictionaryFieldBase(weakFuncRefSet, "{WeakFuncRefSet}", ENTRY_POINT_FIELD_NAME);
                RemoteBaseDictionary sharedPropertyGuards = remoteTyped.Field("sharedPropertyGuards");
                AddDictionaryFieldBase(sharedPropertyGuards, "{SharedPropertyGuardDictionary}", ENTRY_POINT_FIELD_NAME);
                int propertyGuardCount = remoteTyped.Field("propertyGuardCount").GetLong();
                JDRemoteTyped propertyGuardWeakRefs = remoteTyped.Field("propertyGuardWeakRefs");
                addField(propertyGuardWeakRefs, ENTRY_POINT_FIELD_NAME("propertyGuardWeakRefs"));
                for (int i = 0; i < propertyGuardCount; i++)
                {
                    addField(propertyGuardWeakRefs.ArrayElement(i), "Js::FakePropertyGuardWeakReference");
                }

                addField(remoteTyped.Field("equivalentTypeCaches"), ENTRY_POINT_FIELD_NAME("equivalentTypeCaches"));
                JDRemoteTyped constructorCacheList = remoteTyped.Field("constructorCaches");
                while (addField(constructorCacheList, ENTRY_POINT_FIELD_NAME("{ConstructorCacheList}")))
                {
                    constructorCacheList = constructorCacheList.Field("next");
                }
                addField(remoteTyped.Field("polymorphicInlineCacheInfo"), ENTRY_POINT_FIELD_NAME("polymorphicInlineCacheInfo"));
                addField(remoteTyped.Field("jitTransferData"), ENTRY_POINT_FIELD_NAME("jitTransferData"));
                addField(remoteTyped.Field("runtimeTypeRefs"), ENTRY_POINT_FIELD_NAME("runtimeTypeRefs"));
                if (remoteTyped.HasField("fieldAccessStats"))
                {
                    // DBG_DUMP only
                    addField(remoteTyped.Field("fieldAccessStats"), nameIndex == FunctionEntryPointInfoNameIndex?
                        "[DBG_DUMP] Js::FunctionEntryPointInfo.fieldAccessStats" : "[DBG_DUMP] Js::LoopEntryPointInfo.fieldAccessStats");
                }
            };

            bool isCrossSiteObject = strncmp(simpleTypeName, "Js::CrossSiteObject<", _countof("Js::CrossSiteObject<") - 1) == 0;

#define IsTypeOrCrossSite(type) strcmp(simpleTypeName,  isCrossSiteObject? "Js::CrossSiteObject<" ## type ## "> *" : type ## " *") == 0
#define TypeOrCrossSiteFieldName(type, field) isCrossSiteObject? "Js::CrossSiteObject<" ## type ## ">." ## field : type ## "." ## field

            auto addFunctionFields = [&](JDRemoteTyped remoteTyped)
            {
                JDRemoteTyped constructorCache = remoteTyped.Field("constructorCache");
                addField(constructorCache, "Js::ConstructorCache");
                JDRemoteTyped constructorCacheType = constructorCache.Field("content").Field("type");
                if (constructorCacheType.GetPtr() != 0)
                {
                    addDynamicTypeField(constructorCacheType);
                }
            };

            auto addScriptFunctionFields = [&](JDRemoteTyped remoteTyped)
            {
                addDynamicObjectFields(remoteTyped, "Js::ScriptFunctionType");
                addFunctionFields(remoteTyped);
                JDRemoteTyped frameDisplay = remoteTyped.Field("environment");
                addField(frameDisplay, "Js::FrameDisplay");
                uint16 frameDisplayLength = frameDisplay.Field("length").GetUshort();
                JDRemoteTyped scopes = frameDisplay.Field("scopes");
                for (uint16 i = 0; i < frameDisplayLength; i++)
                {
                    addField(scopes.ArrayElement(i), "Js::FrameDisplayScopeSlots");
                }
            };

            if (strcmp(simpleTypeName, "Js::LiteralString *") == 0)
            {
                addField(remoteTyped.Field("m_pszValue"), "Js::LiteralString.m_pszValue");
            }
            else if (strcmp(simpleTypeName, "Js::LiteralStringWithPropertyStringPtr *") == 0)
            {
                addField(remoteTyped.Field("m_pszValue"), "Js::LiteralStringWithPropertyStringPtr.m_pszValue");
            }
            else if (strcmp(simpleTypeName, "Js::BufferStringBuilder::WritableString *") == 0)
            {
                addField(remoteTyped.Field("m_pszValue"), "Js::BufferStringBuilder::WritableString.m_pszValue");
            }            
            else if (strcmp(simpleTypeName, "Js::SubString *") == 0)
            {
                addField(remoteTyped.Field("m_pszValue"), "Js::SubString.m_pszValue");
                addField(remoteTyped.Field("originalFullStringReference"), "Js::SubString.originalFullStringReference");
            }
            else if (strcmp(simpleTypeName, "Js::CompoundString *") == 0)
            {
                addField(remoteTyped.Field("m_pszValue"), "Js::CompoundString.m_pszValue");
                JDRemoteTyped block = remoteTyped.Field("lastBlock");
                while (block.GetPtr() != 0)
                {
                    addField(block, "Js::CompoundString::Block");
                    block = block.Field("previous");
                }

                addField(remoteTyped.Field("lastBlockInfo").Field("buffer"), "Js::CompoundString::lastBlockInfo.buffer");
            }
            else if (IsTypeOrCrossSite("Js::DynamicObject")
                || IsTypeOrCrossSite("Js::JavascriptStringObject")
                || IsTypeOrCrossSite("Js::JavascriptBooleanObject")
                || IsTypeOrCrossSite("Js::JavascriptNumberObject")
                || IsTypeOrCrossSite("Js::JavascriptSIMDObject")
                || IsTypeOrCrossSite("Js::RuntimeFunction")
                || IsTypeOrCrossSite("Js::ActivationObject")
                || IsTypeOrCrossSite("Js::ActivationObjectEx"))
            {
                addDynamicObjectFields(remoteTyped);
            }
            else if (IsTypeOrCrossSite("Js::JavascriptMap"))
            {
                addDynamicObjectFields(remoteTyped);
                RemoteBaseDictionary mapDataMap = remoteTyped.Field("map");
                AddDictionaryField(mapDataMap, "Js::JavascriptMap::MapDataMap");
            }
            else if (IsTypeOrCrossSite("Js::JavascriptSet"))
            {
                addDynamicObjectFields(remoteTyped);
                RemoteBaseDictionary setDataSet = remoteTyped.Field("set");
                AddDictionaryField(setDataSet, "Js::JavascriptSet::SetDataSet");
            }
            else if (IsTypeOrCrossSite("Js::JavascriptWeakMap")
                || IsTypeOrCrossSite("Js::JavascriptWeakSet"))
            {
                addDynamicObjectFields(remoteTyped);
                RemoteWeaklyReferencedKeyDictionary keySet = remoteTyped.Field("keySet");
                AddDictionaryFieldOnly(keySet, "Js::JavascriptWeakMap::KeySet");
            }
            else if (IsTypeOrCrossSite("Js::JavascriptExternalFunction"))
            {
                addDynamicObjectFields(remoteTyped);
                addFunctionFields(remoteTyped);
            }
            else if (IsTypeOrCrossSite("Js::ScriptFunction"))
            {
                addScriptFunctionFields(remoteTyped);
            }
            else if (IsTypeOrCrossSite("Js::ScriptFunctionWithInlineCache"))
            {
                addScriptFunctionFields(remoteTyped);
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
                addNativeArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptNativeIntArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptNativeFloatArray"))
            {
                addNativeArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptNativeFloatArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptCopyOnAccessNativeIntArray"))
            {
                addNativeArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptCopyOnAccessNativeIntArray", "{SparseArraySegments}"));
            }
            else if (IsTypeOrCrossSite("Js::JavascriptCopyOnAccessNativeFloatArray"))
            {
                addNativeArrayFields(remoteTyped, TypeOrCrossSiteFieldName("Js::JavascriptCopyOnAccessNativeFloatArray", "{SparseArraySegments}"));
            }
            else if (strcmp(simpleTypeName, "Js::CustomExternalObject *") == 0)
            {
                addDynamicObjectFields(remoteTyped, "Js::ExternalType");
            }
            else if (IsTypeOrCrossSite("Js::GlobalObject"))
            {
                addDynamicObjectFields(remoteTyped);
                RemoteBaseDictionary loadInlineCacheMap = remoteTyped.Field("loadInlineCacheMap");
                AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");

                // IE11 doesn't separate loadMethodInlineCaches
                if (remoteTyped.HasField("loadMethodInlineCacheMap"))
                {
                    RemoteBaseDictionary loadMethodInlineCacheMap = remoteTyped.Field("loadMethodInlineCacheMap");
                    AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");
                }

                RemoteBaseDictionary storeInlineCacheMap = remoteTyped.Field("storeInlineCacheMap");
                AddDictionaryField(loadInlineCacheMap, "Js::GlobalObject.{RootObjectInlineCacheMap}");

                addField(remoteTyped.Field("reservedProperties"), "Js::GlobalObject.{ReservedPropertiesHashSet}");
            }
            else if (strcmp(simpleTypeName, "Js::LoopEntryPointInfo *") == 0)
            {
                addEntryPointInfoField(remoteTyped, LoopEntryPointInfoNameIndex);
            }
            else if (strcmp(simpleTypeName, "Js::FunctionEntryPointInfo *") == 0)
            {
                addEntryPointInfoField(remoteTyped, FunctionEntryPointInfoNameIndex);
            }
            else if (strcmp(simpleTypeName, "Js::DeferDeserializeFunctionInfo *") == 0)
            {
                addFunctionProxyFields(remoteTyped, DeferDeserializedNameindex);
            }
            else if (strcmp(simpleTypeName, "Js::ParseableFunctionInfo *") == 0)
            {
                addParseableFunctionInfoFields(remoteTyped, ParseableFunctionInfoNameIndex);
            }
            else if (strcmp(simpleTypeName, "Js::FunctionBody *") == 0)
            {
                addParseableFunctionInfoFields(remoteTyped, FunctionBodyNameIndex);
                RemoteFunctionBody functionBody(remoteTyped);                
                if (functionBody.HasField("counters"))
                {
                    addField(JDUtil::GetWrappedField(functionBody.Field("counters"), "fields"), "Js::FunctionBody.counters.fields");
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
                    int count = statementMaps.Field("count").GetLong();
                    JDRemoteTyped buffer = statementMaps.Field("buffer");
                    addField(buffer, "Js::FunctionBody.statementMaps.buffer");
                    for (int i = 0; i < count; i++)
                    {
                        addField(buffer.ArrayElement(i), "Js::FunctionBody::StatementMap");
                    }                    
                }

                JDRemoteTyped entryPoints = functionBody.GetEntryPoints();
                if (entryPoints.GetPtr() != 0)
                {
                    // Even though the list itself has a vtable, It would be good to give it a better name, and associate with the library
                    addField(entryPoints, "Js::FunctionBody.entryPoints", true);
                    int count = entryPoints.Field("count").GetLong();
                    JDRemoteTyped buffer = entryPoints.Field("buffer");
                    addField(buffer, "Js::FunctionBody.entryPoints.buffer");
                    for (int i = 0; i < count; i++)
                    {
                        addField(buffer.ArrayElement(i), "Js::FunctionBody.entryPoints.buffer.<functionEntryPointInfoWeakRef>");
                    }
                }

                JDRemoteTyped loopHeaderArray = functionBody.GetLoopHeaderArray();
                if (addField(loopHeaderArray, "Js::FunctionBody.loopHeaderArray"))
                {
                    uint loopCount = functionBody.GetLoopCount();
                    for (uint i = 0; i < loopCount; i++)
                    {
                        JDRemoteTyped loopHeaderEntryPoints = loopHeaderArray.ArrayElement(i).Field("entryPoints");
                        // Even though the entry point list has a vtable, its better to have a better name, and associate with the library
                        addField(loopHeaderEntryPoints, "Js::LoopHeader.entryPoints", true);
                        addField(loopHeaderEntryPoints.Field("buffer"), "Js::LoopHeader.entryPoints.buffer");
                    }
                }
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
                        // IE11 doesn't have ldFldInlinees
                        if (curr.HasField("ldFldInlinees"))
                        {
                            addFunctionCodeGenRuntimeDataArray(curr.Field("ldFldInlinees"), currBody.GetInlineCacheCount());
                        }
                        curr = curr.Field("next");
                    }
                }

                addField(functionBody.GetInlineCaches(), "Js::FunctionBody.<inlineCaches[]>");
                addField(JDUtil::GetWrappedField(functionBody.GetPolymorphicInlineCaches(), "inlineCaches"), "Js::FunctionBody.{PolymorphicInlineCaches[]}");

                JDRemoteTyped polymorphicInlineCachesHead = functionBody.GetPolymorphicInlineCachesHead();
                
                char const * polymorphicInlineCachesTypeName = nullptr;
                while (polymorphicInlineCachesHead.GetPtr())
                {
                    // polymorphicInlineCache has a vtable, but it is a leaf, so if we have false reference to it, 
                    // the data that it points to may be invalid. So better to override the vtable inferred info here.

                    if (!polymorphicInlineCachesTypeName)
                    {
                        // Get the type name from CastWithVtable so it will be the same as other vtable inferred typename
                        JDRemoteTyped::FromPtrWithVtable(polymorphicInlineCachesHead.GetPtr(), &polymorphicInlineCachesTypeName);
                    }

                    setAddressData(polymorphicInlineCachesTypeName, polymorphicInlineCachesHead, true, javascriptLibrary, false);
                    polymorphicInlineCachesHead = JDUtil::GetWrappedField(polymorphicInlineCachesHead, "next");
                }

                addField(functionBody.GetConstTable(), "Js::FunctionBody.m_constTable");
                addField(functionBody.GetCacheIdToPropertyIdMap(), "Js::FunctionBody.cacheIdToPropertyIdMap");
                addField(functionBody.GetInlineCacheTypes(), "[DBG] Js::FunctionBody.m_inlineCacheTypes");
                addField(functionBody.GetReferencedPropertyIdMap(), "Js::FunctionBody.referencedPropertyIdMap");
                addField(functionBody.GetLiteralRegexes(), "Js::FunctionBody.literalRegexes");                
                addField(functionBody.GetObjectLiteralTypes(), "Js::FunctionBody.objLiteralTypes[]");

                RemoteBaseDictionary boundPropertyRecords = functionBody.GetBoundPropertyRecords();
                AddDictionaryField(boundPropertyRecords, "Js::FunctionBody.{BoundPropertyRecordsDictionary}");

                JDRemoteTyped sourceInfo = functionBody.GetSourceInfo();
                addField(sourceInfo.Field("pSpanSequence"), "Js::FunctionBody.sourceInfo.pSpanSequence");
                JDRemoteTyped auxStatementData = JDUtil::GetWrappedField(sourceInfo, "m_auxStatementData");
                if (addField(auxStatementData, "Js::FunctionBody::AuxStatementData"))
                {
                    JDRemoteTyped statementAdjustmentRecords = auxStatementData.Field("m_statementAdjustmentRecords");
                    if (addField(statementAdjustmentRecords, "Js::FunctionBody::StatementAdjustmentRecordList", true))
                    {
                        addField(statementAdjustmentRecords.Field("buffer"), "Js::FunctionBody::StatementAdjustmentRecordList.buffer");
                    }
                    JDRemoteTyped crossFrameBlockEntryExisRecords = auxStatementData.Field("m_crossFrameBlockEntryExisRecords");
                    if (addField(crossFrameBlockEntryExisRecords, "Js::FunctionBody::CrossFrameEntryExitRecordList", true))
                    {
                        addField(crossFrameBlockEntryExisRecords.Field("buffer"), "Js::FunctionBody::CrossFrameEntryExitRecordList.buffer");
                    }
                }
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

                
                JDRemoteTyped propertyIdOnRegSlotsContainer = functionBody.GetPropertyIdOnRegSlotsContainer();
                if (addField(propertyIdOnRegSlotsContainer, "Js::FunctionBody.propertyIdOnRegSlotsContainer"))
                {
                    addField(propertyIdOnRegSlotsContainer.Field("propertyIdsForRegSlots"), "Js::FunctionBody.propertyIdOnRegSlotsContainer.propertyIdsForRegSlots");
                    addField(propertyIdOnRegSlotsContainer.Field("propertyIdsForFormalArgs"), "Js::FunctionBody.propertyIdOnRegSlotsContainer.propertyIdsForFormalArgs");
                }
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
                    ULONG64 successorTypePtr = successorTypeWeakRef.Field("strongRef").GetPtr();
                    if (successorTypePtr != 0)
                    {
                        addDynamicTypeField(JDRemoteTyped::FromPtrWithType(successorTypePtr, "Js::DynamicType"));
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::PathTypeHandler *") == 0)
            {
                addPathTypeBase(remoteTyped, "Js::PathTypeHandler.typePath");
                JDRemoteTyped propertySuccessors = remoteTyped.Field("propertySuccessors");
                // The propertySuccessor dictionary has a vtable, but we are overrideing it to give it a better name and associate it with a library
                if (addField(propertySuccessors, "Js::PathTypeHandler.propertySuccessors", true))
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
                                ULONG64 successorTypePtr = successorTypeWeakRef.Field("strongRef").GetPtr();
                                if (successorTypePtr != 0)
                                {
                                    addDynamicTypeField(JDRemoteTyped::FromPtrWithType(successorTypePtr, "Js::DynamicType"));
                                }
                            }
                        }
                    }
                }
            }
            else if (STR_START_WITH(simpleTypeName, "Js::SimpleDictionaryTypeHandlerBase") 
                || STR_START_WITH(simpleTypeName, "Js::SimpleDictionaryUnorderedTypeHandler"))
            {
                addField(remoteTyped.Field("singletonInstance"), "RecyclerWeakReference<Js::DynamicObject>");
                RemoteBaseDictionary propertyMap = remoteTyped.Field("propertyMap");
                AddDictionaryField(propertyMap, "Js::SimpleDictionaryTypeHandlerBase<>.{PropertyDescriptorMap}");
            }
            else if (STR_START_WITH(simpleTypeName, "Js::DictionaryTypeHandlerBase"))
            {
                addField(remoteTyped.Field("singletonInstance"), "RecyclerWeakReference<Js::DynamicObject>");
                RemoteBaseDictionary propertyMap = remoteTyped.Field("propertyMap");
                AddDictionaryField(propertyMap, "Js::DictionaryTypeHandlerBase<>.{PropertyDescriptorMap}");
            }
            else if (strcmp(simpleTypeName, "Js::Utf8SourceInfo *") == 0)
            {
                RemoteUtf8SourceInfo utf8SourceInfo(remoteTyped);
                if (!GetExtension()->IsJScript9())
                {
                    JDRemoteTyped lineOffsetCache = utf8SourceInfo.GetLineOffsetCache();
                    if (addField(lineOffsetCache, "Js::Utf8SourceInfo.m_lineOffsetCache"))
                    {
                        JDRemoteTyped lineOffsetCacheList = lineOffsetCache.Field("lineOffsetCacheList");
                        if (addField(lineOffsetCacheList, "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList", true))
                        {
                            addField(lineOffsetCacheList.Field("buffer"), "Js::Utf8SourceInfo.m_lineOffsetCache.lineOffsetCacheList.buffer");
                        }
                    }
                    RemoteBaseDictionary deferredFunctionsDictionary = utf8SourceInfo.GetDeferredFunctionsDictionary();
                    AddDictionaryField(deferredFunctionsDictionary, "Js::Utf8SourceInfo.{DeferredFunctionsDictionary}");
                }
                RemoteBaseDictionary functionBodyDictionary = utf8SourceInfo.GetFunctionBodyDictionary();
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
                if (ENUM_EQUAL(program.Field("tag").GetSimpleValue(), InstructionsTag)
                    || ENUM_EQUAL(program.Field("tag").GetSimpleValue(), BOIInstructionsTag))
                {
                    JDRemoteTyped repInsts = program.Field("rep").Field("insts");
                    addField(repInsts.Field("insts"), "UnifiedRegex::Program::Instructions.insts");
                    addField(repInsts.Field("litbuf"), "UnifiedRegex::Program::Instructions.litbuf");
                    JDRemoteTyped scannerInfoArray = repInsts.Field("scannersForSyncToLiterals");
                    if (addField(scannerInfoArray, "UnifiedRegex::ScannerInfo *[]"))
                    {
                        int scannerInfoArraySize = ExtRemoteTyped("UnifiedRegex::ScannersMixin::MaxNumSyncLiterals").GetLong();
                        for (int i = 0; i < scannerInfoArraySize; i++)
                        {
                            addField(scannerInfoArray.ArrayElement(i), "UnifiedRegex::ScannerInfo");
                        }
                    }
                }
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptPromise *") == 0)
            {
                auto addPromiseReactionList = [&](JDRemoteTyped reactionList)
                {                    
                    if (addField(reactionList, "Js::JavascriptPromiseReactionList", true))
                    {
                        int count = reactionList.Field("count").GetLong();
                        JDRemoteTyped buffer = reactionList.Field("buffer");
                        addField(buffer, "Js::JavascriptPromiseReactionList.buffer");
                        for (int i = 0; i < count; i++)
                        {
                            JDRemoteTyped reaction = buffer.ArrayElement(i);
                            addField(reaction, "Js::JavascriptPromiseReaction", true);
                            addField(reaction.Field("capabilities"), "Js::JavascriptPromiseCapability", true);
                        }
                    }
                };
                addPromiseReactionList(remoteTyped.Field("resolveReactions"));
                addPromiseReactionList(remoteTyped.Field("rejectReactions"));
            }
            else if (strcmp(simpleTypeName, "Js::JavascriptLibrary *") == 0)
            { 
                auto addStaticTypeField = [&](JDRemoteTyped type)
                {
                    addField(type, "Js::StaticType");
                    addField(type.Field("propertyCache"), "Js::StaticType.propertyCache", false, javascriptLibrary);
                };

                if (remoteTyped.HasField("stringCache"))
                {
                    // From RS4
                    addStaticTypeField(remoteTyped.Field("stringCache").Field("stringTypeStatic"));
                }
                else
                {
                    addStaticTypeField(remoteTyped.Field("stringTypeStatic"));
                }
                
                addStaticTypeField(remoteTyped.Field("booleanTypeStatic"));
                addStaticTypeField(remoteTyped.Field("variantDateType"));
                addStaticTypeField(remoteTyped.Field("symbolTypeStatic"));
                addStaticTypeField(remoteTyped.Field("enumeratorType"));
                addStaticTypeField(remoteTyped.Field("numberTypeStatic"));
                addStaticTypeField(remoteTyped.Field("int64NumberTypeStatic"));
                addStaticTypeField(remoteTyped.Field("uint64NumberTypeStatic"));
                addStaticTypeField(remoteTyped.Field("withType"));
                addStaticTypeField(remoteTyped.Field("throwErrorObjectType"));

                auto tryAddDynamicTypeField = [&](char const * fieldName)
                {
                    if (remoteTyped.HasField(fieldName))
                    {
                        JDRemoteTyped field = remoteTyped.Field(fieldName);
                        if (field.GetPtr() != 0)
                        {
                            addDynamicTypeField(field);
                        }
                    }
                };
                tryAddDynamicTypeField("generatorConstructorPrototypeObjectType");
                tryAddDynamicTypeField("constructorPrototypeObjectType");
                tryAddDynamicTypeField("heapArgumentsType");
                tryAddDynamicTypeField("activationObjectType");
                tryAddDynamicTypeField("arrayType");
                tryAddDynamicTypeField("nativeIntArrayType");
                tryAddDynamicTypeField("copyOnAccessNativeIntArrayType");
                tryAddDynamicTypeField("nativeFloatArrayType");
                tryAddDynamicTypeField("arrayBufferType");
                tryAddDynamicTypeField("sharedArrayBufferType");
                tryAddDynamicTypeField("dataViewType");
                tryAddDynamicTypeField("int8ArrayType");
                tryAddDynamicTypeField("uint8ArrayType");
                tryAddDynamicTypeField("uint8ClampedArrayType");
                tryAddDynamicTypeField("int16ArrayType");
                tryAddDynamicTypeField("uint16ArrayType");
                tryAddDynamicTypeField("int32ArrayType");
                tryAddDynamicTypeField("uint32ArrayType");
                tryAddDynamicTypeField("float32ArrayType");
                tryAddDynamicTypeField("float64ArrayType");
                tryAddDynamicTypeField("int64ArrayType");
                tryAddDynamicTypeField("uint64ArrayType");
                tryAddDynamicTypeField("boolArrayType");
                tryAddDynamicTypeField("charArrayType");                
                tryAddDynamicTypeField("booleanTypeDynamic");
                tryAddDynamicTypeField("dateType");
                tryAddDynamicTypeField("symbolTypeDynamic");
                tryAddDynamicTypeField("iteratorResultType");
                tryAddDynamicTypeField("arrayIteratorType");
                tryAddDynamicTypeField("mapIteratorType");
                tryAddDynamicTypeField("setIteratorType");
                tryAddDynamicTypeField("stringIteratorType");
                tryAddDynamicTypeField("promiseType");
                tryAddDynamicTypeField("listIteratorType");

                tryAddDynamicTypeField("externalFunctionWithDeferredPrototypeType");
                tryAddDynamicTypeField("wrappedFunctionWithDeferredPrototypeType");
                tryAddDynamicTypeField("stdCallFunctionWithDeferredPrototypeType");
                tryAddDynamicTypeField("idMappedFunctionWithPrototypeType");
                tryAddDynamicTypeField("externalConstructorFunctionWithDeferredPrototypeType");
                tryAddDynamicTypeField("defaultExternalConstructorFunctionWithDeferredPrototypeType");
                tryAddDynamicTypeField("boundFunctionType");
                tryAddDynamicTypeField("regexConstructorType");
                tryAddDynamicTypeField("crossSiteDeferredPrototypeFunctionType");
                tryAddDynamicTypeField("crossSiteIdMappedFunctionWithPrototypeType");
                tryAddDynamicTypeField("crossSiteExternalConstructFunctionWithPrototypeType");
                tryAddDynamicTypeField("errorType");
                tryAddDynamicTypeField("evalErrorType");
                tryAddDynamicTypeField("rangeErrorType");
                tryAddDynamicTypeField("referenceErrorType");
                tryAddDynamicTypeField("syntaxErrorType");
                tryAddDynamicTypeField("typeErrorType");
                tryAddDynamicTypeField("uriErrorType");
                tryAddDynamicTypeField("webAssemblyCompileErrorType");
                tryAddDynamicTypeField("webAssemblyRuntimeErrorType");
                tryAddDynamicTypeField("webAssemblyLinkErrorType");
                tryAddDynamicTypeField("webAssemblyModuleType");
                tryAddDynamicTypeField("webAssemblyInstanceType");
                tryAddDynamicTypeField("webAssemblyMemoryType");
                tryAddDynamicTypeField("webAssemblyTableType");

                tryAddDynamicTypeField("numberTypeDynamic");
                tryAddDynamicTypeField("regexPrototypeType");
                tryAddDynamicTypeField("regexType");
                tryAddDynamicTypeField("regexResultType");
                tryAddDynamicTypeField("stringTypeDynamic");
                tryAddDynamicTypeField("mapType");
                tryAddDynamicTypeField("setType");
                tryAddDynamicTypeField("weakMapType");
                tryAddDynamicTypeField("weakSetType");
                tryAddDynamicTypeField("proxyType");
                tryAddDynamicTypeField("SpreadArgumentType");
                tryAddDynamicTypeField("moduleNamespaceType");

                JDRemoteTyped list = remoteTyped.Field("typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain");
                if (addField(list, "Js::JavascriptLibrary.typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain", true))
                {
                    addField(list.Field("buffer"), "Js::JavascriptLibrary.typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain.buffer");
                }

                // Added in commit 0f84cdec8bcff4651d364aad2160d97cce379ed0 for RS2
                if (remoteTyped.HasField("jsrtExternalTypesCache"))
                {
                    RemoteBaseDictionary dictionary = remoteTyped.Field("jsrtExternalTypesCache");
                    AddDictionaryField(dictionary, "Js::JavascriptLibrary.jsrtExternalTypesCache");
                }

                RemoteBaseDictionary propertyStringMap = remoteTyped.Field("propertyStringMap");
                AddDictionaryField(propertyStringMap, "Js::JavascriptLibrary.propertyStringMap");
                propertyStringMap.ForEachValue([&](JDRemoteTyped value)
                {
                    addField(value, "RecyclerWeakReference<Js::PropertyString>");
                    return false;
                });
                JDRemoteTyped bindRefChunk = remoteTyped.Field("bindRefChunkBegin");
                do
                {
                    addField(bindRefChunk, "Js::JavascriptLibrary.{bindRefChunk}");
                    bindRefChunk = *JDRemoteTyped("(void **)@$extin", bindRefChunk.GetPtr() + remoteThreadContext.GetRecycler().GetObjectGranularity() - g_Ext->m_PtrSize);
                }
                while (bindRefChunk.GetPtr() != 0);
                addField(remoteTyped.Field("rootPath"), "Js::JavascriptLibrary.rootPath");
                if (remoteTyped.HasField("referencedPropertyRecords"))
                {
                    RemoteBaseDictionary referencedPropertyRecords = remoteTyped.Field("referencedPropertyRecords");
                    AddDictionaryField(referencedPropertyRecords, "Js::JavascriptLibrary.referencedPropertyRecords");
                }
                if (remoteTyped.HasField("cache"))
                {
                    auto addEvalCacheDictionary = [&](JDRemoteTyped evalCacheDictionary)
                    {
                        if (addField(evalCacheDictionary, "Js::EvalCacheDictionary"))
                        {
                            JDRemoteTyped dictionary = evalCacheDictionary.Field("dictionary");
                            addField(dictionary, "Js::EvalCacheTopLevelDictionary");
                            RemoteBaseDictionary cacheStore = dictionary.Field("cacheStore");
                            AddDictionaryField(cacheStore, "Js::EvalCacheTopLevelDictionary.cacheStore");
                            cacheStore.ForEachEntry([&](JDRemoteTyped entry)
                            {
                                JDRemoteTyped evalString = entry.Field("key");
                                addField(evalString.Field("str").Field("string"), "Js::EvalMapString");
                                JDRemoteTyped record = entry.Field("value");
                                if (!record.Field("singleValue").GetStdBool())
                                {
                                    RemoteBaseDictionary secondLevelDictionary = record.Field("nestedMap");
                                    AddDictionaryField(secondLevelDictionary, "Js::SecondLevelEvalCache");
                                }
                                return false;
                            });
                        }
                    };
                    JDRemoteTyped cache = remoteTyped.Field("cache");
                    addEvalCacheDictionary(cache.Field("evalCacheDictionary"));
                    addEvalCacheDictionary(cache.Field("indirectEvalCacheDictionary"));                   
                    addField(cache.Field("newFunctionCache"), "Js::JavascriptLibrary.Cache.newFunctionCache");
                    addField(cache.Field("dynamicRegexMap"), "Js::JavascriptLibrary.Cache.dynamicRegexMap");

                    auto addSourceContextField = [&](JDRemoteTyped sourceContextInfo)
                    {
                        addField(sourceContextInfo, "SourceContextInfo");
                        addField(sourceContextInfo.Field("sourceDynamicProfileManager"), "SourceDynamicProfileManager");
                        return false;
                    };

                    RemoteBaseDictionary sourceContextInfoMap = cache.Field("sourceContextInfoMap");
                    AddDictionaryField(sourceContextInfoMap, "Js::JavascriptLibrary.Cache.sourceContextInfoMap");
                    if (sourceContextInfoMap.GetExtRemoteTyped().GetPtr() != 0)
                    {
                        sourceContextInfoMap.ForEachValue(addSourceContextField);
                    }
                    RemoteBaseDictionary dynamicSourceContextInfoMap = cache.Field("dynamicSourceContextInfoMap");
                    AddDictionaryField(dynamicSourceContextInfoMap, "Js::JavascriptLibrary.Cache.dynamicSourceContextInfoMap");
                    if (dynamicSourceContextInfoMap.GetExtRemoteTyped().GetPtr() != 0)
                    {
                        dynamicSourceContextInfoMap.ForEachValue(addSourceContextField);
                    }
                    addField(cache.Field("noContextSourceContextInfo"), "Js::JavascriptLibrary.Cache.noContextSourceContextInfo");
                    addField(cache.Field("noContextGlobalSourceInfo"), "Js::JavascriptLibrary.Cache.noContextGlobalSourceInfo");

                }
            }
            else if (strcmp(simpleTypeName, "HostDispatch *") == 0)
            {
                JDRemoteTyped refCountedHostVariant = remoteTyped.Field("refCountedHostVariant");

                // RefCountedHostVariant vtable is ICF with others, force it from HostDispatch which will also assign the library as well
                addField(refCountedHostVariant, "RefCountedHostVariant", true);
                addField(refCountedHostVariant.Field("hostVariant"), "HostVariant", true);
            }
            else if (strcmp(simpleTypeName, "Js::CaseInvariantPropertyListWithHashCode *") == 0)
            {
                
                int count = remoteTyped.Field("count").GetLong();
                JDRemoteTyped buffer = remoteTyped.Field("buffer");
                addField(buffer, "Js::CaseInvariantPropertyListWithHashCode.buffer");
                for (int i = 0; i < count; i++)
                {
                    addField(buffer.ArrayElement(i), "RecyclerWeakReference<Js::PropertyRecord>");
                }
            }
            else
            {
                noScanFieldVtable.insert(typeName);
            }
        }
#undef IsTypeOrCrossSite
    });

    autoError.Clear();
    numNodes = 0;
    std::stack<RecyclerObjectGraph::GraphImplNodeType*> updated;
    this->MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        if (node->HasTypeInfo())
        {
            return;
        }

        numNodes++;
        if (numNodes % 10000 == 0)
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rPropagating objects for type info %s- %11d/%11d", infer? "" : "without infer", numNodes, this->GetNodeCount());
        }

        bool setInfo = false;
        if (node->IsRoot())
        {
            setNodeData("<Root>", "<Root>", node, false, true, 0);
            setInfo = true;
        }

        node->MapPredecessors([&](RecyclerObjectGraph::GraphImplNodeType* pred)
        {
            if (!pred->HasTypeInfo())
            {
                return false;
            }

            setNodeData(pred->GetTypeName(), pred->GetTypeNameOrField(), node, pred->HasVtable(), true, pred->GetAssociatedJavascriptLibrary());
            setInfo = true;
            return true;
        });

        if (setInfo)
        {
            updated.push(node);
        }
    });
    
    while (!updated.empty())
    {
        RecyclerObjectGraph::GraphImplNodeType* node = updated.top();
        updated.pop();
        node->MapAllSuccessors([&](RecyclerObjectGraph::GraphImplNodeType* succ)
        {
            if (succ->HasTypeInfo())
            {
                return;
            }

            setNodeData(node->GetTypeName(), node->GetTypeNameOrField(), succ, node->HasVtable(), true, node->GetAssociatedJavascriptLibrary());
            updated.push(succ);
        });
    }

    m_hasTypeName = true;
    m_trident = trident;
    m_hasTypeNameAndFields = infer;
    g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL,
        "\rObject graph type info completed %s- elapsed time: %us                                                    \n",
        infer? "" : "without infer ", (ULONG)(_time64(nullptr) - start));
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
                g_Ext->Out("Finding path from 0x%P\n", rootAddress);
                std::vector<ULONG64> path = _objectGraph.FindPath(rootAddress, address);

                if (shortestPath.size() == 0 || shortestPath.size() > path.size())
                {
                    shortestPath = path;
                    if (shortestPath.size() != 0) {
                        g_Ext->Out("Shortest path so far is from 0x%P\n", rootAddress);
                        pathsFound++;
                    }
                }
            });
        }

        g_Ext->Out("Shortest path has %d nodes\n", shortestPath.size());
        for (auto it = shortestPath.begin(); it != shortestPath.end(); it++)
        {
            g_Ext->Out("0x%P\n", (*it));
        }
    }
    else
    {
        g_Ext->Out("Object not found\n");
    }
}
#endif

void RecyclerObjectGraph::MarkObject(ConstructData& constructData,  ULONG64 address, Set<GraphImplNodeType *> * successors, RootType rootType)
{
    if (address == NULL ||
        !constructData.recycler.IsAlignedAddress(address))
    {
        return;
    }

    RemoteHeapBlock *remoteHeapBlock = constructData.hbm.FindHeapBlock(address);
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
        constructData.markStack.push(entry);
    }
}

void RecyclerObjectGraph::ScanBytes(ConstructData& constructData, RemoteHeapBlock * remoteHeapBlock, GraphImplNodeType * node)
{
    ULONG64 objectAddress = node->Key();
    Assert(objectAddress != 0);

    uint objectSize = node->GetObjectSize();
    RemoteHeapBlock::AutoDebuggeeMemory object(remoteHeapBlock, objectAddress, objectSize);   
    char* current = (char *)object;
    char* end = current + objectSize;
    ulong ptrSize = g_Ext->m_PtrSize;
    Set<GraphImplNodeType *> successors;
    while (current < end)
    {
        ULONG64 value = (ptrSize == 8) ? *((ULONG64*)current) : *((ULONG32*)current);
        if (value != objectAddress)
        {
            MarkObject(constructData, value, &successors, RootType::RootTypeNone);
        }
        current += ptrSize;
    }

    this->_objectGraph.AddEdges(node, successors);
}

