//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Authoring
{
    //
    // Represents a doc comments information attached to a JS object.
    //
    class JsValueDoc
    {
    public:
        bool isDefinitionRef;
        bool annotation;
        int fileId;
        charcount_t pos;
        LPCWSTR type;
        LPCWSTR description;
        LPCWSTR locid;
        LPCWSTR elementType;
        LPCWSTR helpKeyword;
        LPCWSTR externalFile;
        LPCWSTR externalid;
        Deprecated* deprecated;
        JsUtil::List<CompatibleWith*, ArenaAllocator> compatibleWith;

    public:
        JsValueDoc(ArenaAllocator* alloc) : compatibleWith(alloc), isDefinitionRef(false), fileId(-1), pos(0), type(nullptr), description(nullptr), locid(nullptr), elementType(nullptr), 
            helpKeyword(nullptr), externalFile(nullptr), externalid(nullptr), original(nullptr), annotation(false), deprecated(nullptr) 
        { }

        // Creates a new object representing a doc reference
        static JsValueDoc* CreateRef(ArenaAllocator* alloc, int fileId, charcount_t pos);

        JsValueDoc *Original() { return original ? original : this; }

        bool IsEmpty();

        void MergeFrom(JsValueDoc* other);

        // Create runtime object with the values set.
        Js::RecyclableObject* ToRecyclableObject(Js::ScriptContext* scriptContext);

        // Applies the doc info to the specified field - sets parentObject._$fieldDoc$<fieldName> value
        void SetFieldDoc(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext);
        
        // Gets attached doc info if exists as parentObject._$fieldDoc$<fieldName> value. Returns nullptr if none is available.
        static JsValueDoc* GetFieldDoc(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext);
        
        // Gets the doc info from object _$doc property if available.
        static JsValueDoc* GetDoc(ArenaAllocator* alloc, Js::RecyclableObject* value, Js::ScriptContext* scriptContext);

        // Gets the doc object if it exists as parentObject._fieldDoc$<fieldName>. Returns nullptr if none is available
        static Js::RecyclableObject* GetFieldDocObj(ArenaAllocator* alloc, Js::RecyclableObject* parentObject, LPCWSTR fieldName, Js::ScriptContext* scriptContext);

        // Gets the doc object from an instance or returns nullptr if one is not present.
        static Js::RecyclableObject* GetDocObj(ArenaAllocator *alloc, Js::RecyclableObject* value, Js::ScriptContext* scriptContext);

        // Returns a JsValueDoc if value refers to a defintion reference or nullptr if it doesn't.
        static JsValueDoc* FromDocRef(ArenaAllocator* alloc, Js::Var value, Js::ScriptContext* scriptContext);
        
        // Return a name to use for the field doc property of the given memberName
        static LPCWSTR DocFieldName(ArenaAllocator* alloc, LPCWSTR memberName);

        template<typename TDoc>
        static JsValueDoc* FromDocComments(ArenaAllocator* alloc, TDoc* doc)
        {
            Assert(alloc);
            Assert(doc);
            JsValueDoc* result = Anew(alloc, JsValueDoc, alloc);
            result->isDefinitionRef = false;
            result->type = doc->type;
            result->description = doc->description;
            result->locid = doc->locid;
            result->elementType = doc->elementType;
            result->helpKeyword = doc->helpKeyword;
            result->externalFile = doc->externalFile;
            result->externalid = doc->externalid;
            result->deprecated = doc->deprecated;
            result->compatibleWith.Copy(&doc->compatibleWith);

            return result;
        }

    private:
        JsValueDoc *original;
        static JsValueDoc* FromDocObj(ArenaAllocator* alloc, Js::RecyclableObject* docObj, Js::ScriptContext* scriptContext);
        static JsValueDoc* FromDocObj(ArenaAllocator* alloc, Js::RecyclableObject* docObj, bool isDefinitionRef, Js::ScriptContext* scriptContext);
    };
}
