//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//
// Helper class to map between external ErrorType enum in ScriptDirect.idl and 
// internal ErrorTypeEnum enum in screrror.h and other functions
//----------------------------------------------------------------------------

class ErrorTypeHelper
{
  private:
    struct MapArrayElem
    {
        ErrorTypeEnum internalType;
        JsErrorType externalType;
        LPCWSTR messageInEnglish;
    };
    static const MapArrayElem mapArray[];
#ifdef _DEBUG
    static void ValidateMapArray();
#endif
  
  public:
    static HRESULT MapToInternal(JsErrorType externalErrorType, ErrorTypeEnum &internalErrorType);
    static JsErrorType MapToExternal(ErrorTypeEnum internalErrorType);
    static LPCWSTR MapExternalToErrorText(JsErrorType externalErrorType);
};


