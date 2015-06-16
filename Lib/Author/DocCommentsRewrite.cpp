//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#define MAX_MEMBERS_IN_FUNCTION_CALL 1000

namespace Authoring
{
    template<typename THandler>
    void ForEachMemberToDescribe(ParseNodePtr pnodeObject, bool includeComputedProperties, THandler handler)
    {
        Assert(pnodeObject != nullptr);
        Assert(pnodeObject->nop == knopObject);
        ASTHelpers::List::ForEach(pnodeObject->sxUni.pnode1, [&](ParseNode *pnodeMember)
        {
            // skip getters and setters
            if (pnodeMember->nop == knopMember || pnodeMember->nop == knopMemberShort)
            {
                if (includeComputedProperties && pnodeMember->sxBin.pnode1->nop == knopComputedName)
                {
                    handler(pnodeMember);
                }
                else
                {
                    // skipping computed name.
                    if (pnodeMember->sxBin.pnode1->nop != knopComputedName)
                    {
                        auto pidFieldName = pnodeMember->sxBin.pnode1->sxPid.pid;
                        if (pidFieldName && !InternalName(pidFieldName) && !String::IsNullOrEmpty(pidFieldName->Psz()))
                        {
                            handler(pnodeMember);
                        }
                    }
                }
            }
        });
    }

    // Pids cache
    class Pids : public PidCache
    {
        IdentPtr _isDefinitionRef;
        IdentPtr _fileId;
        IdentPtr _pos;
        IdentPtr _type;
        IdentPtr _description;
        IdentPtr _locid;
        IdentPtr _elementType;
        IdentPtr _helpKeyword;
        IdentPtr _name;
        IdentPtr _doc;
        IdentPtr _fields;
        IdentPtr _retDoc;
        IdentPtr _returnFunc;
        IdentPtr _initArgFunc;
        IdentPtr _thisAssignmentFunc;
        IdentPtr _externalFile;
        IdentPtr _externalid;
        IdentPtr _Object;
        IdentPtr _defineProperties;
        IdentPtr _definePropertiesInternal;
        IdentPtr _initFieldsFunc;
        IdentPtr _ctor;
        IdentPtr _elementCtor;
        IdentPtr _value;
        IdentPtr _get;
        IdentPtr _set;
        IdentPtr _configurable;
        IdentPtr _enumerable;
        IdentPtr _writable;
        IdentPtr _isUnsafeType;
        IdentPtr _isUnsafeElementType;
        IdentPtr _message;
        IdentPtr _deprecated;
        IdentPtr _offset;
        IdentPtr _endOffset;
        IdentPtr _defLoc;
        IdentPtr _compatibleWith;
        IdentPtr _platform;
        IdentPtr _minVersion;

    public:
        Pids(Parser* parser) : PidCache(parser),
            _isDefinitionRef(nullptr),
            _fileId(nullptr),
            _pos(nullptr),
            _type(nullptr),
            _description(nullptr),
            _locid(nullptr),
            _elementType(nullptr),
            _helpKeyword(nullptr),
            _name(nullptr),
            _doc(nullptr),
            _fields(nullptr),
            _retDoc(nullptr),
            _returnFunc(nullptr),
            _initArgFunc(nullptr),
            _thisAssignmentFunc(nullptr),
            _externalFile(nullptr),
            _externalid(nullptr),
            _Object(nullptr),
            _defineProperties(nullptr),
            _definePropertiesInternal(nullptr),
            _initFieldsFunc(nullptr),
            _ctor(nullptr),
            _elementCtor(nullptr),
            _value(nullptr),
            _get(nullptr),
            _set(nullptr),
            _configurable(nullptr),
            _enumerable(nullptr),
            _writable(nullptr),
            _isUnsafeType(nullptr),
            _isUnsafeElementType(nullptr),
            _message(nullptr),
            _deprecated(nullptr), 
            _offset(nullptr),
            _endOffset(nullptr),
            _defLoc(nullptr),
            _compatibleWith(nullptr),
            _platform(nullptr),
            _minVersion(nullptr)
        { }

        IdentPtr isDefinitionRef() { return GetPid(Names::isDefinitionRef, _isDefinitionRef); }
        IdentPtr fileId() { return GetPid(Names::fileId, _fileId); }
        IdentPtr pos() { return GetPid(Names::pos, _pos); }
        IdentPtr type() { return GetPid(Names::type, _type); }
        IdentPtr description() { return GetPid(Names::description, _description); }
        IdentPtr locid() { return GetPid(Names::locid, _locid); }
        IdentPtr elementType() { return GetPid(Names::elementType, _elementType); }
        IdentPtr helpKeyword() { return GetPid(Names::helpKeyword, _helpKeyword); }
        IdentPtr name() { return GetPid(Names::name, _name); }
        IdentPtr doc() { return GetPid(Names::doc, _doc); }
        IdentPtr fields() { return GetPid(Names::fields, _fields); }
        IdentPtr retDoc() { return GetPid(Names::retDoc, _retDoc); }
        IdentPtr returnFunc() { return GetPid(Names::_return, _returnFunc); }
        IdentPtr initArgFunc() { return GetPid(Names::initArg, _initArgFunc); }
        IdentPtr thisAssignmentFunc() { return GetPid(Names::thisAssignment, _thisAssignmentFunc); }
        IdentPtr externalFile() { return GetPid(Names::externalFile, _externalFile); }
        IdentPtr externalid() { return GetPid(Names::externalid, _externalid); }
        IdentPtr Object() { return GetPid(Names::Object, _Object); }
        IdentPtr defineProperties() { return GetPid(Names::defineProperties, _defineProperties); }
        IdentPtr definePropertiesInternal() { return GetPid(Names::_defineProperties, _definePropertiesInternal); }
        IdentPtr initFieldsFunc() { return GetPid(Names::initFields, _initFieldsFunc); }
        IdentPtr ctor() { return GetPid(Names::ctor, _ctor); }
        IdentPtr elementCtor() { return GetPid(Names::elementCtor, _elementCtor); }
        IdentPtr value() { return GetPid(Names::value, _value); }
        IdentPtr get() { return GetPid(Names::get, _get); }
        IdentPtr set() { return GetPid(Names::set, _set); }
        IdentPtr configurable() { return GetPid(Names::configurable, _configurable); }
        IdentPtr enumerable() { return GetPid(Names::enumerable, _enumerable); }
        IdentPtr writable() { return GetPid(Names::writable, _writable); }
        IdentPtr isUnsafeType() { return GetPid(Names::isUnsafeType, _isUnsafeType); }
        IdentPtr isUnsafeElementType() { return GetPid(Names::isUnsafeElementType, _isUnsafeElementType); }
        IdentPtr message() { return GetPid(Names::message, _message); }
        IdentPtr deprecated() { return GetPid(Names::deprecated, _deprecated); }
        IdentPtr offset() { return GetPid(Names::offset, _offset); }
        IdentPtr endOffset() { return GetPid(Names::endOffset, _endOffset); }
        IdentPtr defLoc() { return GetPid(Names::_defLoc, _defLoc); }
        IdentPtr compatibleWith() { return GetPid(Names::compatibleWith, _compatibleWith); }
        IdentPtr platform() { return GetPid(Names::platform, _platform); }
        IdentPtr minVersion() { return GetPid(Names::minVersion, _minVersion); }
    };

    RewriteContext::RewriteContext(ArenaAllocator* alloc, ParseNodeTree* tree, AuthoringFileHandle* handle, int fileId, long cursorPos)
        : alloc(alloc), tree(tree), handle(handle), fileId(fileId), level(0), blockScopeLevel(0), memberCount(0), cursorPos(cursorPos), nodeStack(alloc), memberCountStack(alloc), previousNode(nullptr), lastReferenceToNode(nullptr)
    {
        Assert(tree);
        Assert(alloc);
        this->pids = Anew(alloc, Pids, tree->GetParser());
    }

    inline ParseNodePtr RewriteContext::GetParent()
    {
        auto parentNode = FindParent(0, [&](ParseNodePtr pnode)
        {
            // Ignore lists and synthetic nodes
            return pnode->nop != knopList && !(pnode->grfpn & PNodeFlags::fpnSyntheticNode);
        });

        // Ignore autogenerated nodes such as scope records
        if (parentNode && IsZeroExtent(parentNode))
            parentNode = nullptr;

        return parentNode;
    }

    inline ParseNodePtr RewriteContext::GetFunction()
    {
        return FindParent(0, [&](ParseNodePtr pnode)
        {
            return pnode->nop == knopFncDecl || pnode->nop == knopProg;
        });
    }

    inline ParseNodePtr RewriteContext::GetBlockFromGlobal()
    {
        ParseNodePtr functionParent = GetFunction();

        Assert(functionParent->nop == knopProg && functionParent->sxFnc.pnodeScopes->nop == knopBlock);
        return functionParent->sxFnc.pnodeScopes;
    }

    static LPCWSTR ToExpression(ArenaAllocator* alloc, LPCWSTR script)
    {
        Assert(alloc);
        if(String::IsNullOrEmpty(script))
            return script;
        TextBuffer adjusted(alloc);
        adjusted.Add(L"_$evaltmp=");
        adjusted.Add(script);
        return adjusted.Sz(true);
    }

    static ParseNodePtr EvalExpressionAST(ASTBuilder<Pids>& ast, LPCWSTR expr)
    {
        auto pids = ast.Pids();
        return ast.Call(ast.Name(pids.eval()), ast.String(expr));
    }

    bool IsSafeTypeExpression(LPCWSTR typeExpression)
    {
        if (String::IsNullOrEmpty(typeExpression))
            return false;

        for (;typeExpression && *typeExpression; typeExpression++)
            if (!FileAuthoring::GetLSCharClassifier()->IsNameSpaceChar(*typeExpression))
                return false;

        return true;
    }

    static ParseNodePtr TypeConstructorAST(ASTBuilder<Pids>& ast, LPCWSTR typeExpression) 
    {
        return !IsSafeTypeExpression(typeExpression) ? ast.Undefined() : EvalExpressionAST(ast, typeExpression);
    }

    static ParseNodePtr ValueAttributeAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, LPCWSTR value)
    {
        return String::IsNullOrEmpty(value) ? ast.Undefined() : EvalExpressionAST(ast, ToExpression(alloc, value));
    }

    static ParseNode* DefinitionRefAST(ASTBuilder<Pids>& ast, int fileId, charcount_t pos)
    {
        auto pids = ast.Pids();
        return ast.Object(ast.List(
            ast.Member(pids.isDefinitionRef(), ast.True()),
            ast.Member(pids.fileId(), ast.Int(fileId)),
            ast.Member(pids.pos(), ast.Int(pos))));
    }

    static ParseNode* DefinitionRefPropertyDescriptorAST(ASTBuilder<Pids>& ast, int fileId, charcount_t pos) 
    {
        auto pids = ast.Pids();
        return ast.Object(ast.List(
            ast.Member(pids.value(), DefinitionRefAST(ast, fileId, pos)),
            ast.Member(pids.writable(), ast.True()),
            ast.Member(pids.configurable(), ast.True()),
            ast.Member(pids.enumerable(), ast.False())));
    }

    static ParseNode* ObjectLiteralDefinitionLocationAST(ASTBuilder<Pids>& ast, int fileId, charcount_t startOffset, charcount_t endOffset)
    {
        auto pids = ast.Pids();
        return ast.Object(ast.List(
            ast.Member(pids.offset(), ast.Int(startOffset)),
            ast.Member(pids.endOffset(), ast.Int(endOffset)),
            ast.Member(pids.fileId(), ast.Int(fileId))));
    }

    static ParseNode* ObjectLiteralDefinitionLocationDescriptorAST(ASTBuilder<Pids>& ast, ParseNode* objectLiteralNode, int fileId)
    {
        Assert(objectLiteralNode);
        Assert(objectLiteralNode->nop == knopObject);
        auto pids = ast.Pids();
        return ast.Object(ast.List(
            ast.Member(pids.value(), ObjectLiteralDefinitionLocationAST(ast, fileId, objectLiteralNode->ichMin, objectLiteralNode->ichLim)),
            ast.Member(pids.writable(), ast.True()),
            ast.Member(pids.configurable(), ast.True()),
            ast.Member(pids.enumerable(), ast.False())));
    }

    static void DocCommentAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, ParseNodePtr& members, DocComment* docComment)
    {
        Assert(docComment);
        auto pids = ast.Pids();
        if(docComment->description)
            ast.Prepend(members, ast.Member(pids.description(), ast.String(docComment->description)));
        if(docComment->locid)
            ast.Prepend(members, ast.Member(pids.locid(), ast.String(docComment->locid)));
        if(docComment->value)
            ast.Prepend(members, ast.Member(pids.value(), ValueAttributeAST(ast, alloc, docComment->value)));
        if(docComment->type)
        {
            ast.Prepend(members, ast.Member(pids.type(), ast.String(docComment->type)));
            ast.Prepend(members, ast.Member(pids.ctor(), TypeConstructorAST(ast, docComment->type)));
            if (!IsSafeTypeExpression(docComment->type))
                ast.Prepend(members, ast.Member(pids.isUnsafeType(), ast.True()));
        }
        if(docComment->elementType)
        {
            ast.Prepend(members, ast.Member(pids.elementType(), ast.String(docComment->elementType)));
            ast.Prepend(members, ast.Member(pids.elementCtor(), TypeConstructorAST(ast, docComment->elementType)));
            if (!IsSafeTypeExpression(docComment->elementType))
                ast.Prepend(members, ast.Member(pids.isUnsafeElementType(), ast.True()));
        }
    }

    static void VarDocCommentAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, ParseNodePtr& members, VarDocComments* docComment)
    {
        Assert(docComment);
        DocCommentAST(ast, alloc, members, docComment);
        auto pids = ast.Pids();
        if(docComment->externalFile)
            ast.Prepend(members, ast.Member(pids.externalFile(), ast.String(docComment->externalFile)));
        if(docComment->externalid)
            ast.Prepend(members, ast.Member(pids.externalid(), ast.String(docComment->externalid)));
        if(docComment->helpKeyword)
            ast.Prepend(members, ast.Member(pids.helpKeyword(), ast.String(docComment->helpKeyword)));
    }

    static ParseNodePtr DeprecatedDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, Deprecated* deprecated)
    {
        Assert(deprecated);
        ParseNodePtr members = nullptr;
        auto pids = ast.Pids();
        if (deprecated->type)
            ast.Prepend(members, ast.Member(pids.type(), ast.String(deprecated->type)));
        if (deprecated->message)
            ast.Prepend(members, ast.Member(pids.message(), ast.String(deprecated->message)));
        return ast.Object(members);
    }

    static ParseNodePtr CompatibleWithDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, CompatibleWith* compatibleWith)
    {
        Assert(compatibleWith);
        ParseNodePtr members = nullptr;
        auto pids = ast.Pids();
        if (compatibleWith->platform)
            ast.Prepend(members, ast.Member(pids.platform(), ast.String(compatibleWith->platform)));
        if (compatibleWith->minVersion)
            ast.Prepend(members, ast.Member(pids.minVersion(), ast.String(compatibleWith->minVersion)));
        return ast.Object(members);
    }

    static ParseNodePtr FieldDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, FieldDocComments* field, int fileId)
    {
        Assert(field);
        ParseNodePtr members = nullptr;
        VarDocCommentAST(ast, alloc, members, field);
        auto pids = ast.Pids();
        if(field->name)
            ast.Prepend(members, ast.Member(pids.name(), ast.String(field->name)));
        // We need to include the fileId since it is required to initialize IAuthorSymbolHelp.SourceFileHandle property 
        // when completion hint is requested for the field.
        if(fileId >= 0)
            ast.Prepend(members, ast.Member(pids.fileId(), ast.Int(fileId)));

        ParseNodePtr deprecatedAST = field->deprecated ? DeprecatedDocAST(ast, alloc, field->deprecated) : nullptr;
        if (deprecatedAST)
            ast.Prepend(members, ast.Member(pids.deprecated(), deprecatedAST));

        ParseNodePtr compatibleWithItems = nullptr;
        ListOperations::ForEach(&field->compatibleWith, [&](int, CompatibleWith* compat)
        {
            ast.Prepend(compatibleWithItems, CompatibleWithDocAST(ast, alloc, compat));
        });
        if (compatibleWithItems)
        {
            ast.Prepend(members, ast.Member(pids.compatibleWith(), ast.Array(compatibleWithItems)));
        }

        return members ? ast.Object(members) : nullptr;
    }

    static ParseNodePtr ParamDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, FunctionDocComments::Param* param)
    {
        Assert(param);
        ParseNodePtr members = nullptr;
        DocCommentAST(ast, alloc, members, param);
        return members ? ast.Object(members) : nullptr;
    }

    static ParseNodePtr RetDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, FunctionDocComments::ReturnValue* returnValue)
    {
        Assert(returnValue);
        ParseNodePtr members = nullptr;
        VarDocCommentAST(ast, alloc, members, returnValue);
        return members ? ast.Object(members) : nullptr;
    }

    static ParseNodePtr FuncDocAST(ASTBuilder<Pids>& ast, ArenaAllocator* alloc, FunctionDocComments* funcDoc, int fileId)
    {
        Assert(funcDoc);
        auto signature = funcDoc->FirstSignature();
        Assert(signature);
        auto pids = ast.Pids();
        ParseNodePtr members = nullptr;

        ParseNodePtr fieldsAST = nullptr;
        ListOperations::ForEach(&funcDoc->fields, [&](int, FieldDocComments* fieldDoc)
        {
            if(!fieldDoc->isStatic)
                ast.Prepend(fieldsAST, FieldDocAST(ast, alloc, fieldDoc, fileId));   
        });
        if(fieldsAST)
        {
            ast.Prepend(members, ast.Member(pids.fields(), ast.Array(fieldsAST))); 
        }
       
        ParseNodePtr returnValueAST = signature->returnValue ? RetDocAST(ast, alloc, signature->returnValue) : nullptr;
        if(returnValueAST)
        {
            ast.Prepend(members, ast.Member(pids.doc(), returnValueAST)); 
        }
        
        ParseNodePtr deprecatedAST = signature->deprecated ? DeprecatedDocAST(ast, alloc, signature->deprecated) : nullptr;
        if (deprecatedAST)
        {
            ast.Prepend(members, ast.Member(pids.deprecated(), deprecatedAST));
        }

        ParseNodePtr compatibleWithItems = nullptr;
        ListOperations::ForEach(&signature->compatibleWith, [&](int, CompatibleWith* compat)
        {
            ast.Prepend(compatibleWithItems, CompatibleWithDocAST(ast, alloc, compat));
        });
        if (compatibleWithItems)
        {
            ast.Prepend(members, ast.Member(pids.compatibleWith(), ast.Array(compatibleWithItems)));
        }

        return members ? ast.Object(members) : nullptr;
    }

    static ParseNodePtr ReturnExprAST(ASTBuilder<Pids>& ast, ParseNodePtr returnExpr)
    {
        auto pids = ast.Pids();
        return ast.Call(ast.Name(pids.returnFunc()), ast.List(
            ast.This(),
            ast.Optional(returnExpr),
            ast.Name(pids.retDoc())));
    }

    bool HasRelevantTags(LPCWSTR text)
    {
        // TODO (andrewau) include all the tags
        // TODO (andrewau) consider optimizing the check for all tag for speed
        return wcsstr(text, Names::param)
            || wcsstr(text, Names::returns)
            || wcsstr(text, Names::field) 
            || wcsstr(text, Names::jsdoc_returns_tag)
            || wcsstr(text, Names::jsdoc_property_tag)
            || wcsstr(text, Names::jsdoc_param_tag);
    }

    static bool IsNameNode(ParseNodePtr pnode)
    {
        return pnode && (pnode->nop == knopName || pnode->nop == knopStr);
    }

    static bool IsThisNode(ParseNodePtr pnode)
    {
        return pnode && pnode->nop == knopThis;
    }

    static bool IsDotNode(ParseNodePtr pnode)
    {
        return pnode && pnode->nop == knopDot;
    }

    static bool IsThisDotAssignment(ParseNodePtr pnode)
    {
        return pnode && pnode->nop == knopAsg && 
            IsDotNode(pnode->sxBin.pnode1) &&
            IsThisNode(pnode->sxBin.pnode1->sxBin.pnode1) &&
            IsNameNode(pnode->sxBin.pnode1->sxBin.pnode2);
    }

    ParseNode* CallObjectDefinePropertiesAST(ASTBuilder<Pids>& ast, ParseNode* object, ParseNodePtr propertyDefinitions)
    {
        Assert(object);
        Assert(propertyDefinitions);

        // Returns:
        //    Object.defineProperties(<object>, <propertyDefinitions>)

        auto pids = ast.Pids();
        ast.SetExtent(object->ichMin, object->ichMin);
        auto target = ast.Dot(ast.Name(pids.Object()), ast.Name(pids.defineProperties()));
        ast.SetExtent(object->ichMin, object->ichLim);
        return ast.Call(target, ast.List(object, propertyDefinitions));
    }

    ParseNode* DefineInternalProperties(ASTBuilder<Pids>& ast, ParseNode* object, ParseNodePtr propertyDefinitions)
    {
        Assert(object);
        Assert(propertyDefinitions);

        // Returns:
        //      _$defineProperties(<object>, <propertyDefinitions>)
        // 
        //      <propertyDefinitions> should be of the form:
        //                      { <propertyName> : <propertyValue> }
        // Note: The over all extent of the call should be that of the original object; nodes in the call appering before
        //       the object will have its min for their extent, and those appering after it will have its lim.

        auto pids = ast.Pids();
        ast.SetExtent(object->ichMin, object->ichMin);
        auto definePropertyFunctionNode = ast.Name(pids.definePropertiesInternal());

        ast.SetExtent(object->ichMin, object->ichLim);
        return ast.Call(
                    definePropertyFunctionNode,
                    ast.List(
                        object,
                        propertyDefinitions
                        )
                    );
    }

    void RewriteFunctionDocComments(ParseNode *pnode, RewriteContext *context)
    {
        Assert(pnode);
        Assert(context);
        Assert(pnode->nop == knopFncDecl || pnode->nop == knopProg);
        Parser* parser = context->tree->GetParser();
        if (pnode->nop == knopProg && !context->handle->GetIsInDeferredParsingMode())
        {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::HiResTimer timer;
            double start = timer.Now();
#endif
            for (CommentTableIterator* typeDefIterator = context->handle->GetCommentTable()->GetTypeDefIterator(); typeDefIterator->HasNext(); typeDefIterator->MoveNext())
            {
                int min, lim;
                CommentBuffer* typeDefComment = typeDefIterator->GetCurrentComment(context->handle->Text(), &min, &lim);
                TypeDefintionSetDocComments* typeDefintionSetDocComments = nullptr;
                Authoring::ParseTypeDefinitionComments(context->alloc, typeDefComment->Sz(), typeDefComment->GetCommentType(), &typeDefintionSetDocComments);
                
                /* Place the generated functions at the absolute top of the program */
                pnode->ichMin = 0;
                pnode->sxFnc.pnodeBody->ichMin = 0;
                ASTBuilder<Pids> ast(parser, pnode, context->pids);
                
                // Make sure we inject function at the beginning of the program
                ast.SetExtent(ast.Min());

                for (int typeIndex = 0; typeIndex < typeDefintionSetDocComments->typeDefinitions.Count(); typeIndex++)
                {
                    TypeDefintionDocComments* typeDefintionDocComments = typeDefintionSetDocComments->typeDefinitions.Item(typeIndex);
                    const WCHAR* typeName = typeDefintionDocComments->name;
                    
                    if (String::IsNullOrEmpty(typeName))
                    {
                        continue;
                    }

                    // This create a blank function expression
                    auto functionDefinition = ast.Function(nullptr, ast.Block(), nullptr, nullptr);
                    ASTBuilder<Pids> functionBuilder(parser, functionDefinition, context->pids);
                    auto pids = functionBuilder.Pids();
                    for (int propertyIndex = 0; propertyIndex < typeDefintionDocComments->fields.Count(); propertyIndex++)
                    {
                        FieldDocComments* field = typeDefintionDocComments->fields.Item(propertyIndex);
                        const WCHAR* propertyName = field->name;
                        const WCHAR* propertyType = field->type;

                        if (String::IsNullOrEmpty(propertyName))
                        {
                            continue;
                        }

                        functionBuilder.SetExtent(ast.Min());

                        // this.propertyName = _$getInstanceByType(propertyType)
                        functionBuilder.Prepend(ast.Assign(
                            ast.Dot(ast.This(), ast.Name(ast.Pid(propertyName))),
                            ast.Call(ast.Name(ast.Pid(L"_$getInstanceByType")), ast.Name(ast.Pid(propertyType)))
                            ));
                    }

                    ParseNodePtr fieldsAST = nullptr;
                    for (int propertyIndex = 0; propertyIndex < typeDefintionDocComments->fields.Count(); propertyIndex++)
                    {
                        FieldDocComments* field = typeDefintionDocComments->fields.Item(propertyIndex);
                        ast.Prepend(fieldsAST, FieldDocAST(functionBuilder, context->alloc, field, context->fileId));
                    }

                    if (fieldsAST != nullptr)
                    {
                        // Represents: 
                        //       _$initFields(this, retDoc)
                        functionBuilder.Prepend(functionDefinition->sxFnc.pnodeBody,
                            functionBuilder.Call(functionBuilder.Name(pids.initFieldsFunc()), functionBuilder.List(
                            functionBuilder.This(),
                            functionBuilder.Name(pids.retDoc()))));

                        // If we add a non-this dot assignment we need to let the code generator that it cannot optimize for this dot assignments.
                        functionDefinition->sxFnc.SetHasNonThisStmt();
                        functionBuilder.Prepend(functionBuilder.Var(pids.retDoc(), functionBuilder.Object(ast.Member(pids.fields(), ast.Array(fieldsAST)))));
                    }
                    
                    // This create a variable declaration initialized to the function expression
                    // var typeName = function expression
                    auto variableDeclaration = parser->CreateNode(knopVarDecl);
                    variableDeclaration->ichMin = variableDeclaration->ichLim = ast.Min();
                    variableDeclaration->sxVar.InitDeclNode(NULL, NULL);
                    variableDeclaration->sxVar.pid = parser->CreatePid(typeName, wcslen(typeName));
                    variableDeclaration->sxVar.pnodeInit = functionDefinition;

                    ast.Prepend(ast.Assign(ast.Dot(ast.Name(ast.Pid(typeName)), ast.Name(ast.Pid(L"_$isTypeDefGenerated"))), ast.True()));
                    ast.Prepend(variableDeclaration);
                }
            }
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            double diff = timer.Now() - start;
            if (diff > 2.0) // greater then 2.0 msec should make sense
            {
                OUTPUT_TRACE(Js::JSLSStatsPhase, L"DocCommentsRewrite::TypeDefParsing time spent %8.3f\n", diff);
            }
#endif
        }
        if (pnode->nop == knopFncDecl)
        {
            if (pnode->ichLim > pnode->ichMin) // Make sure we don't try to associate DocComment with injected functions
            {
                context->handle->RememberSingleNodeComments(context->alloc, /* searchLimOverride= */-1, pnode, context->GetParent(), context->previousNode, /* searchPreviousNode = */false, pnode->ichMin);
                CommentBuffer* functionComments = context->handle->GetFunctionComments(context->alloc, pnode, context->tree->LanguageServiceExtension(), commenttypeAnyDoc);
                LPCWSTR functionCommentsText = functionComments == nullptr ? nullptr : functionComments->Sz();
                if (!String::IsNullOrEmpty(functionCommentsText))
                {
                    if ((functionCommentsText != nullptr) && HasRelevantTags(functionCommentsText))
                    {
                        FunctionDocComments* funcDoc = nullptr;
                        if (SUCCEEDED(Authoring::ParseFuncDocComments(context->alloc, functionCommentsText, functionComments->GetCommentType(), &funcDoc)) && funcDoc)
                        {
                            ParseNodePtr fncNode = pnode;
                            ParseNodePtr originalBody = pnode->sxFnc.pnodeBody;
                            auto signature = funcDoc->FirstSignature();
                            Assert(signature);

                            ASTBuilder<Pids> ast(parser, fncNode, context->pids);
                            auto pids = ast.Pids();

                            // Apply <param> doc comments when the cursor is inside the function.
                            // When argument is not initialized _$initArg will try to initialize it by calling Type as a constructor.
                            // It will also attach doc comments information for better completion hint.
                            // For each argument:
                            // argN = _$initArg(argN, <type>, <value> { < doc object > })

                            ast.SetExtent(ast.Min());
                            bool atCursor = context->cursorPos >= 0 && ContainsOffset(fncNode, context->cursorPos);
                            ListOperations::ForEach(&signature->params, [&](int index, FunctionDocComments::Param* argDoc)
                            {
                                auto argName = argDoc->name;
                                if (!String::IsNullOrEmpty(argName))
                                {
                                    // When <param value=''> attribute is set we need to force the argument value regardless whether the cursor
                                    // is in the function or not since the value can be returned from the function and we still want to use it.
                                    if (atCursor || !String::IsNullOrEmpty(argDoc->value))
                                    {
                                        auto argDocAST = ParamDocAST(ast, context->alloc, argDoc);
                                        if (argDocAST)
                                        {
                                            ast.Prepend(pnode->sxFnc.pnodeBody, ast.Assign(ast.Name(argName),
                                                ast.Call(ast.Name(pids.initArgFunc()), ast.List(
                                                ast.Name(argName),
                                                argDocAST))));

                                            // If we add a non-this dot assignment we need to let the code generator that it cannot optimize for this dot assignments.
                                            pnode->sxFnc.SetHasNonThisStmt();
                                        }
                                    }
                                }
                            });

                            //
                            // Apply <returns> and <field> doc comments 
                            //
                            auto returnDoc = signature->returnValue;
                            auto returnValue = returnDoc ? returnDoc->value : nullptr;
                            bool fieldsAvailable = funcDoc->fields.Count() > 0;
                            if (returnDoc || fieldsAvailable)
                            {
                                // Add _$retDoc variable, which holds return value doc comments
                                ast.SetExtent(ast.Min());

                                // Add members specified via <field> doc comments, and set their value according to type or value attributes. 
                                if (fieldsAvailable)
                                {
                                    // Represents: 
                                    //       _$initFields(this, retDoc)
                                    ast.Prepend(pnode->sxFnc.pnodeBody,
                                        ast.Call(ast.Name(pids.initFieldsFunc()), ast.List(
                                        ast.This(),
                                        ast.Name(pids.retDoc()))));

                                    // If we add a non-this dot assignment we need to let the code generator that it cannot optimize for this dot assignments.
                                    pnode->sxFnc.SetHasNonThisStmt();
                                }

                                ast.Prepend(ast.Var(pids.retDoc(), FuncDocAST(ast, context->alloc, funcDoc, context->fileId)));

                                if (!returnValue)
                                {
                                    // When <returns value=''> not specified 

                                    // Replace: 
                                    //      return <expr>
                                    // With:
                                    //      return _$return(thisVal, <expr>, <retDoc>);
                                    ASTMutatingVisitor::Visit(pnode->sxFnc.pnodeBody,
                                        [&](ParseNodePtr& nodeRef) -> bool
                                    {
                                        if (nodeRef)
                                        {
                                            // when visiting child nodes, ignore nested functions and their children
                                            if (nodeRef->nop == knopFncDecl)
                                                return false;

                                            if (nodeRef->nop == knopReturn)
                                            {
                                                ParseNodePtr& retNode = nodeRef;
                                                ast.SetExtentAs(retNode);
                                                retNode->sxReturn.pnodeExpr = ReturnExprAST(ast, retNode->sxReturn.pnodeExpr);
                                            }
                                        }
                                        return true;
                                    });
                                    // Add a return statement to the end of the function, as a fallback.
                                    ast.SetExtent(ast.Lim());
                                    ast.Append(ast.Return(ReturnExprAST(ast, ast.Undefined())));

                                    // If we add a non-this dot assignment we need to let the code generator that it cannot optimize for this dot assignments.
                                    pnode->sxFnc.SetHasNonThisStmt();
                                }
                                else
                                {
                                    // <returns value=''> specified. Replace the original function body with 'return $_return(eval('<value>'))'
                                    auto evalCall = EvalExpressionAST(ast, ToExpression(context->alloc, returnValue));
                                    // Represents: if(true) return _$return(eval(<value>), _$retDoc) else <original body>;
                                    auto ifNode = ast.If(ast.True(), ast.Return(ReturnExprAST(ast, evalCall)), nullptr);
                                    ast.SetExtentAs(originalBody);
                                    auto newBody = ast.List(ifNode, originalBody);
                                    bool replaced = ast.Replace(originalBody, newBody);
                                    Assert(replaced);
                                }
                            }
                        }
                    }

                    if (functionComments)
                    {
                        functionComments->Release();
                    }
                }
            }
        }

        if (pnode->nop == knopFncDecl && pnode->sxFnc.HasThisStmt())
        {
            // Rewrite the function to add _$fieldDoc entries for this. assignments

            // The function has at least one statement of the form this.<name> = <expr>. Mark all such statements as definitions of the field.
            ASTHelpers::TraverseStatements(pnode->sxFnc.pnodeBody, [&] (ParseNodePtr stmt) {
                if (pnode->nop == knopFncDecl && IsThisDotAssignment(stmt))
                {
                    // this. assignment
                    ParseNodePtr nameNode = stmt->sxBin.pnode1->sxBin.pnode2;
                    IdentPtr name = nameNode->sxPid.pid;
                    if (!InternalName(name))
                    {
                        ASTBuilder<Pids> ast(parser, pnode, context->pids);
                        ast.SetExtent(stmt->ichLim);
                        auto definitionRef = DefinitionRefAST(ast, context->fileId, nameNode->ichMin);

                        // Following this.<property>=value assignment, add a call to _$thisAssignment: 
                        // _$thisAssignment(this, '<property>', value, definitionRef);
                        // _$thisAssignment will set _$fieldDoc$<property> conditionally, if this.<property> was not defined.
                        // 
                        ast.SetExtent(stmt->ichLim);
                        auto thisAssignmentNode = ast.Name(ast.Pids().thisAssignmentFunc());
                        auto callNode = ast.Call(thisAssignmentNode, ast.List(
                            ast.This(), ast.String(name), ast.Dot(ast.This(), ast.Name(name)), definitionRef));
                        ast.SetExtent(stmt->ichMin, stmt->ichLim);
                        ast.Replace(stmt, ast.List(stmt, callNode));

                        context->handle->RememberSingleNodeComments(context->alloc, /*searchLimOveride = */-1, nameNode, pnode, /* previousNode =*/nullptr, /* searchPreviousNode = */true, nameNode->ichMin);

                        // If we add a non-this dot assignment we need to let the code generator that it cannot optimize for this dot assignments.
                        pnode->sxFnc.SetHasNonThisStmt();
                    }
                }
            });
        }
    }

    // Returns true if the given node tree represents an object literal that
    // looks like a property descriptor.
    bool IsPropertyDescriptor(Pids* pids, ParseNode *object)
    {
        if (object->nop != knopObject) return false;
        bool result = true;
        bool anyFlagProperty = false;
        bool anyValueProperty = false;
        ASTHelpers::List::Traverse(object->sxUni.pnode1, [&](ParseNode *node) -> bool
        {
            if ((node->nop == knopMember || node->nop == knopMemberShort) && IsNameNode(node->sxBin.pnode1))
            {
                auto name = node->sxBin.pnode1->sxPid.pid;
                if(name == pids->enumerable() || name == pids->configurable() || name == pids->writable()) anyFlagProperty = true;
                    else if(name == pids->value() || name == pids->get() || name == pids->set()) anyValueProperty = true;
                        else result = false;
            } 
            else
            {
                result = false;
            }
            return result;
        }, false);
        // One or more of the flag properties (enumerable/configurable/writable) are required.
        // One or more of (value/get/set) are required.
        result = result && anyFlagProperty && anyValueProperty; 
        return result;
    }

    void RewriteObjectDocComments(ParseNode *pnodeObject, RewriteContext *context)
    {
        Assert(pnodeObject);
        Assert(context);
        Assert(pnodeObject->nop == knopObject);

        if (IsZeroExtent(pnodeObject) || (pnodeObject->grfpn & fpnSyntheticNode))
            return;

        if (!pnodeObject->sxUni.pnode1)
            return;

        // Don't generate meta-data for property descriptors.
        if (IsPropertyDescriptor(context->pids, pnodeObject))
            return;

        Parser* parser = context->tree->GetParser();


        // If the reference to the current node is already passed, check that first.
        ParseNode ** ppnodeObject = context->lastReferenceToNode;
        if (ppnodeObject == nullptr || *ppnodeObject != pnodeObject)
        {
            auto pnodeParent = context->GetParent();
            if (pnodeParent)
            {
                // Find the object node reference in the parent -- slower find.
                ppnodeObject = FindNodeReference(pnodeParent, pnodeObject, true);
                Assert(ppnodeObject && *ppnodeObject == pnodeObject);
            }
        }
#if DBG
        else
        {
            // Do verification that the reference to the object due to reference passing is same as returned of FindNodeReferenced.
            auto pnodeParent = context->GetParent();
            if (pnodeParent)
            {
                ParseNode ** ppnode = FindNodeReference(pnodeParent, pnodeObject, true);
                Assert(ppnode == ppnodeObject);
            }
        }
#endif

        if (ppnodeObject == nullptr || *ppnodeObject != pnodeObject)
        {
            Assert(false); // Lets find out when we don't get the ppnodeObject.
            return;
        }

        auto fncNode = context->GetFunction();
        Assert(fncNode);

        // Rewrite the object to add documentation for its fields
        ASTBuilder<Pids> ast(parser, fncNode, context->pids);        
        ast.SetExtent(pnodeObject->ichLim);

        // Add fieldDoc property for every field to the parent object
        //
        // Replace:
        //  { 
        //      f1: value1, 
        //      f2: value2,
        //      ...
        //      f1001:value1001,
        //  }
        //
        //  With:
        //          Object.defineProperties(
        //                          { 
        //                              f1: value1, 
        //                              f2: value2 
        //                              ...
        //                              f1000:value1001,
        //                          },
        //                          {
        //                              _$fieldDoc_f1000 : {value: <DefinitionRefAST>, enumerable: false, writable: true, configurable: true},
        //                              ...
        //                              _$fieldDoc_f2: {value: <DefinitionRefAST>, enumerable: false, writable: true, configurable: true},
        //                              _$fieldDoc_f1: {value: <DefinitionRefAST>, enumerable: false, writable: true, configurable: true},
        //                              _$defLoc: { value: { offset: ichMin, endOffset: ichLim }, enumerable: false, writable: true, configurable: true},
        //                          }
        //                      )
        // Notes:
        //      1. Use Object.defineProperties to make sure the fieldDoc properties are not enumerable
        //      2. FieldDoc definitions introduce new integer constants that can fill the constant table and cause the code generator
        //         to run out of memory. We limit the number of total fieldDocs we add per function to avoid filling the constant table

        ParseNodePtr fieldDocMembers = nullptr;
        ParseNodePtr previousMember = nullptr;
        int currentMemberCount = context->memberCount;
        ForEachMemberToDescribe(pnodeObject, /* includeComputedProperties = */true, [&](ParseNode *pnodeMember)
        {
            if (currentMemberCount < MAX_MEMBERS_IN_FUNCTION_CALL)
            {
                bool isComputedProperty = pnodeMember->sxBin.pnode1->nop == knopComputedName;

                bool vsdocFound = context->handle->RememberNodeComments(context->alloc, /* searchLimOverride= */-1, pnodeMember, pnodeObject, previousMember, /* searchPreviousNode = */false,
                    [&](CommentTable* commentTable, int commentTableIndex){
                    if (!isComputedProperty)
                    {
                        // Associate the comment location before the field with the field
                        commentTable->Associate(pnodeMember->ichMin, commentTableIndex);
                    }

                    if (pnodeMember->sxBin.pnode2 && pnodeMember->sxBin.pnode2->nop == knopFncDecl)
                    {
                        // For functions, also associate the comment before the field with the function definition location.
                        // This allows to find the comment when we only have a function object but not the parent object and
                        // therefore we can't get the comment location from _$fieldDoc$ parent object field.
                        // for example:
                        // var x = {
                        //      /* comment */
                        //      f: function() {}
                        //      ^  ^
                        //      ASSOCIATE BOTH LOCATIONS  
                        // };  
                        commentTable->Associate(pnodeMember->sxBin.pnode2->ichMin, commentTableIndex);
                    }
                });

                if (!isComputedProperty)
                {
                    auto pidFieldName = pnodeMember->sxBin.pnode1->sxPid.pid;

                    // Construct a field name for doc _$fieldDoc$<documented field name>
                    auto fieldDocMemberName = JsValueDoc::DocFieldName(context->alloc, pidFieldName->Psz());

                    auto fieldDocMemberValue = DefinitionRefPropertyDescriptorAST(ast, context->fileId, pnodeMember->ichMin);

                    // Add _$fieldDoc$<documented field name> : <fieldDocMemberValue> to the list to be added to the parent object
                    auto fieldDocMember = ast.Member(fieldDocMemberName, fieldDocMemberValue);
                    ast.Prepend(fieldDocMembers, fieldDocMember);

                    // Check if there is a field document comment with a elementType, if so, rewrite the initialization 
                    // to record the element type on the array.
                    CommentBuffer* comments;

                    if (vsdocFound)
                    {
                        comments = context->handle->GetLastRememberedComments(context->alloc, CommentType::commenttypeVSDoc);
                    }
                    else
                    {
                        //
                        // This is done to support the @property syntax for JSDoc
                        //
                        // The JSDoc spec says that a property is described using the following syntax
                        //
                        // /**
                        //  * @property {String} name - the name of the propertyBag
                        //  */
                        // var propertyBag = {
                        //   name: "propertyValue"
                        // };
                        //
                        // In order to support this, we need to use the JSDoc comment buffer associated with the member.
                        // The association work is done in RewriteVarDeclDocComments and RewriteAssignmentComments previously.
                        // Now we are merely consuming the previously built association to build the fieldDocs AST.
                        // Also note that if VSDoc is found, it takes precedence.
                        //
                        comments = context->handle->GetCommentTable()->GetCommentsBefore(context->alloc, context->handle->Text(), pnodeMember->ichMin, CommentType::commenttypeJSDoc);
                    }

                    if (comments && comments->GetBuffer() && HasRelevantTags(comments->Sz()))
                    {
                        FieldDocComments *docComments;
                        if (SUCCEEDED(ParseFieldDocComments(context->alloc, pidFieldName->Psz(), comments->Sz(), comments->GetCommentType(), /* isGlobalVariableAsField = */false, &docComments)))
                        {
                            if (docComments && (docComments->elementType || (!pnodeMember->sxBin.pnode2 && docComments->type)))
                            {
                                auto parser = context->tree->GetParser();
                                ASTBuilder<Pids> ast(parser, context->GetFunction(), context->pids);
                                ast.SetExtent(pnodeMember->sxBin.pnode2 ?
                                    pnodeMember->sxBin.pnode2->ichMin :
                                    pnodeMember->ichLim, pnodeMember->ichLim);

                                ParseNodePtr members = nullptr;
                                DocCommentAST(ast, context->alloc, members, docComments);
                                if (members)
                                {
                                    auto docAst = ast.Object(members);
                                    auto value = pnodeMember->sxBin.pnode2;
                                    pnodeMember->sxBin.pnode2 = ast.Call(ast.Name(Names::initVar), ast.List(value, docAst));
                                }
                            }
                        }
                    }

                    currentMemberCount++;
                }

                if (pnodeMember->nop != knopList)
                {
                    previousMember = pnodeMember;
                }
            }
        });

        // add the _$defLoc member to the literal which contains the offset, endOffset, and fileId
        auto defLocValue = ObjectLiteralDefinitionLocationDescriptorAST(ast, pnodeObject, context->fileId);
        auto pids = ast.Pids();
        auto objectLiteralLocationDefMember = ast.Member(pids.defLoc(), defLocValue);
        ast.Prepend(fieldDocMembers, objectLiteralLocationDefMember);
        currentMemberCount++;

        context->memberCount = currentMemberCount;

        if (fieldDocMembers)
        {
            auto newObjectDefinition = CallObjectDefinePropertiesAST(ast, pnodeObject, ast.Object(fieldDocMembers));

            // Point the parent of the object to the new version of the object
            *ppnodeObject = newObjectDefinition;
        }

    }

    static bool ExpressionMightResultInNullOrUndefined(ParseNode *pnode)
    {
        if (!pnode) return true;

        switch (pnode->nop)
        {
        case knopObject: 
        case knopStr:
        case knopFlt:
        case knopInt:
        case knopFncDecl:
            return false;

        case knopNull: 
        case knopName:
        case knopCall:
            return true;

        default:
            {
                uint fnop = ParseNode::Grfnop(pnode->nop);
                if (fnop & fnopBin)
                    return ExpressionMightResultInNullOrUndefined(pnode->sxBin.pnode1) || ExpressionMightResultInNullOrUndefined(pnode->sxBin.pnode2);
                else if (fnop & fnopUni)
                    return ExpressionMightResultInNullOrUndefined(pnode->sxUni.pnode1);
            }

        }
        return true;
    }

    void AddFieldDocForDecl(ParseNode *pnode, RewriteContext *context, IdentPtr name, uint nameLocation)
    {
        Parser* parser = context->tree->GetParser();
        ASTBuilder<Pids> ast(parser, context->GetFunction(), context->pids, context->GetBlockFromGlobal());

        auto docFieldName = JsValueDoc::DocFieldName(context->alloc, name->Psz());

        ast.SetExtent(pnode->ichMin);

        auto definitionRef = DefinitionRefAST(ast, context->fileId, nameLocation);
        ParseNodePtr docNode = nullptr;
        if (pnode->nop == knopVarDecl)
        {
            docNode = ast.Var(docFieldName, definitionRef);
        }
        else
        {
            docNode = ast.Let(docFieldName, definitionRef);
        }

        ast.Replace(pnode, ast.List(docNode, pnode));
    }

    int __cdecl ParseNodeComparer(__in void* context, __in const void* item1, __in const void* item2)
    {
        const DWORD_PTR *p1 = reinterpret_cast<const DWORD_PTR*>(item1);
        const DWORD_PTR *p2 = reinterpret_cast<const DWORD_PTR*>(item2);

        ParseNode * pnode1 = (ParseNode *)(*p1);
        ParseNode * pnode2 = (ParseNode *)(*p2);

        if (pnode1->ichMin < pnode2->ichMin)
        {
            return -1;
        }
        else if (pnode1->ichMin > pnode2->ichMin)
        {
            return 1;
        }

        return 0;
    }

    void RewriteClassDeclDocComments(ParseNode *pnode, RewriteContext *context)
    {
        Assert(pnode);
        Assert(context);
        Assert(pnode->nop == knopClassDecl);

        // Add field docs for class declarations in the global scope 
        if (pnode->sxClass.isDeclaration && context->level == 0 && context->blockScopeLevel == 0)
        {
            Assert(pnode->sxClass.pnodeDeclName && pnode->sxClass.pnodeDeclName->nop == knopLetDecl);
            auto name = pnode->sxClass.pnodeDeclName;
            auto nameLocation = context->tree->LanguageServiceExtension()->IdentMin(name);
            if (!nameLocation) nameLocation = name->ichMin;
            AddFieldDocForDecl(pnode, context, name->sxVar.pid, nameLocation);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::HiResTimer timer;
        double start = timer.Now();
#endif

        // Traversing class's members happens out of order - putting comments on each member will be very difficult.
        // Below code will try to sort them based on member's ichMin and then apply comments on them.

        JsUtil::List<ParseNode*, ArenaAllocator> allClassChildren(context->alloc);

        if (pnode->sxClass.pnodeConstructor)
        {
            allClassChildren.Add(pnode->sxClass.pnodeConstructor);
        }
        if (pnode->sxClass.pnodeMembers)
        {
            ASTHelpers::List::ForEach(pnode->sxClass.pnodeMembers, [&](ParseNode *pnodeMember)
            {
                allClassChildren.Add(pnodeMember);
            });
        }
        if (pnode->sxClass.pnodeStaticMembers)
        {
            ASTHelpers::List::ForEach(pnode->sxClass.pnodeStaticMembers, [&](ParseNode *pnodeMember)
            {
                allClassChildren.Add(pnodeMember);
            });
        }

        if (allClassChildren.Count() > 1)
        {
            allClassChildren.Sort(ParseNodeComparer, NULL/*no context*/);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        double diff = timer.Now() - start;
        if (diff > 5.0) // greater then 5.0 msec should make sense
        {
            OUTPUT_TRACE(Js::JSLSStatsPhase, L"DocCommentsRewrite::RewriteClassDeclDocComments time spent %8.3f on entries : %d\n", diff, allClassChildren.Count());
        }
#endif

        ParseNodePtr previousMember = nullptr;
        for (int i = 0; i < allClassChildren.Count(); i++)
        {
            ParseNode * currentNode = allClassChildren.Item(i);
            if ((currentNode->nop == knopMember || currentNode->nop == knopMemberShort)
                && currentNode->sxBin.pnode2
                && currentNode->sxBin.pnode2->nop == knopFncDecl)
            {
                context->handle->RememberSingleNodeComments(context->alloc, /*searchLimOveride = */-1, currentNode, pnode, previousMember, /* searchPreviousNode = */false, currentNode->sxBin.pnode2->ichMin);
            }
            previousMember = currentNode;
        }
    }

    void RewriteVarDeclDocComments(ParseNode *pnode, RewriteContext *context)
    {
        Assert(pnode);
        Assert(context);
        Assert(pnode->nop == knopVarDecl || pnode->nop == knopConstDecl || pnode->nop == knopLetDecl);

        auto varPid = pnode->sxVar.pid;
        if (varPid && !InternalName(varPid))
        {
            auto parent = context->GetParent();
            if (parent && parent->nop != knopForIn && parent->nop != knopForOf)
            {
                auto nameLocation = context->tree->LanguageServiceExtension()->IdentMin(pnode);
                if (!nameLocation) nameLocation = pnode->ichMin;

                context->handle->RememberNodeComments(context->alloc, /* searchLimOverride= */pnode->ichMin, /* node = */nullptr, context->GetParent(), context->previousNode, /* searchPreviousNode = */false, [&](CommentTable* commentTable, int commentTableIndex)
                {
                    if (pnode->sxVar.pnodeInit != nullptr && pnode->sxVar.pnodeInit->nop == knopObject)
                    {
                        ForEachMemberToDescribe(pnode->sxVar.pnodeInit, /* includeComputedProperties = */false, [&](ParseNode *pnodeMember){
                            commentTable->Associate(pnodeMember->ichMin, commentTableIndex);
                        });
                    }
                    commentTable->Associate(nameLocation, commentTableIndex);
                });

                // Add field document values for vars in the global scope
                // Except if we're inside of a block and the node is a let or a const
                if (context->level == 0 && (pnode->nop == knopVarDecl || context->blockScopeLevel == 0))
                {
                    AddFieldDocForDecl(pnode, context, varPid, nameLocation);
                }

                // If there is a doc comment with an elementType, or an expression that might result in null or undefined and the doc
                // comment has a type attribute, rewrite the initialization to save the doc comment information.
                if (!ASTHelpers::IsArgument(context->GetFunction(), pnode))
                {
                    auto comments = context->handle->GetLastRememberedComments(context->alloc, commenttypeAnyDoc);
                    if (comments && comments->GetBuffer())
                    {
                        VarDocComments *docComments;
                        if (SUCCEEDED(ParseVarDocComments(context->alloc, comments->Sz(), comments->GetCommentType(), &docComments)))
                        {
                            if (docComments && (docComments->elementType || (docComments->type && ExpressionMightResultInNullOrUndefined(pnode->sxVar.pnodeInit))))
                            {
                                auto parser = context->tree->GetParser();
                                ASTBuilder<Pids> ast(parser, context->GetFunction(), context->pids);
                                ast.SetExtent(pnode->sxVar.pnodeInit ? pnode->sxVar.pnodeInit->ichMin : pnode->ichLim, pnode->ichLim);
                            
                                ParseNodePtr members = nullptr;
                                DocCommentAST(ast, context->alloc, members, docComments);
                                if (members)
                                {
                                    auto docAst = ast.Object(members);
                                    // Make sure we don't generate meta-data information for this literal.
                                    docAst->grfpn |= fpnSyntheticNode;
                                    auto value = pnode->sxVar.pnodeInit ? pnode->sxVar.pnodeInit : ast.Undefined();
                                    pnode->sxVar.pnodeInit = ast.Call(ast.Name(Names::initVar), ast.List(value, docAst));
                                }
                            }
                        }
                    }
                }

                // Associate comments to the function in the intialization of the varDecl if it exists
                if (pnode->sxVar.pnodeInit && pnode->sxVar.pnodeInit->nop == knopFncDecl)
                {
                    // Something like: var f = function() { ... }
                    auto funcDecl = pnode->sxVar.pnodeInit;
                    context->handle->RememberSingleNodeComments(context->alloc, /* searchLimOverride= */-1, funcDecl, parent, context->previousNode, /* searchPreviousNode = */false, funcDecl->ichMin);
                }
            }
        }
    }

    void RewriteAssignmentComments(ParseNode *pnodeAssignment, RewriteContext *context)
    {
        Assert(pnodeAssignment);
        Assert(context);
        Assert(pnodeAssignment->nop == knopAsg);

        // Associate comments to the function in the target of the assignment if it exists
        if (pnodeAssignment->sxBin.pnode2 && pnodeAssignment->sxBin.pnode2->nop == knopFncDecl)
        {
            // Something like: obj.f = function() { ... }
            auto funcDecl = pnodeAssignment->sxBin.pnode2;
            context->handle->RememberSingleNodeComments(context->alloc, /* searchLimOverride= */-1, funcDecl, context->GetParent(), context->previousNode, /* searchPreviousNode = */false, funcDecl->ichMin);
        }

        // Associate comments to the object members in the target of the assignment if it exists
        if (pnodeAssignment->sxBin.pnode2 != nullptr && pnodeAssignment->sxBin.pnode2->nop == knopObject)
        {
            context->handle->RememberNodeComments(context->alloc, /* searchLimOverride= */pnodeAssignment->ichMin, /* node = */nullptr, context->GetParent(), context->previousNode, /* searchPreviousNode = */false, [&](CommentTable* commentTable, int commentTableIndex)
            {
                ForEachMemberToDescribe(pnodeAssignment->sxBin.pnode2, /* includeComputedProperties = */false, [&](ParseNode *pnodeMember){
                    commentTable->Associate(pnodeMember->ichMin, commentTableIndex);
                });
            });
        }
    }

    void DocCommentsRewrite::PassReference(ParseNode **ppnode, RewriteContext * context)
    {
        // Store the current reference node to the context. This will be useful when trying to get the reference of the current node.
        // Currently PassReference will be called only walking the list.
        context->lastReferenceToNode = ppnode;
    }

    bool DocCommentsRewrite::Preorder(ParseNode *pnode, RewriteContext *context)
    {
        Assert(pnode);
        Assert(context);

        switch(pnode->nop)
        {
            case knopProg:
                RewriteFunctionDocComments(pnode, context);
                context->previousNode = nullptr;
                break;
            case knopFncDecl:
                context->memberCountStack.Push(context->memberCount);
                context->memberCount = 0;
                context->level++;
#if DEBUG
                if (context->level == 1)
                    context->nestedCount = pnode->sxFnc.nestedCount;
#endif
                RewriteFunctionDocComments(pnode, context);
                context->previousNode = nullptr;
                break;

            case knopBlock:
                // and constructs with implicit block scoping
            case knopFor:
            case knopForIn:
            case knopForOf:
            case knopSwitch:
                context->blockScopeLevel++;
                break;

            case knopObject:
                RewriteObjectDocComments(pnode, context);
                break;

            case knopVarDecl:
            case knopConstDecl:
            case knopLetDecl:
                if (InternalName(pnode->sxVar.pid))
                  break;

                RewriteVarDeclDocComments(pnode, context);
                break;

            case knopAsg:
                RewriteAssignmentComments(pnode, context);
                break;

            case knopClassDecl:
                RewriteClassDeclDocComments(pnode, context);
                break;
        }

        context->nodeStack.Push(pnode);

        return true;
    }

    void DocCommentsRewrite::Postorder(ParseNode *pnode, RewriteContext *context)
    {
        Assert(context);

        context->nodeStack.Pop();

        switch (pnode->nop)
        {
            case knopFncDecl:
            {
#if DEBUG
                if (context->level == 1)
                {
                    AssertMsg(
                        pnode->sxFnc.nestedCount == context->nestedCount, 
                        "Cannot add a nested function during this rewritting because it is used during deferred parsing which cannot tolerate the nestedCount changing");
                }
#endif
                context->level--;
                
                context->memberCount = context->memberCountStack.Pop();
                break;
            }
            case knopBlock:
                // and constructs with implicit block scoping
            case knopFor:
            case knopForIn:
            case knopForOf:
            case knopSwitch:
            {
                context->blockScopeLevel--;
                break;
            }
            default:
                break;
        }

        context->previousNode = pnode;
    }
} 