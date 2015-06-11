/**ref:basic_paramHelp_sub.js**/


function f1() {
}
f1(/**pl:!a**/);

function f2(a) {
}
f2(/**pl:a,!b**/);
f2(/**pl:***/);

intellisense.addEventListener(/**pl:***/'statementcompletion', function() {});

test1(/**pl:***/);

function foo(a, b, c) {
}
foo(/**pl(forceOrder):**/);
foo(/**pl:a,b**/);
foo(/**pl:a,b,c**/);
foo(/**pl:c,a,b**/);
foo(/**pl(forceOrder):a,b**/);
foo(/**pl(forceOrder):a,b,c**/);
foo(/**pl(forceOrder):c,a,b**/);
foo(/**pl(false):a,b**/);
foo(/**pl(false):a,b,c**/);
foo(/**pl(false):c,a,b**/);
