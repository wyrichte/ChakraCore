var a=true;
// eval used as a function name in strict mode
try {
    eval('(function(){if(a) { function foo3() {"use strict"; function eval(a) {} }; foo3();}})();');
}
catch(e) {
    WScript.Echo(e.message);
}
try {
    eval('(function(){if(a) { function foo3() { function eval(a) {"use strict";} }; foo3();}})();');
}
catch(e) {
    WScript.Echo(e.message);
}

