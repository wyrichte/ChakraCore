var objArgs = WScript.Arguments;

if (objArgs.length != 1 && objArgs.length != 2) {
    WScript.Echo("Supply a modified version of the output of JsGen you want as the new prolog and the name of the resulting file (probably Prolog.h)\r\nFor example:\r\n\r\n  CreateProlog winrt.js Prolog.h\r\n\r\nIn no file is supplied the result is echoed.");
}
else {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var file = fso.OpenTextFile(objArgs(0), 1);
    var text = file.ReadAll();
    file.Close();

    var index = text.indexOf('// Begin Pass 1');
    if (index > 0)
        text = text.substring(0, index);
    var lines = text.split('\n');

    var r = "//---------------------------------------------------------------------------\r\n// Copyright (C) 1995 - 2011 by Microsoft Corporation.  All rights reserved.\r\n//\r\n// Runtime details of promises implementation\r\n//----------------------------------------------------------------------------\r\n\r\n#pragma once\r\n\r\n#define JSGEN_PROLOG_BODY \\\r\n";

    for (var i = 0, len = lines.length; i < len; i++) {
        var line = lines[i].replace('\n', '').replace('\r','');
        r += '    "' + line + '\\n" \\\r\n';
    }
    r += "\r\n";

    if (objArgs.length == 2) {
        var outfile = fso.CreateTextFile(objArgs(1), true);
        outfile.Write(r);
        outfile.Close();
    }
    else
        WScript.Echo(r);

}

