//
// blue: 575533: CAS:WebCrawler: AV: UNKNOWN: jscript9.dll!Js::HeapArgumentsObject::AdvanceWalkerToArgsFrame
//

var a = (function f1(){
    return arguments;
})();

// Now this arguments object isn't on stack at all. Finding its "caller" will
// run across the "glob" function at the bottom.
//
// Uncomment following line and we'll AV with just "-version:2 -forcenative".
// a.caller;

// bp needs to be in another function so that "glob" function can be jitted.
(function f2() {
    a; /**bp:evaluate("a", 1)**/
})();

WScript.Echo("pass");
