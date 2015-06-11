//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

AuthorParseNodeFlags GetAuthorParseNodeFlags(ParseNodePtr pnode)
{
    Assert(pnode);

    uint flags = AuthorParseNodeFlags::apnfNone;

    if (pnode->grfpn & PNodeFlags::fpnExplicitSimicolon)
        flags |= AuthorParseNodeFlags::apnfExplicitSimicolon;
    else if (pnode->grfpn & PNodeFlags::fpnAutomaticSimicolon)
        flags |= AuthorParseNodeFlags::apnfAutomaticSimicolon;
    else if (pnode->grfpn & PNodeFlags::fpnMissingSimicolon)
        flags |= AuthorParseNodeFlags::apnfMissingSimicolon;

    if (pnode->grfpn & PNodeFlags::fpnSyntheticNode)
        flags |= AuthorParseNodeFlags::apnfSyntheticNode;

    if (pnode->nop == knopFncDecl && pnode->sxFnc.IsSubsumed())
        flags |= AuthorParseNodeFlags::apnfSubsumedFunction;

    return static_cast<AuthorParseNodeFlags>(flags);
}

/******************************************************************
                     JSON Serializer
*******************************************************************/

#define Append(buffer, x) buffer->Add(x)
#define Append2(buffer, x,y) buffer->Add(x,y)

static const wchar_t *JsonNames[] = {
#define PTNODE(nop,sn,pc,nk,ok,json,apnk) OLESTR(json),
#include "ptlist.h"
#undef PTNODE 
};

static const wchar_t *ChildLabelNames[] = {
#define PNELABEL(label,name,apnel)  OLESTR(name),
#include "PNELABEL.h"
#undef PNELABEL
};

using namespace Authoring;

struct JSONParseNodeSerializerContext
{
        TextBuffer* buffer;
        ParseNodeTree* parseTree;

        JSONParseNodeSerializerContext(TextBuffer* buffer, ParseNodeTree* parseTree, ArenaAllocator* alloc)
            :buffer(buffer),
            parseTree(parseTree)
        {
        }
};

struct JSONParseNodeSerializerPolicy : public SerializerPolicyBase<JSONParseNodeSerializerContext>
{
private:
    static bool HasSpecialChars(const wchar_t *text, size_t len)
    {
        for (size_t i = 0; i < len; i++) 
        {
            wchar_t ch = text[i];
            switch (ch)
            {
            case '"':
            case '\\':
            case '\b': 
            case '\f': 
            case '\n': 
            case '\r': 
            case '\t':
                return true;
            default:
                if (ch < ' ') return true;
            }
        }
        return false;
    }

    static void AddEncodedStringLiteral(TextBuffer *buffer, const wchar_t *text, size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            wchar_t ch = text[i];
            switch (ch)
            {
            case '"': buffer->Add(L"\\\""); break;
            case '\\': buffer->Add(L"\\\\"); break;
            case '\b': buffer->Add(L"\\b"); break;
            case '\f': buffer->Add(L"\\f"); break;
            case '\n': buffer->Add(L"\\n"); break;
            case '\r': buffer->Add(L"\\r"); break;
            case '\t': buffer->Add(L"\\t"); break;
            default:
                if (ch < ' ' )
                {
                    buffer->Add('\\');
                    buffer->Add('u');
                    buffer->Add4Hex(ch);
                }
                else
                    buffer->Add(ch);
            }
        }
    }

    static void AppendString(TextBuffer* buffer, wchar_t const *text, charcount_t len)
    {
        Append(buffer, '"');
        if (!HasSpecialChars(text, len))
            Append2(buffer, text, len);
        else
            AddEncodedStringLiteral(buffer, text, len);
        Append(buffer, '"');
    }

    static void AppendPid(TextBuffer *buffer, IdentPtr name)
    {
        AppendString(buffer, name->Psz(), name->Cch());
    }

    static void AppendInt(TextBuffer *buffer, long value)
    {
        buffer->AddInt(value);
    }

    static bool EqualDouble(double value1, double value2)
    {
        ulong high1 = Js::NumberUtilities::LuHiDbl(value1);
        ulong low1 = Js::NumberUtilities::LuLoDbl(value1);

        ulong high2 = Js::NumberUtilities::LuHiDbl(value2);
        ulong low2 = Js::NumberUtilities::LuLoDbl(value2);

        return ((high1 == high2) && (low1 == low2));
    }

    static void AppendDouble(TextBuffer *buffer, double value)
    {
        buffer->AddDouble(value);
    }

#if 0
    static bool InternalName(IdentPtr name)
    {
        return name && name->Cch() >= 2 && name->Psz()[0] == '_' && name->Psz()[1] == '$';
    }
#endif

    static void AppendLocation(TextBuffer *buffer, ParseNode *node)
    {
        if (node && (node->ichLim > node->ichMin || (node->nop == knopProg)))
        {
            auto min = ActualMin(node);
            Append(buffer, L", \"offset\": ");
            AppendInt(buffer, min);
            Append(buffer, L", \"length\": ");
            AppendInt(buffer, ActualLim(node) - min);
        }
    }

    static void AppendStatementProperties(TextBuffer *buffer, ParseNodeTree* parseTree, ParseNode *pnode)
    {
        if (pnode != nullptr)
        {
            if (pnode->grfpn & PNodeFlags::fpnAutomaticSimicolon)
            {
                Append(buffer, L", \"semicolon\": \"automatic\"");
            }
            else if (pnode->grfpn & PNodeFlags::fpnMissingSimicolon)
            {
                Append(buffer, L", \"semicolon\": \"missing\"");
            }

            // Append label
            LPCWSTR label = parseTree->LanguageServiceExtension()->GetLabel(pnode);
            if (label  && *label)
            {
                Append(buffer, L", \"label\": \"");
                Append(buffer, label);
                Append(buffer, '"');
            }

            int parenCount = parseTree->LanguageServiceExtension()->GetParenthesesCount(pnode);
            if (parenCount > 0)
            {
                Append(buffer, L", \"parencount\": ");
                AppendInt(buffer, parenCount);
            }

            if (pnode->nop == knopBlock && pnode->grfpn & PNodeFlags::fpnSyntheticNode)
            {
                Append(buffer, L", \"autogenerated\": \"true\"");
            }
        }
    }

    static bool ShowAsList(ParseNodePtr pnode, ParseNodeEdge edgeLabel)
    {
        bool showAsList = false;
        switch(edgeLabel)
        {
            case ParseNodeEdge::pneBlockBody:
            case ParseNodeEdge::pneElements:
            case ParseNodeEdge::pneArguments:
            case ParseNodeEdge::pneMembers:
                showAsList = true;
                break;
        };

        if (pnode && pnode->nop == knopList)
        {
            showAsList = true;
        }

        return showAsList;
    }

protected:

    inline void PreSerializeNodeListItem(ParseNodePtr pnode, bool isFirst, JSONParseNodeSerializerContext* context) 
    {
        if (!isFirst)
            Append(context->buffer, ',');
    }

    inline void SerializeTargetLabel(ParseNodePtr pnodeTarget, JSONParseNodeSerializerContext* context)
    {
        LPCWSTR targetLabel = context->parseTree->LanguageServiceExtension()->GetLabel(pnodeTarget);
        if (targetLabel && *targetLabel)
        {
            Append(context->buffer, L", \"targetlabel\": \"");
            Append(context->buffer, targetLabel);
            Append(context->buffer, '"');
        }
    }

    inline void SerializeName(IdentPtr id, JSONParseNodeSerializerContext* context)
    {
        // Don't Append the value of internal names.
        if (InternalName(id))
            return;

        Append(context->buffer, L", \"name\": ");
        AppendPid(context->buffer, id);
    }

    inline void SerializeNameNode(ParseNodePtr pnode, JSONParseNodeSerializerContext* context)
    {
        // Don't Append the value of internal names.
        if (InternalName(pnode->sxPid.pid))
            return;

        Append(context->buffer, L", \"value\": ");
        AppendPid(context->buffer, pnode->sxPid.pid);
    }

    inline void SerializeStr(ParseNodePtr pnode, JSONParseNodeSerializerContext* context)
    {
        Append(context->buffer, L", \"value\": ");
        AppendPid(context->buffer, pnode->sxPid.pid);
    }

    inline void SerializeInt(ParseNodePtr pnode, JSONParseNodeSerializerContext* context)
    {
        Append(context->buffer, L", \"value\": ");
        AppendInt(context->buffer, pnode->sxInt.lw);
    }

    inline void SerializeFlt(ParseNodePtr pnode, JSONParseNodeSerializerContext* context)
    {
        Append(context->buffer, L", \"value\": ");

        // NaN, Infinity, and -Inifity are not valid JSON values, so use properties to identify them
        if (EqualDouble(pnode->sxFlt.dbl, Js::JavascriptNumber::NaN))
        {
            Append(context->buffer, L"0 , \"isNaN\": true");
        }
        else if (EqualDouble(pnode->sxFlt.dbl, Js::JavascriptNumber::POSITIVE_INFINITY))
        {
            Append(context->buffer, L"0 , \"isInfinity\": true");
        }
        else if (EqualDouble(pnode->sxFlt.dbl, Js::JavascriptNumber::NEGATIVE_INFINITY))
        {
            Append(context->buffer, L"0 , \"isNegativeInfinity\": true");
        }
        else
            AppendDouble(context->buffer, pnode->sxFlt.dbl);
    }

    inline void SerializeRegExp(ParseNodePtr pnode, JSONParseNodeSerializerContext* context)
    {
        UnifiedRegex::RegexPattern *pattern = pnode->sxPid.regexPattern;
        Js::InternalString regexSource = pattern->GetSource();
        Append(context->buffer, L", \"value\": ");
        AppendString(context->buffer, regexSource.GetBuffer(), regexSource.GetLength());

        if (pattern->IsIgnoreCase() || pattern->IsGlobal() || pattern->IsMultiline() || pattern->IsUnicode() || pattern->IsSticky())
        {
            Append(context->buffer, L", \"options\": \"");
            if (pattern->IsGlobal())
                Append(context->buffer, 'g');
            if (pattern->IsIgnoreCase())
                Append(context->buffer, 'i');
            if (pattern->IsMultiline())
                Append(context->buffer, 'm');
            if (pattern->IsUnicode())
                Append(context->buffer, 'u');
            if (pattern->IsSticky())
                Append(context->buffer, 'y');
            Append(context->buffer, '"');
        }
    }

    inline bool PreSerializeCases(ParseNodePtr pnodeCases, ParseNodePtr pnodeDefault, JSONParseNodeSerializerContext* context) 
    {
        if (pnodeCases)
        {
            Append(context->buffer, L", \"cases\": [");
        }

        return true;
    }

    inline void PostSerializeCases(ParseNodePtr pnodeCases, ParseNodePtr pnodeDefault, JSONParseNodeSerializerContext* context) 
    {
        if (pnodeCases)
        {
            Append(context->buffer, ']');
        }
    }

    inline bool PreSerializeArguments(ParseNodePtr pnodeArguments, JSONParseNodeSerializerContext* context) 
    {
        if (pnodeArguments)
        {
            Append(context->buffer, L", \"arguments\": [");
        }

        return true;
    }

    inline void PostSerializeArguments(ParseNodePtr pnodeArguments, JSONParseNodeSerializerContext* context) 
    {
        if (pnodeArguments)
        {
            Append(context->buffer, ']');
        }
    }

    inline bool PreSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, JSONParseNodeSerializerContext* context) 
    {
        LPCWSTR childName = ChildLabelNames[edgeLabel];
        if (childName && *childName)
        {
            Append(context->buffer, L", \"");
            Append(context->buffer, childName);
            Append(context->buffer, L"\":");
        }

        if (ShowAsList(pnode, edgeLabel))
        {
            Append(context->buffer, '[');
        }

        const wchar_t *name = JsonNames[pnode->nop];
        if (name && *name) 
        {
            Append(context->buffer, L"{\"__type\": \"");
            Append(context->buffer, name);
            Append(context->buffer, '\"');
            AppendLocation(context->buffer, pnode);
            AppendStatementProperties(context->buffer, context->parseTree, pnode);

            if (ParseNodeSerializerHelpers::IsDeclList(pnode->nop))
                Append(context->buffer, L", \"list\": [");
        }

        return true; // keep serializing
    }

    inline void PostSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, JSONParseNodeSerializerContext* context) 
    {
        const wchar_t *name = JsonNames[pnode->nop];
        if (name && *name) 
        {
            if (ParseNodeSerializerHelpers::IsDeclList(pnode->nop))
                Append(context->buffer, ']');
            Append(context->buffer, '}');
        }
        
        if (ShowAsList(pnode, edgeLabel))
        {
                Append(context->buffer, ']');
        }
    }
};

HRESULT Authoring::FileAuthoring::SerializeTreeIntoJSON(TextBuffer *buffer, ArenaAllocator* alloc)
{
    METHOD_PREFIX;

    JSONParseNodeSerializerContext serializerContext(buffer, &m_primaryTree, alloc);
    ParseNodeSerializer<JSONParseNodeSerializerPolicy> serializer;
    serializer.SerializeNode(m_primaryTree.TreeRoot(), ParseNodeEdge::pneNone, &serializerContext);

    METHOD_POSTFIX;
}

/******************************************************************
                        List Serializer
*******************************************************************/
static const AuthorParseNodeKind AuthorParseNodeKinds[] = {
        #define PTNODE(nop,sn,pc,nk,ok,json,apnk) apnk,
        #include "ptlist.h"
        #undef PTNODE
};

static const AuthorParseNodeEdge ChildLabelToAuthorChildLabelMap[] = {
#define PNELABEL(label,name,apnel)  apnel,
#include "PNELABEL.h"
#undef PNELABEL
};

struct ListParseNodeSerializerContext 
{
public:
    JsUtil::List<AuthorParseNode*, ArenaAllocator> *nodes;
    Js::ScriptContext* scriptContext;
    ParseNodeTree* parseTree;
    long currentLevel;
    long maxLevel;
    AuthorParseNode* currentNode;
    ArenaAllocator* alloc;

    ListParseNodeSerializerContext(JsUtil::List<AuthorParseNode*, ArenaAllocator> *nodes, Js::ScriptContext* scriptContext, ParseNodeTree* parseTree,  ArenaAllocator* alloc, long maxLevel)
        : nodes(nodes),
        scriptContext(scriptContext),
        parseTree(parseTree),
        currentLevel(0),
        currentNode(NULL),
        alloc(alloc),
        maxLevel(maxLevel)
    {
    }
};

struct ListParseNodeSerializerPolicy : public SerializerPolicyBase<ListParseNodeSerializerContext>
{
private:
   
    int InsertNode(AuthorParseNode* serializedNode, ListParseNodeSerializerContext* context)
    {
        if (serializedNode == nullptr)
            return 0;

        int index = context->nodes->Count();
        context->nodes->Add(serializedNode);
        return index;
    }

    AuthorParseNode* CreateNode(long min, long lim, AuthorParseNodeKind kind, LPCWSTR label,  ParseNodeEdge edgeLabel, AuthorParseNodeFlags flags, ListParseNodeSerializerContext* context)
    {
        AuthorParseNode* serializedNode = reinterpret_cast<AuthorParseNode*> (context->alloc->Alloc(sizeof(AuthorParseNode)));
        serializedNode->details.kind = kind;
        serializedNode->details.startOffset = min;
        serializedNode->details.endOffset = lim;
        serializedNode->details.flags = flags;
        serializedNode->name = 0;
        serializedNode->level = context->currentLevel;
        serializedNode->edgeLabel = ChildLabelToAuthorChildLabelMap[edgeLabel];

        if (label && *label)
        {
            serializedNode->label = context->scriptContext->GetOrAddPropertyIdTracked(label, ::wcslen(label));
        }
        else
        {
            serializedNode->label = 0;
        }
        return serializedNode;
    }

    Js::PropertyId StoreString(LPCWSTR text, ListParseNodeSerializerContext* context)
    {
        return context->scriptContext->GetOrAddPropertyIdTracked(text, ::wcslen(text));
    }

    bool BeginSerializeNode(AuthorParseNode* apnode, Context* context)
    {
        // maxlevel < 0 implies recursive
        if ((context->maxLevel >= 0) && (context->currentLevel > context->maxLevel))
        {
            return false; // max level reached; stop serializing
        }

        // insert node in the output list
        InsertNode(apnode, context);

        // Note: This value will not be valid after the first child is serialized. 
        //       If the parent is needed throughout all child node serializations a stack should be used.
        //       This is only used for serializing names for varibales and functions, which is garuanteed
        //       to be the first child.
        context->currentNode = apnode;

        Assert(context->currentLevel >= 0);
        context->currentLevel ++;

        return true;    // keep serialzing
    }

    void EndSerializeNode(Context* context)
    {
        context->currentLevel --;
        Assert(context->currentLevel >= 0);
    }

protected:

    void SerializeTargetLabel(ParseNodePtr pnodeTarget, ListParseNodeSerializerContext* context)
    {
        Assert(context->currentNode);

        LPCWSTR targetLabel = context->parseTree->LanguageServiceExtension()->GetLabel(pnodeTarget);
        if (targetLabel && *targetLabel)
        {
            context->currentNode->name = StoreString(targetLabel, context);
        }
    }

    void SerializeName(IdentPtr id, ListParseNodeSerializerContext* context)
    {
        Assert(context->currentNode);

        if (id != NULL)
        {
            context->currentNode->name = StoreString(id->Psz(), context);
        }
    }

    void SerializeNameNode(ParseNodePtr pnode, ListParseNodeSerializerContext* context)
    {
        SerializeName(pnode->sxPid.pid, context);
    }

    bool PreSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, ListParseNodeSerializerContext* context) 
    {
        // create a new AuthorParseNode for the current node
        LPCWSTR label = context->parseTree->LanguageServiceExtension()->GetLabel(pnode);
        AuthorParseNode* serializedNode = CreateNode(ActualMin(pnode), ActualLim(pnode), AuthorParseNodeKinds[pnode->nop], label, edgeLabel, GetAuthorParseNodeFlags(pnode), context);

        return BeginSerializeNode(serializedNode, context);
    }

    void PostSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, ListParseNodeSerializerContext* context) 
    {
        EndSerializeNode(context);
    }
};


/******************************************************************
                        ParseNodeCursor
*******************************************************************/
template <class Context>
struct EdgeLabelProcessorPolicyBase
{
    typedef Context Context;

protected:
    inline bool ProcessIndexedChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, int index, Context* context) { return false; }
    inline bool ProcessChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, Context* context) { return false; }
    inline bool FailOnNode(ParseNodePtr pnode, Context* context) { return true;}
};

template<class EdgeLabelProcessorPolicy>
class EdgeLabelProcessor : EdgeLabelProcessorPolicy
{
    typedef typename EdgeLabelProcessorPolicy::Context EdgeLabelProcessorContext;

    HRESULT ProcessNodeInAList (ParseNodePtr list, EdgeLabelProcessorContext* context)
    {
        OpCode listActualNop = ParseNodeSerializerHelpers::ActualNop(list);
        int index = 0;
        for (ParseNode *item = list; item; item = item->sxBin.pnode2)
        {
            OpCode actualNop = ParseNodeSerializerHelpers::ActualNop(item);

            if (actualNop == knopEndCode) break;

            // Skip list nodes and nested VarDeclList nodes
            if (actualNop == knopList || 
                (ParseNodeSerializerHelpers::IsDeclList(actualNop) && ParseNodeSerializerHelpers::IsDeclList(listActualNop)))
            {
                if (ProcessIndexedChildNode(item->sxBin.pnode1, AuthorParseNodeEdge::apneListItem, index, context))
                {
                    return S_OK;
                }
            }
            else
            {
                // last iteration
                if (ProcessIndexedChildNode(item, AuthorParseNodeEdge::apneListItem, index, context))
                {
                    return S_OK;
                }
                else
                {
                    break;
                }
            }
            index++;
        }

        if (FailOnNode(list, context))
            return E_INVALIDARG;
        else
            return S_OK;
    }

public:
    HRESULT ProcessNode(ParseNodePtr pnode, EdgeLabelProcessorContext* context)
    {
        if (pnode == nullptr)
        {
            return E_FAIL;
        }

        OpCode actualNop = ParseNodeSerializerHelpers::ActualNop(pnode);

        switch (actualNop)
        {
            case knopName:
            case knopStr: 
            case knopInt:
            case knopFlt:
            case knopRegExp:
            case knopThis:
            case knopNull:
            case knopFalse:
            case knopTrue:
            case knopEmpty:
            case knopEndCode:
            case knopDebugger:
            case knopLabel:
                goto Invalid;
                break;

            case knopBreak:
            case knopContinue:
                if (!ProcessChildNode(pnode->sxJump.pnodeTarget, AuthorParseNodeEdge::apneTarget, context))
                {
                    goto Invalid;
                }
                break;

            case knopNot:
            case knopNeg:
            case knopPos:
            case knopLogNot:
            case knopIncPre:
            case knopDecPre:
            case knopTypeof:
            case knopVoid:
            case knopDelete:
            case knopIncPost:
            case knopDecPost:
                if (!ProcessChildNode(pnode->sxUni.pnode1, AuthorParseNodeEdge::apneOperand, context))
                {
                    goto Invalid;
                }
                break;

            case knopArray:
                if (!ProcessChildNode(pnode->sxUni.pnode1, AuthorParseNodeEdge::apneElements, context))
                {
                    goto Invalid;
                }
                break;

             case knopObject:
                 if (!ProcessChildNode(pnode->sxUni.pnode1, AuthorParseNodeEdge::apneMembers, context))
                {
                    goto Invalid;
                }
                break;

            case knopGetMember:
            case knopSetMember:
                if (!ProcessChildNode(pnode->sxBin.pnode1, AuthorParseNodeEdge::apneTarget, context) &&
                    !ProcessChildNode(pnode->sxBin.pnode2, AuthorParseNodeEdge::apneValue, context))
                {
                    goto Invalid;
                }
                break;

            case knopAdd:
            case knopSub:
            case knopMul:
            case knopDiv:
            case knopMod:
            case knopOr:
            case knopXor:
            case knopAnd:
            case knopEq:
            case knopNe:
            case knopLt:
            case knopLe:
            case knopGe:
            case knopGt:
            case knopDot:
            case knopAsg:
            case knopInstOf:
            case knopIn:
            case knopEqv:
            case knopNEqv:
            case knopComma:
            case knopLogOr:
            case knopLogAnd:
            case knopLsh:
            case knopRsh:
            case knopRs2:
            case knopAsgAdd:
            case knopAsgSub:
            case knopAsgMul:
            case knopAsgDiv:
            case knopAsgMod:
            case knopAsgAnd:
            case knopAsgXor:
            case knopAsgOr:
            case knopAsgLsh:
            case knopAsgRsh:
            case knopAsgRs2:
            case knopScope:
                if (!ProcessChildNode(pnode->sxBin.pnode1, AuthorParseNodeEdge::apneLeft, context) &&
                    !ProcessChildNode(pnode->sxBin.pnode2, AuthorParseNodeEdge::apneRight, context))
                {
                    goto Invalid;
                }
                break;

            case knopMember:
            if (!ProcessChildNode(pnode->sxBin.pnode1, AuthorParseNodeEdge::apneTarget, context) &&
                !ProcessChildNode(pnode->sxBin.pnode2, AuthorParseNodeEdge::apneMember, context))
            {
                goto Invalid;
            }
            break;

            case knopCall:
            case knopNew:
                if (!ProcessChildNode(pnode->sxCall.pnodeTarget, AuthorParseNodeEdge::apneTarget, context) &&
                    !ProcessChildNode(pnode->sxCall.pnodeArgs, AuthorParseNodeEdge::apneArguments, context))
                {
                    goto Invalid;
                }
                break;

            case knopIndex:
                if (!ProcessChildNode(pnode->sxCall.pnodeTarget, AuthorParseNodeEdge::apneTarget, context) &&
                    !ProcessChildNode(pnode->sxCall.pnodeArgs, AuthorParseNodeEdge::apneValue, context))
                {
                    goto Invalid;
                }
                break;

            case knopQmark:
                if (!ProcessChildNode(pnode->sxTri.pnode1, AuthorParseNodeEdge::apneCondition, context) &&
                    !ProcessChildNode(pnode->sxTri.pnode2, AuthorParseNodeEdge::apneThen, context) &&
                    !ProcessChildNode(pnode->sxTri.pnode3, AuthorParseNodeEdge::apneElse, context))
                {
                    goto Invalid;
                }
                break;

            case knopProg:
                if (!ProcessChildNode(pnode->sxProg.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopFor:
                if (!ProcessChildNode(pnode->sxFor.pnodeInit, AuthorParseNodeEdge::apneInitialization, context) &&
                    !ProcessChildNode(pnode->sxFor.pnodeCond, AuthorParseNodeEdge::apneCondition, context) &&
                    !ProcessChildNode(pnode->sxFor.pnodeIncr, AuthorParseNodeEdge::apneIncrement, context) &&
                    !ProcessChildNode(pnode->sxFor.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopIf:
                if (!ProcessChildNode(pnode->sxIf.pnodeCond, AuthorParseNodeEdge::apneCondition, context) &&
                    !ProcessChildNode(pnode->sxIf.pnodeTrue, AuthorParseNodeEdge::apneThen, context) &&
                    !ProcessChildNode(pnode->sxIf.pnodeFalse, AuthorParseNodeEdge::apneElse, context))
                {
                    goto Invalid;
                }
                break;

            case knopWhile:
            case knopDoWhile:
                if (!ProcessChildNode(pnode->sxWhile.pnodeCond, AuthorParseNodeEdge::apneCondition, context) &&
                    !ProcessChildNode(pnode->sxWhile.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopForIn:
            case knopForOf:
                if (!ProcessChildNode(pnode->sxForInOrForOf.pnodeLval, AuthorParseNodeEdge::apneVariable, context) &&
                    !ProcessChildNode(pnode->sxForInOrForOf.pnodeObj, AuthorParseNodeEdge::apneObject, context) &&
                    !ProcessChildNode(pnode->sxForInOrForOf.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopReturn:
                if (!ProcessChildNode(pnode->sxReturn.pnodeExpr, AuthorParseNodeEdge::apneValue, context))
                {
                    goto Invalid;
                }
                break;

            case knopConstDeclList:
            case knopLetDeclList:
            case knopList:
            case knopVarDeclList:
                return ProcessNodeInAList(pnode, context);
                break;

            case knopVarDecl:
            case knopLetDecl:
            case knopConstDecl:
                if (!ProcessChildNode(pnode->sxVar.pnodeInit, AuthorParseNodeEdge::apneInitialization, context))
                {
                    goto Invalid;
                }
                break;

            case knopFncDecl:
                 {
                    // loop over all the arguments
                    int index = 0;
                    ParseNodePtr pnodeArg = nullptr;
                    for (pnodeArg = pnode->sxFnc.pnodeArgs; pnodeArg; pnodeArg = pnodeArg->sxVar.pnodeNext)
                    {
                        if (ProcessIndexedChildNode(pnodeArg, AuthorParseNodeEdge::apneArgument, index, context))
                        {
                            break;
                        }
                        index ++;
                    }

                    if (pnode->sxFnc.pnodeRest != nullptr)
                    {
                        pnodeArg = pnode->sxFnc.pnodeRest;
                        if (ProcessIndexedChildNode(pnodeArg, AuthorParseNodeEdge::apneArgument, index, context))
                        {
                            break;
                        }
                    }

                    if (!pnodeArg)
                    {
                        if (pnode->sxFnc.pnodeBody && pnode->sxFnc.pnodeBody->nop != knopEndCode &&
                            !ProcessChildNode(pnode->sxFnc.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                        {
                            if (FailOnNode(pnode, context))
                                goto Invalid;
                        }
                    }
                }
                break;

            case knopBlock:
                if (!ProcessChildNode(pnode->sxBlock.pnodeStmt, AuthorParseNodeEdge::apneBlockBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopWith:
                if (!ProcessChildNode(pnode->sxWith.pnodeObj, AuthorParseNodeEdge::apneObject, context) &&
                    !ProcessChildNode(pnode->sxWith.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopSwitch:
                if (!ProcessChildNode(pnode->sxSwitch.pnodeVal, AuthorParseNodeEdge::apneValue, context) &&
                    !ProcessChildNode(pnode->sxSwitch.pnodeDefault, AuthorParseNodeEdge::apneDefaultCase, context))
                {
                    // loop over all the cases
                    int index = 0;
                    ParseNodePtr pnodeCase = nullptr;
                    for (pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext)
                    {
                        if (ProcessIndexedChildNode(pnodeCase, AuthorParseNodeEdge::apneCase, index, context))
                        {
                            break;
                        }
                        index ++;
                    }

                    if (!pnodeCase && FailOnNode(pnode, context))
                    {
                        goto Invalid;
                    }
                }
                break;

            case knopCase:
                if (!ProcessChildNode(pnode->sxCase.pnodeExpr, AuthorParseNodeEdge::apneValue, context) &&
                    !ProcessChildNode(pnode->sxCase.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopTryFinally:
                if (!ProcessChildNode(pnode->sxTryFinally.pnodeTry, AuthorParseNodeEdge::apneTry, context) &&
                    !ProcessChildNode(pnode->sxTryFinally.pnodeFinally, AuthorParseNodeEdge::apneFinally, context))
                {
                    goto Invalid;
                }
                break;

            case knopFinally:
                if (!ProcessChildNode(pnode->sxFinally.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopCatch:
                if (!ProcessChildNode(pnode->sxCatch.pnodeParam, AuthorParseNodeEdge::apneVariable, context) &&
                    !ProcessChildNode(pnode->sxCatch.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;

            case knopTryCatch:
                if (!ProcessChildNode(pnode->sxTryCatch.pnodeTry, AuthorParseNodeEdge::apneTry, context) &&
                    !ProcessChildNode(pnode->sxTryCatch.pnodeCatch, AuthorParseNodeEdge::apneCatch, context))
                {
                    goto Invalid;
                }
                break;

            case knopTry:
                if (!ProcessChildNode(pnode->sxTry.pnodeBody, AuthorParseNodeEdge::apneBody, context))
                {
                    goto Invalid;
                }
                break;


            case knopThrow:
                if (!ProcessChildNode(pnode->sxUni.pnode1, AuthorParseNodeEdge::apneValue, context))
                {
                    goto Invalid;
                }
                break;

            case knopClassDecl:
                if (!ProcessChildNode(pnode->sxClass.pnodeExtends, AuthorParseNodeEdge::apneExtends, context) &&
                    !ProcessChildNode(pnode->sxClass.pnodeConstructor, AuthorParseNodeEdge::apneCtor, context) &&
                    !ProcessChildNode(pnode->sxClass.pnodeStaticMembers, AuthorParseNodeEdge::apneStaticMembers, context) &&
                    !ProcessChildNode(pnode->sxClass.pnodeMembers, AuthorParseNodeEdge::apneMembers, context))
                {
                    goto Invalid;
                }
                break;
            case knopStrTemplate:
                if (!ProcessChildNode(pnode->sxStrTemplate.pnodeStringLiterals, AuthorParseNodeEdge::apneStringLiterals, context) &&
                    !ProcessChildNode(pnode->sxStrTemplate.pnodeSubstitutionExpressions, AuthorParseNodeEdge::apneSubstitutionExpression, context) &&
                    !ProcessChildNode(pnode->sxStrTemplate.pnodeStringRawLiterals, AuthorParseNodeEdge::apneStringRawLiterals, context))
                {
                    goto Invalid;
                }
                break;
        };

        return S_OK;

Invalid:
        return E_INVALIDARG;
    }
};

class AuthorParseNodeSet : public SimpleComObjectWithAlloc<IAuthorParseNodeSet>
{
private:
    typedef JsUtil::List<AuthorParseNode*, ArenaAllocator> NodeList;

    NodeList *m_nodeList;
    ParseNodePtr m_rootNode;
    Js::ScriptContext* m_scriptContext;
    ParseNodeTree* m_parseTree;

public:
    AuthorParseNodeSet(PageAllocator* pageAlloc, ParseNodePtr rootNode, ParseNodeTree* parseTree, Js::ScriptContext* scriptContext, long depth)
        : SimpleComObjectWithAlloc<IAuthorParseNodeSet>(pageAlloc, L"ls: AuthorParseNodeSet"),
        m_rootNode(rootNode),
        m_parseTree(parseTree),
        m_scriptContext(scriptContext)
    { 
        m_nodeList = NodeList::New(Alloc());

        ListParseNodeSerializerContext serializerContext(m_nodeList, m_scriptContext, m_parseTree, Alloc(), depth);
        ParseNodeSerializer<ListParseNodeSerializerPolicy> serializer;
        serializer.SerializeNode(rootNode, ParseNodeEdge::pneNone, &serializerContext);
    }

    STDMETHOD(get_Count)(int *result) 
    {
        STDMETHOD_PREFIX;

        ValidatePtr(result, E_POINTER);

        *result = m_nodeList->Count();

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetItems)(int startIndex, int count, AuthorParseNode *nodes)
    {
        STDMETHOD_PREFIX;

        ValidatePtr(nodes, E_POINTER);
        ValidateArg(startIndex + count <= this->m_nodeList->Count());

        for (int i = 0; i < count; i++)
        {
            int sourceIndex = startIndex + i;
            AuthorParseNode *src = this->m_nodeList->Item(sourceIndex);
            AuthorParseNode *dest = &nodes[i];
            memcpy_s(dest, sizeof(AuthorParseNode), src, sizeof(AuthorParseNode));
        }

        STDMETHOD_POSTFIX;
    }
};

TYPE_STATS(AuthorParseNodeSet, L"AuthorParseNodeSet")

class Authoring::AuthorParseNodeCursor : public SimpleComObjectWithAlloc<IAuthorParseNodeCursor>
{
    friend class ParseNodeTree;

private:
    ParseNodeTree* m_nodeTree;
    Js::ScriptContext* m_scriptContext;
    ParseNodeCursor* m_pnodeCursor;
    AuthoringFileHandle* m_file;
    PageAllocator* pageAlloc;
    bool m_isActive;

    void CopyAuthorParseNode (AuthorParseNodeDetails* result, ParseNode* pnode, ParseNode* pnodeParent)
    {
       if (pnode!= nullptr)
       {
            OpCode actualNop = ParseNodeSerializerHelpers::ActualNop(pnode);
            if (actualNop == knopCase)
            {
                Assert(pnodeParent);
                Assert(pnodeParent->nop == knopSwitch);

                if (pnode == pnodeParent->sxSwitch.pnodeDefault)
                {
                    actualNop = knopDefaultCase;
                }
            }

            result->kind = AuthorParseNodeKinds[actualNop];
            result->startOffset = ActualMin(pnode);
            result->endOffset =  ActualLim(pnode);
            result->flags = GetAuthorParseNodeFlags(pnode);
       }
       else
       {
           // create an empty node
            result->kind = AuthorParseNodeKind::apnkEmptyNode;
            result->startOffset = 0;
            result->endOffset = 0;
            result->flags = AuthorParseNodeFlags::apnfNone;

       }
    }

    void Deactivate()
    {
        m_isActive = false;
        m_nodeTree = nullptr;
    }

    void ReleaseCursor()
    {
        if (IsActive())
        {
            m_nodeTree->RemoveCursor(this);
            m_nodeTree = nullptr;
            m_scriptContext = nullptr;
            m_isActive = false;
        }
    }

protected:
    virtual void OnDelete() override
    {
        ReleaseCursor();

        // Call the base class's OnDelete
        SimpleComObjectWithAlloc<IAuthorParseNodeCursor>::OnDelete();
    }

public:
    AuthorParseNodeCursor(PageAllocator* pageAlloc, ParseNodeTree* m_nodeTree, AuthoringFileHandle *file, Js::ScriptContext* scriptContext)
        : SimpleComObjectWithAlloc<IAuthorParseNodeCursor>(pageAlloc, L"ls: AuthorParseNodeCursor"),
        pageAlloc(pageAlloc),
        m_nodeTree(m_nodeTree),
        m_scriptContext(scriptContext),
        m_file(file)
    {
        m_isActive = false;
        if (m_nodeTree != nullptr)
        {
            m_nodeTree->AddCursor(this);
            m_pnodeCursor = Anew(Alloc(), ParseNodeCursor, Alloc(), m_nodeTree, true);
            m_isActive = true;
        }
    }

    bool IsActive()
    {
        return (m_isActive && m_nodeTree != nullptr);
    }

    HRESULT EnsureIsActive()
    {
        return IsActive() ? S_OK : E_ILLEGAL_METHOD_CALL;
    }

    STDMETHOD(MoveUp)(AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr newNode = m_pnodeCursor->Up();
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(Current)(AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr newNode = m_pnodeCursor->Current();
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(Parent)(AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr newNode = m_pnodeCursor->Parent();
        ParseNodePtr parentNode = m_pnodeCursor->NthParent(2); // get the parent of the parent node

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(SeekToOffset)(long offset, VARIANT_BOOL excludeEndOffset, AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        // validate offset
        if (offset < 0)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        ParseNodePtr newNode = m_pnodeCursor->SeekToOffset(offset, excludeEndOffset == VARIANT_TRUE);
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(MoveToEnclosingNode)(long startOffset, long endOffset, VARIANT_BOOL excludeEndOffset, AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        // validate offset
        if (startOffset < 0 || endOffset < 0 || endOffset <= startOffset)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        bool execludeEndOffsetValue = (excludeEndOffset == VARIANT_TRUE);

        ParseNodePtr encolosingNode = m_pnodeCursor->SeekToOffset(startOffset, execludeEndOffsetValue);
        while (encolosingNode != nullptr && !InRange(endOffset, ActualMin(encolosingNode), ActualLim(encolosingNode), execludeEndOffsetValue))
        {
            encolosingNode = m_pnodeCursor->Up();
        }
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        CopyAuthorParseNode(result, encolosingNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(Child)(AuthorParseNodeEdge edgeLabel, long index, AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr newNode = nullptr;
        ParseNodePtr parentNode = m_pnodeCursor->Current();

        IfFailGo(m_pnodeCursor->Child(edgeLabel, index, &newNode));

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(MoveToChild)(AuthorParseNodeEdge edgeLabel, long index, AuthorParseNodeDetails *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        IfFailGo(m_pnodeCursor->MoveToChild(edgeLabel, index));

        ParseNodePtr newNode = m_pnodeCursor->Current();
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        CopyAuthorParseNode(result, newNode, parentNode);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetNodeProperty)(AuthorParseNodeProperty nodeProperty, long *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        long propertyValue = 0;

        switch (nodeProperty)
        {
            case apnpLCurlyMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->LCurly(currentNode);
                break;
            case apnpRCurlyMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->RCurly(currentNode);
                break;
            case apnpLParenMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->LParen(currentNode);
                break;
            case apnpRParenMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->RParen(currentNode);
                break;
             case apnpLBrackMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->LBrack(currentNode);
                break;
             case apnpRBrackMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->RBrack(currentNode);
                break;
            case apnpIdentifierMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->IdentMin(currentNode);
                break;
            case apnpFunctionKeywordMin:
                propertyValue = m_nodeTree->LanguageServiceExtension()->TkFunctionMin(currentNode);
                break;
            default:
                hr = E_INVALIDARG;
                goto Error;
                break;
        };

        *result = propertyValue;

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetSubTree)(long depth, IAuthorParseNodeSet **result)
    {
        STDMETHOD_PREFIX;

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTSubTreeBegin);

        IfFailGo(EnsureIsActive());

        AuthorParseNodeSet* subTree = new AuthorParseNodeSet(pageAlloc, m_pnodeCursor->Current(), m_nodeTree, m_scriptContext, depth);
        *result = subTree;

        STDMETHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTSubTreeEnd);
        STDMETHOD_POSTFIX_CLEAN_END;
    }

    STDMETHOD(GetPropertyById)(long propertyId, BSTR *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        if (propertyId < 0)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        Js::PropertyRecord const * propertyName = m_scriptContext->GetPropertyName(propertyId);

        if (propertyName == NULL)
        {
            hr = E_FAIL;
            goto Error;
        }

        *result = AllocBSTR(propertyName);

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetStringValue)(BSTR *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        *result = NULL;

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        LPCWSTR stringValue = NULL;
        switch (currentNode->nop)
        {
            case knopVarDecl:
            case knopLetDecl:
            case knopConstDecl:
                Assert (currentNode->sxVar.pid);
                stringValue = currentNode->sxVar.pid->Psz();
                break;

            case knopFncDecl:
                // check if currentNode is an anonymous function
                if(currentNode->sxFnc.pid)
                {
                    stringValue = currentNode->sxFnc.pid->Psz();
                }
                break;

            case knopClassDecl:
                if (currentNode->sxClass.isDeclaration)
                {
                    Assert(currentNode->sxClass.pnodeDeclName && currentNode->sxClass.pnodeDeclName->nop == knopLetDecl);
                    stringValue = currentNode->sxClass.pnodeDeclName->sxVar.pid->Psz();
                }
                break;

            case knopStr:
            case knopName:
                Assert (currentNode->sxPid.pid);
                stringValue = currentNode->sxPid.pid->Psz();
                break;

            default:
                hr = E_INVALIDARG;
                goto Error;
        };

        if (stringValue && *stringValue)
        {
            *result = ::SysAllocStringLen(stringValue, ::wcslen(stringValue));
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetIntValue)(int *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL || currentNode->nop != knopInt)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        *result = currentNode->sxInt.lw;

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetFloatValue)(double *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL || currentNode->nop != knopFlt)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        *result = currentNode->sxFlt.dbl;

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetRegExpValue)(BSTR *result, AuthorRegExpOptions *options)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        *result = NULL;
        *options = AuthorRegExpOptions::areoNone;

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL || currentNode->nop != knopRegExp)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        Assert(currentNode->sxPid.regexPattern);

        UnifiedRegex::RegexPattern *pattern = currentNode->sxPid.regexPattern;
        Js::InternalString regexSource = pattern->GetSource();

        uint regexpOptions = AuthorRegExpOptions::areoNone;
        if (pattern->IsIgnoreCase())
        {
            regexpOptions |= AuthorRegExpOptions::areoIgnoreCase;
        }
        if (pattern->IsGlobal())
        {
            regexpOptions |= AuthorRegExpOptions::areoGlobal;
        }
        if (pattern->IsMultiline())
        {
            regexpOptions |= AuthorRegExpOptions::areoMultiline;
        }
        if (pattern->IsUnicode())
        {
            regexpOptions |= AuthorRegExpOptions::areoUnicode;
        }
        if (pattern->IsSticky())
        {
            regexpOptions |= AuthorRegExpOptions::areoSticky;
        }

        *result = ::SysAllocStringLen(regexSource.GetBuffer(), regexSource.GetLength());
        *options = (AuthorRegExpOptions)regexpOptions;

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetStatementLabel)(BSTR *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        *result = NULL;

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        LPCWSTR statementLabel = m_nodeTree->LanguageServiceExtension()->GetLabel(currentNode);
        if (statementLabel && *statementLabel)
        {
            *result = ::SysAllocStringLen(statementLabel, ::wcslen(statementLabel));
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetTargetLabel)(BSTR *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        *result = NULL;

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL || (currentNode->nop != knopBreak && currentNode->nop != knopContinue))
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        if (currentNode->sxJump.hasExplicitTarget && currentNode->sxJump.pnodeTarget)
        {
            LPCWSTR targetLabel = m_nodeTree->LanguageServiceExtension()->GetLabel(currentNode->sxJump.pnodeTarget);
            if (targetLabel && *targetLabel)
            {
                *result = ::SysAllocStringLen(targetLabel, ::wcslen(targetLabel));
            }
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(GetStatementSpan)(long *startOffset, long *endOffset)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ValidatePtr(startOffset, E_POINTER);
        ValidatePtr(endOffset, E_POINTER);

        *startOffset = 0;
        *endOffset = 0;

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        if (currentNode == NULL)
        {
            hr = E_INVALIDARG;
            goto Error;
        }

        switch (currentNode->nop)
        {
        case knopDoWhile:
            *startOffset = m_nodeTree->LanguageServiceExtension()->WhileMin(currentNode);
            *endOffset = currentNode->ichLim;
            break;
        case knopSwitch:
            *startOffset = currentNode->ichMin;
            *endOffset = m_nodeTree->LanguageServiceExtension()->SwitchLim(currentNode);
            break;
        default:
            *startOffset = currentNode->ichMin;
            *endOffset = currentNode->ichLim;
            break;
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHOD(OffsetInComment)(long offset, VARIANT_BOOL *result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ValidatePtr(result, E_POINTER);

        bool offsetInComment = m_file->GetCommentTable()->OffsetInComment(offset);
        if (offsetInComment)
        {
            *result = VARIANT_TRUE;
        }
        else
        {
            *result = VARIANT_FALSE;
        }

        STDMETHOD_POSTFIX;
    }

    struct FindEdgeLabelProcessorContext 
    {
    public:
       ParseNodePtr currentNode;
       AuthorParseNodeEdge edgeLabel;

        FindEdgeLabelProcessorContext(ParseNodePtr currentNode)
            : currentNode(currentNode),
            edgeLabel(AuthorParseNodeEdge::apneNone)
        {
        }
    };

    struct FindEdgeLabelProcessorPolicy : public EdgeLabelProcessorPolicyBase<FindEdgeLabelProcessorContext>
    {
    protected:
        inline bool ProcessIndexedChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, int index, FindEdgeLabelProcessorContext* context) 
        { 
            if (context->currentNode == childNode)
            {
                context->edgeLabel = edgeLabel;
                return true;
            }
            return false;
        }

        inline bool ProcessChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, FindEdgeLabelProcessorContext* context) 
        { 
            return ProcessIndexedChildNode(childNode, edgeLabel, 0, context);
        }
    };

    STDMETHOD(GetEdgeLabel)(AuthorParseNodeEdge* result)
    {
        STDMETHOD_PREFIX;

        IfFailGo(EnsureIsActive());

        ValidatePtr(result, E_POINTER);

        ParseNodePtr currentNode = m_pnodeCursor->Current();
        ParseNodePtr parentNode = m_pnodeCursor->Parent();

        if (currentNode == NULL || parentNode == NULL)
        {
            *result = AuthorParseNodeEdge::apneNone;
            return hr;
        }

        FindEdgeLabelProcessorContext findEdgeLabelContext(currentNode);
        EdgeLabelProcessor<FindEdgeLabelProcessorPolicy> edgeLabelFinder;
        IfFailGo(edgeLabelFinder.ProcessNode(parentNode, &findEdgeLabelContext));
        *result = findEdgeLabelContext.edgeLabel;

        STDMETHOD_POSTFIX;
    }
};

TYPE_STATS(AuthorParseNodeCursor, L"AuthorParseNodeCursor")

struct FindChildNodeProcessorContext 
{
public:
    ParseNodePtr childNode;
    AuthorParseNodeEdge edgeLabel;
    int index;

    FindChildNodeProcessorContext(AuthorParseNodeEdge edgeLabel, int index)
        : edgeLabel(edgeLabel),
        childNode(nullptr),
        index(index)
    {
    }
};

struct FindChildNodeProcessorPolicy : public EdgeLabelProcessorPolicyBase<FindChildNodeProcessorContext>
{
protected:
    inline bool ProcessIndexedChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, int index, FindChildNodeProcessorContext* context) 
    { 
        if (context->edgeLabel == edgeLabel && context->index == index)
        {
            context->childNode = childNode;
            return true;
        }
        return false;
    }

    inline bool ProcessChildNode(ParseNodePtr childNode, AuthorParseNodeEdge edgeLabel, FindChildNodeProcessorContext* context) 
    { 
        if (context->edgeLabel == edgeLabel)
        {
            context->childNode = childNode;
            return true;
        }
        return false;
    }

    inline bool FailOnNode(ParseNodePtr pnode, FindChildNodeProcessorContext* context)
    {
        if (pnode && (pnode->nop == knopSwitch && context->edgeLabel == AuthorParseNodeEdge::apneCase) ||
            (pnode->nop == knopFncDecl && context->edgeLabel == AuthorParseNodeEdge::apneArgument) ||
            (pnode->nop == knopList && context->edgeLabel == AuthorParseNodeEdge::apneListItem))
            return false;
        return true;
    }
};

HRESULT ParseNodeCursor::Child(AuthorParseNodeEdge edgeLabel, long index, ParseNodePtr* result)
{
    STDMETHOD_PREFIX;

    ParseNodePtr currentNode = Current();

    FindChildNodeProcessorContext findChildContext(edgeLabel, index);
    EdgeLabelProcessor<FindChildNodeProcessorPolicy> childFinder;
    IfFailGo(childFinder.ProcessNode(currentNode, &findChildContext));
    *result = findChildContext.childNode;

    STDMETHOD_POSTFIX;
}

HRESULT ParseNodeCursor::MoveToChild(AuthorParseNodeEdge edgeLabel, long index)
{
    STDMETHOD_PREFIX;

    ParseNodePtr childNode = nullptr;

    IfFailGo(this->Child(edgeLabel, index, &childNode));

    // push the child node on the stack
    m_parents->Add(childNode);

    STDMETHOD_POSTFIX;
}

HRESULT Authoring::FileAuthoring::CreateASTCursor(IAuthorParseNodeCursor **result)
{
    STDMETHOD_PREFIX;

    *result = NULL;

    IfFailGo(UpdatePrimary(false));

    Assert(m_primaryFile);

    AuthorParseNodeCursor* cursor = new AuthorParseNodeCursor(m_scriptContext->GetThreadContext()->GetPageAllocator(), &m_primaryTree, m_primaryFile, m_scriptContext);

    *result = cursor;
    cursor = nullptr;

    STDMETHOD_POSTFIX;
}

void Authoring::ParseNodeTree::ReleaseActiveCursors()
{
    for (int i= 0; i < m_activeCursors.Count(); i++)
    {
        AuthorParseNodeCursor* cursor = m_activeCursors.Item(i);
        Assert(cursor);

        cursor->Deactivate();
    }

    m_activeCursors.Clear();
}

