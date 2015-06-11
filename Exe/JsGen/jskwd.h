//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

class JsKwd
{
    // Make sure that when the engine keyword list changes we get a build error
    // So we don't forget to update the switch statement in Is function.
    enum Keywords 
    {
        #define KEYWORD(tk,f,prec2,nop2,prec1,nop1,name) tk,
        #include "keywords.h"
        lastKeyword
    };
    static_assert(lastKeyword == 77, 
        "It looks like kwd-lsc.h has changed and new entries may need to be added to the switch below. Adjust the assert value once done."); 

public:

    static bool Is(LPCWSTR value)
    {
        if(value && *value)
        {
            switch(value[0])
            {
            case 'b':
                if( wcscmp(value, L"break") == 0 ) 
                    return true;
                break;
            case 'c':
                if( wcscmp(value, L"case") == 0 || 
                    wcscmp(value, L"catch") == 0 ||
                    wcscmp(value, L"continue") == 0 ||
                    wcscmp(value, L"class") == 0)
                    return true;
                break;
            case 'd':
                if( wcscmp(value, L"debugger") == 0 || 
                    wcscmp(value, L"default") == 0 ||
                    wcscmp(value, L"delete") == 0 ||
                    wcscmp(value, L"do") == 0)
                    return true;
                break;
            case 'e':
                if( wcscmp(value, L"else") == 0 || 
                    wcscmp(value, L"enum") == 0 ||
                    wcscmp(value, L"export") == 0)
                    return true;
                break;
            case 'f':
                if( wcscmp(value, L"finally") == 0 || 
                    wcscmp(value, L"for") == 0 ||
                    wcscmp(value, L"function") == 0)
                    return true;
                break;
            case 'i':
                if( wcscmp(value, L"if") == 0 || 
                    wcscmp(value, L"in") == 0 ||
                    wcscmp(value, L"instanceof") == 0 ||
                    wcscmp(value, L"import") == 0 ||
                    wcscmp(value, L"implements") == 0 ||
                    wcscmp(value, L"interface") == 0)
                    return true;
                break;
            case 'l':
                if( wcscmp(value, L"let") == 0)
                    return true;
                break;
            case 'n':
                if( wcscmp(value, L"new") == 0)
                    return true;
                break;
            case 'p':
                if( wcscmp(value, L"package") == 0 || 
                    wcscmp(value, L"private") == 0 ||
                    wcscmp(value, L"protected") == 0 ||
                    wcscmp(value, L"public") == 0)
                    return true;
                break;
            case 'r':
                if( wcscmp(value, L"return") == 0)
                    return true;
                break;
            case 's':
                if( wcscmp(value, L"switch") == 0 || 
                    wcscmp(value, L"super") == 0 ||
                    wcscmp(value, L"static") == 0)
                    return true;
                break;
            case 't':
                if( wcscmp(value, L"this") == 0 || 
                    wcscmp(value, L"throw") == 0 ||
                    wcscmp(value, L"try") == 0 ||
                    wcscmp(value, L"typeof") == 0)
                    return true;
                break;
            case 'v':
                if( wcscmp(value, L"var") == 0 || 
                    wcscmp(value, L"void") == 0)
                    return true;
                break;
            case 'w':
                if( wcscmp(value, L"while") == 0 || 
                    wcscmp(value, L"with") == 0)
                    return true;
                break;
            case 'y':
                if( wcscmp(value, L"yield") == 0)
                    return true;
                break;
            }
        }
        return false;
    }
};