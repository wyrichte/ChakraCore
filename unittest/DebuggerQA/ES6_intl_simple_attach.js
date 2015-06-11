

/*
    Intl Object
    Intl Collator
    Intl Number Format
    Intl DateTime Format
    Chakra Implementation should be hidden from the user
*/

function Run() {
    var coll = Intl.Collator();
    var numFormat = Intl.NumberFormat();
    var dttmFormat = Intl.DateTimeFormat();

    WScript.Echo('PASSED');/**bp:
    locals(1);
	evaluate('coll',4);
	evaluate('numFormat',4);
	evaluate('dttmFormat',4);
	evaluate('coll.compare.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
    evaluate('coll.resolvedOptions.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
    evaluate('numFormat.format.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
    evaluate('numFormat.resolvedOptions.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
    evaluate('dttmFormat.format.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
    evaluate('dttmFormat.resolvedOptions.toString() == \'\\nfunction() {\\n    [native code]\\n}\\n\'');
	**/
}

var x; /**bp:evaluate('Intl.Collator')**/
WScript.Attach(Run);
WScript.Detach(Run);

