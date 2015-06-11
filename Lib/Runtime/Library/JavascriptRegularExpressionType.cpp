//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    /*static*/
    PropertyId JavascriptRegExp::specialPropertyIds[] = 
    {
        PropertyIds::lastIndex,
        PropertyIds::global,
        PropertyIds::multiline,
        PropertyIds::ignoreCase,
        PropertyIds::source,
        PropertyIds::options,
        PropertyIds::unicode,
        PropertyIds::sticky
    };

    BOOL JavascriptRegExp::HasProperty(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            return true;        
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                return true;
            }
            return DynamicObject::HasProperty(propertyId);
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                return true;
            }
            return DynamicObject::HasProperty(propertyId);
        default:
            return DynamicObject::HasProperty(propertyId);
        }
    }

    BOOL JavascriptRegExp::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        return JavascriptRegExp::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptRegExp::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {        
        BOOL result;
        if (GetPropertyBuiltIns(propertyId, value, &result))
        {
            return result;
        }

        return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptRegExp::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, &result))
        {
            return result;
        }

        return DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    bool JavascriptRegExp::GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
            if (this->lastIndexVar == null)
            {
                Assert(lastIndexOrFlag <= MaxCharCount);
                this->lastIndexVar = JavascriptNumber::ToVar(lastIndexOrFlag, GetScriptContext());
            }
            *value = this->lastIndexVar;
            *result = true;
            return true;
        case PropertyIds::global:
            *value = this->GetLibrary()->CreateBoolean(this->GetPattern()->IsGlobal());
            *result = true;
            return true;
        case PropertyIds::multiline:
            *value = this->GetLibrary()->CreateBoolean(this->GetPattern()->IsMultiline());
            *result = true;
            return true;
        case PropertyIds::ignoreCase:
            *value = this->GetLibrary()->CreateBoolean(this->GetPattern()->IsIgnoreCase());
            *result = true;
            return true;
        case PropertyIds::unicode:
            if (GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                *value = this->GetLibrary()->CreateBoolean(this->GetPattern()->IsUnicode());
                *result = true;
                return true;
            }
            else
            {
                return false;
            }
        case PropertyIds::sticky:
            if (GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                *value = this->GetLibrary()->CreateBoolean(this->GetPattern()->IsSticky());
                *result = true;
                return true;
            }
            else
            {
                return false;
            }
        case PropertyIds::source:
            {
                *value = this->ToString(true);                
                *result = true;
                return true;
            }
        case PropertyIds::options:
        {
            ScriptContext* scriptContext = this->GetLibrary()->GetScriptContext();
            BEGIN_TEMP_ALLOCATOR(tempAlloc, scriptContext, L"JavascriptRegExp")
            {
                StringBuilder<ArenaAllocator> bs(tempAlloc, 4);
                
                if(GetPattern()->IsGlobal())
                {
                    bs.Append(L'g');
                }
                if(GetPattern()->IsIgnoreCase())
                {
                    bs.Append(L'i');
                }
                if(GetPattern()->IsMultiline())
                {
                    bs.Append(L'm');
                }
                if (GetPattern()->IsUnicode())
                {
                    bs.Append(L'u');
                }
                if (GetPattern()->IsSticky())
                {
                    bs.Append(L'y');
                }
                *value = Js::JavascriptString::NewCopyBuffer(bs.Detach(), bs.Count(), scriptContext);
            }
            END_TEMP_ALLOCATOR(tempAlloc, scriptContext);           
            *result = true;
            return true;
        }
        default:
            return false;
        }
    }

    BOOL JavascriptRegExp::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL result;
        if (SetPropertyBuiltIns(propertyId, value, flags, &result))
        {
            return result;
        }

        return DynamicObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptRegExp::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {        
        BOOL result;
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && SetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, flags, &result))
        {
            return result;
        }

        return DynamicObject::SetProperty(propertyNameString, value, flags, info);
    }

    bool JavascriptRegExp::SetPropertyBuiltIns(PropertyId propertyId, Var value, PropertyOperationFlags flags, BOOL* result)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
            this->lastIndexVar = value;
            lastIndexOrFlag = NotCachedValue;
            *result = true;
            return true;
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            JavascriptError::ThrowCantAssignIfStrictMode(flags, this->GetScriptContext());
            *result = false;
            return true;
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                JavascriptError::ThrowCantAssignIfStrictMode(flags, this->GetScriptContext());
                *result = false;
                return true;
            }
            return false;
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                JavascriptError::ThrowCantAssignIfStrictMode(flags, this->GetScriptContext());
                *result = false;
                return true;
            }
            return false;
        default:
            return false;
        }
    }

    BOOL JavascriptRegExp::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptRegExp::DeleteProperty(PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            JavascriptError::ThrowCantDeleteIfStrictMode(propertyOperationFlags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
            return false;
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                JavascriptError::ThrowCantDeleteIfStrictMode(propertyOperationFlags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
                return false;
            }
            return DynamicObject::DeleteProperty(propertyId, propertyOperationFlags);
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                JavascriptError::ThrowCantDeleteIfStrictMode(propertyOperationFlags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
                return false;
            }
            return DynamicObject::DeleteProperty(propertyId, propertyOperationFlags);
        default:
            return DynamicObject::DeleteProperty(propertyId, propertyOperationFlags);
        }
    }

    DescriptorFlags JavascriptRegExp::GetSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags result;
        if (GetSetterBuiltIns(propertyId, info, &result))
        {
            return result;
        }

        return DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
    }

    DescriptorFlags JavascriptRegExp::GetSetter(JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags result;
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);
        
        if (propertyRecord != null && GetSetterBuiltIns(propertyRecord->GetPropertyId(), info, &result))
        {
            return result;
        }

        return DynamicObject::GetSetter(propertyNameString, setterValue, info, requestContext);
    }

    bool JavascriptRegExp::GetSetterBuiltIns(PropertyId propertyId, PropertyValueInfo* info, DescriptorFlags* result)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            PropertyValueInfo::SetNoCache(info, this);
            *result = JavascriptRegExp::IsWritable(propertyId) ? WritableData : Data;
            return true;       
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                PropertyValueInfo::SetNoCache(info, this);
                *result = JavascriptRegExp::IsWritable(propertyId) ? WritableData : Data;
                return true;
            }
            return false;
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                PropertyValueInfo::SetNoCache(info, this);
                *result = JavascriptRegExp::IsWritable(propertyId) ? WritableData : Data;
                return true;
            }
            return false;
        default:
            return false;
        }
    }

    BOOL JavascriptRegExp::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        Js::InternalString str = pattern->GetSource();
        stringBuilder->Append(str.GetBuffer(), str.GetLength());
        return TRUE;
    }

    BOOL JavascriptRegExp::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(JS_DIAG_TYPE_JavascriptRegExp);
        return TRUE;
    }
    
    BOOL JavascriptRegExp::IsEnumerable(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            return false;
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                return false;
            }
            return DynamicObject::IsEnumerable(propertyId);
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                return false;
            }
            return DynamicObject::IsEnumerable(propertyId);
        default:
            return DynamicObject::IsEnumerable(propertyId);
        }
    }

    BOOL JavascriptRegExp::IsConfigurable(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            return false;
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                return false;
            }
            return DynamicObject::IsConfigurable(propertyId);
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                return false;
            }
            return DynamicObject::IsConfigurable(propertyId);
        default:
            return DynamicObject::IsConfigurable(propertyId);
        }
    }

    BOOL JavascriptRegExp::IsWritable(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::lastIndex:
            return true;
        case PropertyIds::global:
        case PropertyIds::multiline:
        case PropertyIds::ignoreCase:
        case PropertyIds::source:
        case PropertyIds::options:
            return false;
        case PropertyIds::unicode:
            if (this->GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
            {
                return false;
            }
            return DynamicObject::IsWritable(propertyId);
        case PropertyIds::sticky:
            if (this->GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                return false;
            }
            return DynamicObject::IsWritable(propertyId);
        default:
            return DynamicObject::IsWritable(propertyId);
        }
    }
    BOOL JavascriptRegExp::GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        uint length = GetSpecialPropertyCount();
      
        if (index < length)
        {
            *propertyName = requestContext->GetPropertyString(specialPropertyIds[index]);
            return true;
        }
        return false;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptRegExp::GetSpecialPropertyCount() const
    {
        // Since -es6regex is off if -es6unicodeExtension is off, 
        // we will never have -es6Regex = true, -es6UnicodeExtension = false
        if (GetScriptContext()->GetConfig()->IsES6UnicodeExtensionsEnabled())
        {
            if (GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled())
            {
                return defaultSpecialPropertyIdsCount + 2;
            }
            else
            {
                return defaultSpecialPropertyIdsCount + 1;
            }
        }
        else
        {
            AssertMsg(!GetScriptContext()->GetConfig()->IsES6RegExChangesEnabled(), "-Es6RegexFlags should be disable if -ES6UnicodeExtension is disabled.");
            return defaultSpecialPropertyIdsCount;
        }
    }

    // Returns the list of special non-enumerable properties for the type.
    PropertyId* JavascriptRegExp::GetSpecialPropertyIds() const
    {
        return specialPropertyIds;
    }

    JavascriptRegExp* JavascriptRegExp::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<JavascriptRegExp> *result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptRegExp>, scriptContext->GetLibrary()->GetRegexType(), this, scriptContext);

        // Copy internal properties immediately.
        result->pattern = scriptContext->CopyPattern(pattern);
        result->lastIndexOrFlag = lastIndexOrFlag;
        if (lastIndexVar)
            result->lastIndexVar = scriptContext->CopyOnWrite(lastIndexVar);

        return result;
    }

} // namespace Js
