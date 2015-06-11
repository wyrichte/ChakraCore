
//
//    Copyright (C) Microsoft.  All rights reserved.
//
BUILT_IN_OPERATION(BuiltInOperation_ObjectToString, GetObjectPrototype(), Js::JavascriptObject::EntryInfo::ToString)
BUILT_IN_OPERATION(BuiltInOperation_ObjectSeal, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::Seal)
BUILT_IN_OPERATION(BuiltInOperation_ObjectFreeze, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::Freeze)
BUILT_IN_OPERATION(BuiltInOperation_ObjectPreventExtensions, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::PreventExtensions)
BUILT_IN_OPERATION(BuiltInOperation_ObjectIsSealed, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::IsSealed)
BUILT_IN_OPERATION(BuiltInOperation_ObjectIsFrozen, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::IsFrozen)
BUILT_IN_OPERATION(BuiltInOperation_ObjectIsExtensible, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::IsExtensible)
BUILT_IN_OPERATION(BuiltInOperation_ObjectDefineProperty, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::DefineProperty)
BUILT_IN_OPERATION(BuiltInOperation_ObjectDefineProperties, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::DefineProperties)
BUILT_IN_OPERATION(BuiltInOperation_ObjectGetPrototypeOf, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::GetPrototypeOf)
BUILT_IN_OPERATION(BuiltInOperation_ObjectGetOwnPropertyDescriptor, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::GetOwnPropertyDescriptor)
BUILT_IN_OPERATION(BuiltInOperation_ObjectGetOwnPropertyNames, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::GetOwnPropertyNames)
BUILT_IN_OPERATION(BuiltInOperation_ObjectGetOwnPropertySymbols, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::GetOwnPropertySymbols)
BUILT_IN_OPERATION(BuiltInOperation_ObjectKeys, GetObjectConstructor(), Js::JavascriptObject::EntryInfo::Keys)
BUILT_IN_OPERATION(BuiltInOperation_StringToString, GetStringPrototype(), Js::JavascriptString::EntryInfo::ToString)
BUILT_IN_OPERATION(BuiltInOperation_StringValueOf, GetStringPrototype(), Js::JavascriptString::EntryInfo::ValueOf)
BUILT_IN_OPERATION(BuiltInOperation_NumberToString, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ToString)
BUILT_IN_OPERATION(BuiltInOperation_NumberToLocaleString, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ToLocaleString)
BUILT_IN_OPERATION(BuiltInOperation_NumberValueOf, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ValueOf)
BUILT_IN_OPERATION(BuiltInOperation_NumberToFixed, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ToFixed)
BUILT_IN_OPERATION(BuiltInOperation_NumberToExponential, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ToExponential)
BUILT_IN_OPERATION(BuiltInOperation_NumberToPrecision, GetNumberPrototype(), Js::JavascriptNumber::EntryInfo::ToPrecision)
BUILT_IN_OPERATION(BuiltInOperation_BooleanToString, GetBooleanPrototype(), Js::JavascriptBoolean::EntryInfo::ToString)
BUILT_IN_OPERATION(BuiltInOperation_BooleanValueOf, GetBooleanPrototype(), Js::JavascriptBoolean::EntryInfo::ValueOf)
BUILT_IN_OPERATION(BuiltInOperation_DateGetDate, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetDate)
BUILT_IN_OPERATION(BuiltInOperation_DateGetDay, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetDay)
BUILT_IN_OPERATION(BuiltInOperation_DateGetFullYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetFullYear)
BUILT_IN_OPERATION(BuiltInOperation_DateGetHours, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetHours)
BUILT_IN_OPERATION(BuiltInOperation_DateGetMilliseconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetMilliseconds)
BUILT_IN_OPERATION(BuiltInOperation_DateGetMinutes, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetMinutes)
BUILT_IN_OPERATION(BuiltInOperation_DateGetMonth, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetMonth)
BUILT_IN_OPERATION(BuiltInOperation_DateGetSeconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetSeconds)
BUILT_IN_OPERATION(BuiltInOperation_DateGetTime, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetTime)
BUILT_IN_OPERATION(BuiltInOperation_DateGetTimezoneOffset, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetTimezoneOffset)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCDate, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCDate)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCDay, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCDay)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCFullYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCFullYear)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCHours, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCHours)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCMilliseconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCMilliseconds)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCMinutes, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCMinutes)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCMonth, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCMonth)
BUILT_IN_OPERATION(BuiltInOperation_DateGetUTCSeconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetUTCSeconds)
BUILT_IN_OPERATION(BuiltInOperation_DateGetVarDate, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetVarDate)
BUILT_IN_OPERATION(BuiltInOperation_DateGetYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::GetYear)
BUILT_IN_OPERATION(BuiltInOperation_DateSetDate, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetDate)
BUILT_IN_OPERATION(BuiltInOperation_DateSetFullYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetFullYear)
BUILT_IN_OPERATION(BuiltInOperation_DateSetHours, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetHours)
BUILT_IN_OPERATION(BuiltInOperation_DateSetMilliseconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetMilliseconds)
BUILT_IN_OPERATION(BuiltInOperation_DateSetMinutes, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetMinutes)
BUILT_IN_OPERATION(BuiltInOperation_DateSetMonth, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetMonth)
BUILT_IN_OPERATION(BuiltInOperation_DateSetSeconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetSeconds)
BUILT_IN_OPERATION(BuiltInOperation_DateSetTime, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetTime)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCDate, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCDate)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCFullYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCFullYear)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCHours, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCHours)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCMilliseconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCMilliseconds)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCMinutes, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCMinutes)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCMonth, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCMonth)
BUILT_IN_OPERATION(BuiltInOperation_DateSetUTCSeconds, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetUTCSeconds)
BUILT_IN_OPERATION(BuiltInOperation_DateSetYear, GetDatePrototype(), Js::JavascriptDate::EntryInfo::SetYear)
BUILT_IN_OPERATION(BuiltInOperation_DateToDateString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToDateString)
BUILT_IN_OPERATION(BuiltInOperation_DateToISOString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToISOString)
BUILT_IN_OPERATION(BuiltInOperation_DateToJSON, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToJSON)
BUILT_IN_OPERATION(BuiltInOperation_DateToLocaleDateString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToLocaleDateString)
BUILT_IN_OPERATION(BuiltInOperation_DateToLocaleString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToLocaleString)
BUILT_IN_OPERATION(BuiltInOperation_DateToLocaleTimeString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToLocaleTimeString)
BUILT_IN_OPERATION(BuiltInOperation_DateToString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToString)
BUILT_IN_OPERATION(BuiltInOperation_DateToTimeString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToTimeString)
BUILT_IN_OPERATION(BuiltInOperation_DateToUTCString, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ToUTCString)
BUILT_IN_OPERATION(BuiltInOperation_DateValueOf, GetDatePrototype(), Js::JavascriptDate::EntryInfo::ValueOf)
BUILT_IN_OPERATION(BuiltInOperation_JSONStringify, GetJSONObject(), JSON::EntryInfo::Stringify)