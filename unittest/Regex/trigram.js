// The input is too small for trigram mega match to be done on it, so it should properly fall back to the regex engine
var s = "GGCCGGGTAAAGTGGCTCACGCCTGTAATCCCAGCACTTTACCCCCCGAGGCGGGCGGA";
writeLine(s.match(/[cgt]gggtaaa|tttaccc[acg]/ig));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

var web;
function writeLine(s) {
    if (web)
        document.writeln(s + "<br/>");
    else
        WScript.Echo("" + s);
}

function safeCall(f) {
    try {
        f();
    }
    catch (ex) {
        writeLine(ex.name + ": " + ex.message);
    }
}
