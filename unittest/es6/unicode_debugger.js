function f() {
    var 𠮷 = "test";
    var ar = [];
    ar["𠮷"] = "2";
}

/**bp:evaluate("f")**/
WScript.StartProfiling(f);