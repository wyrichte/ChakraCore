var a=true;
// arguments used as a function name in strict mode
(function(){if(a) { function bar3() {"use strict"; function arguments(a) {} }; bar3();}})();