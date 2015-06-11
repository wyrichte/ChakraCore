/**exception(resume_ignore):dumpSourceList();**/

var diag = WScript.LoadScriptFile("debugEvalParamsTest_child.js", "samethread", "diagnostics");

// Should only see debugEvalParamsTest.js and debugEvalParamsTest_child.js in source tree
var x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x1 = 1;", /* isNonUserCode */ true)); // No shouldRegisterDocument argument
// Shouldn't see var x1 = 1; in source tree as this is non user code
x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x2 = 2;", /* isNonUserCode */ false)); // No shouldRegisterDocument argument
// Should see var x2 = 2; in source tree as this is user code
x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x3 = 3;", /* isNonUserCode */ true, /* shouldRegisterDocument */ true));
// Shouldn't see var x3 = 3; in source tree as this is non user code. shouldRegisterDocument should be no-op
x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x4 = 4;", /* isNonUserCode */ true, /* shouldRegisterDocument */ false));
// Shouldn't see var x4 = 4; in source tree as this is non user code. shouldRegisterDocument should be no-op
x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x5 = 5;", /* isNonUserCode */ false, /* shouldRegisterDocument */ true));
// Should see var x5 = 5; in source tree as this is user code and shouldRegisterDocument is true
x = 1; /**bp:dumpSourceList();**/

WScript.Echo(diag.diagnosticsScript.debugEval("var x6 = 6;", /* isNonUserCode */ false, /* shouldRegisterDocument */ false));
// Shouldn't see var x6 = 6; in source tree though this is user code but shouldRegisterDocument is false
x = 1; /**bp:dumpSourceList();**/

var retObj1 = diag.diagnosticsScript.debugEval("var obj1 = {};obj1.foo=function(){return 1;};obj1;",  /* isNonUserCode */ true);
// toString of non user code should be [native code]
WScript.Echo(retObj1.foo);

var retObj2 = diag.diagnosticsScript.debugEval("var obj2 = {};obj2.foo=function(){return 1;};obj2;",  /* isNonUserCode */ false);
// toString of user code should be visible correctly
WScript.Echo(retObj2.foo);

var retObj3 = diag.diagnosticsScript.debugEval("var obj3 = {};obj3.foo=function(){return this.b.c;};obj3;",  /* isNonUserCode */ false, /* shouldRegisterDocument */ false);
// On exception we should register un-registered user code source
WScript.Echo(retObj3.foo());

var retObj4 = diag.diagnosticsScript.debugEval("var obj4 = {};obj4.foo=function(){return this.b.c;};obj4;",  /* isNonUserCode */ true);
// On exception we should not register un-registered non user code source
WScript.Echo(retObj4.foo());