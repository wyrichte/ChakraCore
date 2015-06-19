// Copyright Microsoft 2015. All rights reserved.
// GENERATED FILE, DO NOT HAND MODIFIY
// Generated with the following command line: cscript ESBuiltInsTypeNames.trie.Generator.js ESBuiltInsTypeNames.inc ESBuiltInsTypeNames.trie.inc value valueLength
// This should be regenerated whenever the type-name list changes.

    const wchar_t* p = value;
    switch( p[0] ) {
        case 'A':
            if( valueLength >= 5 && p[1] == 'r' && p[2] == 'r' && p[3] == 'a' ) {
                switch( p[4] ) {
                    case 'y':
                        if (valueLength == 5) {
                            return ESBuiltInTypeNameId::Array;
                        }
                        switch( p[5] ) {
                            case 'B':
                                if (valueLength == 11 && p[6] == 'u' && p[7] == 'f' && p[8] == 'f' && p[9] == 'e' && p[10] == 'r'){
                                    return ESBuiltInTypeNameId::ArrayBuffer;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'B':
            if (valueLength == 7 && p[1] == 'o' && p[2] == 'o' && p[3] == 'l' && p[4] == 'e' && p[5] == 'a' && p[6] == 'n'){
                return ESBuiltInTypeNameId::Boolean;
            }
            return ESBuiltInTypeNameId::_None;
        case 'D':
            if( valueLength >= 3 && p[1] == 'a' ) {
                switch( p[2] ) {
                    case 't':
                        switch( p[3] ) {
                            case 'a':
                                if (valueLength == 8 && p[4] == 'V' && p[5] == 'i' && p[6] == 'e' && p[7] == 'w'){
                                    return ESBuiltInTypeNameId::DataView;
                                }
                                break;
                            case 'e':
                                if (valueLength == 4){
                                    return ESBuiltInTypeNameId::Date;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'E':
            if (valueLength == 5 && p[1] == 'r' && p[2] == 'r' && p[3] == 'o' && p[4] == 'r'){
                return ESBuiltInTypeNameId::Error;
            }
            return ESBuiltInTypeNameId::_None;
        case 'F':
            switch( p[1] ) {
                case 'l':
                    if( valueLength >= 5 && p[2] == 'o' && p[3] == 'a' ) {
                        switch( p[4] ) {
                            case 't':
                                switch( p[5] ) {
                                    case '3':
                                        if (valueLength == 12 && p[6] == '2' && p[7] == 'A' && p[8] == 'r' && p[9] == 'r' && p[10] == 'a' && p[11] == 'y'){
                                            return ESBuiltInTypeNameId::Float32Array;
                                        }
                                        break;
                                    case '6':
                                        if (valueLength == 12 && p[6] == '4' && p[7] == 'A' && p[8] == 'r' && p[9] == 'r' && p[10] == 'a' && p[11] == 'y'){
                                            return ESBuiltInTypeNameId::Float64Array;
                                        }
                                        break;
                                }
                                break;
                        }
                    }
                    break;
                case 'u':
                    if (valueLength == 8 && p[2] == 'n' && p[3] == 'c' && p[4] == 't' && p[5] == 'i' && p[6] == 'o' && p[7] == 'n'){
                        return ESBuiltInTypeNameId::Function;
                    }
                    break;
            }
            return ESBuiltInTypeNameId::_None;
        case 'G':
            switch( p[1] ) {
                case 'e':
                    if( valueLength >= 9 && p[2] == 'n' && p[3] == 'e' && p[4] == 'r' && p[5] == 'a' && p[6] == 't' && p[7] == 'o' ) {
                        switch( p[8] ) {
                            case 'r':
                                if (valueLength == 9) {
                                    return ESBuiltInTypeNameId::Generator;
                                }
                                switch( p[9] ) {
                                    case 'F':
                                        if (valueLength == 17 && p[10] == 'u' && p[11] == 'n' && p[12] == 'c' && p[13] == 't' && p[14] == 'i' && p[15] == 'o' && p[16] == 'n'){
                                            return ESBuiltInTypeNameId::GeneratorFunction;
                                        }
                                        break;
                                }
                                break;
                        }
                    }
                    break;
                case 'l':
                    if (valueLength == 6 && p[2] == 'o' && p[3] == 'b' && p[4] == 'a' && p[5] == 'l'){
                        return ESBuiltInTypeNameId::Global;
                    }
                    break;
            }
            return ESBuiltInTypeNameId::_None;
        case 'I':
            if( valueLength >= 3 && p[1] == 'n' ) {
                switch( p[2] ) {
                    case 't':
                        switch( p[3] ) {
                            case '1':
                                if (valueLength == 10 && p[4] == '6' && p[5] == 'A' && p[6] == 'r' && p[7] == 'r' && p[8] == 'a' && p[9] == 'y'){
                                    return ESBuiltInTypeNameId::Int16Array;
                                }
                                break;
                            case '3':
                                if (valueLength == 10 && p[4] == '2' && p[5] == 'A' && p[6] == 'r' && p[7] == 'r' && p[8] == 'a' && p[9] == 'y'){
                                    return ESBuiltInTypeNameId::Int32Array;
                                }
                                break;
                            case '8':
                                if (valueLength == 9 && p[4] == 'A' && p[5] == 'r' && p[6] == 'r' && p[7] == 'a' && p[8] == 'y'){
                                    return ESBuiltInTypeNameId::Int8Array;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'J':
            if (valueLength == 4 && p[1] == 'S' && p[2] == 'O' && p[3] == 'N'){
                return ESBuiltInTypeNameId::JSON;
            }
            return ESBuiltInTypeNameId::_None;
        case 'M':
            if( valueLength >= 2 ) {
                switch( p[1] ) {
                    case 'a':
                        switch( p[2] ) {
                            case 'p':
                                if (valueLength == 3){
                                    return ESBuiltInTypeNameId::Map;
                                }
                                break;
                            case 't':
                                if (valueLength == 4 && p[3] == 'h'){
                                    return ESBuiltInTypeNameId::Math;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'N':
            if (valueLength == 6 && p[1] == 'u' && p[2] == 'm' && p[3] == 'b' && p[4] == 'e' && p[5] == 'r'){
                return ESBuiltInTypeNameId::Number;
            }
            return ESBuiltInTypeNameId::_None;
        case 'O':
            if (valueLength == 6 && p[1] == 'b' && p[2] == 'j' && p[3] == 'e' && p[4] == 'c' && p[5] == 't'){
                return ESBuiltInTypeNameId::Object;
            }
            return ESBuiltInTypeNameId::_None;
        case 'P':
            if( valueLength >= 3 && p[1] == 'r' ) {
                switch( p[2] ) {
                    case 'o':
                        switch( p[3] ) {
                            case 'm':
                                if (valueLength == 7 && p[4] == 'i' && p[5] == 's' && p[6] == 'e'){
                                    return ESBuiltInTypeNameId::Promise;
                                }
                                break;
                            case 'x':
                                if (valueLength == 5 && p[4] == 'y'){
                                    return ESBuiltInTypeNameId::Proxy;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'R':
            if( valueLength >= 2 ) {
                switch( p[1] ) {
                    case 'e':
                        switch( p[2] ) {
                            case 'f':
                                if (valueLength == 7 && p[3] == 'l' && p[4] == 'e' && p[5] == 'c' && p[6] == 't'){
                                    return ESBuiltInTypeNameId::Reflect;
                                }
                                break;
                            case 'g':
                                if (valueLength == 6 && p[3] == 'E' && p[4] == 'x' && p[5] == 'p'){
                                    return ESBuiltInTypeNameId::RegExp;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'S':
            switch( p[1] ) {
                case 'e':
                    if (valueLength == 3 && p[2] == 't'){
                        return ESBuiltInTypeNameId::Set;
                    }
                    break;
                case 't':
                    if( valueLength >= 6 && p[2] == 'r' && p[3] == 'i' && p[4] == 'n' ) {
                        switch( p[5] ) {
                            case 'g':
                                if (valueLength == 6) {
                                    return ESBuiltInTypeNameId::String;
                                }
                                switch( p[6] ) {
                                    case 'I':
                                        if (valueLength == 14 && p[7] == 't' && p[8] == 'e' && p[9] == 'r' && p[10] == 'a' && p[11] == 't' && p[12] == 'o' && p[13] == 'r'){
                                            return ESBuiltInTypeNameId::StringIterator;
                                        }
                                        break;
                                }
                                break;
                        }
                    }
                    break;
                case 'y':
                    if (valueLength == 6 && p[2] == 'm' && p[3] == 'b' && p[4] == 'o' && p[5] == 'l'){
                        return ESBuiltInTypeNameId::Symbol;
                    }
                    break;
            }
            return ESBuiltInTypeNameId::_None;
        case 'U':
            if( valueLength >= 4 && p[1] == 'i' && p[2] == 'n' ) {
                switch( p[3] ) {
                    case 't':
                        switch( p[4] ) {
                            case '1':
                                if (valueLength == 11 && p[5] == '6' && p[6] == 'A' && p[7] == 'r' && p[8] == 'r' && p[9] == 'a' && p[10] == 'y'){
                                    return ESBuiltInTypeNameId::Uint16Array;
                                }
                                break;
                            case '3':
                                if (valueLength == 11 && p[5] == '2' && p[6] == 'A' && p[7] == 'r' && p[8] == 'r' && p[9] == 'a' && p[10] == 'y'){
                                    return ESBuiltInTypeNameId::Uint32Array;
                                }
                                break;
                            case '8':
                                switch( p[5] ) {
                                    case 'A':
                                        if (valueLength == 10 && p[6] == 'r' && p[7] == 'r' && p[8] == 'a' && p[9] == 'y'){
                                            return ESBuiltInTypeNameId::Uint8Array;
                                        }
                                        break;
                                    case 'C':
                                        if (valueLength == 17 && p[6] == 'l' && p[7] == 'a' && p[8] == 'm' && p[9] == 'p' && p[10] == 'e' && p[11] == 'd' && p[12] == 'A' && p[13] == 'r' && p[14] == 'r' && p[15] == 'a' && p[16] == 'y'){
                                            return ESBuiltInTypeNameId::Uint8ClampedArray;
                                        }
                                        break;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        case 'W':
            if( valueLength >= 4 && p[1] == 'e' && p[2] == 'a' ) {
                switch( p[3] ) {
                    case 'k':
                        switch( p[4] ) {
                            case 'M':
                                if (valueLength == 7 && p[5] == 'a' && p[6] == 'p'){
                                    return ESBuiltInTypeNameId::WeakMap;
                                }
                                break;
                            case 'S':
                                if (valueLength == 7 && p[5] == 'e' && p[6] == 't'){
                                    return ESBuiltInTypeNameId::WeakSet;
                                }
                                break;
                        }
                        break;
                }
            }
            return ESBuiltInTypeNameId::_None;
        }
    return ESBuiltInTypeNameId::_None;
