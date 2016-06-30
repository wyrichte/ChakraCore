function test1() {
    WScript.LoadScriptFile('module_1.js', 'module'); 
}

test1();

function test2() {
    WScript.LoadModule('import { modVar3 } from "module_3.js"; print(modVar3);', 'samethread');
}
WScript.SetTimeout('WScript.Attach(test2);', 100);
