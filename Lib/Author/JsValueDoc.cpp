//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t annotation[] = L"annotation";
    }

    JsValueDoc* JsValueDoc::CreateRef(ArenaAllocator* alloc, int fileId, charcount_t pos)
    {
        auto doc = Anew(alloc, JsValueDoc, alloc);
        doc->isDefinitionRef = true;
        doc->fileId = fileId;
        doc->pos = pos;
        return doc;
    }

    void JsValueDoc::SetFieldDoc(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext)
    {
        Assert(alloc);
        Assert(parentObject);
        Assert(scriptContext);
        Assert(!String::IsNullOrEmpty(fieldName));
        auto jsdoc = JsHelpers::CreateObject(scriptContext);
        JsHelpers::SetField(jsdoc, Names::type, type, scriptContext);
        JsHelpers::SetField(jsdoc, Names::description, description, scriptContext);
        JsHelpers::SetField(jsdoc, Names::locid, locid, scriptContext);
        JsHelpers::SetField(jsdoc, Names::elementType, elementType, scriptContext);
        JsHelpers::SetField(jsdoc, Names::helpKeyword, helpKeyword, scriptContext);
        JsHelpers::SetField(jsdoc, Names::externalFile, externalFile, scriptContext);
        JsHelpers::SetField(jsdoc, Names::externalid, externalid, scriptContext);
        if (deprecated)
        {
            auto jsDeprecated = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsDeprecated, Names::type, deprecated->type, scriptContext);
            JsHelpers::SetField(jsDeprecated, Names::message, deprecated->message, scriptContext);
            JsHelpers::SetField(jsdoc, Names::deprecated, jsDeprecated, scriptContext);
        }

        JsHelpers::SetField(jsdoc, Names::compatibleWith,
            JsHelpers::CreateArray(compatibleWith.Count(), [&] (uint index) -> Js::RecyclableObject*
        {
            auto jsCompat = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsCompat, Names::platform, compatibleWith.Item(index)->platform, scriptContext);
            JsHelpers::SetField(jsCompat, Names::minVersion, compatibleWith.Item(index)->minVersion, scriptContext);
            return jsCompat;
        }, scriptContext), scriptContext);

        JsHelpers::SetField(parentObject, DocFieldName(alloc, fieldName), jsdoc, scriptContext, false);
    }

    JsValueDoc* JsValueDoc::GetDoc(ArenaAllocator* alloc, Js::RecyclableObject* value, Js::ScriptContext* scriptContext)
    {
        auto docObj = GetDocObj(alloc, value, scriptContext);
        if(docObj)
        {
            return FromDocObj(alloc, docObj, scriptContext);
        }
        return nullptr;
    };

    JsValueDoc* JsValueDoc::GetFieldDoc(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext)
    {
        auto docObj = GetFieldDocObj(alloc, parentObject, fieldName, scriptContext);
        if(docObj)
        {
            return FromDocObj(alloc, docObj, scriptContext);
        }
        return nullptr;
    };

    LPCWSTR JsValueDoc::DocFieldName(ArenaAllocator* alloc, LPCWSTR memberName)
    {
        Assert(alloc);
        Assert(!String::IsNullOrEmpty(memberName));
        TextBuffer fieldDocMember(alloc);
        fieldDocMember.Add(Names::fieldDocPrefix);
        fieldDocMember.Add(memberName);
        return fieldDocMember.Sz(true);
    }

    JsValueDoc* JsValueDoc::FromDocObj(ArenaAllocator* alloc, Js::RecyclableObject* docObj, Js::ScriptContext* scriptContext)
    {
        Assert(alloc);
        Assert(docObj);
        Assert(scriptContext);

        auto isDefinitionRef = JsHelpers::GetProperty<bool>(docObj, Names::isDefinitionRef, alloc, scriptContext);
        return FromDocObj(alloc, docObj, isDefinitionRef, scriptContext);
    }

    JsValueDoc* JsValueDoc::FromDocObj(ArenaAllocator* alloc, Js::RecyclableObject* docObj, bool isDefinitionRef, Js::ScriptContext* scriptContext)
    {
        Assert(alloc);
        Assert(docObj);
        Assert(scriptContext);
        JsValueDoc* result = Anew(alloc, JsValueDoc, alloc);
        
        // fileId and pos may be available in both cases. They are always set when isDefinitionRef=true and are also available for fields defined via <field> inside a function.
        // (see FieldDocAST in DocCommentsRewrite.cpp)
        result->fileId = JsHelpers::GetProperty<uint>(docObj, Names::fileId, alloc, scriptContext);
        result->pos = JsHelpers::GetProperty<uint>(docObj, Names::pos, alloc, scriptContext);
        // annotation is available when intellisense.annotate is being used
        result->annotation = JsHelpers::GetProperty<bool>(docObj, Names::annotation, alloc, scriptContext);
        if(isDefinitionRef)
        {
            result->isDefinitionRef = true;
            auto originalObj = JsHelpers::GetProperty<Js::RecyclableObject*>(docObj, Names::original, alloc, scriptContext);
            if (originalObj && !Js::JavascriptOperators::IsUndefinedObject(originalObj))
                result->original = JsValueDoc::FromDocObj(alloc, originalObj, scriptContext);
        }
        else
        {
            result->isDefinitionRef = false;
            result->type = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::type, alloc, scriptContext);
            result->description = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::description, alloc, scriptContext);
            result->locid = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::locid, alloc, scriptContext);
            result->elementType = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::elementType, alloc, scriptContext);
            result->helpKeyword = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::helpKeyword, alloc, scriptContext);
            result->externalFile = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::externalFile, alloc, scriptContext);
            result->externalid = JsHelpers::GetProperty<LPCWSTR>(docObj, Names::externalid, alloc, scriptContext);

            auto jsDeprecated = JsHelpers::GetProperty<Js::RecyclableObject*>(docObj, Names::deprecated, alloc, scriptContext);
            if (jsDeprecated && !Js::JavascriptOperators::IsUndefinedOrNullType(jsDeprecated->GetTypeId()))
            {
                result->deprecated = Anew(alloc, Deprecated);
                result->deprecated->type = JsHelpers::GetProperty<LPCWSTR>(jsDeprecated, Names::type, alloc, scriptContext);
                result->deprecated->message = JsHelpers::GetProperty<LPCWSTR>(jsDeprecated, Names::message, alloc, scriptContext);
            }

            auto jsCompatibleWith = JsHelpers::GetProperty<Js::JavascriptArray*>(docObj, Names::compatibleWith, alloc, scriptContext);
            if (jsCompatibleWith)
            {
                JsHelpers::ForEach<Js::RecyclableObject*>(jsCompatibleWith, alloc, [&] (uint index, Js::RecyclableObject* jsCompat) 
                {
                    auto compat = Anew(alloc, CompatibleWith);
                    compat->platform = JsHelpers::GetProperty<LPCWSTR>(jsCompat, Names::platform, alloc, scriptContext);
                    compat->minVersion = JsHelpers::GetProperty<LPCWSTR>(jsCompat, Names::minVersion, alloc, scriptContext);
                    result->compatibleWith.Add(compat);
                });
            }

        }
        return result;
    };

    Js::RecyclableObject* JsValueDoc::GetDocObj(ArenaAllocator *alloc, Js::RecyclableObject* instance, Js::ScriptContext* scriptContext)
    {
        Assert(alloc);
        Assert(instance);
        Assert(scriptContext);

        auto docObj = JsHelpers::GetProperty<Js::RecyclableObject*>(instance, Names::_doc, alloc, scriptContext);
        if (docObj && !Js::JavascriptOperators::IsUndefinedObject(docObj))
        {
            return docObj;
        }

        return nullptr;
    }

    Js::RecyclableObject* JsValueDoc::GetFieldDocObj(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext)
    {
        Assert(alloc);
        Assert(parentObject);
        Assert(scriptContext);
        Assert(!String::IsNullOrEmpty(fieldName));
        auto docObj = JsHelpers::GetProperty<Js::RecyclableObject*>(parentObject, DocFieldName(alloc, fieldName), alloc, scriptContext);
        if(docObj && !Js::JavascriptOperators::IsUndefinedObject(docObj))
        {
            return docObj;
        }
        return nullptr;
    };

    JsValueDoc* JsValueDoc::FromDocRef(ArenaAllocator* alloc, Js::Var value, Js::ScriptContext* scriptContext)
    {
        auto object = Js::RecyclableObject::FromVar(value);
        if (object)
        {
            if (JsHelpers::GetProperty<bool>(object, Names::isDefinitionRef, alloc, scriptContext))
                return JsValueDoc::FromDocObj(alloc, object, true, scriptContext);
        }

        return nullptr;
    }

    Js::RecyclableObject* JsValueDoc::ToRecyclableObject(Js::ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        auto result = scriptContext->GetLibrary()->CreateObject();
        if (isDefinitionRef)
        {
            JsHelpers::SetField<bool>(result, Names::isDefinitionRef, true, scriptContext);
            JsHelpers::SetField<uint>(result, Names::fileId, fileId, scriptContext);
            JsHelpers::SetField<uint>(result, Names::pos, pos, scriptContext);
        }
        else
        {
            if (type)
                JsHelpers::SetField(result, Names::type, type, scriptContext);
            if (description)
                JsHelpers::SetField(result, Names::description, description, scriptContext);
            if (locid)
                JsHelpers::SetField(result, Names::locid, locid, scriptContext);
            if (elementType)
                JsHelpers::SetField(result, Names::elementType, locid, scriptContext);
            if (helpKeyword)
                JsHelpers::SetField(result, Names::helpKeyword, locid, scriptContext);
            if (externalFile)
                JsHelpers::SetField(result, Names::externalFile, locid, scriptContext);
            if (externalid)
                JsHelpers::SetField(result, Names::externalid, locid, scriptContext);
            if (deprecated)
            {
                auto jsDeprecated = JsHelpers::CreateObject(scriptContext);
                if (deprecated->type)
                    JsHelpers::SetField(jsDeprecated, Names::type, deprecated->type, scriptContext);
                if (deprecated->message)
                    JsHelpers::SetField(jsDeprecated, Names::message, deprecated->message, scriptContext);
                JsHelpers::SetField(result, Names::deprecated, jsDeprecated, scriptContext);
            }
            if (compatibleWith.Count() > 0)
            {
                auto jsCompatibleWith = JsHelpers::CreateArray(compatibleWith.Count(), [&] (uint index) -> Js::RecyclableObject*
                {
                    auto jsCompat = JsHelpers::CreateObject(scriptContext);
                    if (compatibleWith.Item(index)->platform)
                        JsHelpers::SetField(jsCompat, Names::platform, compatibleWith.Item(index)->platform, scriptContext);
                    if (compatibleWith.Item(index)->minVersion)
                        JsHelpers::SetField(jsCompat, Names::minVersion, compatibleWith.Item(index)->minVersion, scriptContext);
                    return jsCompat;
                }, scriptContext);
                JsHelpers::SetField(result, Names::compatibleWith, jsCompatibleWith, scriptContext);
            }
        }

        return result;
    }

    bool JsValueDoc::IsEmpty()
    {
        return !type && !description && !locid && !elementType && !helpKeyword && !externalFile && !externalid && (compatibleWith.Count() == 0);
    }

    void JsValueDoc::MergeFrom(JsValueDoc* other)
    {
        // Take type && elementType if none are set
        if(!type && !elementType)
        {
            type = other->type;
            elementType = other->elementType;
        }

        // Take description related attributes if none are set
        if(!description && !locid && !externalFile && !externalid)
        {
            description = other->description;
            locid = other->locid;
            externalFile = other->externalFile;
            externalid = other->externalid;
        }

        helpKeyword = helpKeyword ? helpKeyword : other->helpKeyword;
    }
}