function test() {
    var f;
    (f = -6.92053759608629E+18) | 0;
    //f = -6.92053759608629E+18 | 0; // the above line should not be equivalent to this when bailing out

    var a = 0; // bail-out here
    var b = a !== null ? 0 : f |= 0;

    return f;
}

WScript.Echo(test());
