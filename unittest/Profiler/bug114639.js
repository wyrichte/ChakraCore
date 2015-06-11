// Tests that Set.set isn't added twice in  causing an assert to fire (occurs on profile attach).

function profileSetTest() {
    var s = new Set();
    s.add("1");
}

WScript.StartProfiling(profileSetTest);