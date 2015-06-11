//
// test exception (catch scopes)
//

function f() {
    var x = 3;
    try {
        throw new Error("1 Error");
    }
    catch(e1)
    {
        var y = 2;
        try {
            throw "2 string";
        }
        catch(e2) {
            var z = 1;           
            z; /**bp:locals(1, LOCALS_TYPE)**/
        }
    }
}
f.apply({});

WScript.Echo("pass");