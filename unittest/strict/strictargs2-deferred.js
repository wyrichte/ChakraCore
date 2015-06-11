var a=true;
// arguments declared as a var in strict mode
(function(){if(a){function bar2() {"use strict"; var x = function() { var arguments=print(); };};bar2();}})();