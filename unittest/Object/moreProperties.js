function write(v) { WScript.Echo(v + ""); }

function normal(o, nStart, nEnd)
{
    try {
        for (var i = nStart; i < nEnd; i++) {
            o["x" + i] = i;
        }
    } catch (e) {
        write(e);
    }

    return 0;
}

function withGetter(o, nStart, nEnd) {
    if (!o.hasOwnProperty("y")) {
        Object.defineProperty(o, "y", { get: function () { write("getter withGetter"); return -1; } });
    }

    try {
        for (var i = nStart; i < nEnd; i++) {
            o["x" + i] = i;
        }
    } catch (e) {
        write(e);
    }

    return 0;
}

function withEnumerableGetter(o, nStart, nEnd) {
    if (!o.hasOwnProperty("y")) {
        Object.defineProperty(o, "y", { get: function () { write("getter withGetter"); return -1; }, enumerable : true });
    }

    try {
        for (var i = nStart; i < nEnd; i++) {
            o["x" + i] = i;
        }
    } catch (e) {
        write(e);
    }

    return 1;
}

function check(nStart, nEnd, fname, f) {
    var obj = {};
    var extra = f(obj, 0, nStart);

    checkAllProperties(nStart);

    for (var i = nStart; i < nEnd; i += 1) {
        var typeBefore = Debug.getTypeHandlerName(obj);
        extra = f(obj, i, i+1);
        var typeAfter = Debug.getTypeHandlerName(obj);

        if (65532 <= i && i <= 65536) {
            // Check enumeration of object with no. of properties near boundary between PropertyIndex and BigPropertyIndex
            // Note: 65532 through 65536 because sometimes there is a y property which shifts the boundary
            checkAllProperties(i+1);
        } else {
            if (typeBefore !== typeAfter) {
                // If this occurs then the above range, 65532 - 65536 is incorrect for capturing the transition
                write("TEST ERROR: Type handler transition not captured by checkAllProperties");
            }
            // Don't bother with a full check of all properties away from boundary
            checkIndividualProperty(i);
        }
    }

    function checkAllProperties(n) {
        var count = 0;
        try {
            for (var i in obj) {
                if (i != "y") {
                    if (obj[i] != count) {
                        write("FAIL1 : " + i + " " + obj[i] + " " + count);
                        return;
                    }
                    count++;
                }
            }
        } catch (e) {
            write(e);
        }

        if ( n != count) {
            write(" FAIL2 : " + n + " " + fname + " " + count);
        } else {
            write(n + " " + fname + " PASS");
        }
    }

    function checkIndividualProperty(i) {
        var xi = "x" + i;

        try {
            if (obj[xi] != i) {
                write("FAIL1 : " + xi + " " + obj[xi] + " " + (i+1));
                return false;
            }
        } catch (e) {
            write(e);
        }

        write((i+1) + " " + fname + " PASS");
        return true;
    }
}

var fns = [["normal", normal],
           ["withGetter", withGetter],
           ["withEnumerableGetter", withEnumerableGetter]
          ];

for (var f = 0; f < fns.length; f++) {
    check(65532, 65540, fns[f][0], fns[f][1]);
}

// Test case from Kangax miscellaneous
function cannotNewAccessorMethods() {
    try {
        new (Object.getOwnPropertyDescriptor({ get a() { write("fail"); } }, 'a')).get;
    } catch (e) {
        if (!(e instanceof TypeError)) {
            return false;
        }
    }

    try {
        new (Object.getOwnPropertyDescriptor({ set a(v) { write("fail"); } }, 'a')).set;
    } catch (e) {
        if (!(e instanceof TypeError)) {
            return false;
        }
    }

    return true;
}
write(cannotNewAccessorMethods());
