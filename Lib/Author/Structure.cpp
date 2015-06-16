//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    static const int ItemParameterLimit = 20;
    static const int LiteralMemberNameLimit = 3;

    namespace Names
    {
        const wchar_t auxValue[] = L"auxValue";
    }

    static int nextKey = 0x1BCABCAB;

    const AuthorStructureNodeKind asnkRoot = (AuthorStructureNodeKind)-1;

    class StructureNode;
    class StructureResult;

    ArenaAllocator *GetAllocator(StructureResult *result);

    typedef JsUtil::List<StructureNode *, ArenaAllocator> StructureNodeChildren;

    class StructureNode
    {
    public:
        StructureNode(StructureResult *result, Js::InternalString *itemName, Js::InternalString *containerName, Js::InternalString *glyph, AuthorStructureNodeKind kind, Js::InternalString *customKind,
            charcount_t min, charcount_t lim) :
            key(nextKey++ ? nextKey : nextKey++), // Skip 0.
            itemName(itemName),
            containerName(containerName),
            glyph(glyph),
            kind(kind),
            customKind(customKind),
            min(min),
            lim(lim),
            children(GetAllocator(result), 4) { }

        static StructureNode* New(StructureResult *result, IdentPtr name, AuthorStructureNodeKind kind, ParseNode *node);
        static StructureNode* New(StructureResult *result, Js::InternalString *itemName, IdentPtr containerName, AuthorStructureNodeKind kind, ParseNode *node);
        static StructureNode* New(StructureResult *result, Js::InternalString *name, AuthorStructureNodeKind kind, ParseNode *node);
        static StructureNode* New(StructureResult *result, Js::InternalString *itemName, Js::InternalString *containerName, AuthorStructureNodeKind kind, ParseNode *node);
        static StructureNode* New(StructureResult *result, Js::InternalString *name, Js::InternalString *customKind, ParseNode *node);
        static StructureNode* New(StructureResult *result, AuthorStructureNodeKind kind, ParseNode *node) { return New(result, (Js::InternalString *)nullptr, kind, node); }
        static StructureNode* New(StructureResult *result, Js::InternalString *name, AuthorStructureNodeKind kind, charcount_t min, charcount_t lim);
        static StructureNode* New(StructureResult *result, Js::InternalString *name, Js::InternalString *customKind, charcount_t min, charcount_t lim);
        static StructureNode* New(StructureResult *result, Js::InternalString *itemName, Js::InternalString *containerName, Js::InternalString *glyph, AuthorStructureNodeKind kind, Js::InternalString *customKind, charcount_t min, charcount_t lim);

        int key;
        Js::InternalString *itemName;
        Js::InternalString *containerName;
        Js::InternalString *glyph;
        AuthorStructureNodeKind kind;
        Js::InternalString *customKind;
        charcount_t min;
        charcount_t lim;
        StructureNodeChildren children;
        StructureNode *container;

        BOOL HasChildren() { return children.Count() > 0; }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        LPCWSTR GetAuthorStructureNodeKindName()
        {
            switch (kind)
            {
            case asnkCustom: return L"asnkCustom";
            case asnkGlobal: return L"asnkGlobal";
            case asnkObjectLiteral: return L"asnkObjectLiteral";
            case asnkFunction: return L"asnkFunction";
            case asnkVariable: return L"asnkVariable";
            case asnkField: return L"asnkField";
            case asnkClass: return L"asnkClass";
            case asnkNamespace: return L"asnkNamespace";
            case asnkModule: return L"asnkModule";
            case asnkRegion: return L"asnkRegion";
            case asnkProperty: return L"asnkProperty";
            }
            return L"unknown";
        }

        void Print() 
        {
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"{itemName:%ls, containerName:%ls, kind:%ls}\n", itemName ? itemName->GetBuffer() : L"", containerName ? containerName->GetBuffer() : L"", GetAuthorStructureNodeKindName());
            for (int i = 0; i < children.Count(); i++)
            {
                children.Item(i)->Print();
            }
        }
#endif

        BOOL IsContainer()
        {
            // The node is a container if it has children or is a container kind such as a class or global.
            if (HasChildren()) return true;

            switch (kind)
            {
            case asnkGlobal:
            case asnkObjectLiteral:
            case asnkClass:
            case asnkNamespace:
            case asnkModule:
                return true;
            }
            return false;
        }

        void Append(StructureNode *node)
        {
            children.Add(node);

            // If we are adding a non-auxiliary child and we don't have a container name already, adopt the item name.
            if (node->kind != asnkRegion && !this->containerName && this->itemName)
            {
                this->containerName = this->itemName;
            }
        }

        void AddCoveringRegion(StructureResult* structureResult, charcount_t min, charcount_t lim)
        {
            if (!this->children.Any([min, lim](StructureNode* child) { return child->min <= min && child->lim >= lim; }))
            {
                auto region = StructureNode::New(structureResult, nullptr, asnkRegion, min, lim);
                this->Append(region);
            }
        }

        void AppendIfDistinct(StructureNode* newChild)
        {
            if (!this->children.Any([newChild](StructureNode* child) { return child->min == newChild->min && child->lim == newChild->lim && newChild->kind == child->kind; }))
            {
                this->Append(newChild);
            }
        }

        void AppendDistinctChildren(const StructureNodeChildren& childrenToMerge)
        {
            childrenToMerge.Map([&](int index, StructureNode* childToMerge) {
                this->AppendIfDistinct(childToMerge);
            });
        }

        template <typename THandler>
        void TraverseAll(THandler handler)
        {
            children.Map([&](int index, StructureNode *node) -> void
            {
                // Traversed nodes always have the correct container set.
                node->container = this->kind != asnkRoot ? this : nullptr;
                handler(node);
                if (node->HasChildren())
                    node->TraverseAll(handler);
            });
        }

        template <typename THandler>
        bool TraverseAllUntil(THandler handler, int keyToMatch)
        {
            return children.MapUntil([&](int index, StructureNode *node) -> bool
            {
                if (node->key == keyToMatch)
                {
                    handler(node);
                    return true;
                }
                return node->HasChildren() ? node->TraverseAllUntil(handler, keyToMatch) : false;
            });
        }

        template <typename TFilter, typename THandler>
        void TraverseAllMatching(TFilter filter, THandler handler)
        {
            children.Map([&](int index, StructureNode *node) -> void
            {
                // Traversed nodes always have the correct container set.
                node->container = this->kind != asnkRoot ? this : nullptr;
                if (filter(node))
                {
                    handler(node);
                    if (node->HasChildren())
                        node->TraverseAllMatching(filter, handler);
                }
            });
        }

        template <typename THandler>
        void TraverseChildren(THandler handler)
        {
            children.Map([&](int index, StructureNode *node) -> void
            {
                // Traversed nodes always have the correct container set.
                node->container = this->kind != asnkRoot ? this : nullptr;
                handler(node);
            });
        }
    };

    class StructureNodeSet : public SimpleComObjectWithAlloc<IAuthorStructureNodeSet>
    {
    protected:
        JsUtil::List<StructureNode *, ArenaAllocator> nodes;
    public:
        StructureNodeSet(StructureResult *result);

        virtual void Initialize(StructureResult *result) = 0;

        STDMETHOD(get_Count)(int *result)
        {
            STDMETHOD_PREFIX;

            ValidateArg(result);

            *result = nodes.Count();

            STDMETHOD_POSTFIX;
        }

        STDMETHOD(GetItems)(int startIndex, int count, AuthorStructureNode *nodes)
        {
            STDMETHOD_PREFIX;

            auto endIndex = startIndex + count;

            ValidateArg(startIndex >= 0);
            ValidateArg(count >= 0);
            ValidateArg(endIndex <= this->nodes.Count());

            for (int i = 0; i < count; i++)
            {
                auto outNode = &nodes[i];
                auto structureNode = this->nodes.Item(startIndex + i);

                outNode->key = structureNode->key;
                outNode->container = structureNode->container ? structureNode->container->key : 0;
                outNode->kind = structureNode->kind;
                outNode->hasChildren = structureNode->HasChildren();
                outNode->itemName = structureNode->itemName ? AllocBSTR(structureNode->itemName) : nullptr;
                outNode->containerName = structureNode->containerName ? AllocBSTR(structureNode->containerName) : nullptr;
                outNode->glyph = structureNode->glyph ? AllocBSTR(structureNode->glyph) : nullptr;
                outNode->customKind = structureNode->customKind ? AllocBSTR(structureNode->customKind) : nullptr;
                outNode->region.offset = structureNode->min;
                outNode->region.length = structureNode->lim - structureNode->min;
            }

            STDMETHOD_POSTFIX;
        }
    };

    TYPE_STATS(StructureNodeSet, L"StructureNodeSet")

        // The subsets returned by calls to IAuthorStructure
        // The dummy parameters simplify the template used to construct the instance. If all the classes have the same
        // arity then the template need only specify the parameters. Templates that call constructures that differ in arity 
        // are challenging.

    class CompleteStructureNodeSet : public StructureNodeSet
    {
    public:
        CompleteStructureNodeSet(StructureResult *result, int dummy) : StructureNodeSet(result) { }

        virtual void Initialize(StructureResult *result) override;
    };

    class ContainersStructureNodeSet : public StructureNodeSet
    {
    public:
        ContainersStructureNodeSet(StructureResult *result, int dummy) : StructureNodeSet(result) { }

        virtual void Initialize(StructureResult *result) override;
    };

    StructureNode *FindNode(StructureResult *result, int key);

    class ChildrenStructureNodeSet : public StructureNodeSet
    {
    private:
        StructureNode *node;

    public:
        ChildrenStructureNodeSet(StructureResult *result, int key) : StructureNodeSet(result), node(FindNode(result, key)) { }

        virtual void Initialize(StructureResult *result) override;
    };

    struct ResolvedItem; // forward declaration.
    typedef JsUtil::List<ResolvedItem*> ResolvedItemList;

    // A container of holding data while analyzing the script context
    struct ResolvedItem
    {
        Js::InternalString *name;
        AuthorStructureNodeKind type;
        Js::Var value;
        Js::Var auxValue;
        Js::Var instance;
        Js::Var glyph;
        ResolvedItemList * children;

        ResolvedItem(Js::InternalString * _name, AuthorStructureNodeKind _type, Js::Var _value, Js::Var _instance, Js::Var _glyph, ResolvedItemList * _children, Js::Var _auxValue)
            : name(_name),
            type(_type),
            value(_value),
            instance(_instance),
            glyph(_glyph),
            children(_children),
            auxValue(_auxValue)
        {
        }

        static ResolvedItem * GetResolvedItem(Recycler * recycler,
            ArenaAllocator *alloc,
            Js::Var _name,
            AuthorStructureNodeKind _type,
            Js::Var _value,
            Js::Var _instance,
            Js::Var _glyph,
            ResolvedItemList * _children,
            Js::Var _auxValue = nullptr)
        {
            ResolvedItem *item = nullptr;
            if (alloc)
            {
                Js::InternalString *name = nullptr;
                if (_name 
                    && Convert::FromVar(alloc, _name, name)
                    && name
                    && name->GetLength() > 0
                    && !InternalName(name->GetBuffer()))
                {
                    item = RecyclerNew(recycler, Authoring::ResolvedItem, name, _type, _value, _instance, _glyph, _children, _auxValue);
                }
            }
            return item;
        }
        const wchar_t* GetTypeString()
        {
            switch (type)
            {
            case asnkFunction: return L"method";
            case asnkClass: return L"class";
            case asnkField: return L"field";
            case asnkProperty: return L"property";
            case asnkNamespace: return L"namespace";
            default: return L"unknown";
            }
        }
        void Print()
        {
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"{name:%ls, type:%ls, hasChildren:%ls}\n", name->GetBuffer(), GetTypeString(), children ? L"yes" : L"no");
        }

    };


    class StructureResult : public SimpleComObjectWithAlloc<IAuthorStructure>
    {
    private:
        typedef JsUtil::BaseDictionary<int, StructureNode *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> NodeMap;
        FileAuthoring *fileAuthoring;
        StructureNode *root;
        Js::ScriptContext *scriptContext;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        int m_indentNum;
#endif

        ResolvedItemList *m_topItemList;
        AuthoringFileHandle *m_primaryFile;
        Js::PropertyId m_inContainerListPropertyId;
        Js::PropertyId m_defLocPropertyId;

        typedef JsUtil::List<Js::Var, ArenaAllocator> RecursionMarkList;
        RecursionMarkList *m_recursionMarkList;

        typedef JsUtil::List<Js::Var, ArenaAllocator> PropertyNameList;

        struct Span {
            charcount_t start;
            charcount_t end;
            Span() : start(0), end(0) { }
            Span(charcount_t startOffset, charcount_t endOffset) : start(startOffset), end(endOffset) { }
        };

        template<typename TKey, typename TValue>
        class MultiMap
        {
        public:
            typedef JsUtil::List<TValue, ArenaAllocator> ValueList;

        private:
            JsUtil::BaseDictionary<TKey, ValueList*, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> multiMap;
            ArenaAllocator* allocator;

        public:

            MultiMap(ArenaAllocator* alloc) : multiMap(alloc), allocator(alloc)
            {
            }

            void Add(TKey key, TValue value)
            {
                ValueList* list = GetOrAddListForKey(key);
                list->Add(value);
            }

            ValueList* GetOrAddListForKey(TKey key)
            {
                ValueList* list;
                if (!multiMap.TryGetValue(key, &list))
                {
                    list = Anew(allocator, ValueList, allocator);
                    multiMap.Add(key, list);
                }

                return list;
            }

            bool TryGet(TKey key, ValueList** values)
            {
                return multiMap.TryGetValue(key, values);
            }

        };

        typedef int FunctionOffset;

        typedef MultiMap<FunctionOffset, Span>::ValueList SpanList;
        MultiMap<FunctionOffset, Span> functionSpanMap;

        typedef MultiMap<FunctionOffset, StructureNode*>::ValueList DynamicFunctionChildNodeList;
        MultiMap<FunctionOffset, StructureNode*> dynamicFunctionChildrenMap;

        // Template to construct the requested set instance. The set instance is a filtered subset of the nodes lineralized 
        // into a list.
        template <typename TSetType>
        HRESULT CreateResultSet(IAuthorStructureNodeSet **result, int param = 0)
        {
            TSetType *set = nullptr;

            STDMETHOD_PREFIX;

            ValidateArg(result);

            *result = nullptr;

            set = new TSetType(this, param);
            set->Initialize(this);
            *result = set;
            set = nullptr;

            STDMETHOD_POSTFIX_CLEAN_START;

            if (set) ((IUnknown *)set)->Release();

            STDMETHOD_POSTFIX_CLEAN_END;
        }

        bool IsArgumentOf(ParseNode *node, ParseNode *fnc)
        {
            Assert(node && fnc);
            if (node->nop == knopVarDecl && fnc->nop == knopFncDecl)
            {
                for (auto arg = fnc->sxFnc.pnodeArgs; arg; arg = arg->sxVar.pnodeNext)
                {
                    if (arg == node)
                    {
                        return true;
                    }
                }
                if (fnc->sxFnc.pnodeRest == node)
                {
                    return true;
                }
            }
            return false;
        }


        bool WriteDottedName(TextBuffer &text, ParseNode *dottedName)
        {
        LTop:
            if (!dottedName) return false;
            switch (dottedName->nop)
            {
            case knopName:
                text.Add(dottedName->sxPid.pid);
                return true;
            case knopDot:
                if (!WriteDottedName(text, dottedName->sxBin.pnode1)) return false;
                text.Add('.');

                // The following is a manual tail-recursion optimization of
                // return WriteDottedName(text, dottedName->sxBin.pnode2); 
                dottedName = dottedName->sxBin.pnode2;
                goto LTop;
            }
            return false;
        }

        void AppendArguments(TextBuffer &buffer, ParseNode *pnodeFnc, int parameterLimit)
        {
            bool first = true;
            int count = 0;
            buffer.Add('(');

            // Returns true if we have hit the arg limit.
            auto appendArg = [&](ParseNode *arg) -> bool
            {
                // Limited to displaying parameterLimit arguments.
                if (count >= parameterLimit)
                {
                    if (!first) buffer.Add(L", ");
                    buffer.Add(L"...");
                    return true;
                }

                Assert(arg->nop == knopVarDecl);
                auto argName = arg->sxVar.pid;
                if (argName)
                {
                    if (!first) buffer.Add(L", ");
                    first = false;
                    buffer.Add(argName->Psz(), argName->Cch());
                }
                return false;
            };

            bool hitArgLimit = false;
            for (auto arg = pnodeFnc->sxFnc.pnodeArgs; arg != nullptr && !hitArgLimit; arg = arg->sxVar.pnodeNext, count++)
            {
                hitArgLimit = appendArg(arg);
            }
            if (!hitArgLimit && pnodeFnc->sxFnc.pnodeRest != nullptr)
            {
                appendArg(pnodeFnc->sxFnc.pnodeRest);
            }

            buffer.Add(')');
        }

        bool IsContainingAssignmentOf(ParseNode *node, ParseNode *fncDecl)
        {
            if (node)
            {
                switch (node->nop)
                {
                case knopAsg:
                    return node->sxBin.pnode2 == fncDecl;
                case knopConstDecl:
                case knopLetDecl:
                case knopVarDecl:
                    return node->sxVar.pnodeInit == fncDecl;
                }
            }

            return false;
        }

        bool TryAddAssignmentName(TextBuffer &buffer, ParseNode *node, ParseNode *containingAssignment)
        {
            if (IsContainingAssignmentOf(containingAssignment, node))
            {
                if (containingAssignment->nop == knopAsg)
                {
                    return WriteDottedName(buffer, containingAssignment->sxBin.pnode1);
                }
                else
                {
                    Assert(containingAssignment->nop == knopLetDecl || containingAssignment->nop == knopVarDecl || containingAssignment->nop == knopConstDecl);
                    buffer.Add(containingAssignment->sxVar.pid);
                    return true;
                }

            }

            return false;
        }

        Js::InternalString *TryGetLiteralName(ArenaAllocator *alloc, ParseNode *node, ParseNode *containingAssignment)
        {
            TextBuffer buffer(alloc);

            if (TryAddAssignmentName(buffer, node, containingAssignment))
                return buffer.ToInternalString(Alloc());

            return nullptr;
        }

        bool TryGetStructureAssignmentStartOffset(ParseNode* containingAssignment, charcount_t& offset)
        {
            Assert(containingAssignment);

            while (containingAssignment)
            {
                switch (containingAssignment->nop)
                {
                case knopName:
                    offset = containingAssignment->ichMin;
                    return true;
                case knopDot:
                {
                                auto leftNode = containingAssignment->sxBin.pnode1;

                                // if we wouldn't look down the left anymore then we use the right as start offset.
                                if (leftNode->nop == knopDot || leftNode->nop == knopName)
                                {
                                    containingAssignment = leftNode;
                                    break;
                                }

                                // The right node of a dot should never (legally) be another dot node,
                                // so end the search with the start offset of the right node.
                                offset = ActualMin(containingAssignment->sxBin.pnode2);
                                return true;
                }
                default:
                    // unimportant node, return the default start offset.
                    return false;
                }
            }

            return false;
        }

        charcount_t GetStartOffset(ParseNode* functionNode, ParseNode* containingAssignment)
        {
            Assert(functionNode && functionNode->nop == knopFncDecl);

            charcount_t functionStartOffset = ActualMin(functionNode);

            if (functionNode->sxFnc.hint)
            {
                if (containingAssignment)
                {
                    if (IsContainingAssignmentOf(containingAssignment, functionNode))
                    {
                        if (containingAssignment->nop == knopAsg)
                        {
                            charcount_t startOffset;
                            if (TryGetStructureAssignmentStartOffset(containingAssignment->sxBin.pnode1, /*offset*/startOffset))
                            {
                                functionStartOffset = startOffset;
                            }
                        }
                        else
                        {
                            Assert(containingAssignment->nop == knopLetDecl || containingAssignment->nop == knopVarDecl || containingAssignment->nop == knopConstDecl);
                            functionStartOffset = containingAssignment->ichMin;
                        }
                    }
                    else if (containingAssignment->nop == knopMember || containingAssignment->nop == knopMemberShort)
                    {
                        return containingAssignment->ichMin;
                    }
                }
            }

            return functionStartOffset;
        }

        bool TryGetLiteralAssignmentOffset(ParseNode* containingAssignment, ParseNode* objectLiteral, charcount_t& min)
        {
            Assert(objectLiteral);
            Assert(objectLiteral->nop == knopObject);

            if (containingAssignment &&
                IsContainingAssignmentOf(containingAssignment, objectLiteral))
            {
                if (containingAssignment->nop == knopAsg)
                {
                    if (TryGetStructureAssignmentStartOffset(containingAssignment->sxBin.pnode1, /*defaultStartOffset*/min))
                    {
                        return true;
                    }
                }
                else
                {
                    Assert(containingAssignment->nop == knopVarDecl || containingAssignment->nop == knopLetDecl || containingAssignment->nop == knopConstDecl);
                    min = containingAssignment->ichMin;
                    return true;
                }
            }

            return false;
        }

        Js::InternalString *TryGetAnonymousFunctionName(ArenaAllocator *alloc, ParseNode *pnodeFnc, ParseNode *containingAssignment)
        {
            Assert(pnodeFnc && pnodeFnc->nop == knopFncDecl);

            TextBuffer buffer(alloc);
            if (pnodeFnc->sxFnc.IsClassConstructor() && pnodeFnc->sxFnc.hint)
            {
                buffer.Add(pnodeFnc->sxFnc.hint);
            }
            else if (pnodeFnc->sxFnc.IsClassMember())
            {
                Assert(pnodeFnc->sxFnc.pid || pnodeFnc->sxFnc.hint);
                // We want to take the short name instead of fully qualified name ('method' instead of 'class.prototype.method')
                if (pnodeFnc->sxFnc.pid)
                {
                    buffer.Add(pnodeFnc->sxFnc.pid);
                }
                else
                {
                    buffer.Add(pnodeFnc->sxFnc.hint);
                }

                if (pnodeFnc->sxFnc.IsAccessor())
                {
                    // We don't need to add any arguments for the accessors
                    return buffer.ToInternalString(Alloc());
                }
            }
            else if (pnodeFnc->sxFnc.hint)
            {
                bool addHint = true;
                // If we can calculate the dotted name, do that.
                if (containingAssignment && containingAssignment->nop == knopAsg && IsContainingAssignmentOf(containingAssignment, pnodeFnc))
                {
                    addHint = !TryAddAssignmentName(buffer, pnodeFnc, containingAssignment);
                }

                if (addHint)
                {
                    buffer.Add(pnodeFnc->sxFnc.hint);
                }
            }
            else
            {
                // If the function has an explict name use that.
                auto name = pnodeFnc->sxFnc.pnodeNames;
                if (name && name->nop == knopVarDecl)
                {
                    buffer.Add(name->sxVar.pid);
                }
                else
                {
                    // If we cannot determine a name return null to the caller so they know we didn't find a name.
                    return nullptr;
                }
            }

            AppendArguments(buffer, pnodeFnc, ItemParameterLimit);
            return buffer.ToInternalString(Alloc());
        }

        Js::InternalString *AnonymousFunctionName(ArenaAllocator *alloc, ParseNode *pnodeFnc, ParseNode *containingAssignment)
        {
            auto result = TryGetAnonymousFunctionName(alloc, pnodeFnc, containingAssignment);
            if (!result)
            {
                TextBuffer buffer(alloc);
                buffer.Add(L"function ");
                AppendArguments(buffer, pnodeFnc, ItemParameterLimit);
                result = buffer.ToInternalString(Alloc());
            }

            return result;
        }

        Js::InternalString *GetClassName(ArenaAllocator *alloc, ParseNode *pnodeClass)
        {
            Assert(pnodeClass && pnodeClass->nop == knopClassDecl);

            TextBuffer buffer(alloc);
            buffer.Add(Parser::GetClassName(&pnodeClass->sxClass));
            Assert(buffer.Length() != 0);
            return buffer.ToInternalString(Alloc());
        }

        Js::InternalString *FunctionItemName(ArenaAllocator *alloc, ParseNode *pnodeFnc, IdentPtr name, Js::InternalString *nameString)
        {
            Assert(pnodeFnc && pnodeFnc->nop == knopFncDecl);

            // Build the display name
            TextBuffer buffer(alloc);

            if (!name && !nameString)
                buffer.Add(L"function ");
            else
            {
                if (name)
                    buffer.Add(name);
                else
                    buffer.Add(nameString);
            }
            AppendArguments(buffer, pnodeFnc, ItemParameterLimit);

            return buffer.ToInternalString(Alloc());
        }

        void GetFunctionBodyRange(ArenaAllocator *alloc, ParseNodeTree *parseTree, Js::ParseableFunctionInfo *body, charcount_t &min, charcount_t &lim)
        {
            min = body->StartInDocument();
            lim = min + body->LengthInChars();
        }

        Js::InternalString *FunctionItemName(ArenaAllocator *alloc, ParseNodeTree *parseTree, charcount_t location, Js::InternalString *nameString)
        {
            ParseNodeCursor cursor(alloc, parseTree);
            cursor.SeekToOffset(location);
            auto node = cursor.Current();
            if (node && node->nop == knopFncDecl && (node->sxFnc.pnodeNames || nameString))
            {
                auto name = node->sxFnc.pnodeNames;
                if (name && name->nop == knopVarDecl)
                {
                    return FunctionItemName(alloc, node, name->sxVar.pid, nameString);
                }
                else if (nameString)
                {
                    return FunctionItemName(alloc, node, nullptr, nameString);
                }
            }
            return nullptr;
        }

        static PageAllocator *GetPageAllocator(FileAuthoring *fileAuthoring) { return fileAuthoring->GetScriptContext()->GetThreadContext()->GetPageAllocator(); }

        class StaticScopeAnalyzer
        {
        private:
            struct ScopeDynamicInfo
            {
                bool isDynamicNode;
                bool isUnderDynamicNode;
                ScopeDynamicInfo(bool isDynamicNode, bool isUnderDynamicNode) :
                    isDynamicNode(isDynamicNode),
                    isUnderDynamicNode(isUnderDynamicNode)
                { }
            };

            struct ItemVisitScope
            {
                StructureNode   *structureNode;
                ParseNode       *parseNode;
                ParseNode       *containingAssignment;
                bool            isConditionalNode;
                ScopeDynamicInfo ScopeDynamicInformation;

                ItemVisitScope() : structureNode(nullptr), parseNode(nullptr), isConditionalNode(false), containingAssignment(nullptr), ScopeDynamicInformation(false, false) { }
                ItemVisitScope(StructureNode *structureNode, ParseNode *parseNode, bool isConditionalNode, bool isDynamicNode, bool isUnderDynamicNode) :
                    structureNode(structureNode),
                    parseNode(parseNode),
                    isConditionalNode(isConditionalNode),
                    ScopeDynamicInformation(isDynamicNode, isUnderDynamicNode),
                    containingAssignment(nullptr)
                { }
            };

            JsUtil::Stack<ItemVisitScope> scopeStack;

            int blockNestingCount;

            ArenaAllocator* alloc;
            StructureResult* structureResult;


            void SetContainingAssignment(ParseNode* assignmentNode, ItemVisitScope& currentAnalysisScope)
            {
                // Pop & Repush needed due to mutable struct
                ItemVisitScope currentScope = scopeStack.Pop();
                currentAnalysisScope.containingAssignment = currentScope.containingAssignment = assignmentNode;
                scopeStack.Push(currentScope);
            }

            StructureNode* AnalyzeClassDeclaration(ParseNode* node)
            {
                auto className = structureResult->GetClassName(alloc, node);
                return StructureNode::New(structureResult, className, asnkClass, node);
            }

            StructureNode* AnalyzeFunctionDeclaration(ParseNode* node, bool& isDynamicFunction)
            {
                // Function declarations are considered part of the structure implicitly.
                if (node->sxFnc.IsDeclaration())
                {
                    auto nameNode = node->sxFnc.pnodeNames;
                    if (nameNode && nameNode->nop == knopVarDecl)
                    {
                        auto functionName = structureResult->FunctionItemName(alloc, node, nameNode->sxVar.pid, nullptr);
                        return StructureNode::New(structureResult, functionName, nameNode->sxVar.pid, asnkFunction, node);
                    }
                    // else Ignore knopScope as they are most likely the result of error correction.
                }
                else
                {
                    // Ignore the function expression of something we detected as an immediately invoked function expression (IIFE).
                    auto current = scopeStack.Peek();
                    if (current.parseNode->nop == knopCall && current.parseNode->sxCall.pnodeTarget == node)
                    {
                        return nullptr;
                    }

                    charcount_t lim = ActualLim(node);
                    charcount_t min = ActualMin(node);

                    auto TryGetStartOffset = [this, node, min](charcount_t& startOffset) {
                        auto currentScope = scopeStack.Peek();

                        if (currentScope.ScopeDynamicInformation.isUnderDynamicNode)
                        {
                            return false;
                        }

                        startOffset = structureResult->GetStartOffset(node, currentScope.containingAssignment);

                        if (startOffset < min)
                        {
                            return true;
                        }

                        return false;
                    };

                    // Function expressions are only added to the structure if we can determine a name for them.
                    if (node->sxFnc.pnodeNames)
                    {
                        auto nameNode = node->sxFnc.pnodeNames;
                        Assert(nameNode && nameNode->nop == knopVarDecl);
                        auto functionName = structureResult->FunctionItemName(alloc, node, nameNode->sxVar.pid, nullptr);
                        auto newNode = StructureNode::New(structureResult, functionName, nameNode->sxVar.pid, asnkFunction, node);

                        charcount_t startOffset;
                        if (TryGetStartOffset(startOffset))
                        {
                            newNode->AddCoveringRegion(structureResult, startOffset, lim);
                        }

                        return newNode;
                    }
                    else
                    {
                        auto currentScope = scopeStack.Peek();
                        Js::InternalString * functionName = structureResult->TryGetAnonymousFunctionName(alloc, node, currentScope.containingAssignment);

                        // For the accessor we don't need to create an individual nodes for both getter and setter. If one of them is 
                        // added already we just need to add the covering region.
                        bool isAccessor = node->sxFnc.IsClassMember() && node->sxFnc.IsAccessor();
                        if (functionName && isAccessor)
                        {
                            StructureNode* primaryAccessorNode = nullptr;
                            currentScope.structureNode->children.Any([&](StructureNode* node) {
                                if (Js::InternalStringComparer::Equals(*functionName, *node->itemName))
                                {
                                    primaryAccessorNode = node;
                                    node->AddCoveringRegion(structureResult, min, lim);
                                    return true;
                                }
                                return false;
                            });

                            if (primaryAccessorNode != nullptr)
                            {
                                // We don't need to add new node, instead we should re-use the earlier one which was created and add that as scope.
                                AddStaticAnalysisNodeResult(
                                    currentScope,
                                    node,
                                    primaryAccessorNode,
                                    false/*isConditionalNode*/,
                                    true/*shouldPushScope*/,
                                    false/*isPossibleDynamicNode*/,
                                    false /*appendToParentNode*/);

                                return nullptr; // This way no new node will be appended
                            }
                        }

                        if (functionName)
                        {
                            auto newNode = StructureNode::New(structureResult, functionName, isAccessor ? asnkProperty : asnkFunction, min, lim);

                            charcount_t startOffset;
                            if (TryGetStartOffset(startOffset))
                            {
                                newNode->AddCoveringRegion(structureResult, startOffset, lim);
                                structureResult->AddKnownFunctionAlias(node, startOffset, lim);
                            }

                            return newNode;
                        }
                        else
                        {
                            // we need to observe children still, 
                            // because this node might be dynamic with static analyzable nodes under it
                            isDynamicFunction = true;
                        }
                    }
                }

                return nullptr;
            }

            StructureNode* AnalyzeObjectLiteral(ParseNode* node, bool& isDynamicObject, bool& isConditionalNode)
            {
                auto currentScope = scopeStack.Peek();
                auto literalName = structureResult->TryGetLiteralName(alloc, node, currentScope.containingAssignment);
                if (literalName)
                {
                    StructureNode* newStructureNode = StructureNode::New(structureResult, literalName, asnkObjectLiteral, node);

                    charcount_t assignmentMin = ActualMin(node);
                    if (structureResult->TryGetLiteralAssignmentOffset(currentScope.containingAssignment, node, assignmentMin))
                    {
                        newStructureNode->AddCoveringRegion(structureResult, assignmentMin, ActualLim(node));
                    }

                    // Only add the literal if it has something interesting in it.
                    isConditionalNode = true;
                    return newStructureNode;
                }

                isDynamicObject = true;
                return nullptr;
            }

            void AddStaticAnalysisNodeResult(
                ItemVisitScope& currentAnalysisScope,
                ParseNode* analyzedNode,
                StructureNode* staticAnalysisNodeResult,
                bool isConditionalNode,
                bool shouldPushScope,
                bool isDynamicNode,
                bool appendToParentNode = true)
            {
                Assert(analyzedNode);

                if (isDynamicNode)
                {
                    Assert(staticAnalysisNodeResult == null);

                    scopeStack.Push(
                        ItemVisitScope(
                        StructureNode::New(structureResult, (Js::InternalString*)nullptr, asnkCustom, analyzedNode),
                        analyzedNode,
                        isConditionalNode,
                        isDynamicNode,
                        true));
                }
                else if (staticAnalysisNodeResult)
                {
                    if (!isConditionalNode && appendToParentNode)
                    {
                        currentAnalysisScope.structureNode->Append(staticAnalysisNodeResult);
                    }
                    // else if it is a conditionalNode then it will be added in the postfix function below.

                    if (shouldPushScope)
                    {
                        scopeStack.Push(
                            ItemVisitScope(
                            staticAnalysisNodeResult,
                            analyzedNode,
                            isConditionalNode,
                            isDynamicNode,
                            currentAnalysisScope.ScopeDynamicInformation.isUnderDynamicNode || isDynamicNode));
                    }
                }
            }

            bool PreVisitNode(ParseNode* node)
            {
                StructureNode *newStructureNode = nullptr;

                bool shouldPushScope = true;

                bool isConditionalNode = false;
                bool isPossibleDynamicNode = false;

                ItemVisitScope currentAnalysisScope = scopeStack.Peek();


                switch (node->nop)
                {
                case knopProg:
                    // The program node represents the global scope.
                    newStructureNode = StructureNode::New(structureResult, asnkGlobal, node);
                    break;

                case knopMember:
                case knopMemberShort:
                case knopAsg:
                case knopGetMember:
                case knopSetMember:
                    // Remember the assignment for function and literal assignments.
                    SetContainingAssignment(node, currentAnalysisScope);
                    break;

                case knopFncDecl:
                    newStructureNode = AnalyzeFunctionDeclaration(node, isPossibleDynamicNode);
                    break;

                case knopClassDecl:
                {
                    // If the class is defined on the global level we need to pull this out and show them at the root level (sibling to global)
                    newStructureNode = AnalyzeClassDeclaration(node);
                    if (currentAnalysisScope.structureNode->kind == asnkGlobal)
                    {
                        ItemVisitScope rootScope = scopeStack.Peek(1);

                        AddStaticAnalysisNodeResult(
                            rootScope,
                            node,
                            newStructureNode,
                            false/*isConditionalNode*/,
                            true/*shouldPushScope*/,
                            false/*isPossibleDynamicNode*/);
                        return true;
                    }
                }

                case knopCall:
                {
                                 // Check for an immediately invoked function expression (iife) which is interpreted as a module.
                                 auto target = node->sxCall.pnodeTarget;
                                 if (target && target->nop == knopFncDecl)
                                 {
                                     // Always create a node for immediately invoked functions (IIFEs).
                                     newStructureNode = StructureNode::New(structureResult, structureResult->AnonymousFunctionName(alloc, target, nullptr), asnkFunction, node);
                                 }
                }
                    break;

                case knopConstDecl:
                case knopLetDecl:
                    // Only record const and let at the global scope. If they are nested in a block they are local to the block.
                    if (blockNestingCount > 0)
                        break;
                    __fallthrough;

                case knopVarDecl:
                {
                    // Only record variables for the global scope.
                    auto contextKind = currentAnalysisScope.structureNode->kind;
                    if (contextKind == asnkGlobal)
                    {
                        // If the var is initialized by a function expression ignore it for now as we will see it as a named function when we see the knopFuncDec.
                        if (!node->sxVar.pnodeInit || (node->sxVar.pnodeInit->nop != knopFncDecl && node->sxVar.pnodeInit->nop != knopClassDecl))
                        {
                            newStructureNode = StructureNode::New(structureResult, node->sxVar.pid, asnkVariable, node);

                            // Don't consider the variable initializer a scope of the variable.
                            shouldPushScope = false;
                        }
                    }

                    // Remember the var declaration for function and literal assignments.
                    SetContainingAssignment(node, currentAnalysisScope);
                }
                    break;

                case knopObject:
                    newStructureNode = AnalyzeObjectLiteral(node, isPossibleDynamicNode, isConditionalNode);
                    break;

                case knopBlock:
                case knopSwitch:
                case knopFor:
                case knopForIn:
                case knopForOf:
                    // These nodes increase the block nesting. The switch and for statements have implicit blocks.
                    blockNestingCount++;
                    break;
                }

                AddStaticAnalysisNodeResult(
                    currentAnalysisScope,
                    node,
                    newStructureNode,
                    isConditionalNode,
                    shouldPushScope,
                    isPossibleDynamicNode);

                // always traverse children.
                return true;
            }

            void PostVisitNode(ParseNode* node)
            {
                auto currentScope = scopeStack.Peek();
                if (currentScope.parseNode == node)
                {
                    scopeStack.Pop();

                    if (scopeStack.Count() > 0)
                    {
                        auto parentScope = scopeStack.Peek();

                        // If the current node is conditional we only want to add it if it has real children.
                        // and it's not a dynamic node or we're not adding a dynamic to a static
                        if (currentScope.isConditionalNode &&
                            currentScope.structureNode->children.Count() > 0 &&
                            currentScope.structureNode->children.Any([](StructureNode* node) { return node->kind != asnkRegion; }))
                        {
                            parentScope.structureNode->Append(currentScope.structureNode);
                        }
                    }

                    if (currentScope.ScopeDynamicInformation.isUnderDynamicNode)
                    {
                        auto addDynamicFunctionChildrenMapping = [this, currentScope, node](StructureNode* dynamicFunctionStructureNode)
                        {
                            DynamicFunctionChildNodeList* childNodeList = structureResult->dynamicFunctionChildrenMap.GetOrAddListForKey(node->ichMin);

                            dynamicFunctionStructureNode->TraverseChildren([childNodeList](StructureNode* child)
                            {
                                childNodeList->Add(child);
                            });
                        };

                        if (node->nop == knopFncDecl)
                        {
                            addDynamicFunctionChildrenMapping(currentScope.structureNode);
                        }
                    }

                    if (node->nop == knopFncDecl &&
                        currentScope.structureNode->kind == asnkFunction &&
                        !currentScope.structureNode->HasChildren())
                    {
                        // If the function doesn't have a child we don't need its container name.
                        currentScope.structureNode->containerName = nullptr;
                    }
                }

                switch (node->nop)
                {
                case knopBlock:
                case knopSwitch:
                case knopFor:
                case knopForIn:
                case knopForOf:
                    blockNestingCount--;
                    break;
                }
            }

        public:
            StaticScopeAnalyzer(ArenaAllocator* alloc, StructureResult* structureResult) :
                alloc(alloc),
                structureResult(structureResult),
                scopeStack(alloc),
                blockNestingCount(0)
            {
            }

            void Visit(ParseNode* rootNode)
            {
                Assert(rootNode);

                // initial scope
                scopeStack.Push(ItemVisitScope(structureResult->root, rootNode, /*isConditionalNode*/ false, /*isPossibleDynamicNode*/ false, /*isUnderDynamicNode*/false));

                ASTHelpers::Visit(rootNode, [this](ParseNode* node) { return PreVisitNode(node); }, [this](ParseNode* node) { PostVisitNode(node); });

                Assert(scopeStack.Count() == 1); // just for sanity
                scopeStack.Clear();
            }
        };

    public:
        StructureResult(FileAuthoring *fileAuthoring) :
            SimpleComObjectWithAlloc<IAuthorStructure>(GetPageAllocator(fileAuthoring), L"ls:Structure"),
            fileAuthoring(fileAuthoring),
            functionSpanMap(Alloc()),
            dynamicFunctionChildrenMap(Alloc()),
            m_topItemList(nullptr),
            m_inContainerListPropertyId(Js::Constants::NoProperty),
            m_recursionMarkList(nullptr),
            m_primaryFile(nullptr),
            scriptContext(nullptr),
            m_defLocPropertyId(Js::Constants::NoProperty)
        {
            root = StructureNode::New(this, asnkRoot, nullptr);
        }

        PageAllocator *GetPageAllocator() { return GetPageAllocator(fileAuthoring); }

        StructureNode *GetRoot() { return root; }

        StructureNode *Find(int key)
        {
            StructureNode * result = nullptr;
            bool found = root->TraverseAllUntil([&](StructureNode *node) -> void
            {
                result = node;
            }, key);

            return found ? result : nullptr;
        }

        void AddKnownFunctionAlias(ParseNodePtr functionNode, charcount_t calculatedFunctionStartOffset, charcount_t lim)
        {
            Assert(functionNode && functionNode->nop == knopFncDecl);

            functionSpanMap.Add(functionNode->ichMin, Span(calculatedFunctionStartOffset, lim));
        }

        // Produce structure nodes using just the AST.
        void AnalyzeStructure(ArenaAllocator *alloc, ParseNode *rootNode)
        {
            StaticScopeAnalyzer staticAnalyzer(alloc, this);
            staticAnalyzer.Visit(rootNode);
        }

        bool HasRegion(StructureNode *node)
        {
            return node->min || node->lim;
        }

        bool TryGetMinAndLimFromDefinitionLocation(
            ArenaAllocator* alloc,
            Js::ScriptContext* scriptContext,
            Js::Var value,
            int fileId,
            charcount_t& min,
            charcount_t& lim)
        {
            Assert(Js::RecyclableObject::Is(value));
            Js::RecyclableObject* containerObject = Js::RecyclableObject::FromVar(value);
            auto literalDefinitionLocation = JsHelpers::GetProperty<Js::RecyclableObject *>(containerObject, Names::_defLoc, alloc, scriptContext, /*getOwnProperty*/true, /*ignoreExceptions*/true);
            if (literalDefinitionLocation)
            {
                int objectFileId = JsHelpers::GetProperty<int>(literalDefinitionLocation, Names::fileId, alloc, scriptContext, true, true);
                if (fileId == objectFileId)
                {
                    charcount_t startOffset = JsHelpers::GetProperty<charcount_t>(literalDefinitionLocation, Names::offset, alloc, scriptContext, true, true);
                    charcount_t endOffset = JsHelpers::GetProperty<charcount_t>(literalDefinitionLocation, Names::endOffset, alloc, scriptContext, true, true);
                    if (endOffset >= startOffset)
                    {
                        min = startOffset;
                        lim = endOffset;
                        return true;
                    }
                }
            }

            return false;
        }

        bool TryGetMinAndLimFromFieldDocOfInstance(
            ArenaAllocator* alloc,
            Js::ScriptContext* scriptContext,
            Js::RecyclableObject* instance,
            Js::InternalString* fieldName,
            ParseNodeTree* tree,
            int fileId,
            charcount_t& min,
            charcount_t& lim)
        {
            if (instance)
            {
                auto fieldDoc = JsValueDoc::GetFieldDoc(alloc, instance, fieldName->GetBuffer(), scriptContext);
                if (fieldDoc)
                {
                    auto pos = fieldDoc->pos;
                    auto fieldFileId = fieldDoc->fileId;
                    if (fieldFileId == fileId)
                    {
                        // Find the assignment or the member node in the tree
                        ParseNodeCursor cursor(alloc, tree);
                        cursor.SeekToOffset(pos);
                        while (cursor.Current()
                            && cursor.Current()->nop != knopAsg
                            && cursor.Current()->nop != knopMember
                            && cursor.Current()->nop != knopMemberShort
                            && cursor.Current()->nop != knopProg
                            && cursor.Current()->nop != knopFncDecl)
                        {
                            cursor.Up();
                        }

                        if (cursor.Current() && (cursor.Current()->nop == knopAsg || cursor.Current()->nop == knopMember || cursor.Current()->nop == knopMemberShort))
                        {
                            auto node = cursor.Current();
                            min = ActualMin(node);
                            lim = ActualLim(node);
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        bool TryGetMinAndLimFromFieldDoc(
            ArenaAllocator* alloc,
            Js::ScriptContext* scriptContext,
            Js::RecyclableObject* object,
            Js::InternalString* fieldName,
            ParseNodeTree* tree,
            int fileId,
            charcount_t& min,
            charcount_t& lim)
        {
            Assert(object && fieldName);

            if (!fieldName)
            {
                return false;
            }

            return TryGetMinAndLimFromFieldDocOfInstance(alloc,
                scriptContext,
                JsHelpers::GetProperty<Js::RecyclableObject*>(object, Names::instance, alloc, scriptContext, true, true),
                fieldName,
                tree,
                fileId,
                min,
                lim);
        }

        bool FetchAnalyzedFunction(ArenaAllocator *alloc,
            AuthoringFileHandle *file,
            ParseNodeTree *tree,
            Js::Var value,
            AuthorStructureNodeKind type,
            charcount_t &min,
            charcount_t &lim)
        {
            auto function = Js::JavascriptFunction::FromVar(value);
            auto body = function->GetParseableFunctionInfo();
            if (body != nullptr)
            {
                // Only record a range if the function is in the current file.
                if (file->IsSourceMatched(body->GetUtf8SourceInfo()))
                {
                    GetFunctionBodyRange(alloc, tree, body, /*ref*/min, /*ref*/lim);
                }
                // Only look at functions that look like classes as they might contain methods in this file.
                else if (
                    type != asnkClass ||
                    !TryGetMinAndLimFromDefinitionLocation(alloc, scriptContext, function, file->FileId(), min, lim))
                {
                    return false;
                }

                return true;
            }

            return type == asnkClass;
        }

        void RecordDynamicStructureScope(ArenaAllocator *alloc, ResolvedItemList *resolvedItems, StructureNode *node, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree)
        {
            Assert(resolvedItems != nullptr);

            for (int i = 0; i < resolvedItems->Count(); i++)
            {
                ResolvedItem * item = resolvedItems->Item(i);
                Assert(item);
                if (item->value != nullptr)
                {
                    Js::InternalString *containerName = item->name;
                    Assert(!InternalName(containerName->GetBuffer()));

                    auto itemName = (Js::InternalString *)nullptr;

                    Js::InternalString * glyph = nullptr;
                    if (item->glyph != nullptr)
                    {
                        Convert::FromVar(this->Alloc(), item->glyph, glyph);
                    }

                    auto children = item->children;
                    auto value = item->value;
                    auto type = item->type;
                    Assert(type != asnkCustom); // We would like to know when this happens

                    Js::RecyclableObject * objInstance = item->instance ? Js::RecyclableObject::FromVar(item->instance) : nullptr;

                    auto createNamedStructureNode = [this, type, glyph](Js::InternalString* itemName, Js::InternalString* containerName, charcount_t startOffset, charcount_t endOffset)
                    {
                        return StructureNode::New(this, itemName, containerName, glyph, type, nullptr/*typename*/, startOffset, endOffset);
                    };

                    auto createStructureNode = [itemName, containerName, createNamedStructureNode](charcount_t startOffset, charcount_t endOffset)
                    {
                        return createNamedStructureNode(itemName, containerName, startOffset, endOffset);
                    };

                    auto createEmptyStructureNode = [createStructureNode]() { return createStructureNode(0, 0); };

                    charcount_t min = 0;
                    charcount_t lim = 0;

                    StructureNode* newNode = nullptr;

                    // Determine the range of the node. If it is a function we retrive the range from the function script information (the information used 
                    // for function's toString() implementation. If it is a field, look for the assignment that constructed it in the AST because we only record
                    // the start information in the Doc field. If, for some reason, we find a file that is not part of the given source, we produce an empty region.
                    if (Js::JavascriptFunction::Is(value))
                    {
                        if (FetchAnalyzedFunction(alloc, file, tree, value, type, min, lim))
                        {
                            Js::InternalString* fieldName = containerName;

                            if (type == asnkFunction)
                            {
                                itemName = FunctionItemName(this->Alloc(), tree, min, fieldName);
                                containerName = nullptr;
                            }

                            newNode = createNamedStructureNode(itemName, containerName, min, lim);

                            Js::Var auxValue = item->auxValue;
                            if (auxValue && Js::JavascriptFunction::Is(auxValue))
                            {
                                charcount_t auxMin = 0;
                                charcount_t auxLim = 0;
                                // The auxMin and auxLim are an additional range that might be associated with an item. For example, a property
                                // with both a getter and a setter will have the setter function as the auxiliary range of the node. Such ranges
                                // are stored as an asnkRegion child node.
                                if (FetchAnalyzedFunction(alloc, file, tree, auxValue, type, auxMin, auxLim) &&
                                    auxMin != min)
                                {
                                    newNode->AddCoveringRegion(this, auxMin, auxLim);
                                }
                            }

                            // if we can get information from the field doc, use that first
                            // add a region for it's field declaration
                            // Handles:
                            //		function start() {
                            //			// ...
                            //		}
                            //
                            //		function notInTheClass() { }
                            //
                            //		var myClass = WinJS.Class.define(function () { }, {
                            //			|start: start
                            //		});
                            charcount_t fieldMin = 0;
                            charcount_t fieldLim = 0;
                            if (fieldName &&
                                TryGetMinAndLimFromFieldDocOfInstance(alloc, scriptContext, objInstance, fieldName, tree, file->FileId(), fieldMin, fieldLim) &&
                                fieldMin != min)
                            {
                                newNode->AddCoveringRegion(this, fieldMin, fieldLim);
                            }
                            else
                            {
                                // If the function is in a knopMember then extend the range of the function to include the member.
                                ParseNodeCursor cursor(alloc, tree);
                                cursor.SeekToOffset(min);
                                auto node = cursor.Current();
                                if (node && node->nop == knopFncDecl)
                                {
                                    auto parent = cursor.Parent();
                                    if ((parent->nop == knopMember || parent->nop == knopMemberShort) && parent->ichMin != min)
                                    {
                                        newNode->AddCoveringRegion(this, parent->ichMin, parent->ichLim);
                                    }
                                }
                            }

                            // if its not a class, then add the static children (if any)
                            // filter classes because their constructor is where the static children should be added, 
                            // which we'll catch when visiting the classes children
                            if (type == asnkFunction)
                            {
                                // Add any static aliases
                                SpanList* spans;
                                if (functionSpanMap.TryGet(min, &spans))
                                {
                                    spans->Map([newNode, this](int index, Span span) -> void
                                    {
                                        newNode->AddCoveringRegion(this, span.start, span.end);
                                    });
                                }

                                // add any statically analyzed children
                                DynamicFunctionChildNodeList* staticChildrenFunctions;
                                if (dynamicFunctionChildrenMap.TryGet(min, &staticChildrenFunctions))
                                {
                                    staticChildrenFunctions->Map([newNode, this, node](int index, StructureNode* staticallyAnalyzedChild)
                                    {
                                        StructureNode* existingNode = nullptr;

                                        bool foundNode = node->children.MapUntil([staticallyAnalyzedChild, &existingNode](int index, StructureNode* existingChild)
                                        {
                                            if (existingChild &&
                                                existingChild->kind == asnkFunction &&
                                                staticallyAnalyzedChild->min == existingChild->min)
                                            {
                                                existingNode = existingChild;
                                                return true;
                                            }

                                            return false;
                                        });

                                        if (foundNode && existingNode)
                                        {
                                            existingNode->AppendDistinctChildren(staticallyAnalyzedChild->children);
                                        }
                                        else
                                        {
                                            newNode->AppendIfDistinct(staticallyAnalyzedChild);
                                        }
                                    });
                                }
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (type == asnkField || type == asnkProperty)
                    {
                        // Determine if we can determine the location of the field from meta-data on the instance.
                        if (containerName && TryGetMinAndLimFromFieldDocOfInstance(alloc, scriptContext, objInstance, containerName, tree, file->FileId(), min, lim))
                        {
                            newNode = createStructureNode(min, lim);
                        }
                    }
                    else if ((type == asnkNamespace || type == asnkClass) && Js::RecyclableObject::Is(value))
                    {
                        if (TryGetMinAndLimFromDefinitionLocation(alloc, scriptContext, value, file->FileId(), min, lim))
                        {
                            newNode = createStructureNode(min, lim);
                        }
                        else
                        {
                            newNode = createEmptyStructureNode();
                        }
                    }
                    else
                    {
                        newNode = createEmptyStructureNode();
                    }

                    if (children && newNode)
                    {
                        RecordDynamicStructureScope(alloc, children, newNode, scriptContext, file, tree);
                    }

                    // Only record a node if it has a region or has children (which, by transitivity, have at least one child with a region)
                    if (newNode && (HasRegion(newNode) || newNode->HasChildren()))
                    {
                        node->Append(newNode);
                    }
                }
            }
        }

        void Old_RecordDynamicStructureScope(ArenaAllocator *alloc, Js::JavascriptArray *value, StructureNode *node, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree)
        {
            JsHelpers::ForEach<Js::Var>(value, alloc, [&](int index, Js::Var element) -> void
            {
                if (!Js::TaggedNumber::Is(element))
                {
                    auto object = Js::RecyclableObject::FromVar(element);
                    auto containerName = JsHelpers::GetProperty<Js::InternalString *>(object, Names::name, this->Alloc(), scriptContext, true, true);
                    auto itemName = (Js::InternalString *)nullptr;

                    if (InternalName(containerName->GetBuffer()))
                    {
                        return;
                    }

                    auto glyph = JsHelpers::GetProperty<Js::InternalString *>(object, Names::glyph, this->Alloc(), scriptContext, true, true);
                    auto typeName = JsHelpers::GetProperty<Js::InternalString *>(object, Names::type, alloc, scriptContext, true, true);
                    auto children = JsHelpers::GetProperty<Js::JavascriptArray *>(object, Names::children, alloc, scriptContext, true, true);
                    auto value = JsHelpers::GetPropertyVar(object, Names::value, scriptContext, true);

                    auto type = asnkCustom;
                    if (!containerName || !typeName)
                    {
                        return;
                    }

                    if (StringEqualsLiteral(typeName, Names::_class)) { type = asnkClass; }
                    else if (StringEqualsLiteral(typeName, Names::_namespace)) { type = asnkNamespace; }
                    else if (StringEqualsLiteral(typeName, Names::field)) { type = asnkField; }
                    else if (StringEqualsLiteral(typeName, Names::method)) { type = asnkFunction; }

                    auto createNamedStructureNode = [this, type, typeName, glyph](Js::InternalString* itemName, Js::InternalString* containerName, charcount_t startOffset, charcount_t endOffset)
                    {
                        auto typeString = type == asnkCustom ? AllocInternalString(this->Alloc(), typeName->GetBuffer()) : nullptr;
                        return StructureNode::New(this, itemName, containerName, glyph, type, typeString, startOffset, endOffset);
                    };

                    auto createStructureNode = [itemName, containerName, createNamedStructureNode](charcount_t startOffset, charcount_t endOffset)
                    {
                        return createNamedStructureNode(itemName, containerName, startOffset, endOffset);
                    };

                    auto createEmptyStructureNode = [createStructureNode]() { return createStructureNode(0, 0); };

                    charcount_t min = 0;
                    charcount_t lim = 0;

                    StructureNode* newNode = nullptr;

                    // Determine the range of the node. If it is a function we retrive the range from the function script information (the information used 
                    // for function's toString() implementation. If it is a field, look for the assignment that constructed it in the AST because we only record
                    // the start information in the Doc field. If, for some reason, we find a file that is not part of the given source, we produce an empty region.
                    if (Js::JavascriptFunction::Is(value))
                    {
                        auto analyzeFunction = [&](Js::Var value, charcount_t &min, charcount_t &lim) -> bool
                        {
                            auto function = Js::JavascriptFunction::FromVar(value);

                            auto body = function->GetParseableFunctionInfo();
                            if (body)
                            {
                                auto index = body->GetSourceIndex();

                                // Only record a range if the function is in the current file.
                                if (file->IsSourceAtIndex(scriptContext, index))
                                {
                                    GetFunctionBodyRange(alloc, tree, body, /*ref*/min, /*ref*/lim);
                                }
                                // Only look at functions that look like classes as they might contain methods in this file.
                                else if (
                                    type != asnkClass ||
                                    !TryGetMinAndLimFromDefinitionLocation(alloc, scriptContext, function, file->FileId(), min, lim))
                                {
                                    return false;
                                }

                                return true;
                            }

                            return type == asnkClass;
                        };

                        if (analyzeFunction(value, min, lim))
                        {
                            Js::InternalString* fieldName = containerName;

                            if (type == asnkFunction)
                            {
                                itemName = FunctionItemName(alloc, tree, min, fieldName);
                                containerName = nullptr;
                            }

                            newNode = createNamedStructureNode(itemName, containerName, min, lim);

                            auto auxValue = JsHelpers::GetPropertyVar(object, Names::auxValue, scriptContext, true);
                            if (auxValue && Js::JavascriptFunction::Is(auxValue))
                            {
                                charcount_t auxMin = 0;
                                charcount_t auxLim = 0;
                                // The auxMin and auxLim are an additional range that might be associated with an item. For example, a property
                                // with both a getter and a setter will have the setter function as the auxiliary range of the node. Such ranges
                                // are stored as an asnkRegion child node.
                                if (analyzeFunction(auxValue, auxMin, auxLim) &&
                                    auxMin != min)
                                {
                                    newNode->AddCoveringRegion(this, auxMin, auxLim);
                                }
                            }

                            // if we can get information from the field doc, use that first
                            // add a region for it's field declaration
                            // Handles:
                            //		function start() {
                            //			// ...
                            //		}
                            //
                            //		function notInTheClass() { }
                            //
                            //		var myClass = WinJS.Class.define(function () { }, {
                            //			|start: start
                            //		});
                            charcount_t fieldMin = 0;
                            charcount_t fieldLim = 0;
                            if (fieldName &&
                                TryGetMinAndLimFromFieldDoc(alloc, scriptContext, object, fieldName, tree, file->FileId(), fieldMin, fieldLim) &&
                                fieldMin != min)
                            {
                                newNode->AddCoveringRegion(this, fieldMin, fieldLim);
                            }
                            else
                            {
                                // If the function is in a knopMember then extend the range of the function to include the member.
                                ParseNodeCursor cursor(alloc, tree);
                                cursor.SeekToOffset(min);
                                auto node = cursor.Current();
                                if (node && node->nop == knopFncDecl)
                                {
                                    auto parent = cursor.Parent();
                                    if ((parent->nop == knopMember || parent->nop == knopMemberShort) && parent->ichMin != min)
                                    {
                                        newNode->AddCoveringRegion(this, parent->ichMin, parent->ichLim);
                                    }
                                }
                            }



                            // if its not a class, then add the static children (if any)
                            // filter classes because their constructor is where the static children should be added, 
                            // which we'll catch when visiting the classes children
                            if (type == asnkFunction)
                            {
                                // Add any static aliases
                                SpanList* spans;
                                if (functionSpanMap.TryGet(min, &spans))
                                {
                                    spans->Map([newNode, this](int index, Span span) -> void
                                    {
                                        newNode->AddCoveringRegion(this, span.start, span.end);
                                    });
                                }

                                // add any statically analyzed children
                                DynamicFunctionChildNodeList* staticChildrenFunctions;
                                if (dynamicFunctionChildrenMap.TryGet(min, &staticChildrenFunctions))
                                {
                                    staticChildrenFunctions->Map([newNode, this, node](int index, StructureNode* staticallyAnalyzedChild)
                                    {
                                        StructureNode* existingNode = nullptr;

                                        bool foundNode = node->children.MapUntil([staticallyAnalyzedChild, &existingNode](int index, StructureNode* existingChild)
                                        {
                                            if (existingChild &&
                                                existingChild->kind == asnkFunction &&
                                                staticallyAnalyzedChild->min == existingChild->min)
                                            {
                                                existingNode = existingChild;
                                                return true;
                                            }

                                            return false;
                                        });

                                        if (foundNode && existingNode)
                                        {
                                            existingNode->AppendDistinctChildren(staticallyAnalyzedChild->children);
                                        }
                                        else
                                        {
                                            newNode->AppendIfDistinct(staticallyAnalyzedChild);
                                        }
                                    });
                                }
                            }
                        }
                        else
                        {
                            return;
                        }
                    }
                    else if (type == asnkField)
                    {
                        // Determine if we can determine the location of the field from meta-data on the instance.
                        if (TryGetMinAndLimFromFieldDoc(alloc, scriptContext, object, containerName, tree, file->FileId(), min, lim))
                        {
                            newNode = createStructureNode(min, lim);
                        }
                    }
                    else if ((type == asnkNamespace || type == asnkClass) && Js::RecyclableObject::Is(value))
                    {
                        if (TryGetMinAndLimFromDefinitionLocation(alloc, scriptContext, value, file->FileId(), min, lim))
                        {
                            newNode = createStructureNode(min, lim);
                        }
                        else
                        {
                            newNode = createEmptyStructureNode();
                        }
                    }
                    else
                    {
                        newNode = createEmptyStructureNode();
                    }

                    if (children && newNode)
                    {
                        Old_RecordDynamicStructureScope(alloc, children, newNode, scriptContext, file, tree);
                    }

                    // Only record a node if it has a region or has children (which, by transitivity, have at least one child with a region)
                    if (newNode && (HasRegion(newNode) || newNode->HasChildren()))
                    {
                        node->Append(newNode);
                    }
                }
            });
        }

        int LexicalCompare(StructureNode *a, StructureNode *b)
        {
            Assert(a && b);

            auto nextParent = [](StructureNode *node) -> StructureNode *
            {
                for (auto parent = node->container; parent; parent = parent->container)
                if (parent->containerName) return parent;
                return nullptr;
            };

            // First compare the parents.
            auto aParent = nextParent(a);
            auto bParent = nextParent(b);
            if (aParent && !bParent) return -1;
            if (!aParent && bParent) return 1;
            if (aParent && bParent)
            {
                auto parentCompare = LexicalCompare(aParent, bParent);
                if (parentCompare != 0) return parentCompare;
            }

            // If the parents are equal then compare the container names
            if (a->containerName && !b->containerName) return -1;
            if (!a->containerName && b->containerName) return 1;
            if (!a->containerName && !b->containerName) return 0;

            int compare = ::wcsncmp(a->containerName->GetBuffer(), b->containerName->GetBuffer(), min(a->containerName->GetLength(), b->containerName->GetLength()));
            if (compare != 0) return compare;

            int lengthDiff = a->containerName->GetLength() - b->containerName->GetLength();
            if (lengthDiff > 0) return 1;
            if (lengthDiff < 0) return -1;

            return 0;
        }

        void RemoveDuplicateNamespaceMembers(Js::ScriptContext *scriptContext, ParseNodeTree *tree)
        {
            ArenaAllocator localArena(L"ls:GetStructure", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            // A namespace member is duplicate if it appears in more than one namespace.
            // Two members are considered identical if they have the same min location.
            // If a member appears twice, the better of the two is left in the tree and the
            // other is removed.
            // Node A is better than node B if,
            //  1. A has fewer parents with unknown names (e.g. names not in the parse tree) than B.
            //  2. A has fewer parents that begin with '_' than B.
            //  3. A has fewer total parents than B.
            //  4. A has parents (if serialized as P1.P2.P3.A) that are lexically before B's.
            typedef JsUtil::BaseDictionary<int, StructureNode *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> NodeMap;
            typedef JsUtil::List<StructureNode *, ArenaAllocator> NodeList;

            NodeMap bestNodeMap(&localArena);
            NodeList nodesToRemove(&localArena);

            struct ParentInfo
            {
                int totalParents;
                int privateParents;
                int unknownParents;
            };

            auto countParents = [&](StructureNode *node, ParentInfo &info) {
                info.totalParents = 0;
                info.privateParents = 0;
                info.unknownParents = 0;
                for (auto current = node->container; current; current = current->container)
                {
                    if (current->containerName)
                    {
                        if (!tree->ContainsName(current->containerName))
                            info.unknownParents++;
                        if (current->containerName->GetLength() > 0 && current->containerName->GetBuffer()[0] == '_')
                            info.privateParents++;
                        info.totalParents++;
                    }
                }
            };

            this->root->TraverseChildren([&](StructureNode *rootNode) {
                // We only are filtering children of namespaces.
                if (rootNode->kind == asnkNamespace)
                    rootNode->TraverseAllMatching([](StructureNode *node) { return node->kind != asnkClass; }, [&](StructureNode *node) {
                    // Only pay attention to nodes that have a range.
                    if (node->lim > node->min)
                    {
                        StructureNode *other;
                        if (!bestNodeMap.TryGetValue(node->min, &other))
                        {
                            // This is the first node at its position.
                            bestNodeMap.Add(node->min, node);
                        }
                        else
                        {
                            // If node is better than the exisitng node (other), add the existing node to the list of nodes 
                            // to delete and replace it with node otherwise place node in the list of nodes to delete.
                            // See above method comment for the definition of "better".
                            ParentInfo nodeInfo;
                            ParentInfo otherInfo;

                            countParents(node, /*ref*/nodeInfo);
                            countParents(other, /*ref*/otherInfo);

                            bool nodeIsBetter =
                                nodeInfo.unknownParents < otherInfo.unknownParents ||
                                (nodeInfo.unknownParents == otherInfo.unknownParents &&
                                (nodeInfo.privateParents < otherInfo.privateParents ||
                                (nodeInfo.privateParents == otherInfo.privateParents &&
                                (nodeInfo.totalParents < otherInfo.totalParents ||
                                (nodeInfo.totalParents == otherInfo.totalParents &&
                                LexicalCompare(node, other) < 0)))));


                            if (nodeIsBetter)
                            {
                                nodesToRemove.Add(other);
                                bestNodeMap.Item(node->min, node);
                            }
                            else
                            {
                                nodesToRemove.Add(node);
                            }
                        }
                    }
                });
            });

            if (nodesToRemove.Count() > 0)
            {
                // If we have nodes to remove, remove the nodes from their parent. If the parent becomes empty then it should be removed from its parent as well.
                nodesToRemove.Map([this](int index, StructureNode *node)
                {
                    this->RemoveNode(node);
                });
            }
        }

        void RemoveNode(StructureNode* node)
        {
            if (node == nullptr)
            {
                return;
            }

            for (auto current = node; current; current = current->container)
            {
                auto container = current->container;
                if (!container)
                {
                    container = this->root;
                }

                container->children.Remove(current);

                if (container->children.Count() != 0)
                {
                    // Don't remove the node if it still has children other than this one.
                    break;
                }
            }
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        void PrintIndent()
        {
            if (PHASE_TRACE1(Js::NavBarDynamicAnalysisPhase))
            {
                for (int i = 0; i < m_indentNum; i++)
                {
                    Output::Print(L" ");
                }
            }
        }

#define PRINT_INDENT() PrintIndent()
#define INCREASE_INDENT() m_indentNum += 4
#define DECREASE_INDENT() m_indentNum -= 4
#define INIT_INDENT() m_indentNum = 0

        void PrintArray1(const wchar_t* arrayName, PropertyNameList* names)
        {
            if (names)
            {
                PRINT_INDENT();
                OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"%ls {\n", arrayName);
                INCREASE_INDENT();
                for (int i = 0; i < names->Count(); i++)
                {
                    Js::Var nameVar = names->Item(i);
                    Assert(Js::JavascriptString::Is(nameVar));
                    if (Js::JavascriptString::Is(nameVar))
                    {
                        PRINT_INDENT();
                        OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"%ls\n", Js::JavascriptString::FromVar(nameVar)->GetSz());
                    }
                }
                DECREASE_INDENT();
                PRINT_INDENT();
                OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
            }
        }
        void PrintArray2(const wchar_t* arrayName, ResolvedItemList * names)
        {
            if (names)
            {
                PRINT_INDENT();
                OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"%ls {\n", arrayName);
                INCREASE_INDENT();
                for (int i = 0; i < names->Count(); i++)
                {
                    PRINT_INDENT();
                    names->Item(i)->Print();
                }
                DECREASE_INDENT();
                PRINT_INDENT();
                OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
            }
        }

#define PRINT_ARRAY1(arrayName, names) PrintArray1(arrayName, names)
#define PRINT_ARRAY2(arrayName, names) PrintArray2(arrayName, names)
#else
#define PRINT_ARRAY1(arrayName, names) 
#define PRINT_ARRAY2(arrayName, names) 
#define PRINT_INDENT() 
#define INCREASE_INDENT() 
#define DECREASE_INDENT() 
#define INIT_INDENT() 
#endif

        const wchar_t * GetName(Js::Var name)
        {
            const Js::PropertyRecord * propRecord = GetPropertyRecord(name, scriptContext);
            return propRecord ? propRecord->GetBuffer() : L"";
        }

        ResolvedItem * AnalysisOfMember(Js::RecyclableObject * obj, Js::Var name, Js::ScriptContext *scriptContext, bool includeMembers)
        {
            bool isPropertyDefined = false;
            Js::PropertyDescriptor propertyDescriptor;
            BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
            {
                IGNORE_STACKWALK_EXCEPTION(scriptContext);
                const Js::PropertyRecord* propertyRecord = GetPropertyRecord(name, scriptContext);
                isPropertyDefined = propertyRecord && !!Js::JavascriptObject::GetOwnPropertyDescriptorHelper(obj, propertyRecord->GetPropertyId(), scriptContext, propertyDescriptor);
            }
            END_JS_RUNTIME_CALL(scriptContext);

            ResolvedItem * resolvedItem = nullptr;

            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalysisOfMember {\n");
            INCREASE_INDENT();
            if (isPropertyDefined)
            {
                if (propertyDescriptor.GetterSpecified())
                {
                    Js::Var getVar = propertyDescriptor.GetGetter();
                    Js::Var setVar = propertyDescriptor.SetterSpecified() ? propertyDescriptor.GetSetter() : nullptr;
                    if (getVar == scriptContext->GetLibrary()->GetDefaultAccessorFunction())
                    {
                        Assert(setVar != scriptContext->GetLibrary()->GetDefaultAccessorFunction());
                        getVar = setVar;
                    }
                    resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), name, asnkProperty, getVar, obj, nullptr/*glyph*/, nullptr /*children*/, setVar);

                    PRINT_INDENT();
                    OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"{name : %ls, type : %ls}\n", GetName(name), L"field");
                }
                else if (propertyDescriptor.ValueSpecified())
                {
                    Js::Var value = propertyDescriptor.GetValue();
                    if (Js::JavascriptFunction::Is(value))
                    {
                        if (!Js::JavascriptFunction::FromVar(value)->GetFunctionInfo()->IsDeferred())
                        {
                            resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), name, asnkFunction, value, obj, nullptr/*glyph*/, nullptr /*children*/);
                        }
                    }
                    else
                    {
                        resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), name, asnkField, value, obj, nullptr/*glyph*/, nullptr /*children*/);
                    }

                    PRINT_INDENT();
                    OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"{name : %ls, type : %ls, value : %u}\n", GetName(name), Js::JavascriptFunction::Is(value) ? L"method" : L"field", value);
                }
            }
            DECREASE_INDENT();
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");

            return resolvedItem;
        }

        BOOL GetPropertyWithScriptEnter(Js::RecyclableObject* instance, Js::PropertyId propertyId, Js::Var* value, Js::ScriptContext* scriptContext)
        {
            BOOL retValue = FALSE;
            HRESULT hr = S_OK;
            BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
            {
                if (!scriptContext->GetThreadContext()->IsScriptActive())
                {
                    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
                    {
                        IGNORE_STACKWALK_EXCEPTION(scriptContext);
                        retValue = Js::JavascriptOperators::GetProperty(instance, instance, propertyId, value, scriptContext);
                    }
                    END_JS_RUNTIME_CALL(scriptContext);
                }
                else
                {
                    retValue = Js::JavascriptOperators::GetProperty(instance, instance, propertyId, value, scriptContext);
                }
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);

            return retValue && hr == S_OK;
        }

        template <class CallbackFunction>
        void GetOwnPropertyName(Js::Var instance, FileAuthoring* fileAuthoring, CallbackFunction callback)
        {
            Js::RecyclableObject *object = Js::RecyclableObject::FromVar(instance);
            Js::Var enumeratorVar = nullptr;

            if (!scriptContext->GetThreadContext()->IsScriptActive())
            {
                BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
                {
                    IGNORE_STACKWALK_EXCEPTION(scriptContext);
                    if (!object->GetEnumerator(TRUE, &enumeratorVar, scriptContext, false, false))
                    {
                        return;
                    }
                }
                END_JS_RUNTIME_CALL(scriptContext);
            }
            else
            {
                if (!object->GetEnumerator(TRUE, &enumeratorVar, scriptContext, false, false))
                {
                    return;
                }
            }

            Assert(enumeratorVar != nullptr);

            Js::JavascriptEnumerator *pEnumerator = Js::JavascriptEnumerator::FromVar(enumeratorVar);
            Js::RecyclableObject *undefinedObj = scriptContext->GetLibrary()->GetUndefined();
            Js::Var propertyName = nullptr;
            Js::PropertyId propertyId;

            while ((propertyName = pEnumerator->GetCurrentAndMoveNext(propertyId)) != nullptr)
            {
                if (fileAuthoring->IsHurryCalled())
                {
                    throw ExecutionStop();
                }
                if (propertyName != nullptr && !Js::JavascriptOperators::IsUndefinedObject(propertyName, undefinedObj) && Js::JavascriptString::Is(propertyName)) // There are some code paths in which GetCurrentIndex can return undefined
                {
                    callback(propertyName);
                }
            }

            uint32 index = 0;
            while (object->GetSpecialPropertyName(index, &propertyName, scriptContext))
            {
                if (!Js::JavascriptOperators::IsUndefinedObject(propertyName, undefinedObj))
                {
                    callback(propertyName);
                }
                index++;
            }
        }

        const Js::PropertyRecord * GetPropertyRecord(Js::Var propertyName, Js::ScriptContext * scriptContext)
        {
            Js::PropertyRecord const * propRecord = nullptr;
            if (Js::JavascriptString::Is(propertyName))
            {
                if (!scriptContext->GetThreadContext()->IsScriptActive())
                {
                    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
                    {
                        IGNORE_STACKWALK_EXCEPTION(scriptContext);
                        Js::JavascriptObject::GetPropertyRecordFromVar<true>(propertyName, scriptContext, &propRecord);
                    }
                    END_JS_RUNTIME_CALL(scriptContext);
                }
                else
                {
                    Js::JavascriptObject::GetPropertyRecordFromVar<true>(propertyName, scriptContext, &propRecord);
                }
            }
            return propRecord;
        }

        bool IsEmptyFunctionOwnProperty(Js::PropertyId propertyId)
        {
            return propertyId == Js::PropertyIds::arguments
                || propertyId == Js::PropertyIds::caller
                || propertyId == Js::PropertyIds::prototype
                || propertyId == Js::PropertyIds::length;
        }

        bool IsConstructorProperty(Js::Var propertyName, Js::ScriptContext * scriptContext)
        {
            Js::PropertyRecord const * propRecord = GetPropertyRecord(propertyName, scriptContext);
            if (propRecord)
            {
                return propRecord->GetPropertyId() == Js::PropertyIds::constructor;
            }
            return false;
        }

        int FindNameInPropertyNameList(PropertyNameList *propertyNameList, Js::Var name)
        {
            for (int i = 0; i < propertyNameList->Count(); i++)
            {
                if (propertyNameList->Item(i) == name)
                {
                    return i;
                }
            }
            return -1;
        }

        bool IsCopyOnWriteProxy(Js::Var value)
        {
            if (value != nullptr && Js::DynamicType::Is(Js::JavascriptOperators::GetTypeId(value)))
            {
                return !!Js::DynamicObject::FromVar(value)->IsCopyOnWriteProxy();
            }

            return false;
        }

        void EnsurePropertyId(const wchar_t * name, Js::PropertyId &outPropertyId)
        {
            Assert(name != nullptr);
            if (outPropertyId == Js::Constants::NoProperty)
            {
                Js::PropertyRecord const * propertyRecord;
                scriptContext->GetOrAddPropertyRecord(name, (int)wcslen(name), &propertyRecord);
                Assert(propertyRecord != nullptr);
                scriptContext->TrackPid(propertyRecord);
                outPropertyId = propertyRecord->GetPropertyId();
            }
        }

        bool ShouldAnalyzeFunction(Js::JavascriptFunction* functionValue)
        {
            if (functionValue->GetFunctionInfo()->IsDeferred())
            {
                return false;
            }

            if (!functionValue->GetFunctionInfo()->HasBody())
            {
                // If there is no body (Such as builtins) we should still analyze the function as anything can be added to the prototype object
                return true;
            }
            else if (functionValue->GetFunctionBody()->GetIsClassMember())
            {
                // Good news is this function is statically analyzed, no need to go any further down.
                return false;
            }

            bool isCurrentFile = this->m_primaryFile->IsSourceMatched(functionValue->GetFunctionBody()->GetUtf8SourceInfo());
            if (isCurrentFile)
            {
                return true;
            }

            EnsurePropertyId(Names::_defLoc, m_defLocPropertyId);

            return !!Js::JavascriptOperators::HasOwnProperty(functionValue, m_defLocPropertyId, scriptContext);
        }

        void AnalyzeFunction(ResolvedItemList * resolvedItemList, Js::Var functionName, Js::Var instance, Js::JavascriptFunction* functionValue, Js::Var glyph, bool includeMembers)
        {
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeFunction {\n");
            INCREASE_INDENT();

            // Bailout if the function deferred parsed or defined on a different file.
            if (!ShouldAnalyzeFunction(functionValue))
            {
                DECREASE_INDENT();
                PRINT_INDENT();
                OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
                return;
            }

            PropertyNameList protoChildNames(Alloc());
            ResolvedItemList *childrenItems = ResolvedItemList::New(scriptContext->GetRecycler());

            // First get the properties from the prototype object

            Js::Var prototypeObjectVar = nullptr;
            BOOL ret = GetPropertyWithScriptEnter(functionValue, Js::PropertyIds::prototype, &prototypeObjectVar, scriptContext);
            if (ret)
            {
                Assert(prototypeObjectVar != nullptr);

                GetOwnPropertyName(prototypeObjectVar, this->fileAuthoring, [&](Js::Var propertyName)
                {
                    if (!InternalName(Js::JavascriptString::FromVar(propertyName)->GetSz())
                        && !IsConstructorProperty(propertyName, scriptContext))
                    {
                        ResolvedItem * resolvedItem = AnalysisOfMember(Js::RecyclableObject::FromVar(prototypeObjectVar), propertyName, scriptContext, includeMembers);
                        if (resolvedItem)
                        {
                            protoChildNames.Add(propertyName);
                            childrenItems->Add(resolvedItem);
                        }
                    }
                });
                PRINT_ARRAY1(L"protoChildNames", &protoChildNames);
            }

            // Get the static properties.

            bool hasStaticFunction = false;
            GetOwnPropertyName(functionValue, this->fileAuthoring, [&](Js::Var propertyName)
            {
                if (InternalName(Js::JavascriptString::FromVar(propertyName)->GetSz()))
                {
                    return;
                }

                if (!hasStaticFunction)
                {
                    const Js::PropertyRecord * propRecord = GetPropertyRecord(propertyName, scriptContext);
                    if (propRecord != nullptr && !IsEmptyFunctionOwnProperty(propRecord->GetPropertyId()))
                    {
                        Js::Var value = nullptr;
                        if (GetPropertyWithScriptEnter(Js::RecyclableObject::FromVar(functionValue), propRecord->GetPropertyId(), &value, scriptContext)
                            && value != nullptr
                            && Js::JavascriptFunction::Is(value))
                        {
                            hasStaticFunction = true;
                        }
                    }
                }
                ResolvedItem * resolvedItem = AnalysisOfMember(Js::RecyclableObject::FromVar(functionValue), propertyName, scriptContext, includeMembers);
                if (resolvedItem) childrenItems->Add(resolvedItem);
            });

            PRINT_ARRAY2(L"childrenItems", childrenItems);

            if (protoChildNames.Count() > 0 || hasStaticFunction)
            {
                Js::Var result = nullptr;
                Js::JavascriptFunction * settingFunction = JsHelpers::GetProperty<Js::JavascriptFunction*>(scriptContext->GetGlobalObject(), Names::callFunctionWithSettings, nullptr/*check this*/, scriptContext);
                // The settingFunction could be null if the helpers.js is not executed properly (let's say if Hurry is called while executing the helpers.js)
                if (settingFunction != nullptr)
                {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                    Js::HiResTimer timer;
                    double start = timer.Now();
#endif

                    JsHelpers::WithArguments([&](Js::Arguments& arguments) {
                        OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Calling function with settings : %ls \n", GetName(functionName));
                        this->fileAuthoring->ExecuteFunction(settingFunction, arguments, &result, /* doNotDisableLoopGuard = */true, /* doNotReportPhase = */true);
                    }, scriptContext, scriptContext->GetGlobalObject(), functionValue);

                    OUTPUT_TRACE(Js::JSLSStatsPhase, L"Calling function (%ls) with settings : time spent %8.3f\n", GetName(functionName), timer.Now() - start);
                    if (result != nullptr && result != scriptContext->GetLibrary()->GetUndefined())
                    {
                        OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Calling function with settings Has returned\n");
                        PropertyNameList instanceProperties(Alloc());

                        GetOwnPropertyName(result, this->fileAuthoring, [&](Js::Var propertyName)
                        {
                            if (InternalName(Js::JavascriptString::FromVar(propertyName)->GetSz()))
                            {
                                return;
                            }

                            ResolvedItem * resolvedItem = AnalysisOfMember(Js::RecyclableObject::FromVar(result), propertyName, scriptContext, includeMembers);
                            if (resolvedItem)
                            {
                                int index = FindNameInPropertyNameList(&protoChildNames, propertyName);
                                if (index != -1)
                                {
                                    childrenItems->Item(index, resolvedItem);
                                }
                                else
                                {
                                    childrenItems->Add(resolvedItem);
                                }
                            }
                        });
                    }

                    PRINT_ARRAY2(L"childrenItems", childrenItems);
                }

                // Add constructor function
                ResolvedItem * item1 = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), functionName, asnkFunction, functionValue, result, nullptr/*glyph*/, nullptr /*children*/);
                if (item1) childrenItems->Add(item1);

                // Add to the scope
                ResolvedItem * item2 = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), functionName, asnkClass, functionValue, instance, glyph, childrenItems);
                if (item2) resolvedItemList->Add(item2);
            }
            else if (includeMembers)
            {
                ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), functionName, asnkFunction, functionValue, instance, nullptr/*glyph*/, nullptr /*children*/);
                if (resolvedItem) resolvedItemList->Add(resolvedItem);
            }

            DECREASE_INDENT();
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
        }

        bool AnalyzeChildren(ResolvedItemList * resolvedItemList, Js::Var childValue, Js::Var childValueCtor)
        {
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeChildren {\n");
            INCREASE_INDENT();

            // Consideration : Why only Object and Array? (We could allow other builtin constructors such as String, Map etc)
            Assert(resolvedItemList != null);
            if (childValueCtor != nullptr
                && !IsCopyOnWriteProxy(childValue) 
                && (childValueCtor == (Js::Var)scriptContext->GetLibrary()->GetObjectConstructor() || childValueCtor == (Js::Var)scriptContext->GetLibrary()->GetArrayConstructor()))
            {
                int index = m_recursionMarkList->Add(childValue);
                AnalyzeValues(resolvedItemList, childValue, true);
                m_recursionMarkList->RemoveAt(index);
            }

            DECREASE_INDENT();
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");

            return resolvedItemList->Count() > 0;
        }

        void AnalyzeObject(ResolvedItemList * resolvedItemList, Js::Var objectName, Js::Var instance, Js::RecyclableObject *objectValue, Js::Var glyph, bool includeMembers)
        {
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeObject {\n");
            INCREASE_INDENT();

            Js::RecyclableObject * protoObject = objectValue->GetPrototype();
            Js::Var objectProtoCtor = nullptr;
            if (protoObject)
            {
                GetPropertyWithScriptEnter(protoObject, Js::PropertyIds::constructor, &objectProtoCtor, scriptContext);
            }

            ResolvedItemList *valueItemList = ResolvedItemList::New(scriptContext->GetRecycler());
            ResolvedItemList *valueProtoItemList = ResolvedItemList::New(scriptContext->GetRecycler());

            int index = m_recursionMarkList->Add(objectValue);

            if (protoObject != scriptContext->GetLibrary()->GetObjectPrototype()
                && protoObject != scriptContext->GetLibrary()->GetArrayPrototype()
                && AnalyzeChildren(valueProtoItemList, protoObject, objectProtoCtor))
            {
                AnalyzeChildren(valueProtoItemList, objectValue, objectProtoCtor);
                ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), objectName, asnkClass, protoObject, instance, glyph, valueProtoItemList /*children*/);
                if (resolvedItem) resolvedItemList->Add(resolvedItem);
            }
            else if (AnalyzeChildren(valueItemList, objectValue, objectProtoCtor))
            {
                ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), objectName, asnkNamespace, objectValue, instance, glyph, valueItemList /*children*/);
                if (resolvedItem) resolvedItemList->Add(resolvedItem);
            }
            else if (includeMembers)
            {
                ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), objectName, asnkField, objectValue, instance, glyph, nullptr /*children*/);
                if (resolvedItem) resolvedItemList->Add(resolvedItem);
            }
            m_recursionMarkList->RemoveAt(index);

            DECREASE_INDENT();
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
        }

        bool IsRecursionMarked(Js::Var item)
        {
            for (int i = 0; i < m_recursionMarkList->Count(); i++)
            {
                if (m_recursionMarkList->Item(i) == item)
                {
                    return true;
                }
            }
            return false;
        }

        bool IsTypeObject(Js::Var value)
        {
            return Js::RecyclableObject::Is(value) && (Js::RecyclableObject::FromVar(value)->GetTypeId() >= Js::TypeIds_Object
                && Js::RecyclableObject::FromVar(value)->GetTypeId() <= Js::TypeIds_LastBuiltinDynamicObject);
        }

        void AnalyzeProperty(ResolvedItemList * resolvedItemList, Js::Var instance, Js::JavascriptString * propertyName, bool includeMembers, bool skipContainerListItems)
        {
            const Js::PropertyRecord* propertyRecord = GetPropertyRecord(propertyName,scriptContext);
            if (propertyRecord == nullptr)
            {
                Assert(false); // Lets find out when this happens
                return;
            }
            Js::PropertyId propertyId = propertyRecord->GetPropertyId();
            const wchar_t * propName = propertyRecord->GetBuffer();

            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeProperty, propertyName : %ls\n", propName);
            if (propertyRecord->GetLength() == 0 || InternalName(propName))
            {
                return;
            }

            Js::PropertyDescriptor propertyDescriptor;
            bool isPropertyDefined = false;
            BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
            {
                IGNORE_STACKWALK_EXCEPTION(scriptContext);
                isPropertyDefined = !!Js::JavascriptObject::GetOwnPropertyDescriptorHelper(Js::RecyclableObject::FromVar(instance), propertyId, scriptContext, propertyDescriptor);
            }
            END_JS_RUNTIME_CALL(scriptContext);

            // If the object is getter and setter we will not be invoking getter to get the value. Instead we are going to show them as a field with the region.
            Js::Var value = nullptr;
            Js::Var auxValue = nullptr;
            bool isAccessorDefined = propertyDescriptor.GetterSpecified() || propertyDescriptor.SetterSpecified();
            if (isPropertyDefined && isAccessorDefined)
            {
                value = propertyDescriptor.GetterSpecified() ? propertyDescriptor.GetGetter() : propertyDescriptor.GetSetter();
                auxValue = propertyDescriptor.SetterSpecified() ? propertyDescriptor.GetSetter() : nullptr;
                if (value == scriptContext->GetLibrary()->GetDefaultAccessorFunction())
                {
                    Assert(auxValue != scriptContext->GetLibrary()->GetDefaultAccessorFunction());
                    value = auxValue;
                }
                if (IsCopyOnWriteProxy(value)) // If the getter is not used it may remain in the copyonwrite state, try with setter in that case.
                {
                    value = auxValue;
                }
            }

            if (value || GetPropertyWithScriptEnter(Js::RecyclableObject::FromVar(instance), propertyId, &value, scriptContext))
            {
                Assert(value != nullptr);
                if (IsCopyOnWriteProxy(value))
                {
                    return;
                }

                if (Js::RecyclableObject::Is(value))
                {
                    Js::Var isTypeDefGenerated = JsHelpers::GetPropertyVar(Js::RecyclableObject::FromVar(value), L"_$isTypeDefGenerated", scriptContext, true);
                    if (isTypeDefGenerated != nullptr)
                    {
                        return;
                    }
                }

                if (!IsRecursionMarked(value))
                {
                    if (skipContainerListItems && Js::JavascriptOperators::HasOwnProperty(value, m_inContainerListPropertyId, scriptContext))
                    {
                        return;
                    }

                    Js::Var glyph = nullptr;

                    if (Js::RecyclableObject::Is(value))
                    {
                        glyph = JsHelpers::GetPropertyVar(Js::RecyclableObject::FromVar(value), L"_$glyph", scriptContext, true);
                    }

                    if (isAccessorDefined && includeMembers)
                    {
                        ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), propertyName, asnkProperty, value, instance, glyph, nullptr /*children*/, auxValue);
                        if (resolvedItem) resolvedItemList->Add(resolvedItem);
                    }
                    else if (Js::JavascriptFunction::Is(value))
                    {
                        INCREASE_INDENT();
                        AnalyzeFunction(resolvedItemList, propertyName, instance, Js::JavascriptFunction::FromVar(value), glyph, includeMembers);
                        DECREASE_INDENT();
                    }
                    else if (IsTypeObject(value))
                    {
                        INCREASE_INDENT();
                        AnalyzeObject(resolvedItemList, propertyName, instance, Js::RecyclableObject::FromVar(value), glyph, includeMembers);
                        DECREASE_INDENT();
                    }
                    else if (includeMembers)
                    {
                        ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), propertyName, asnkField, value, instance, glyph, nullptr /*children*/);
                        if (resolvedItem) resolvedItemList->Add(resolvedItem);
                    }
                }
                else if (includeMembers)
                {
                    ResolvedItem * resolvedItem = ResolvedItem::GetResolvedItem(scriptContext->GetRecycler(), Alloc(), propertyName, asnkField, value, instance, nullptr/*glyph*/, nullptr /*children*/);
                    if (resolvedItem) resolvedItemList->Add(resolvedItem);
                }
            }

        }

        void AnalyzeValues(ResolvedItemList * resolvedItemList, Js::Var instance, bool includeMembers, bool skipContainerListItems = false)
        {
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeValues {\n");
            INCREASE_INDENT();
            GetOwnPropertyName(instance, this->fileAuthoring, [&](Js::Var propertyName)
            {
                Assert(Js::JavascriptString::Is(propertyName));
                if (Js::JavascriptString::Is(propertyName))
                {
                    AnalyzeProperty(resolvedItemList, instance, Js::JavascriptString::FromVar(propertyName), includeMembers, skipContainerListItems);
                }
            });

            DECREASE_INDENT();
            PRINT_INDENT();
            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"}\n");
        }

        void AnalyzeScriptContext(AuthoringFileHandle *file, Js::ScriptContext * scriptContext)
        {
            Assert(scriptContext != nullptr);
            m_recursionMarkList = RecursionMarkList::New(Alloc());

            INIT_INDENT();

            this->scriptContext = scriptContext;
            this->m_primaryFile = file;

            EnsurePropertyId(Names::inContainerList, m_inContainerListPropertyId);

            auto global = scriptContext->GetGlobalObject();

            // First we analyze the user defined containers
            Js::Var containerListVar = JsHelpers::GetPropertyVar(global, Names::containerList, scriptContext, true /*getOwnProperty*/);
            if (containerListVar != nullptr)
            {
                AnalyzeValues(m_topItemList, containerListVar, false);
            }

            OUTPUT_TRACE_2(Js::NavBarDynamicAnalysisPhase, L"Native-AnalyzeScriptContext\n");

            m_recursionMarkList->Add(global);
            AnalyzeValues(m_topItemList, global, false, true);
            PRINT_ARRAY2(L"m_topItemList", m_topItemList);
            OUTPUT_FLUSH();
        }

        void Old_AnalyzeScriptContext(ArenaAllocator *alloc, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree)
        {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::HiResTimer timer;
            double start = timer.Now();
#endif
            root = StructureNode::New(this, asnkRoot, nullptr);

            // Most of the actual work of dynamic scope anaysis happens in the _$analyzeClasses call in helpers.js. This calls that function
            // which returns the dynamic scopes as a tree. Old_RecordDynamicStructureScope deserializes that tree into root.
            JsHelpers::WithArguments([&](Js::Arguments& arguments) {

                auto func = JsHelpers::GetProperty<Js::JavascriptFunction*>(scriptContext->GetLibrary()->GetGlobalObject(), Names::analyzeClasses, nullptr, scriptContext);

                Js::Var result = nullptr;
                if (func)
                {
                    this->fileAuthoring->ExecuteFunction(func, arguments, &result);
                }

                OUTPUT_TRACE(Js::JSLSStatsPhase, L"Analyze using script execution time %8.3f\n", timer.Now() - start);
                if (result && Js::JavascriptArray::Is(result))
                {
                    Old_RecordDynamicStructureScope(alloc, Js::JavascriptArray::FromVar(result), this->root, scriptContext, file, tree);
                    RemoveDuplicateNamespaceMembers(scriptContext, tree);
                }

            }, scriptContext, scriptContext->GetLibrary()->GetGlobalObject());

        }

        class AutoReleaseResolvedItemList
        {
        public:
            AutoReleaseResolvedItemList(Recycler* recycler, ResolvedItemList* topItemList) : m_recycler(recycler), m_topItemList(topItemList)
            {
                Assert(recycler != nullptr);
                Assert(topItemList != nullptr);
                recycler->RootAddRef(m_topItemList);
            }
            ~AutoReleaseResolvedItemList()
            {
                this->m_recycler->RootRelease(this->m_topItemList);
            }
        private:
            Recycler* m_recycler;
            ResolvedItemList* m_topItemList;
        };

        void AnalyzeScriptContext(ArenaAllocator *alloc, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree)
        {
            try
            {
                if (CONFIG_FLAG(NativeADS))
                {
                    /// Analyzing using the native code,
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                    Js::HiResTimer timer;
                    double start = timer.Now();
#endif
                    OUTPUT_TRACE(Js::JSLSPhase, L"Starting Native-AnalyzeScriptContext\n");
                    Recycler * recycler = scriptContext->GetRecycler();
                    m_topItemList = ResolvedItemList::New(recycler);

                    AutoReleaseResolvedItemList AutoReleaseResolvedItemList(recycler, m_topItemList);
                    AnalyzeScriptContext(file, scriptContext);

                    OUTPUT_TRACE(Js::JSLSStatsPhase, L"Native-AnalyzeScriptContext  execution time %8.3f\n", timer.Now() - start);

                    RecordDynamicStructureScope(alloc, m_topItemList, this->root, scriptContext, file, tree);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                    if (PHASE_TRACE1(Js::NavBarDynamicAnalysisPhase))
                    {
                        this->root->Print();
                    }
#endif
                    RemoveDuplicateNamespaceMembers(scriptContext, tree);
                }

                if (CONFIG_FLAG(ScriptADS))
                {
                    Old_AnalyzeScriptContext(alloc, scriptContext, file, tree);
                }
            }
            catch (Js::JavascriptExceptionObject*)
            {
#ifdef DEBUG
                Output::Trace(Js::Phase::ParsePhase, L"Exception skipped");
#endif
            }
        }

        // IAuthorStructure implementation

        STDMETHOD(GetAllNodes)(IAuthorStructureNodeSet **result)
        {
            return CreateResultSet<CompleteStructureNodeSet>(result);
        }

        STDMETHOD(GetContainerNodes)(IAuthorStructureNodeSet **result)
        {
            return CreateResultSet<ContainersStructureNodeSet>(result);
        }

        STDMETHOD(GetChildrenOf)(int key, IAuthorStructureNodeSet **result)
        {
            return CreateResultSet<ChildrenStructureNodeSet>(result, key);
        }
    };

    TYPE_STATS(StructureResult, L"StructureResult")

        void CompleteStructureNodeSet::Initialize(StructureResult *result)
    {
            result->GetRoot()->TraverseAll([&](StructureNode *node) -> void
            {
                nodes.Add(node);
            });
        };

    void ContainersStructureNodeSet::Initialize(StructureResult *result)
    {
        result->GetRoot()->TraverseAll([&](StructureNode *node) -> void
        {
            if (node->IsContainer())
                nodes.Add(node);
        });
    };

    void ChildrenStructureNodeSet::Initialize(StructureResult *result)
    {
        if (node)
        {
            node->TraverseChildren([&](StructureNode *node) -> void
            {
                nodes.Add(node);
            });
        }
    };

    ArenaAllocator *GetAllocator(StructureResult *result)
    {
        return result->Alloc();
    }

    StructureNode *FindNode(StructureResult *result, int key)
    {
        return result->Find(key);
    }

    StructureNodeSet::StructureNodeSet(StructureResult *result) :
        SimpleComObjectWithAlloc<IAuthorStructureNodeSet>(result->GetPageAllocator(), L"ls:Structure"),
        nodes(Alloc(), 4) { }

    BOOL IsNaturalContainer(AuthorStructureNodeKind kind)
    {
        switch (kind)
        {
        case asnkGlobal:
        case asnkObjectLiteral:
        case asnkClass:
        case asnkNamespace:
        case asnkModule:
            return true;
        }
        return false;
    }

    StructureNode* StructureNode::New(StructureResult *result, IdentPtr name, AuthorStructureNodeKind kind, ParseNode *node)
    {
        return New(result, AllocInternalString(result->Alloc(), name), kind, node);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *itemName, IdentPtr containerName, AuthorStructureNodeKind kind, ParseNode *node)
    {
        return New(result, itemName, AllocInternalString(result->Alloc(), containerName), nullptr, kind, nullptr, node ? ActualMin(node) : 0, node ? ActualLim(node) : 0);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *itemName, Js::InternalString *containerName, AuthorStructureNodeKind kind, ParseNode *node)
    {
        return New(result, itemName, containerName, nullptr, kind, nullptr, node ? ActualMin(node) : 0, node ? ActualLim(node) : 0);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *name, AuthorStructureNodeKind kind, ParseNode *node)
    {
        return New(result, name, nullptr, nullptr, kind, nullptr, node ? ActualMin(node) : 0, node ? ActualLim(node) : 0);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *name, Js::InternalString *customKind, ParseNode *node)
    {
        return New(result, name, nullptr, nullptr, asnkCustom, customKind, node ? ActualMin(node) : 0, node ? ActualLim(node) : 0);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *name, AuthorStructureNodeKind kind, charcount_t min, charcount_t lim)
    {
        return New(result, name, nullptr, nullptr, kind, nullptr, min, lim);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *name, Js::InternalString *customKind, charcount_t min, charcount_t lim)
    {
        return New(result, name, nullptr, nullptr, asnkCustom, customKind, min, lim);
    }

    StructureNode* StructureNode::New(StructureResult *result, Js::InternalString *itemName, Js::InternalString *containerName, Js::InternalString *glyph, AuthorStructureNodeKind kind, Js::InternalString *customKind, charcount_t min, charcount_t lim)
    {
        if (!containerName && itemName && IsNaturalContainer(kind))
        {
            containerName = itemName;
            itemName = nullptr;
        }
        else if (!itemName && containerName && !IsNaturalContainer(kind))
        {
            itemName = containerName;
            containerName = nullptr;
        }

        if (itemName && containerName)
        {
            if (IsNaturalContainer(kind))
            {
                itemName = nullptr;
            }
            else
            {
                containerName = nullptr;
            }
        }

        return Anew(result->Alloc(), StructureNode, result, itemName, containerName, glyph, kind, customKind, min, lim);
    }

    IAuthorStructure *NewStructure(FileAuthoring *fileAuthoring)
    {
        RefCountedPtr<StructureResult> result;
        result.TakeOwnership(new StructureResult(fileAuthoring));
        return result.Detach();
    }

    void AddStaticStructure(IAuthorStructure *authorStructure, ArenaAllocator *alloc, ParseNode *rootNode)
    {
        auto result = static_cast<StructureResult *>(authorStructure);
        result->AnalyzeStructure(alloc, rootNode);
    }

    void AddDynamicStructure(IAuthorStructure *authorStructure, ArenaAllocator *alloc, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree)
    {
        auto result = static_cast<StructureResult *>(authorStructure);
        result->AnalyzeScriptContext(alloc, scriptContext, file, tree);
    }
}

