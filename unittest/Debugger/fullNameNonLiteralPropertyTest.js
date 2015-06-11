// Tests that non-literal properties display their
// full name correctly.
// Work Item: 769430

function test() {
    var arr = [1, 2, 3, 4];
    arr['abc@#&*(!)#$@'] = "non literal property";
    arr['@#&*(!)#$@'] = { a: { b: 1, c: 2 } };
    arr['@#\'&*\'(!)#$@'] = 5;
    arr['x\\y'] = 6;
    //arr['x\0y'] = 7; // Not working, currently not supporting embeded null. DebugProperty::GetPropertyInfo etc. uses LPCWSTR to handle string.
    
    /**bp:locals(1, LOCALS_FULLNAME);evaluate('arr', 3, LOCALS_FULLNAME)**/

}

test();

WScript.Echo("PASSED");