//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#include "EnCPch.h"
#ifdef EDIT_AND_CONTINUE
#include "EditTest.h"
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

namespace Js
{
    FunctionInfo EditTest::EntryInfo::LoadTextFile(EditTest::LoadTextFile);
    FunctionInfo EditTest::EntryInfo::LCS(EditTest::LCS);
    FunctionInfo EditTest::EntryInfo::Ast(EditTest::Ast);
    FunctionInfo EditTest::EntryInfo::AstDiff(EditTest::AstDiff);

    //
    // Test LCS (Longest Common Subsequence) on 2 strings. Return an edit script string of deletes/inserts/matches.
    //
    Var EditTest::LCS(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();
        PROBE_STACK(scriptContext, Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        Var ret = nullptr;

        if (args.Info.Count >= 3)
        {
            JavascriptString* strA = JavascriptConversion::ToString(args[1], scriptContext);
            JavascriptString* strB = JavascriptConversion::ToString(args[2], scriptContext);
            const wchar_t* a = strA->GetString();
            const wchar_t* b = strB->GetString();

            BEGIN_TEMP_ALLOCATOR(tmpAlloc, scriptContext, L"Test")
            {
                LongestCommonSubsequence<ArenaAllocator> lcs(tmpAlloc, strA->GetLength(), strB->GetLength(), [=](int x, int y)
                {
                    return a[x] == b[y];
                });

                StringBuilder<ArenaAllocator> stringBuilder(tmpAlloc);
                lcs.MapEdits([&](EditKind kind, int x, int y)
                {
                    switch (kind)
                    {
                    case Update:
                        Assert(a[x] == b[y]);
                        stringBuilder.Append(a[x]);
                        break;

                    case Insert:
                        stringBuilder.Append(L'+');
                        stringBuilder.Append(b[y]);
                        break;

                    case Delete:
                        stringBuilder.Append(L'-');
                        stringBuilder.Append(a[x]);
                        break;

                    default:
                        Assert(false);
                        break;
                    }

                    stringBuilder.Append(L' ');
                });

                size_t len = stringBuilder.Count() > 0 ? stringBuilder.Count() - 1 : 0; // Strip last trailing space if appended
                ret = LiteralString::NewCopyBuffer(stringBuilder.Buffer(), len, scriptContext);
            }
            END_TEMP_ALLOCATOR(tmpAlloc, scriptContext);
        }

        if (!ret)
        {
            ret = scriptContext->GetLibrary()->GetUndefined();
        }
        return ret;
    }

    //
    // Parse given script string and return an object representing the AST.
    //
    Var EditTest::Ast(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();
        PROBE_STACK(scriptContext, Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        Var ret = nullptr;

        if (args.Info.Count >= 2)
        {
            JavascriptString* str = JavascriptConversion::ToString(args[1], scriptContext);

            BEGIN_TEMP_ALLOCATOR(alloc, scriptContext, L"Test")
            {
                Recycler* recycler = scriptContext->GetRecycler();
                AutoAllocatorObjectPtr<ScriptParseTree, EditAllocator> tree(Parse(recycler, str, scriptContext), recycler);
                ret = ParseTreeToObject(alloc, tree->GetParseTree(), scriptContext);
            }
            END_TEMP_ALLOCATOR(alloc, scriptContext);
        }

        if (!ret)
        {
            ret = scriptContext->GetLibrary()->GetUndefined();
        }
        return ret;
    }

    ScriptParseTree* EditTest::Parse(EditAllocator* alloc, JavascriptString* str, ScriptContext* scriptContext)
    {
        return EditNew(alloc, ScriptParseTree, alloc, str->GetString(), str->GetLength(), /*srcInfo*/nullptr, fscrGlobalCode, fscrGlobalCode, scriptContext);
    }

    template <class Allocator>
    EditTest::AstDumpContext<Allocator>::AstDumpContext(Allocator* alloc, ScriptContext* scriptContext) :
        alloc(alloc), scriptContext(scriptContext), path(alloc), root(nullptr)
    {
        propertyIds[NodePropertyIds::TYPE] = scriptContext->GetOrAddPropertyIdTracked(L"type");
        propertyIds[NodePropertyIds::CH_MIN] = scriptContext->GetOrAddPropertyIdTracked(L"min");
        propertyIds[NodePropertyIds::CH_LIM] = scriptContext->GetOrAddPropertyIdTracked(L"lim");
        propertyIds[NodePropertyIds::CHILDREN] = scriptContext->GetOrAddPropertyIdTracked(L"children");
    }

    //
    // Create a new JS object to represent a ParseNode.
    //
    template <class Allocator>
    DynamicObject* EditTest::AstDumpContext<Allocator>::NewObject(ParseNode* pnode) const
    {
        DynamicObject* obj = scriptContext->GetLibrary()->CreateObject();

        obj->SetProperty(propertyIds[NodePropertyIds::TYPE], JavascriptString::NewWithSz(ScriptDiff::GetParseNodeName(pnode), scriptContext),
            PropertyOperationFlags::PropertyOperation_Force, nullptr);
        obj->SetProperty(propertyIds[NodePropertyIds::CH_MIN], JavascriptNumber::ToVar(pnode->ichMin, scriptContext),
            PropertyOperationFlags::PropertyOperation_Force, nullptr);
        obj->SetProperty(propertyIds[NodePropertyIds::CH_LIM], JavascriptNumber::ToVar(pnode->ichLim, scriptContext),
            PropertyOperationFlags::PropertyOperation_Force, nullptr);

        return obj;
    }

    template <class Allocator>
    void EditTest::AstDumpContext<Allocator>::Preorder(ParseNode* pnode)
    {
        DynamicObject* obj = NewObject(pnode);
        if (!root)
        {
            root = obj;
        }

        DynamicObject* parentObj = (!path.Empty() ? path.Top().obj : nullptr);
        if (parentObj)
        {
            Var children;
            const PropertyId childrenPropertyId = propertyIds[NodePropertyIds::CHILDREN];
            if (!parentObj->GetProperty(parentObj, childrenPropertyId, &children, nullptr, scriptContext))
            {
                children = scriptContext->GetLibrary()->CreateArray();
                parentObj->SetProperty(childrenPropertyId, children, PropertyOperationFlags::PropertyOperation_Force, nullptr);
            }

            Assert(JavascriptArray::Is(children));
            JavascriptArray::Push(scriptContext, children, obj);
        }

        path.Push(NodeObj(pnode, obj)); // Push current node (as new parent node for future Preorder)
    }

    template <class Allocator>
    void EditTest::AstDumpContext<Allocator>::Postorder(ParseNode* pnode)
    {
        Assert(!path.Empty());
        Assert(path.Top().node == pnode);
        path.Pop(); // Pop current node
    }

    //
    // Convert parse tree to a JS object representation
    //
    Var EditTest::ParseTreeToObject(ArenaAllocator* alloc, ParseNodePtr parseTree, ScriptContext* scriptContext)
    {
        AstDumpContext<ArenaAllocator> astDumpContext(alloc, scriptContext);

        // TODO: This visitor stops at some recursive depth. Probably need a different non-recursive visitor.
        ParseTreePreorder(parseTree, &astDumpContext);

        return astDumpContext.GetRoot();
    }

    template <class Allocator>
    EditTest::EditDumpContext<Allocator>::EditDumpContext(Allocator* alloc, ScriptContext* scriptContext) :
        astDumpContext(alloc, scriptContext)
    {
        propertyIds[EditPropertyIds::KIND] = scriptContext->GetOrAddPropertyIdTracked(L"kind");
        propertyIds[EditPropertyIds::OLD_NODE] = scriptContext->GetOrAddPropertyIdTracked(L"oldNode");
        propertyIds[EditPropertyIds::NEW_NODE] = scriptContext->GetOrAddPropertyIdTracked(L"newNode");
    }

    //
    // Create a new JS object to represent an Edit.
    //
    template <class Allocator>
    DynamicObject* EditTest::EditDumpContext<Allocator>::NewObject(const Edit<ParseNodePtr>& edit) const
    {
        ScriptContext* scriptContext = astDumpContext.GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();
        DynamicObject* obj = library->CreateObject();
        RecyclableObject* nullObject = library->GetNull();

        // WARNING: Following names must be in sync with enum EditKind (rarely changed)
        static PCWSTR s_editKinds[] = { L"None", L"Update", L"Insert", L"Delete", L"Move", L"Reorder" };

        obj->SetProperty(propertyIds[EditPropertyIds::KIND], JavascriptString::NewWithSz(s_editKinds[edit.Kind()], scriptContext),
            PropertyOperationFlags::PropertyOperation_Force, nullptr);
        obj->SetProperty(propertyIds[EditPropertyIds::OLD_NODE], edit.OldNode() ? astDumpContext.NewObject(edit.OldNode()) : nullObject,
            PropertyOperationFlags::PropertyOperation_Force, nullptr);
        obj->SetProperty(propertyIds[EditPropertyIds::NEW_NODE], edit.NewNode() ? astDumpContext.NewObject(edit.NewNode()) : nullObject,
            PropertyOperationFlags::PropertyOperation_Force, nullptr);

        return obj;
    }

    // Templatized so that we can diff at either function level or full tree level using different TreeComparer.
    template <class Allocator, class TreeComparer>
    DynamicObject* EditTest::GenDiff(Allocator* alloc, TreeComparer comparer, ParseNodePtr rootA, ParseNodePtr rootB, ScriptContext* scriptContext)
    {
        JavascriptLibrary* library = scriptContext->GetLibrary();
        DynamicObject* diff = library->CreateObject();
        PropertyId matchPropertyId = scriptContext->GetOrAddPropertyIdTracked(L"match");
        PropertyId editsPropertyId = scriptContext->GetOrAddPropertyIdTracked(L"edits");

        // Do TreeMatch
        TreeMatch<TreeComparer, Allocator> match(alloc, rootA, rootB, comparer);

        EditDumpContext<Allocator> dumpContext(alloc, scriptContext);
        JavascriptArray* matchObject = library->CreateArray();
        ParseTreePreorder(rootA, [&](ParseNode* nodeA)
        {
            ParseNode* nodeB;
            if (match.TryGetPartnerInTree2(nodeA, &nodeB))
            {
                JavascriptArray* pair = library->CreateArray(2);
                pair->SetItem(0, dumpContext.NewObject(nodeA), PropertyOperationFlags::PropertyOperation_Force);
                pair->SetItem(1, dumpContext.NewObject(nodeB), PropertyOperationFlags::PropertyOperation_Force);
                JavascriptArray::Push(scriptContext, matchObject, pair);
            }
        });
        diff->SetProperty(matchPropertyId, matchObject, PropertyOperationFlags::PropertyOperation_Force, nullptr);

        // Collect edit script
        EditScript<TreeComparer, Allocator> editScript(alloc, match);

        JavascriptArray* edits = library->CreateArray();
        editScript.Edits().Map([&](int, const Edit<ParseNodePtr>& edit)
        {
            DynamicObject* e = dumpContext.NewObject(edit);
            JavascriptArray::Push(scriptContext, edits, e);
        });
        diff->SetProperty(editsPropertyId, edits, PropertyOperationFlags::PropertyOperation_Force, nullptr);

        return diff;
    }

    Var EditTest::AstDiff(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();
        PROBE_STACK(scriptContext, Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        Var ret = nullptr;

        if (args.Info.Count >= 3)
        {
            JavascriptString* strA = JavascriptConversion::ToString(args[1], scriptContext);
            JavascriptString* strB = JavascriptConversion::ToString(args[2], scriptContext);

            BEGIN_TEMP_ALLOCATOR(alloc, scriptContext, L"Test")
            {
                Recycler* recycler = scriptContext->GetRecycler();
                AutoAllocatorObjectPtr<ScriptParseTree, EditAllocator> treeA(Parse(recycler, strA, scriptContext), recycler);
                AutoAllocatorObjectPtr<ScriptParseTree, EditAllocator> treeB(Parse(recycler, strB, scriptContext), recycler);
                ParseNodePtr rootA = treeA->GetParseTree();
                ParseNodePtr rootB = treeB->GetParseTree();

                if (args.Info.Count >= 4 && JavascriptConversion::ToBool(args[3], scriptContext))
                {
                    ret = GenDiff(alloc, FullTreeComparer<ArenaAllocator>(alloc), rootA, rootB, scriptContext);
                }
                else
                {
                    ret = GenDiff(alloc, FunctionTreeComparer<ArenaAllocator>(alloc), rootA, rootB, scriptContext);
                }
            }
            END_TEMP_ALLOCATOR(alloc, scriptContext);
        }

        if (!ret)
        {
            ret = scriptContext->GetLibrary()->GetUndefined();
        }
        return ret;
    }

    // LoadTextFile(filename), return the text file content in a string.
    Var EditTest::LoadTextFile(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();
        PROBE_STACK(scriptContext, Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        Var ret = nullptr;

        if (args.Info.Count >= 2)
        {
            JavascriptString* fileName = JavascriptConversion::ToString(args[1], scriptContext);

            BEGIN_TEMP_ALLOCATOR(alloc, scriptContext, L"Test")
            {
                FILE* file;
                LPCOLESTR contents = nullptr;
                LPCUTF8 contentsUtf8 = nullptr;

                if (!_wfopen_s(&file, fileName->GetSz(), L"rb"))
                {
                    fseek(file, 0, SEEK_END);
                    uint lengthBytes = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    if (lengthBytes < INT_MAX)
                    {
                        LPCOLESTR contentsRaw = (LPCOLESTR)AnewArray(alloc, BYTE, lengthBytes + 2);
                        fread((void*)contentsRaw, sizeof(BYTE), lengthBytes, file);
                        fclose(file);

                        const BYTE* pRawBytes = (BYTE*)contentsRaw;
                        if (lengthBytes >= 3 && pRawBytes[0] == 0xEF && pRawBytes[1] == 0xBB && pRawBytes[2] == 0xBF)
                        {
                            contentsUtf8 = (LPCUTF8)contentsRaw;
                        }
                        else if ((lengthBytes >= sizeof(OLECHAR) && contentsRaw[0] == 0xFFFE)
                            || (lengthBytes >= sizeof(OLECHAR) * 2 && contentsRaw[0] == 0x0000 && contentsRaw[1] == 0xFEFF))
                        {
                            fwprintf(stderr, L"unsupported file encoding"); // UTF-16BE or UTF-32BE, both are unsupported
                        }
                        else if (lengthBytes >= sizeof(OLECHAR) && contentsRaw[0] == 0xFEFF)
                        {
                            contents = (LPCOLESTR)contentsRaw; // unicode LE
                        }
                        else
                        {
                            contentsUtf8 = (LPUTF8)contentsRaw; // Assume UTF8
                        }
                    }

                    if (contentsUtf8)
                    {
                        LPOLESTR contentsUtf16 = AnewArray(alloc, OLECHAR, lengthBytes + 1);
                        utf8::DecodeUnitsIntoAndNullTerminate(contentsUtf16, contentsUtf8, contentsUtf8 + lengthBytes, utf8::doAllowThreeByteSurrogates);
                        contents = contentsUtf16;
                        contentsUtf8 = nullptr;
                    }
                }

                if (contents)
                {
                    ret = LiteralString::NewCopySz(contents, scriptContext);
                }
            }
            END_TEMP_ALLOCATOR(alloc, scriptContext);
        }

        if (!ret)
        {
            ret = scriptContext->GetLibrary()->GetUndefined();
        }
        return ret;
    }

} // namespace Js

#endif
#endif
