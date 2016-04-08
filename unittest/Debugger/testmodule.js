function test1() {
    WScript.LoadScriptFile('module_1.js', 'module'); 
    WScript.LoadModule('import { modVar3 } from "module_3.js"; print(modVar3);', 'samethread');
}

test1();
