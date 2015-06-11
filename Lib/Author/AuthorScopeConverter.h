//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    struct AuthorScopeConverter
    {
        static LPCWSTR ToString(AuthorScope authorScope)
        {
            switch(authorScope)
            {
            case ascopeGlobal:
                return L"global";
            case ascopeClosure:
                return L"closure";
            case ascopeLocal:
                return L"local";
            case ascopeParameter:
                return L"parameter";
            case ascopeMember:
                return L"member";
            default:
                return L"unknown";
            }
        }

        static AuthorScope FromString(LPCWSTR s)
        {
            if(!String::IsNullOrEmpty(s))
            {
                switch (s[0])
                {
                case 'c':
                    if(!wcscmp(s, L"closure"))
                        return ascopeClosure;
                    break;
                case 'g':
                    if(!wcscmp(s, L"global"))
                        return ascopeGlobal;
                    break;
                case 'l':
                    if(!wcscmp(s, L"local"))
                        return ascopeLocal;
                    break;
                case 'p':
                    if(!wcscmp(s, L"parameter"))
                        return ascopeParameter;
                    break;
                case 'm':
                    if(!wcscmp(s, L"member"))
                        return ascopeMember;
                    break;
                }
            }
            return ascopeUnknown;
        }
    };
}