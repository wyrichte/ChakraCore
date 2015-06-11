WScript.LoadScriptFile("CompoundStringUtilities.js", "self");

CompoundString.createTestStrings(); // call twice so that it is jitted the second time
var strings = CompoundString.createTestStrings();
for(var i = 0; i < strings.length; ++i)
    WScript.Echo(i + ": " + strings[i]);
