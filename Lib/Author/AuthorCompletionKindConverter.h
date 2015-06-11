//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    struct AuthorCompletionKindConverter
    {
        static bool FromString(LPCWSTR kindString, __out AuthorCompletionKind& kind)
        {
            bool result = false;
            kind = AuthorCompletionKind();
            if (kindString)
            {
                switch (kindString[0])
                {
                case 'f':
                    if(wcscmp(kindString, L"field") == 0)
                    {
                        kind = ackField;
                        result = true;
                    }
                    break;
                case 'i':
                    if(wcscmp(kindString, L"identifier") == 0)
                    {
                        kind = ackIdentifier;
                        result = true;
                    }
                    break;
                case 'l':
                    if(wcscmp(kindString, L"label") == 0)
                    {
                        kind = ackLabel;
                        result = true;
                    }
                    break;
                case 'm':
                    if(wcscmp(kindString, L"method") == 0)
                    {
                        kind = ackMethod;
                        result = true;
                    }
                    break;
                case 'p':
                    if(wcscmp(kindString, L"parameter") == 0)
                    {
                        kind = ackParameter;
                        result = true;
                    }
                    else if(wcscmp(kindString, L"property") == 0)
                    {
                        kind = ackProperty;
                        result = true;
                    }
                    break;
                case 'r':
                    if(wcscmp(kindString, L"reserved") == 0)
                    {
                        kind = ackReservedWord;
                        result = true;
                    }
                    break;
                case 'v':
                    if(wcscmp(kindString, L"variable") == 0)
                    {
                        kind = ackVariable;
                        result = true;
                    }
                    break;
                }
            }
            return result;
        }

        static LPCWSTR ToString(AuthorCompletionKind kind)
        {
            switch(kind)
            {
            case AuthorCompletionKind::ackMethod:
                return L"method";
            case AuthorCompletionKind::ackField:
                return L"field";
            case AuthorCompletionKind::ackLabel:
                return L"label";
            case AuthorCompletionKind::ackProperty:
                return L"property";
            case AuthorCompletionKind::ackIdentifier:
                return L"identifier";
            case AuthorCompletionKind::ackParameter:
                return L"parameter";
            case AuthorCompletionKind::ackVariable:
                return L"variable";
            case AuthorCompletionKind::ackReservedWord:
                return L"reserved";
            }
            Assert(true);
            return nullptr;
        }
    };
}