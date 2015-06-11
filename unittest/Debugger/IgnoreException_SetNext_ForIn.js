// Ignore exception, set next statement expicitly, for-in statement.

/**exception(resume_ignore):setnext('foo1_ForIn')**/

var count = 0;

// Set next to beginning of ForIn.
function foo1()
{
    var o = {"0": 1};

    throw "some exception";
    count += 10;
    WScript.Echo("We should not get here!");

    for (var i in (count++, o))/**loc(foo1_ForIn):stack()**/
    {
        count += o[i];
    }

    count++;
}

foo1();

WScript.Echo(count === 3 ? "PASS" : "Failed: " + count);
