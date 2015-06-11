//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t displayText[] = L"displayText";
        const wchar_t _fire[] = L"_$fire";
        const wchar_t getFunctionComments[] = L"getFunctionComments";
        const wchar_t getText[] = L"getText";
        const wchar_t _getValue[] = L"_$getValue";
        const wchar_t _group[] = L"_$group";
        const wchar_t insertionText[] = L"insertionText";
        const wchar_t _msgs[] = L"_$msgs";
        const wchar_t _pos[] = L"_$pos";
        const wchar_t _setValue[] = L"_$setValue";
        const wchar_t  functionHelp[] = L"functionHelp";
        const wchar_t  completionItem[] = L"completionItem";
        const wchar_t  symbolHelp[] = L"symbolHelp";
        const wchar_t _propertyCompletionItem[] = L"_$propertyCompletionItem";
        const wchar_t  comments[] = L"comments";
        const wchar_t  inside[] = L"inside";
        const wchar_t  insideIsDoc[] = L"insideIsDoc";
        const wchar_t  above[] = L"above";
        const wchar_t  aboveIsDoc[] = L"aboveIsDoc";
        const wchar_t  paramComments[] = L"paramComments";
        const wchar_t  comment[] = L"comment";
        const wchar_t  functionComments[] = L"functionComments";

        // Private fields containing public property values
        const wchar_t  itemsField[] = L"_$items";
        const wchar_t  functionHelpField[] = L"_$functionHelp";
        const wchar_t  functionCommentsField[] = L"_$functionComments";
        const wchar_t  completionItemField[] = L"_$completionItem";
        const wchar_t  symbolHelpField[] = L"_$symbolHelpField";

        // Internal extensions event names - must match the names used in helpers.js.
        const wchar_t _onCompletion[] = L"_onCompletion";
        const wchar_t _onParameterHelp[] = L"_onParameterHelp";
        const wchar_t _onCompletionHint[] = L"_onCompletionHint";
    }

    static LPCWSTR NormalizeComment(ArenaAllocator* alloc, LPCWSTR text)
    {
        Assert(alloc);
        if(String::IsNullOrEmpty(text))
        {
            return text;
        }

        auto trimmed = String::Alloc(alloc, wcslen(text) + 1);
        auto start   = text;
        auto src     = start;
        auto dst     = trimmed;
        auto lastNonWhitespace = (dst - 1);
        while(*src)
        {
            if(src == start)
            {
                // Trim the leading space or a new line
                if(IsWhitespace(*src)) 
                {
                    // Strip the leading whitespace character.
                    src++;
                    continue;
                }
                if(*src == L'\r' && *(src + 1) == L'\n')
                {
                    // Strip the leading new line.
                    src += 2;
                    continue;
                }
            }
            if(IsWhitespace(*src) && src - start >= 2 && *(src - 2) == L'\r' && *(src - 1) == L'\n')
            {
                // Skip a whitespace after a new line.
                src++;
                continue;
            }
            if(!IsWhitespace(*src) && *src != L'\r' && *src != L'\n')
            {
                // Keep the position of the last non-whitespace character
                lastNonWhitespace = dst;
            }
            *dst = *src;
            src++;
            dst++;
        }
        // Add zero terminator after the last non-whitespace character thus stripping all whitespaces after it.
        *(lastNonWhitespace + 1) = L'\0';

        return trimmed;
    }

    static Js::ParseableFunctionInfo* GetTargetFunctionBody(Js::JavascriptFunction*& func, Js::ScriptContext* scriptContext)
    {
        auto docFunc = JsHelpers::GetProperty<Js::JavascriptFunction*>(func, Names::_doc, nullptr, scriptContext);
        if(docFunc)
        {
            // Use the func._$doc instead. This would be the case when a function was annotated using intellisense.annotate.
            func = docFunc;
        }
        return func->GetParseableFunctionInfo();
    }

    static Js::RecyclableObject* GetFunctionComments(Js::JavascriptFunction* func, FileAuthoring* fileAuthoring, ArenaAllocator* alloc, Js::ScriptContext* scriptContext) 
    {
        Assert(func);
        Assert(fileAuthoring);
        Assert(alloc);
        Assert(scriptContext);
        Js::RecyclableObject* obj = JsHelpers::CreateObject(scriptContext);
        auto body = GetTargetFunctionBody(func, scriptContext);
        if (body)
        {
            auto authoringFile = fileAuthoring->GetAuthoringFile(body);
            if (authoringFile)
            {
                //
                //  inside
                //
                CommentBuffer* inside = authoringFile->GetFunctionComments(alloc, scriptContext, func, commenttypeAny);
                if (inside)
                {
                    auto trimmed = NormalizeComment(alloc, inside->Sz());
                    JsHelpers::SetField(obj, Names::inside, trimmed, scriptContext);
                    JsHelpers::SetField(obj, Names::insideIsDoc, ((inside->GetCommentType() & commenttypeAnyDoc) != 0), scriptContext);
                }

                //
                //  above
                //
                auto charOffset = body->StartInDocument();
                auto above = authoringFile->GetNodeComments(alloc, charOffset, commenttypeAny);
                auto aboveText = above->Sz();
                auto trimmed = NormalizeComment(alloc, aboveText);
                JsHelpers::SetField(obj, Names::above, trimmed, scriptContext);
                JsHelpers::SetField(obj, Names::aboveIsDoc, ((above->GetCommentType() & commenttypeAnyDoc) != 0), scriptContext);

                //
                //  paramComments
                //
                Js::JavascriptArray* params = JsHelpers::CreateArray(0, scriptContext);
                JsHelpers::SetField(obj, Names::paramComments, params, scriptContext);
                // Parse the function declaration and iterate argument nodes to get arguments names & location
                authoringFile->ForEachArgument(alloc, scriptContext, func, 
                    [scriptContext, alloc, authoringFile, params](ParseNodePtr node, bool isRest)
                    { 
                        auto param = JsHelpers::CreateObject(scriptContext);
                        //
                        //  name
                        //
                        if(!node->sxVar.pid)
                            return;
                        JsHelpers::SetField(param, Names::name, node->sxVar.pid->Psz(), scriptContext);

                        //
                        //  comment
                        //
                        auto comment = authoringFile->GetNodeComments(alloc, node->ichMin, commenttypeAny)->Sz();
                        auto trimmed = NormalizeComment(alloc, comment);
                        JsHelpers::SetField(param, Names::comment, trimmed, scriptContext);
                        // Add the entry to paramComments
                        JsHelpers::Push(params, Convert::ToVar(param, scriptContext)); 
                    });
            }
        }
        return obj;
    }

    //
    //  Name:   ExtensionsInvoker
    //  Summary: Invokes IntelliSense extensions for the specified event with specified parameters.  
    //          
    template<typename TArg = void_t>
    class ExtensionsInvoker
    {
        Js::ScriptContext*      _scriptContext;
        ArenaAllocator*         _alloc;
        FileAuthoring*          _fileAuthoring;
        Js::RecyclableObject*   _intellisenseObj;
    public:
        ExtensionsInvoker(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, Js::RecyclableObject* intellisenseObj, FileAuthoring* fileAuthoring) : 
            _alloc(alloc), _scriptContext(scriptContext), _fileAuthoring(fileAuthoring), _intellisenseObj(intellisenseObj)
        {
            Assert(_alloc);
            Assert(_scriptContext);
            Assert(_fileAuthoring);
            Assert(_intellisenseObj);
        }

        void InvokeExtensions(LPCWSTR eventName, TArg arg = nullptr)
        {
            LangSvcMethods langSvcMethods(_alloc, _scriptContext, _fileAuthoring); 
            auto invokeHandlers = JsHelpers::GetProperty<Js::JavascriptFunction*>(_intellisenseObj, Names::_fire, nullptr, _scriptContext);
            if (!invokeHandlers)
                return;

            JsHelpers::WithArguments([&] (Js::Arguments& arguments)
            {
                Js::Var result;
                _fileAuthoring->ExecuteFunction(invokeHandlers, arguments, &result);
            }, 
            _scriptContext, 
            _intellisenseObj, 
            Convert::ToVar(eventName, _scriptContext),
            Convert::ToVar(&langSvcMethods, _scriptContext),
            Convert::ToVar(arg, _scriptContext));
        }
    };

    //
    //  Name: JsSourceFile
    //  Summary: Represents a source file object in extensions object model.
    //
    class JsSourceFile : public JsCallable
    {
        uint _sourceIndex;
    public:
        JsSourceFile(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, uint sourceIndex) 
            : JsCallable(alloc, scriptContext), _sourceIndex(sourceIndex) { }
    private:
        void ExposeProperties() override
        {
            ExposeFunction<FuncSignature<LPCWSTR, uint, uint>>(Names::getText, [&] (uint offset, uint length) -> LPCWSTR
            {
                if (length == 0)
                    return L"";
                Js::Utf8SourceInfo *source = _scriptContext->GetSource(_sourceIndex);
                if (!source->GetSource(L"JsSourceFile::ExposeFunction"))
                    return L"";
                auto text = AnewArray(_alloc, wchar_t, (length + 1));
                source->RetrieveSourceText(text, offset, offset + length);
                text[length] = 0;
                return text;
            });

            ExposeProperty<uint, uint>(Names::size, [&] () -> uint
            {
                Js::Utf8SourceInfo *source = _scriptContext->GetSource(_sourceIndex);
                return source->GetCchLength();
            },
                [&] (uint) 
            {
                // This is not allowed, ignore
            });
        }
    };

    //
    //  Name: Extent
    //  Summary: Creates an extent object for use in extensions object model.
    //
    class Extent
    {
    public:
        static Js::RecyclableObject* ToRecyclableObject(const AuthorFileRegion& extent, Js::ScriptContext* scriptContext)
        {
            return ToRecyclableObject(extent.offset, extent.length, scriptContext);
        }

        static Js::RecyclableObject* ToRecyclableObject(charcount_t offset, charcount_t length, Js::ScriptContext* scriptContext)
        {
            auto extent = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(extent, Names::start, (uint)offset, scriptContext);
            JsHelpers::SetField(extent, Names::length, (uint)length, scriptContext);
            return extent;
        }
    };

    //
    //  Name: Declaration
    //  Summary: A declaration of a function or a completion item in extensions object model.
    //
    class Declaration
    {
    public:
        static Js::RecyclableObject* ToRecyclableObject(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring, int fileId, charcount_t pos, charcount_t length)
        {
            auto sourceIndex = fileAuthoring->GetSourceIndexOf(fileId);
            auto obj = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(obj, Names::file, arena_new<JsSourceFile>(alloc, alloc, scriptContext, sourceIndex), scriptContext);
            JsHelpers::SetField(obj, Names::extent, Extent::ToRecyclableObject(pos, length, scriptContext), scriptContext);
            return obj;
        }
    };

    //
    //  Name: JsCompletionItem 
    //  Summary: Represents a completion item in extensions object model.
    //
    class JsCompletionItem : public JsCallable
    {
        Completions::InternalCompletion* _internalCompletion;
        Completions*                     _completions;
        Js::RecyclableObject*            _declaration;
    public:
        JsCompletionItem(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, Completions* completions, Completions::InternalCompletion* internalCompletion)
            : JsCallable(alloc, scriptContext), _completions(completions), _internalCompletion(internalCompletion), _declaration(nullptr) 
        { 
            Assert(alloc);
            Assert(scriptContext);
            Assert(completions);
            Assert(internalCompletion);
        }
        
        static void FromRecyclableObject(Js::RecyclableObject* item, Completions* completionSet, Js::ScriptContext* scriptContext)
        {
            Assert(item);
            Assert(completionSet);
            Assert(scriptContext);
            auto alloc = completionSet->Alloc();
            auto name = JsHelpers::GetProperty<LPCWSTR>(item, Names::name, alloc, scriptContext);
            auto glyph = JsHelpers::GetProperty<LPCWSTR>(item, Names::glyph, alloc, scriptContext);
            auto kindString = JsHelpers::GetProperty<LPCWSTR>(item, Names::kind, alloc, scriptContext);
            if (!name || !kindString) 
                return;
            auto group = JsHelpers::GetProperty<int>(item, Names::_group, alloc, scriptContext);

            AuthorCompletionKind kind = AuthorCompletionKind();
            if (!AuthorCompletionKindConverter::FromString(kindString, kind))
                return;

            Js::Var value = nullptr;
            if(kind != ackProperty)
            {
                value = JsHelpers::GetPropertyVar(item, Names::value, scriptContext);
            }

            auto parentObject = JsHelpers::GetProperty<Js::RecyclableObject*>(item, Names::parentObject, nullptr, scriptContext); 
            
            // Restore HintInfo
            auto hintInfo = Anew(alloc, HintInfo);
            auto fileIdVar = JsHelpers::GetPropertyVar(item, Names::_fileId, scriptContext);
            if(Js::TaggedInt::Is(fileIdVar))
            {
                hintInfo->fileId = Js::TaggedInt::ToInt32(fileIdVar);
                hintInfo->funcSourceMin = hintInfo->fileId >= 0 ? JsHelpers::GetProperty<uint>(item, Names::_pos, alloc, scriptContext) : 0;
            }
            hintInfo->scope = AuthorScopeConverter::FromString(JsHelpers::GetProperty<LPCWSTR>(item, Names::scope, alloc, scriptContext));
            
            completionSet->AddUnique(
                parentObject,
                value,
                kind,
                (AuthorCompletionFlags)group,
                completionSet->ToId(name), // TODO: displayText, insertionText
                glyph,
                hintInfo);
        }

    private:
        void ExposeProperties() override
        {
            ExposeField(Names::name, _internalCompletion->name.Buffer());
            ExposeField(Names::glyph, _internalCompletion->glyph.Buffer());
            ExposeField(Names::_group, (int)_internalCompletion->group);

            LPCWSTR kind = AuthorCompletionKindConverter::ToString(_internalCompletion->kind);
            ExposeField(Names::kind, kind);

            if (_internalCompletion->parentObject)
                ExposeField(Names::parentObject, _internalCompletion->parentObject);

            if (_internalCompletion->value)
            {
                Js::RecyclableObject* value;
                if (Convert::FromVar(nullptr, _internalCompletion->value, value)) 
                {
                    ExposeField(Names::value, value);
                }
                else
                {
                    int value;
                    if (Convert::FromVar(nullptr, _internalCompletion->value, value))
                    {
                        ExposeField(Names::value, value);
                    }
                }                
            } 

            if (_internalCompletion->hintInfo)
            {
                ExposeField(Names::scope, AuthorScopeConverter::ToString(_internalCompletion->hintInfo->scope));

                // Make sure we don't loose sourceFileIndex, funcSourceMin in the roundtrip since they may be needed later for completion hint.
                if (_internalCompletion->hintInfo->fileId >= 0)
                {
                    ExposeField(Names::_fileId, _internalCompletion->hintInfo->fileId);
                    ExposeField(Names::_pos, _internalCompletion->hintInfo->funcSourceMin);
                }
            }

            ExposeProperty<LPCWSTR, LPCWSTR>(Names::comments, [&] () -> LPCWSTR
            {
                //
                //  Nested helper to get a comment given a file id & position
                //
                auto fileAuthoring = _completions->GetFileAuthoring();
                auto alloc         = _alloc;
                auto ResolveComment = [fileAuthoring, alloc](int fileId, charcount_t pos) -> LPCWSTR
                {
                    Assert(fileId >= 0);
                    auto file = fileAuthoring->GetAuthoringFileById(fileId);
                    if(file) 
                    {
                        // Accept any comment type (//, ///, /**/ are fine) for extensions to use. 
                        auto comment = file->GetNodeComments(alloc, pos, commenttypeAny);
                        return comment->Sz();
                    }
                    return L"";
                };

                //
                //  Try to get comment location for the completion item
                //
                JsValueDoc* doc = nullptr;
                auto valueObj = _internalCompletion->value ? Convert::FromVar<Js::RecyclableObject>(_internalCompletion->value) : nullptr;
                if(_internalCompletion->hintInfo && _internalCompletion->hintInfo->fileId >= 0)
                {
                    // Comment location came from scope record
                    doc = JsValueDoc::CreateRef(_alloc, _internalCompletion->hintInfo->fileId, _internalCompletion->hintInfo->funcSourceMin);
                }
                else
                {
                    if(!doc && _internalCompletion->parentObject)
                    {
                        // Try to get field / global variable comments 
                        doc = JsValueDoc::GetFieldDoc(_alloc, _internalCompletion->parentObject, _internalCompletion->name.Buffer(), _scriptContext);
                    }
                }

                LPCWSTR comment = L"";

                if(doc && doc->fileId >= 0) 
                {
                    // Comment location is available, get the comment text 
                    comment = ResolveComment(doc->fileId, doc->pos);
                }

                if(String::IsNullOrEmpty(comment) && valueObj && valueObj->GetTypeId() == Js::TypeIds_Function) 
                {
                    // No comment location was available, or no comment was found at the location.
                    // If completion item represents a function, use function location to get the comment above it.
                    auto function = static_cast<Js::JavascriptFunction*>(valueObj);
                    auto body = GetTargetFunctionBody(function, _scriptContext); 
                    if (body)
                    {
                        auto charOffset = body->StartInDocument();
                        auto fileId = fileAuthoring->GetFileIdOf(body->GetSourceIndex());
                        if(fileId > 0) 
                        {
                            comment = ResolveComment(fileId, charOffset);
                        }
                    }
                }
                return NormalizeComment(_alloc, comment);
            },
                [&] (LPCWSTR) 
            {
                // This is not allowed, ignore
            });
        }

        Js::RecyclableObject* CreateUnderlyingObject() override
        {
            if(!_internalCompletion->value && _internalCompletion->kind == ackProperty && _internalCompletion->parentObject)
            {
                auto propertyCompletionItem = JsHelpers::GetProperty<Js::RecyclableObject*>(_scriptContext->GetGlobalObject(), Names::_propertyCompletionItem, nullptr, _scriptContext);
                if (propertyCompletionItem)
                    return JsHelpers::CreateObject(_scriptContext, propertyCompletionItem);
            }
            return JsCallable::CreateUnderlyingObject();
        }
    };

    //
    //  Name: EventArgsBase
    //  Note: A base class for event argument objects. 
    //        Inherits JsCallable which enabled lazy initialization of properties. 
    class EventArgsBase : public JsCallable
    {
    protected:
        FileAuthoring* _fileAuthoring;
        EventArgsBase(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring)
            : JsCallable(alloc, scriptContext), _fileAuthoring(fileAuthoring)
        {
            Assert(_fileAuthoring);
        }
    };

    //
    //  CompletionEventArgs
    //
    class CompletionEventArgs : public EventArgsBase
    {
        Js::Var               _target;
        LPCWSTR               _targetName;  
        uint                  _offset;
        Completions*          _completions;
    public:
        CompletionEventArgs(
            ArenaAllocator* alloc, 
            Js::ScriptContext* scriptContext, 
            FileAuthoring* fileAuthoring,
            Js::Var target, 
            LPCWSTR targetName,
            uint offset, 
            Completions* completions) 
            : EventArgsBase(alloc, scriptContext, fileAuthoring), _target(target), _targetName(targetName), _offset(offset), _completions(completions) { }

        void TakeExternalChanges(bool preserveNonRemovable)
        {
            auto items = OnDemandProperty::Get<Js::JavascriptArray*>(Names::itemsField, this);
            if (!items)
                return;

            _completions->Clear(preserveNonRemovable);

            uint len = items->GetLength();
            for (uint i = 0; i < len; i++)
            {
                auto item = JsHelpers::GetArrayElement<Js::RecyclableObject*>(items, i, _alloc);
                if (item)
                {
                    JsCompletionItem::FromRecyclableObject(item, _completions, _scriptContext);
                }
            }
        }

    private:

        void ExposeProperties() override
        {
            EventArgsBase::ExposeProperties();

            //
            //  target
            //
            if (_target)
            {
                // Target can be anything including TaggedInt, String, Function or RecyclableObject, expose it as-is.
               JsHelpers::SetFieldVar(this->AsRecyclableObject(), Names::target, _target, _scriptContext, true);
            }
            
            //
            //  targetName
            //
            if (_targetName)
            {
                ExposeField(Names::targetName, _targetName);
            }

            //
            // items
            //

            ExposeProperty<Js::JavascriptArray*, Js::JavascriptArray*>(Names::items, [&] () -> Js::JavascriptArray*
            { 
                auto capturedThis = this;
                return OnDemandProperty::GetOrCreate<Js::JavascriptArray*>(Names::itemsField, capturedThis, [capturedThis]() -> Js::JavascriptArray* 
                {
                    auto recapturedThis = capturedThis;
                    return JsHelpers::CreateArray(recapturedThis->_completions->Items()->Count(), [recapturedThis] (uint index) -> JsCompletionItem*
                    {
                        return arena_new<JsCompletionItem>(recapturedThis->_alloc, recapturedThis->_alloc, recapturedThis->_scriptContext, recapturedThis->_completions, recapturedThis->_completions->Items()->Item(index));
                    }, recapturedThis->_scriptContext);
                });
            },
                [&](Js::JavascriptArray* items)
            {
                // Setter
                if (items)
                {
                    OnDemandProperty::Set(Names::itemsField, this, items);
                }
            });

            //
            //  scope
            //
            LPCWSTR scope = nullptr;
            AuthorCompletionSetKind kind;
            _completions->get_Kind(&kind);
            switch(kind)
            {
            case acskMember:
                scope = Names::members;
                break;
            case acskStatement:
                scope = Names::global;
                break;
            }
            ExposeField<LPCWSTR>(Names::scope, scope);
        }
    };

    //
    //  ParameterHelpEventArgs
    //
    class ParameterHelpEventArgs : public EventArgsBase
    {
        Js::JavascriptFunction* _target;
        Js::RecyclableObject*   _parentObject;
        uint                    _offset;
        IAuthorFunctionHelp**   _funcHelp;
    public:
        ParameterHelpEventArgs(
            ArenaAllocator*         alloc, 
            Js::ScriptContext*      scriptContext, 
            FileAuthoring*          fileAuthoring,
            Js::JavascriptFunction* target, 
            uint                    offset,
            Js::RecyclableObject*   parentObject,
            IAuthorFunctionHelp**   funcHelp) 
            : EventArgsBase(alloc, scriptContext, fileAuthoring), _target(target), _offset(offset), _parentObject(parentObject), _funcHelp(funcHelp) 
        {
            Assert(_funcHelp && *_funcHelp);
        }

        void TakeExternalChanges(PageAllocator* pageAlloc)
        {
            auto jsFuncHelp = OnDemandProperty::Get<Js::RecyclableObject*>(Names::functionHelpField, this);
            if(!jsFuncHelp)
                return;
            auto originalFuncHelp = *_funcHelp;
            // Create a new IAuthorFunctionHelp object
            (*_funcHelp) = FunctionHelpInfo::FromRecyclableObject(jsFuncHelp, pageAlloc, _scriptContext, _fileAuthoring);
            // Release the original IAuthorFunctionHelp object
            originalFuncHelp->Release();
        }

    private:
        void ExposeProperties() override
        {
            EventArgsBase::ExposeProperties();

            //
            //  target 
            //
            if (_target)
            {
                // Target can be anything including TaggedInt, String, Function or RecyclableObject, expose it as-is.
               JsHelpers::SetField(this->AsRecyclableObject(), Names::target, _target, _scriptContext, true);
            }

            //
            //  parentObject 
            //
            if (_parentObject)
            {
                // Target can be anything including TaggedInt, String, Function or RecyclableObject, expose it as-is.
               JsHelpers::SetField(this->AsRecyclableObject(), Names::parentObject, _parentObject, _scriptContext, true);
            }

            //
            // functionHelp
            //
            ExposeProperty<Js::RecyclableObject*, Js::RecyclableObject*>(Names::functionHelp, [&] () -> Js::RecyclableObject*
            { 
                auto capturedThis = this;
                return OnDemandProperty::GetOrCreate<Js::RecyclableObject*>(Names::functionHelpField, capturedThis, [capturedThis]() -> Js::RecyclableObject* 
                {
                    return FunctionHelpInfo::ToRecyclableObject((*capturedThis->_funcHelp), capturedThis->_scriptContext);
                });
            },
                [&](Js::RecyclableObject* value)
            {
                // Setter
                if (value)
                {
                    OnDemandProperty::Set(Names::functionHelpField, this, value);
                }
            });

            //
            // functionComments
            //
            if(_target)
            {
                ExposeProperty<Js::RecyclableObject*, Js::RecyclableObject*>(Names::functionComments, [&] () -> Js::RecyclableObject*
                { 
                    auto capturedThis = this;
                    return OnDemandProperty::GetOrCreate<Js::RecyclableObject*>(Names::functionCommentsField, capturedThis, [capturedThis]() -> Js::RecyclableObject* 
                    {
                        return GetFunctionComments(capturedThis->_target, capturedThis->_fileAuthoring, capturedThis->_alloc, capturedThis->_scriptContext);
                    });
                },
                    [&](Js::RecyclableObject* value)
                {
                    // Setter - not supported
                });
            }
        }
    };

    //
    //  CompletionHintEventArgs
    //
    class CompletionHintEventArgs : public EventArgsBase
    {
        IAuthorSymbolHelp**              _symbolHelp;
        Completions::InternalCompletion* _completionItem;
        Completions*                     _completions;
    public:
        CompletionHintEventArgs(
            ArenaAllocator*                  alloc, 
            Js::ScriptContext*               scriptContext, 
            FileAuthoring*                   fileAuthoring,
            Completions*                     completions,
            Completions::InternalCompletion* completionItem,
            IAuthorSymbolHelp**              symbolHelp) 
            : EventArgsBase(alloc, scriptContext, fileAuthoring), 
            _completions(completions),
            _completionItem(completionItem), 
            _symbolHelp(symbolHelp) 
        {
            Assert(_symbolHelp && *_symbolHelp);
        }

        void TakeExternalChanges(PageAllocator* pageAlloc)
        {
            auto jsSymbolHelp = OnDemandProperty::Get<Js::RecyclableObject*>(Names::symbolHelpField, this);
            if(!jsSymbolHelp)
                return;
            auto originalSymbolHelp = *_symbolHelp;
            // Create a new IAuthorSymbolHelp object
            *_symbolHelp = SymbolHelp::FromRecyclableObject(jsSymbolHelp, pageAlloc, _scriptContext, _fileAuthoring);
            // Release the original IAuthorSymbolHelp object
            originalSymbolHelp->Release();
        }
    private:        
        void ExposeProperties() override
        {
            EventArgsBase::ExposeProperties();

            //
            // completionItem
            //
            ExposeProperty<Js::RecyclableObject*, Js::RecyclableObject*>(Names::completionItem, [&] () -> Js::RecyclableObject*
            { 
                // Getter
                auto capturedThis = this;
                return OnDemandProperty::GetOrCreate<Js::RecyclableObject*>(Names::completionItemField, capturedThis, [capturedThis]() -> Js::RecyclableObject* 
                {
                    return arena_new<JsCompletionItem>(capturedThis->_alloc, capturedThis->_alloc, capturedThis->_scriptContext, capturedThis->_completions, capturedThis->_completionItem)->AsRecyclableObject();
                });
            },
                [&](Js::RecyclableObject* value)
            {
                // Setter - not allowed
            });

            //
            //  symbolHelp
            //
            ExposeProperty<Js::RecyclableObject*, Js::RecyclableObject*>(Names::symbolHelp, [&] () -> Js::RecyclableObject*
            { 
                // Getter
                auto capturedThis = this;
                return OnDemandProperty::GetOrCreate<Js::RecyclableObject*>(Names::symbolHelpField, capturedThis, [capturedThis]() -> Js::RecyclableObject* 
                {
                    return SymbolHelp::ToRecyclableObject(*capturedThis->_symbolHelp, capturedThis->_scriptContext);
                });
            },
                [&](Js::RecyclableObject* value)
            {
                // Setter - not allowed
            });
        }
    };

    //
    //  Name: LangSvcMethods 
    //  Summary: Contains methods to be exposed directly on 'intellisense' object.
    //
    class LangSvcMethods: public JsCallable
    {
        FileAuthoring* _fileAuthoring;
    public:
        LangSvcMethods(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring) 
            : JsCallable(alloc, scriptContext), _fileAuthoring(fileAuthoring)
        {
            Assert(_fileAuthoring);
        }
    private:
        void ExposeProperties() override
        {
            //
            // getFunctionComments method
            //
            ExposeFunction<FuncSignature<Js::RecyclableObject*, Js::JavascriptFunction*>>(Names::getFunctionComments, [&] (Js::JavascriptFunction* func) -> Js::RecyclableObject*
            {
                if (func)
                {
                    return GetFunctionComments(func, _fileAuthoring, _alloc, _scriptContext);
                }
                return nullptr;
            });
        }
    };

    //
    //  Name:    LangSvcExtensibility
    //  Summary: Enables calling extensions event handlers
    //

    LangSvcExtensibility::LangSvcExtensibility(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring) 
        : _fileAuthoring(fileAuthoring), _scriptContext(scriptContext), _alloc(alloc)  
    {
        // The underlying object should be the value of the global 'intellisense' object. This ensures that we can call events registered 
        // up the copy-on-write contexts chain.
        _intellisenseObj = JsHelpers::GetProperty<Js::RecyclableObject*>(_scriptContext->GetGlobalObject(), ExtensionsObjectName, _alloc, _scriptContext);
    }

    void LangSvcExtensibility::FireOnCompletion(Completions* completions, charcount_t offset, Js::Var target, LPCWSTR targetName)
    {
        if (HasExtensions(Names::_onCompletion))
        {
            AuthorCompletionSetKind kind;
            completions->get_Kind(&kind);
            auto eventArgs = arena_new<CompletionEventArgs>(_alloc, _alloc, _scriptContext, _fileAuthoring, (kind == acskMember ? target : nullptr), targetName, offset, completions);
            InvokeExtensions(Names::_onCompletion, eventArgs);
            bool preserveNonRemovable = (kind == acskStatement) || (target == _scriptContext->GetGlobalObject());
            eventArgs->TakeExternalChanges(preserveNonRemovable);
        }
    }

    void LangSvcExtensibility::FireOnParameterHelp(PageAllocator* pageAlloc, IAuthorFunctionHelp*& funcHelp, charcount_t offset, Js::JavascriptFunction* target, Js::RecyclableObject* parentObject)
    {
        if (HasExtensions(Names::_onParameterHelp))
        {
            ParameterHelpEventArgs eventArgs(_alloc, _scriptContext, _fileAuthoring, target, offset, parentObject, &funcHelp);
            InvokeExtensions(Names::_onParameterHelp, &eventArgs);
            eventArgs.TakeExternalChanges(pageAlloc);
        }
    }

    void LangSvcExtensibility::FireOnCompletionHint(PageAllocator* pageAlloc, Completions* completions, int itemIndex, IAuthorSymbolHelp*& symbolHelp)
    {
        if (HasExtensions(Names::_onCompletionHint))
        {
            CompletionHintEventArgs eventArgs(_alloc, _scriptContext, _fileAuthoring, completions, completions->Items()->Item(itemIndex), &symbolHelp);
            InvokeExtensions(Names::_onCompletionHint, &eventArgs);
            eventArgs.TakeExternalChanges(pageAlloc);
        }
    }

    bool LangSvcExtensibility::HasExtensions(LPCWSTR eventName) 
    { 
        if (!_intellisenseObj)
            return false;

        auto handlers = JsHelpers::GetProperty<Js::JavascriptArray*>(_intellisenseObj, eventName, _alloc, _scriptContext);
        return handlers && handlers->GetLength() > 0;
    }

    template<typename TArg>
    void LangSvcExtensibility::InvokeExtensions(LPCWSTR eventName, TArg arg)
    {
        if (!_intellisenseObj)
            return;

        ExtensionsInvoker<TArg> extensionsInvoker(_alloc, _scriptContext, _intellisenseObj, _fileAuthoring);
        extensionsInvoker.InvokeExtensions(eventName, arg);
        ReportMessages();
    }

    void LangSvcExtensibility::ReportMessages()
    {
        if (_intellisenseObj) 
        {
            auto messages = JsHelpers::GetProperty<Js::JavascriptArray*>(_intellisenseObj, Names::_msgs, _alloc, _scriptContext);
            if (messages)
            {
                JsHelpers::ForEach<LPCWSTR>(messages, _alloc, [&] (uint index, LPCWSTR msg) 
                {
                    ReportMessage(msg);
                });
                messages->SetLength((uint32)0);
            }
            else
            {
                Assert(false);
            }
        }
    }

    void LangSvcExtensibility::ReportMessage(LPCWSTR msg)
    {
        if (msg)
        {
            auto fileContext = _fileAuthoring->FileContext();
            Assert(fileContext);
            fileContext->LogMessage(CComBSTR(msg));
        }
    }
}