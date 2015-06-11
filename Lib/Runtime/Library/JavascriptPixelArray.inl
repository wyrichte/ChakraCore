//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

namespace Js
{        
    __inline Var JavascriptPixelArray::DirectGetItem(uint32 index)
    {   
        if (index < bufferlength)
        {
            return Js::TaggedInt::ToVarUnchecked(buffer[index]);
        }
        else
        {
            // Return undefined for interoperability
            return GetType()->GetLibrary()->GetUndefined();
        }
    }    
        
    __inline void JavascriptPixelArray::DirectSetItem(uint32 index, Var value)
    {
        // Silently Ignore any index that is out of range
        // Explicitly expand out the 2 most common types to avoid unncessary checks in the conversion APIs
    
        if (index < bufferlength)
        {                                       
            if (Js::TaggedInt::Is(value))
            {
                // The most common case
                SetIntegerValue(index, Js::TaggedInt::ToInt32(value));
            }
            else
            {              
                TypeId t = JavascriptOperators::GetTypeId(value);
                
                if (t == TypeIds_Number)
                {
#if DBG_DUMP
                    if (Configuration::Global.flags.Trace.IsEnabled(Js::TypedArrayPhase))
                    {
                        Output::Print(L"TRACE: PixelArray set with float value\n");
                    }
#endif
                    // Some pages will work exclusively in floating point pixel data so this path must also be fast                                     
                    SetDoubleValue(index, JavascriptNumber::GetValue(value));
                }
                else
                {
#if DBG_DUMP
                    if (Configuration::Global.flags.Trace.IsEnabled(Js::TypedArrayPhase))
                    {
                        Output::Print(L"TRACE: PixelArray set with ** non-number ** value!!!\n");
                    }
#endif
                    // Everything else can be slow.  Anything that cannot be converted to a number will get mapped to 0                    
                    SetDoubleValue(index, JavascriptConversion::ToNumber_Full(value, GetScriptContext()));                    
                }
            }
        }
    }      

    inline bool JavascriptPixelArray::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_PixelArray;
    }

    inline JavascriptPixelArray* JavascriptPixelArray::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptPixelArray'");
        
        return static_cast<JavascriptPixelArray *>(RecyclableObject::FromVar(aValue));
    }    

    __inline void JavascriptPixelArray::SetIntegerValue(uint32 index, int32 value)
    {
        // Clamp values to a BYTE range.  This is the interoperable behavior, and also the behavior
        // that the Canvas spec desires (despite some W3C discussions that may have implied otherwise).
        
        if (value > 255)
        {
#if DBG_DUMP
            if (Configuration::Global.flags.Trace.IsEnabled(Js::TypedArrayPhase))
            {
                Output::Print(L"TRACE: PixelArray set with int value larger than 255\n");
            }
#endif
            value = 255;
        }
        else if (value < 0)
        {
#if DBG_DUMP
            if (Configuration::Global.flags.Trace.IsEnabled(Js::TypedArrayPhase))
            {
                Output::Print(L"TRACE: PixelArray set with int value smaller than 255\n");
            }
#endif
            value = 0;
        }
        
        buffer[index] = (BYTE) value;
    }     

    __inline void JavascriptPixelArray::SetDoubleValue(uint32 index, double d)
    {        
        if (d > (double) INT_MAX)
        {
            // Positive infinity falls into this case as well (as desired)
            SetIntegerValue(index, 255);
        }
        else
        {
            // NaN, negative infinity, anything less than INT_MIN will map to 0 in this case
            SetIntegerValue(index, (int32) (d + 0.5));
        }
    }         
}

