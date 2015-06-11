// A few tests for restoring values at bailout: making sure we don't restore garbage, etc. 
// This should be enough to run: -forcediagnosticsmode -maxInterpretCount:1 

// Make sure temp -> non-temp conversion in DeadStore/MarkTempObject doesn't 
// cause mismatch with data recorded in BackwardPass.
function testMarkTempObjectTracker()
{
  for(var i = 0; i < 1; i++) {
    var x = { prop: 1 };
  }
}

// Make sure that when BailOutInfo::usedCapturedValues::constantValues are not empty,
// we don't get same syms in BailOutInfo::byteCodeUpwardExposedUsed.
function testCaptureConst(x)
{
  var r = {prop: [0]};
  use(x);
  x = 1;
  for (var q in r.prop) { q == r.prop[1]}
  use(x);
  function use(x) {};
}

function main()
{
  testMarkTempObjectTracker();
  testCaptureConst();
}

main();
main();

WScript.Echo("PASS");
