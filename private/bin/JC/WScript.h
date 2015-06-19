/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class WScript  /* TODO: Determine actual object */
{
public:
    class EntryInfo
    {
    public:
        static Js::FunctionInfo Echo;
        static Js::FunctionInfo Quit;
        static Js::FunctionInfo StdErrWriteLine;
        static Js::FunctionInfo Copy;
    };
    static Js::Var Echo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var Quit(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var StdErrWriteLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static  Js::Var Copy(Js::RecyclableObject*, Js::CallInfo callInfo, ...);
    static void Initialize(Js::ScriptContext* scriptContext);
    static void Write(Js::Var aValue, Js::ScriptContext* scriptContext, FILE* file);
};