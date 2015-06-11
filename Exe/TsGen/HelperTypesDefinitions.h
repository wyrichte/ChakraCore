//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//
// Runtime details of promises, imap and imapview implementations
//----------------------------------------------------------------------------

#pragma once

#define TSGEN_PROMISE_DEFINITION_BODY \
    L"declare module Windows.Foundation {\n" \
    L"    interface IPromise<T> {\n" \
    L"        then<U>(success ? : (value : T) = > IPromise<U>, error ? : (error : any) = > IPromise<U>, progress ? : (progress : any) = > void): Windows.Foundation.IPromise<U>;\n" \
    L"        then<U>(success ? : (value : T) = > IPromise<U>, error ? : (error : any) = > U, progress ? : (progress : any) = > void): Windows.Foundation.IPromise<U>;\n" \
    L"        then<U>(success ? : (value : T) = > U, error ? : (error : any) = > IPromise<U>, progress ? : (progress : any) = > void): Windows.Foundation.IPromise<U>;\n" \
    L"        then<U>(success ? : (value : T) = > U, error ? : (error : any) = > U, progress ? : (progress : any) = > void): Windows.Foundation.IPromise<U>;\n" \
    L"        done ? <U>(success ? : (value : T) = > any, error ? : (error : any) = > any, progress ? : (progress : any) = > void): void;\n" \
    L"    }\n" \
    L"}\n"

#define TSGEN_IMAP_IMAPVIEW_DEFINITION_BODY \
    L"declare module Windows {\n"\
    L"    module Storage {\n"\
    L"        module Foundation {\n"\
    L"            module Collections {\n"\
    L"                interface IMapView<K, V> extends Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<K, V>> {\n"\
    L"                    size: number;\n"\
    L"                    first(): Windows.Foundation.Collections.IIterator<Windows.Foundation.Collections.IKeyValuePair<string, any>>;\n"\
    L"                    hasKey(key: string): boolean;\n"\
    L"                    lookup(key: string): any;\n"\
    L"                    split(): { first: Windows.Foundation.Collections.IMapView<string, any>; second: Windows.Foundation.Collections.IMapView<string, any>; };\n"\
    L"                }\n"\
    L"                interface IMap<K, V> extends Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<K, V>> {\n"\
    L"                    size: number;\n"\
    L"                    clear(): void;\n"\
    L"                    first(): Windows.Foundation.Collections.IIterator<Windows.Foundation.Collections.IKeyValuePair<string, any>>;\n"\
    L"                    getView(): Windows.Foundation.Collections.IMapView<string, any>;\n"\
    L"                    hasKey(key: string): boolean;\n"\
    L"                    insert(key: string, value : any): boolean;\n"\
    L"                    lookup(key: string): any;\n"\
    L"                    remove(key: string): void;\n"\
    L"                }\n"\
    L"            }\n"\
    L"        }\n"\
    L"    }\n"\
    L"}\n"

