//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline BOOL JavascriptObject::DefineOwnPropertyHelper(RecyclableObject* obj, PropertyId propId, const PropertyDescriptor& descriptor, ScriptContext* scriptContext)
    {
        BOOL returnValue;
        obj->ThrowIfCannotDefineProperty(propId, descriptor);

        Type* oldType = obj->GetType();
        obj->ClearWritableDataOnlyDetectionBit();

        // HostDispatch: it doesn't support changing property attributes and default attributes are not per ES5,
        // so there is no benefit in using ES5 DefineOwnPropertyDescriptor for it, use old implementation.
        if (TypeIds_HostDispatch != obj->GetTypeId())
        {
            if (DynamicObject::IsAnyArray(obj))
            {
                returnValue = JavascriptOperators::DefineOwnPropertyForArray(
                    JavascriptArray::FromAnyArray(obj), propId, descriptor, true, scriptContext);
            }
            else
            {
                returnValue = JavascriptOperators::DefineOwnPropertyDescriptor(obj, propId, descriptor, true, scriptContext);
                if (propId == PropertyIds::__proto__)
                {
                    scriptContext->GetLibrary()->GetObjectPrototype()->PostDefineOwnProperty__proto__(obj);
                }
            }
        }
        else
        {
            returnValue = JavascriptOperators::SetPropertyDescriptor(obj, propId, descriptor);
        }


        if (obj->IsWritableDataOnlyDetectionBitSet())
        {
            if (obj->GetType() == oldType)
            {
                // Also, if the object's type has not changed, we need to ensure that 
                // the cached property string for this property, if any, does not
                // specify this object's type.
                scriptContext->InvalidatePropertyStringCache(propId, obj->GetType());
            }
        }

        if (descriptor.IsAccessorDescriptor())
        {
            scriptContext->optimizationOverrides.SetSideEffects(Js::SideEffects_Accessor);
        }
        return returnValue;
    }
}
