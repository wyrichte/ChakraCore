// Validating the bug (VSO:854944) which is asserting when try to get property from tracked undefined object
var arr = new Array();
arr[0]  = {x:20};
arr[4]./**ml:x**/;