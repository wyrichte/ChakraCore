// This tests that block scopes are properly ended (setting byte
// code offsets) when there is a case of a scope slot block scope that
// has a sibling register slot block scope.
// Bug #243560

function Run(){
    let b = "level0";
    const c = "level0";
    {
        b += "level1";/**bp:evaluate('c')**/;
        function level2Func(arg1) {
            c;
        }
    }
};

WScript.Attach(function(){Run();});
WScript.Echo("PASSED");