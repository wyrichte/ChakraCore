
function a(x) {
    var invar1 = "abacaba";
    var invar2 = 444444;

    try {
        throw x;
    } catch {
        var f = function () {
            return "eee" + invar1 + invar2;
        }
        if (x == "eee") {
            return f;
        }
    }
    function g() {
        return invar1 + invar2;
    }
    return g;
}

var x = a("eee");
var y = a("iii");

WScript.Echo(x());
WScript.Echo(y());
Debug.dumpHeap(x);
