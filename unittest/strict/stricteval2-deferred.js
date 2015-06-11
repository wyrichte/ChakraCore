var a=true;
// eval declared as a var in strict mode
(function(){if(a){function bar() {"use strict"; var x = function() { var eval=print(); };};bar();}})();