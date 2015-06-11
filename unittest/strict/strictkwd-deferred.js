// Bug 53107 - Syntax error on deferred re-parse
yoyo=[];
yoyo[(function  yield (x) { "use strict"; } )()];