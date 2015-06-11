// include the 'Do()' method
WScript.LoadScriptFile('justMyCode.threeDocuments.doOuter.js');
WScript.LoadScriptFile('justMyCode.threeDocuments.doInner.js');

WScript.Echo("Main() - IsInJitMode() == " +  Debug.isInJit());
WScript.Echo('Start!');
Do(); 
WScript.Echo('Completed!');
WScript.Echo("/Main() - IsInJitMode() == " +  Debug.isInJit());
