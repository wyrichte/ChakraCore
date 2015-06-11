var e = WScript.LoadScriptFile("needMarshal_c.js", "samethread");
var child_get_object = e.get_object;

child_get_object; /**bp:
                    evaluateAsync("child_get_object()", 1);
                    evaluateAsync("child_get_object()", 1);
                    **/

// This is to regress chakra debug entries (e.g. DebugEval) missing hostScriptContextStack setup.
//
//  "child_get_object" is a cross-site function from child engine "e".
//  (0) async eval executes in this engine.
//  (1) async eval calls "child_get_object" ---------->  switches context to engine "e".
//  (2) In engine "e", Debug.writeln() yields to pdm, which executes next async eval.
//  (3) Next async eval calls "child_get_object" again. But since we didn't update
//      hostScriptContextStack, the crosssite thunk will think we are in engine "e" context,
//      set by step (1). So no marshalling is done on the result.
//  (4) The async eval (in this engine) asserts. An un-marshalled result object belongs to a different engine "e".

WScript.Echo("pass");
