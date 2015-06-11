try {
	eval("{const x = 1;}WScript.Echo(x);");
} catch(e) {
	WScript.Echo(e);
}

// Bugs Win8: 26565, 26558
// Code for const/let tracks assignments to identifiers. Ensure that the tracking tracks identifier foo in this case and 
// not the integer constant 0 which doesn't have a pid
try {
    eval("--foo 0");
} catch(e) {
	WScript.Echo(e);
}