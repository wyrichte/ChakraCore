//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "TelemetryPch.h"
#include "BuiltInMapper.h"

// Parser includes
// TODO: clean up the need of these regex related header here just for GroupInfo needed in JavascriptRegExpConstructor
#include "RegexCommon.h"
#include "Library/JavascriptRegExpConstructor.h"

namespace Js
{

    /**
     *  Given a Js::Var instance and a propertyId, find the corresponding BuiltInFacet, or return BuiltInFacet::_None;
     */
    BuiltInFacet BuiltInMapper::GetFacetForProperty(const Var& instance, const PropertyId& propertyId)
    {

        // 0. If the propertyId is beyond the list of known properties, then abort, we're not interested!
        // 1. Get the ESBuiltInTypeId of `instance`.
        // 1.1. If the Js::TypeId is immediately mappable, get that.
        // 1.2. Otherwise, look up the type name as a string.
        // 1.3. If type name cannot be processed (object has no type name, name not found in database, etc), then abort and return.
        // 2. Get the ESBuiltInPropertyId
        // 2.1. Combine ESBuiltInTypeId and PropertyId together to form the ESBuiltInPropertyIdKey
        // 2.2. Get the ESBuiltInPropertyId from ESBuiltInPropertyIdKey
        // 3. Increment the usage of that member.

        if (propertyId >= Js::PropertyIds::_countJSOnlyProperty)
        {
            return BuiltInFacet::_None;
        }

        ESBuiltInPropertyId esbiPropertyId = ESBuiltInPropertyId::_None;
        bool isConstructorProperty;
        ESBuiltInTypeNameId esbiTypeNameId = this->GetESBuiltInTypeNameId(instance, propertyId, /* out-param */ isConstructorProperty, /* out-param */ esbiPropertyId);

        if (esbiTypeNameId == ESBuiltInTypeNameId::_None)
        {
            return BuiltInFacet::_None;
        }

        if (esbiPropertyId == ESBuiltInPropertyId::_None) // `esbiPropertyId` can be found via shortcut.
        {
            esbiPropertyId = BuiltInMapper::GetESBuiltInPropertyId(esbiTypeNameId, isConstructorProperty, propertyId);
        }

        return BuiltInMapper::GetESBuiltInArrayIndex(esbiPropertyId);
    }

    /**
     *  Given a Js::Var instance return the corresponding BuiltInFacet if it is a constructor, or return BuiltInFacet::_None;
     */
    BuiltInFacet BuiltInMapper::GetFacetForConstructor(const Js::Var& constructorFunction)
    {
        BuiltInFacet f = BuiltInFacet::_None;

        if (JavascriptFunction::Is(constructorFunction))
        {
            JavascriptFunction* function = JavascriptFunction::FromVar(constructorFunction);

            ESBuiltInTypeNameId esbiTypeNameId = GetESBuiltInTypeNameId_ByPointer(function);

            if (esbiTypeNameId != ESBuiltInTypeNameId::_None)
            {
                ESBuiltInPropertyId ctorInvocation = GetESBuiltInPropertyId(esbiTypeNameId, /*isConstructorProperty:*/ true, PropertyIds::__constructor);
                f = BuiltInMapper::GetESBuiltInArrayIndex(ctorInvocation);
            }
        }
        return f;
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId_ByTypeId(const Js::TypeId typeId)
    {
        switch (typeId)
        {
#define TYPENAME(ctor,name)
#define TYPENAME_MAP(typeId, name)                         \
        case Js:: ## typeId ##:                            \
            return ESBuiltInTypeNameId:: ## name ## ;
#include "ESBuiltInsTypeNames.h"
#undef TYPENAME_MAP
#undef TYPENAME
        default:
            return ESBuiltInTypeNameId::_None;
        }
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId_ByPointer(const JavascriptFunction* constructorFunction)
    {
        JavascriptLibrary* lib = this->scriptContext->GetLibrary();
#define TYPENAME(ctor,name) TYPENAME_##ctor(name)
#define TYPENAME_0(name) 
#define TYPENAME_1(name)   if( constructorFunction == lib->Get## name ##Constructor() ) return ESBuiltInTypeNameId::##name;
#define TYPENAME_MAP(typeId,name) 
#include "ESBuiltInsTypeNames.h"
#undef TYPENAME_MAP
#undef TYPENAME_1
#undef TYPENAME_0
#undef TYPENAME

        return ESBuiltInTypeNameId::_None;
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInPropertyId BuiltInMapper::GetESBuiltInPropertyId(ESBuiltInTypeNameId esbiTypeNameId, bool isConstructorProperty, Js::PropertyId propertyId)
    {
        AssertMsg(static_cast<uint32>(propertyId) < 0x8000, "propertyId is under 32768");

        uint32 ctor = isConstructorProperty ? 0x8000 : 0;

        uint32 key = static_cast<uint32>(esbiTypeNameId) << 16 | ctor | static_cast<uint32>(propertyId);

        ESBuiltInPropertyId ret = static_cast<ESBuiltInPropertyId>(key);

        // Ensure `ret` is a valid typeName+location+propertyId combination by looking it up in the database.

        BuiltInFacet usageArrayIndex = GetESBuiltInArrayIndex(ret);
        return usageArrayIndex == BuiltInFacet::_None ? ESBuiltInPropertyId::_None : ret;
    }

    /**
    *
    */
    BuiltInFacet BuiltInMapper::GetESBuiltInArrayIndex(BuiltInMapper::ESBuiltInPropertyId propertyId)
    {
        switch (propertyId) {
#define ENTRY_BUILTIN(esVersion, typeName, location, propertyName)      case ESBuiltInPropertyId:: ## typeName ## _ ## location ## _ ## propertyName : \
        return BuiltInFacet:: ## typeName ## _ ## location ## _ ## propertyName;
#include "ESBuiltins.h"
#undef ENTRY_BUILTIN
        }
        return BuiltInFacet::_None;
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound)
    {
        isConstructorProperty = false;

        if (instance == nullptr) return BuiltInMapper::ESBuiltInTypeNameId::_None;

        Js::TypeId typeId = JavascriptOperators::GetTypeId(instance);

        // Special-cases are needed for Functions and Objects because we don't know what their real type is.
        if (typeId == TypeId::TypeIds_Function)
        {
            return this->GetESBuiltInTypeNameId_Function(instance, propertyId, isConstructorProperty, shortcutPropertyFound);
        }
        else if (typeId == TypeId::TypeIds_Object)
        {
            return this->GetESBuiltInTypeNameId_Object(instance, propertyId, isConstructorProperty, shortcutPropertyFound);
        }
        else
        {
            return this->GetESBuiltInTypeNameId_Other(instance, propertyId, isConstructorProperty, typeId, shortcutPropertyFound);
        }
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId_Function(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound)
    {
        //count_GetBuiltInTypeNameFunction++;

        // BUG: the property-accession `String.prototype` causes this method to return `(isConstructor: false, typeId: ESBuiltInTypeNameId::Function)` because `prototype` is a member of Function.
        // However it should return `(isConstructor: true, typeId: ESBuiltInTypeNameId::String)`

        //#ifdef NEVER
        // `instance` could be accessing a property of Function (e.g. Function.bind), or it could be accessing a Constructor Property (e.g. Array.isArray).
        // So see if the PropertyId belongs to Function, and if so, return immediately before doing a name-check.

        ESBuiltInPropertyId definedOnFunction;
        definedOnFunction = BuiltInMapper::GetESBuiltInPropertyId(ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ false, propertyId);
        if (definedOnFunction != ESBuiltInPropertyId::_None)
        {
            isConstructorProperty = false;
            shortcutPropertyFound = definedOnFunction;
            return ESBuiltInTypeNameId::Function;
        }

        definedOnFunction = BuiltInMapper::GetESBuiltInPropertyId(ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ true, propertyId);
        if (definedOnFunction != ESBuiltInPropertyId::_None) {
            // There is a bug here in that `Function_Constructor_length` or `Function_Constructor_prototype` will never be recorded as a Constructor property, as the names are also used by instances, and this code has no way of knowing if this is *a* function, or *the* Function function.
            isConstructorProperty = true;
            shortcutPropertyFound = definedOnFunction;
            return ESBuiltInTypeNameId::Function;
        }
        //#endif

        // PropertyId does not belong to Function, therefore this is a Constructor property. But which Constructor is it?
        // There are two ways to solve this:
        // 1. Look up the TypeNameId by string (i.e. the name of the function), "Array.isArray" -> "Array" -> the global Array constructor function.
        // 2. Compare the pointer to the function to known pointers, `if( function == library.ArrayConstructor )`

        // Method 1 has the advantage of working to detect polyfills, but is slow as string comparisons, even when using a Trie, require lots of steps, repeatedly.
        // Method 2 only detects usage of implemented and non-overrriden built-ins, but is a lot faster.

        JavascriptFunction* instanceAsFunction = JavascriptFunction::FromVar(instance);

        isConstructorProperty = true;

        ESBuiltInTypeNameId ret = BuiltInMapper::GetESBuiltInTypeNameId_ByPointer(instanceAsFunction);
        return ret;
    }

    /**
    *
    */
    bool BuiltInMapper::IsBuiltInMath(const Var& instance) const
    {
        DynamicObject* math = this->scriptContext->GetLibrary()->GetMathObject();


        return instance == math;
    }

    /**
    *
    */
    bool BuiltInMapper::IsBuiltInJson(const Var& instance) const
    {
        DynamicObject* json = this->scriptContext->GetLibrary()->GetJSONObject();

        return instance == json;
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId_Object(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ BuiltInMapper::ESBuiltInPropertyId& shortcutPropertyFound)
    {
        if (this->IsBuiltInMath(instance))
        {
            return ESBuiltInTypeNameId::Math;
        }

        if (this->IsBuiltInJson(instance))
        {
            return ESBuiltInTypeNameId::JSON;
        }

        // `instance` could be a Polyfill Object. Inspect the name of its Constructor to see if it matches (e.g. `function DataView() { this.someDataViewProperty = 'abc'; };` ).
        // However, an optimization: don't name-check if the propertyId already belongs to Object (e.g. hasOwnProperty).

        ESBuiltInPropertyId definedOnObject = BuiltInMapper::GetESBuiltInPropertyId(ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
        if (definedOnObject != ESBuiltInPropertyId::_None)
        {
            isConstructorProperty = false;
            shortcutPropertyFound = definedOnObject;
            return ESBuiltInTypeNameId::Object;
        }

        // Skip checking for polyfills, abort further processing.
        return ESBuiltInTypeNameId::_None;
    }

    /**
    *
    */
    BuiltInMapper::ESBuiltInTypeNameId BuiltInMapper::GetESBuiltInTypeNameId_Other(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound)
    {
        ESBuiltInTypeNameId esbiTypeNameId = BuiltInMapper::GetESBuiltInTypeNameId_ByTypeId(typeId);
        if (esbiTypeNameId == ESBuiltInTypeNameId::_None) return esbiTypeNameId;

        // See if the specified property is defined on the identified type, if not, see if it's defined for Object (and so, is inherited):
        ESBuiltInPropertyId definedOnType = BuiltInMapper::GetESBuiltInPropertyId(esbiTypeNameId, /*isConstructorProperty:*/ false, propertyId);
        if (definedOnType == ESBuiltInPropertyId::_None)
        {
            ESBuiltInPropertyId definedOnObject = BuiltInMapper::GetESBuiltInPropertyId(ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
            if (definedOnObject != ESBuiltInPropertyId::_None)
            {
                shortcutPropertyFound = definedOnObject;
                return ESBuiltInTypeNameId::Object;
            }
        }
        else
        {
            shortcutPropertyFound = definedOnType;
        }

        return esbiTypeNameId;
    }

}
