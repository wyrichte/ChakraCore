var remoteWin = WScript.LoadScriptFile("valueobjectchild.js", "samethread");

function outputRemoteObjects()
{
var res1 = remoteWin.strObj1 + " one";
WScript.Echo(res1);
var res2 = remoteWin.strObj2 + " two";
WScript.Echo(remoteWin.strObj2 + " two");
var res3 = remoteWin.numberObj + 20;
WScript.Echo(res3);
if(remoteWin.boolObj) {WScript.Echo("bool is true") }
  else {WScript.Echo("bool is false"); }
}

outputRemoteObjects();

WScript.Shutdown(remoteWin);
WScript.Echo("after close");
outputRemoteObjects();
