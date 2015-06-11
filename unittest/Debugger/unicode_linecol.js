//                000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889
//                123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
var unicodeVar = "This is a UNICODE İİİİİİİİİİİİİİİİİ var";
        var unicodeVar2 = "This is another UNICODE ਊഊഊİİİİİİİİİİİİİİ var";
var expectedError1 = "Error: This is a UNICODE İİİİİİİİİİİİİİİİİ var\n   at testStack (unicode_linecol.js:24:5)\n   at Global code (unicode_linecol.js:10:3)";
var expectedError2 = "Error: This is another UNICODE ਊഊഊİİİİİİİİİİİİİİ var\n   at testStack (unicode_linecol.js:24:5)\n   at Global code (unicode_linecol.js:10:3)";

/**bp:logJson('stack');stack(CALLSTACK_LINECOLUMN);**/
if(unicodeVar.length == 39 && unicodeVar2.length == 45) {
  if (testStack(unicodeVar, expectedError1) && testStack(unicodeVar2, expectedError2)) {
    WScript.Echo("PASSED");
  }
} else {
  WScript.Echo("FAILED");
  WScript.Echo("unicodeVar.length = " + unicodeVar.length);
  WScript.Echo("unicodeVar2.length = " + unicodeVar2.length);
}

function testStack(msg, expectedStack) {
  var success = false;

  try {
    /**bp:stack(CALLSTACK_LINECOLUMN);**/
    throw Error(msg);
  }
  catch(ex) {
    var filteredStack = filterFullFilePathFromCallstack([ex.stack].toString());
    if (filteredStack == expectedStack) {
      success = true;
    } else {
      WScript.Echo("FAILED");
      WScript.Echo("\nActual (raw):\n" + [ex.stack]);
      WScript.Echo("\nActual (filtered):\n" + filteredStack);
      WScript.Echo("\n\nExpected:\n" + expectedError);
    }
  }

  return success;
}

function filterFullFilePathFromCallstack(cs) {
    var filteredStack = cs;
    var fileName = "unicode_linecol.js:";
    var startDelim = " (";

    // remove full path from the file name in the call stack (x2)
    var lastInd = filteredStack.lastIndexOf(fileName);
    var firstInd = filteredStack.lastIndexOf(startDelim, lastInd);
    filteredStack = filteredStack.substring(0, firstInd + startDelim.length) + filteredStack.substring(lastInd);

    lastInd = filteredStack.lastIndexOf(fileName);
    lastInd = filteredStack.lastIndexOf(fileName, lastInd - 1);
    firstInd = filteredStack.lastIndexOf(startDelim, lastInd);
    filteredStack = filteredStack.substring(0, firstInd + startDelim.length) + filteredStack.substring(lastInd);

    return filteredStack;
}