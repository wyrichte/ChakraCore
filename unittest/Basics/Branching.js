function main()
{
    var a;        // a = undefined
    var e = 0;

    // We shouldn't invert this branch as relational comparison involving
    // undefined always returns false.
    if (!(a >= 1))
        e = true;

    WScript.Echo("e = " + e);
}

main();
