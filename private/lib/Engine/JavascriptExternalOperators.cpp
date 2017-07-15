//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    BOOL JavascriptExternalOperators::GetProperty(DynamicObject* scriptObject, PropertyId id, Var* varMember, ScriptContext * scriptContext)
    {
        BOOL rc = FALSE;
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            uint32 indexVal;
            if (scriptContext->IsNumericPropertyId(id, &indexVal))
            {
                rc = JavascriptOperators::GetItem(scriptObject, indexVal, varMember, scriptContext);
            }
            else
            {
                TypeId typeId = JavascriptOperators::GetTypeId(scriptObject);
                if (typeId != TypeIds_ModuleRoot && typeId != TypeIds_GlobalObject)
                {
                    // the id here is dispid coming from JavascriptDispatch call, for id in external object, the id could be totally external
                    //  and not in javascript land. directly go to the external object if the dispid is not valid propertyid.
                    if (!scriptContext->GetThreadContext()->IsActivePropertyId(id))
                    {
                        AssertMsg(FALSE, "invalid propertyid");
                        Throw::FatalInternalError();
                    }
                    else
                    {
                        rc = JavascriptOperators::GetProperty(scriptObject, id, varMember, scriptContext, NULL);
                    }
                }
                else if (JavascriptConversion::PropertyQueryFlagsToBoolean(scriptObject->DynamicObject::GetPropertyQuery(scriptObject, id, varMember, NULL, scriptContext))) // Given a root object, retrieve the property without deferring to the host object.
                {
                    rc = true;
                }
                else // Look in the GlobalObject's prototype if it doesn't have the property locally.
                {
                    rc = (typeId == TypeIds_GlobalObject && JavascriptOperators::GetProperty(scriptObject->GetPrototype(), id, varMember, scriptContext, NULL));
                }
            }

        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }
    BOOL JavascriptExternalOperators::GetPropertyReference(DynamicObject* scriptObject, PropertyId id, Var* varMember, ScriptContext * scriptContext)
    {
        BOOL rc = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            uint32 indexVal;
            if (scriptContext->IsNumericPropertyId(id, &indexVal))
            {
                rc = JavascriptOperators::GetItemReference(scriptObject, indexVal, varMember, scriptContext);
            }
            else
            {
                TypeId typeId = JavascriptOperators::GetTypeId(scriptObject);
                if (typeId != TypeIds_ModuleRoot && typeId != TypeIds_GlobalObject)
                {
                    // the id here is dispid coming from JavascriptDispatch call, for id in external object, the id could be totally external
                    //  and not in javascript land. directly go to the external object if the dispid is not valid propertyid.
                    if (!scriptContext->GetThreadContext()->IsActivePropertyId(id))
                    {
                        AssertMsg(FALSE, "invalid propertyid");
                        Throw::FatalInternalError();
                    }
                    else
                    {
                        rc = JavascriptOperators::GetPropertyReference(scriptObject, id, varMember, scriptContext, NULL);
                    }
                }

                else if (JavascriptConversion::PropertyQueryFlagsToBoolean(scriptObject->DynamicObject::GetPropertyReferenceQuery(scriptObject, id, varMember, NULL, scriptContext))) // Given a root object, retrieve the property without deferring to the host object.
                {
                    rc = true;
                }
                else // Look in the GlobalObject's prototype if it doesn't have the property locally.
                {
                    rc = (typeId == TypeIds_GlobalObject && JavascriptOperators::GetPropertyReference(scriptObject->GetPrototype(), id, varMember, scriptContext, NULL));
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    BOOL JavascriptExternalOperators::SetProperty(DynamicObject* scriptObject, PropertyId id, Var value, ScriptContext * scriptContext)
    {
        BOOL rc = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            uint32 indexVal;
            if (scriptContext->IsNumericPropertyId(id, &indexVal))
            {
                rc = JavascriptOperators::SetItem(scriptObject, scriptObject, indexVal, value, scriptContext);
            }
            else
            {
                // the id here is dispid coming from JavascriptDispatch call, for id in external object, the id could be totally external
                //  and not in javascript land. directly go to the external object if the dispid is not valid propertyid.
                if (!scriptContext->GetThreadContext()->IsActivePropertyId(id))
                {
                    AssertMsg(FALSE, "invalid propertyid");
                    Throw::FatalInternalError();
                }
                else
                {
                    rc = JavascriptOperators::SetProperty(scriptObject, scriptObject, id, value, scriptContext);
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    BOOL JavascriptExternalOperators::DeleteProperty(DynamicObject* scriptObject, PropertyId id, ScriptContext * scriptContext)
    {
        BOOL rc = FALSE;
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            uint32 indexVal;
            if (scriptContext->IsNumericPropertyId(id, &indexVal))
            {
                rc = JavascriptOperators::DeleteItem(scriptObject, indexVal);
            }
            else
            {
                // the id here is dispid coming from JavascriptDispatch call, for id in external object, the id could be totally external
                //  and not in javascript land. directly go to the external object if the dispid is not valid propertyid.
                if (!scriptContext->GetThreadContext()->IsActivePropertyId(id))
                {
                    AssertMsg(FALSE, "invalid propertyid");
                    Throw::FatalInternalError();
                }
                else
                {
                    rc = JavascriptOperators::DeleteProperty(scriptObject, id);
                }
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    Var JavascriptExternalConversion::ToPrimitive(DynamicObject* obj,  JavascriptHint hint, ScriptContext * scriptContext)
    {
        Var rc = NULL;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToPrimitive(obj, hint, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    JavascriptString * JavascriptExternalConversion::ToString(Var instance, ScriptContext * scriptContext)
    {
        JavascriptString * rc = NULL;

        if ( JavascriptString::Is(instance) )
        {
            rc = JavascriptString::FromVar(instance);
        }
        else
        {
            BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
            {
                rc = JavascriptConversion::ToString(instance, scriptContext);
            }
            END_JS_RUNTIME_CALL(scriptContext);
        }

        return rc;
    }

    double JavascriptExternalConversion::ToNumber(Var instance, ScriptContext * scriptContext)
    {
        double rc = 0.0;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToNumber(instance, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    int JavascriptExternalConversion::ToInt32(Var instance, ScriptContext * scriptContext)
    {
        int rc = 0;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToInt32(instance, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    __int64 JavascriptExternalConversion::ToInt64(Var instance, ScriptContext * scriptContext)
    {
        __int64 rc = 0;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToInt64(instance, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    unsigned __int64 JavascriptExternalConversion::ToUInt64(Var instance, ScriptContext * scriptContext)
    {
        unsigned __int64 rc = 0;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToUInt64(instance, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }

    BOOL JavascriptExternalConversion::ToBoolean(Var instance, ScriptContext * scriptContext)
    {
        BOOL rc = FALSE;

        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
        {
            rc = JavascriptConversion::ToBoolean(instance, scriptContext);
        }
        END_JS_RUNTIME_CALL(scriptContext);

        return rc;
    }
};
