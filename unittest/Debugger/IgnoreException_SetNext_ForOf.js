// Ignore exception, set next statement expicitly, for-of statement.

/**exception(resume_ignore):setnext('foo1_ForOf')**/

var count = 0;

// Set next to beginning of ForOf.
function foo1()
{
    throw "some exception";
    count += 10;
    WScript.Echo("We should not get here!");

    for (var i of (count++, [1]))/**loc(foo1_ForOf):stack()**/
    {
        count += i;
    }

    count++;
}

foo1();

WScript.Echo(count === 3 ? "PASS" : "Failed");
